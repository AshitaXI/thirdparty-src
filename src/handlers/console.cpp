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

#include "console.hpp"

namespace thirdparty::handlers::console
{
    IAshitaCore* core_{nullptr};
    HANDLE mapping_{nullptr};
    LPVOID view_{nullptr};
    map_t* map_{nullptr};

    auto initialize(IAshitaCore* core) -> bool
    {
        console::release();

        console::core_ = core;

        char name[MAX_PATH]{};
        ::sprintf_s(name, MAX_PATH, "WindowerMMFConsoleHandler_%d", ::GetCurrentProcessId());

        console::mapping_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(console::map_t), name);
        if (console::mapping_ == nullptr)
        {
            if (console::core_ && console::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the console MMF handler. (1)");
                console::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        console::view_ = ::MapViewOfFile(console::mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (console::view_ == nullptr)
        {
            ::CloseHandle(console::mapping_);
            console::mapping_ = nullptr;

            if (console::core_ && console::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the console MMF handler. (2)");
                console::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        console::map_                      = static_cast<console::map_t*>(console::view_);
        console::map_->last_update_        = ::GetTickCount();
        console::map_->command_.arg_count_ = 0;

        return true;
    }

    auto release(void) -> void
    {
        console::map_ = nullptr;

        if (console::view_ != nullptr)
            ::UnmapViewOfFile(console::view_);
        console::view_ = nullptr;

        if (console::mapping_ != nullptr)
            ::CloseHandle(console::mapping_);
        console::mapping_ = nullptr;
    }

    auto command(const char* command) -> void
    {
        if (console::map_ == nullptr || command == nullptr)
            return;

        std::vector<std::string> args{};
        const auto count = Ashita::Commands::GetCommandArgs(command, &args);

        if (count == 0)
            return;

        if (count > 8)
        {
            if (console::core_ && console::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Invalid command attempt was made. Commands cannot have more than 8 arguments!");
                console::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return;
        }

        for (auto x = 0u; x < count; x++)
            ::strcpy_s(console::map_->command_.args_[x].arg_, sizeof(console::arg_t::arg_), args[x].c_str());

        console::map_->command_.arg_count_ = count;
        console::map_->last_update_        = ::GetTickCount();
    }

} // namespace thirdparty::handlers::console