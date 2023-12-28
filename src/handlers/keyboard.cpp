/**
 * Ashita - Copyright (c) 2023 Ashita Development Team
 * Contact: https://www.ashitaxi.com/
 * Contact: https://discord.gg/Ashita
 *
 * This file is part of Ashita.
 *
 * Ashita is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ashita is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ashita.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "keyboard.hpp"

namespace thirdparty::handlers::keyboard
{
    IAshitaCore* core_{nullptr};
    HANDLE mapping_{nullptr};
    LPVOID view_{nullptr};
    map_t* map_{nullptr};

    std::queue<DIDEVICEOBJECTDATA> data_;
    uint32_t sequence_{0};
    uint8_t keystate_[256]{};

    auto initialize(IAshitaCore* core) -> bool
    {
        keyboard::release();

        keyboard::core_     = core;
        keyboard::sequence_ = 0;

        std::memset(&keyboard::keystate_, 0x00, sizeof(keyboard::keystate_));

        char name[MAX_PATH]{};
        ::sprintf_s(name, MAX_PATH, "WindowerMMFKeyboardHandler_%d", ::GetCurrentProcessId());

        keyboard::mapping_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(keyboard::map_t), name);
        if (keyboard::mapping_ == nullptr)
        {
            if (keyboard::core_ && keyboard::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the keyboard MMF handler. (1)");
                keyboard::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        keyboard::view_ = ::MapViewOfFile(keyboard::mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (keyboard::view_ == nullptr)
        {
            ::CloseHandle(keyboard::mapping_);
            keyboard::mapping_ = nullptr;

            if (keyboard::core_ && keyboard::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the keyboard MMF handler. (2)");
                keyboard::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        keyboard::map_           = static_cast<keyboard::map_t*>(keyboard::view_);
        keyboard::map_->handled_ = true;

        if (keyboard::core_->GetInputManager() && core->GetInputManager()->GetKeyboard())
            keyboard::core_->GetInputManager()->GetKeyboard()->AddCallback("__plugin_thirdparty", keyboard::handle_device_data, keyboard::handle_device_state, nullptr);

        return true;
    }

    auto release(void) -> void
    {
        keyboard::map_ = nullptr;

        if (keyboard::view_ != nullptr)
            ::UnmapViewOfFile(keyboard::view_);
        keyboard::view_ = nullptr;

        if (keyboard::mapping_ != nullptr)
            ::CloseHandle(keyboard::mapping_);
        keyboard::mapping_ = nullptr;

        if (keyboard::core_ && keyboard::core_->GetInputManager() && keyboard::core_->GetInputManager()->GetKeyboard())
            keyboard::core_->GetInputManager()->GetKeyboard()->RemoveCallback("__plugin_thirdparty");
    }

    auto update(void) -> void
    {
        if (keyboard::map_ == nullptr || keyboard::map_->handled_)
            return;

        for (auto x = 0u; x < keyboard::map_->num_commands_; x++)
        {
            switch (keyboard::map_->commands_[x].command_)
            {
                case 0x0001: // (Windower) SetKey
                    keyboard::set_key(*reinterpret_cast<uint8_t*>(&keyboard::map_->commands_[x].param1_), *reinterpret_cast<bool*>(&keyboard::map_->commands_[x].param2_));
                    break;

                case 0x0002: // (Windower) SetBlockInput
                {
                    if (keyboard::core_ != nullptr && keyboard::core_->GetInputManager() != nullptr && keyboard::core_->GetInputManager()->GetKeyboard() != nullptr)
                        keyboard::core_->GetInputManager()->GetKeyboard()->SetBlockInput(*reinterpret_cast<bool*>(&keyboard::map_->commands_[x].param1_));

                    break;
                }

                case 0x0003: // (Windower) Command
                {
                    std::string command = keyboard::map_->commands_[x].text_;
                    std::vector<std::string> parts;

                    if (command.find(';') != command.npos)
                    {
                        std::stringstream ss(command);
                        std::string part;

                        while (std::getline(ss, part, ';'))
                        {
                            if (part.length() > 0)
                                parts.push_back(part);
                        }
                    }

                    if (parts.size() == 0)
                        parts.push_back(command);

                    for (auto iter = parts.begin(), iterend = parts.end(); iter != iterend; ++iter)
                    {
                        auto cmd = iter->c_str();

                        while (cmd[0] == '/')
                            cmd++;
                        while (cmd[0] == ' ')
                            cmd++;

                        auto cmd2 = "/" + std::string(cmd);
                        if (keyboard::core_ != nullptr && keyboard::core_->GetChatManager() != nullptr)
                            keyboard::core_->GetChatManager()->ParseCommand(static_cast<int32_t>(Ashita::CommandMode::AshitaParse), cmd2.c_str());
                    }

                    break;
                }

                case 0x03E8: // (Ashita) KeyPress
                    keyboard::set_key(*reinterpret_cast<uint8_t*>(&keyboard::map_->commands_[x].param1_), true);
                    keyboard::set_key(*reinterpret_cast<uint8_t*>(&keyboard::map_->commands_[x].param1_), false);
                    break;

                default:
                    break;
            }
        }

        keyboard::map_->num_commands_ = 0;
        keyboard::map_->handled_      = true;
    }

    auto set_key(const uint8_t key, const bool down) -> void
    {
        const auto create_data = [](const uint32_t k, const bool d) -> DIDEVICEOBJECTDATA {
            DIDEVICEOBJECTDATA data{
                .dwOfs       = k,
                .dwData      = d ? 0x80u : 0x00u,
                .dwTimeStamp = ::GetTickCount(),
                .dwSequence  = 0,
                .uAppData    = 0,
            };

            return data;
        };

        keyboard::set_key_state(key, down);
        keyboard::inject_data(create_data(key, down));
        keyboard::inject_command(key, down, false);
    }

    auto set_key_state(const uint8_t key, const bool down) -> void
    {
        keyboard::keystate_[key] = down ? 1 : 0;
    }

    auto inject_command(const uint8_t key, const bool down, const bool use_virtual_key) -> void
    {
        if (keyboard::core_ == nullptr || keyboard::core_->GetInputManager() == nullptr || keyboard::core_->GetInputManager()->GetKeyboard() == nullptr)
            return;

        if (!use_virtual_key)
        {
            const auto k = keyboard::core_->GetInputManager()->GetKeyboard()->D2V(key);
            if (k == -1 || k == 0)
                return;

            keyboard::inject_command(static_cast<uint8_t>(k), down, true);
            return;
        }

        auto code = ::MapVirtualKeyA(key, 0);
        if (code == 0)
            return;

        if (down)
        {
            ::PostMessageA(nullptr, WM_KEYDOWN, key, code);

            uint8_t keys[256]{};
            if (!::GetKeyboardState(keys))
                return;

            const auto kb = keyboard::core_->GetInputManager()->GetKeyboard();

            for (auto x = 0u; x < 256; x++)
            {
                if (!keyboard::keystate_[x])
                    continue;

                if (const auto vk = kb->D2V(x); vk != 0)
                    keys[vk] = 0x80;
            }

            if (keyboard::keystate_[DIK_LALT] || keyboard::keystate_[DIK_RALT])
                keys[VK_MENU] = 0x80;
            if (keyboard::keystate_[DIK_LCONTROL] || keyboard::keystate_[DIK_RCONTROL])
                keys[VK_CONTROL] = 0x80;
            if (keyboard::keystate_[DIK_LSHIFT] || keyboard::keystate_[DIK_RSHIFT])
                keys[VK_SHIFT] = 0x80;

            uint16_t c = 0;
            if (::ToAsciiEx(key, code, keys, &c, 0, ::GetKeyboardLayout(0)))
                ::PostMessageA(nullptr, WM_CHAR, c, 0);
        }
        else
        {
            code |= 0x40000000;
            code |= 0x80000000;
            ::PostMessageA(nullptr, WM_KEYUP, key, code);
        }
    }

    auto inject_data(const DIDEVICEOBJECTDATA data) -> void
    {
        keyboard::data_.push(data);
    }

    auto __stdcall handle_device_data(IDirectInputDevice8A* device, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags, DWORD dwOriginalCount) -> HRESULT
    {
        UNREFERENCED_PARAMETER(device);
        UNREFERENCED_PARAMETER(cbObjectData);
        UNREFERENCED_PARAMETER(dwFlags);

        keyboard::update();

        if (rgdod == nullptr)
            return DI_OK;

        auto ecount = *pdwInOut;
        if (ecount > 0)
            keyboard::sequence_ = rgdod[ecount - 1].dwSequence;

        while (ecount < dwOriginalCount && !keyboard::data_.empty())
        {
            rgdod[ecount]            = keyboard::data_.front();
            rgdod[ecount].dwSequence = keyboard::sequence_++;

            keyboard::data_.pop();

            ecount++;
        }

        *pdwInOut = ecount;

        return DI_OK;
    }

    auto __stdcall handle_device_state(IDirectInputDevice8A* device, DWORD cbData, LPVOID lpvData) -> HRESULT
    {
        UNREFERENCED_PARAMETER(device);

        const auto state = static_cast<uint8_t*>(lpvData);

        for (auto x = 0u; x < cbData; x++)
        {
            if (keyboard::keystate_[x] == 1)
                state[x] = 0x80;
        }

        return DI_OK;
    }

} // namespace thirdparty::handlers::keyboard