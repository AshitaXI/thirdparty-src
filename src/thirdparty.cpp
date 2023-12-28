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

#include "thirdparty.hpp"
#include "handlers/console.hpp"
#include "handlers/keyboard.hpp"
#include "handlers/text.hpp"

namespace thirdparty
{
    plugin::plugin(void)
        : core_{nullptr}
        , log_{nullptr}
        , device_{nullptr}
        , id_{0}
    {}
    plugin::~plugin(void)
    {}

    auto plugin::GetName(void) const -> const char*
    {
        return "thirdparty";
    }

    auto plugin::GetAuthor(void) const -> const char*
    {
        return "atom0s";
    }

    auto plugin::GetDescription(void) const -> const char*
    {
        return "Enables third-party program usage with Ashita.";
    }

    auto plugin::GetLink(void) const -> const char*
    {
        return "https://www.ashitaxi.com/";
    }

    auto plugin::GetVersion(void) const -> double
    {
        return 4.1f;
    }

    auto plugin::GetInterfaceVersion(void) const -> double
    {
        return ASHITA_INTERFACE_VERSION;
    }

    auto plugin::GetPriority(void) const -> int32_t
    {
        return 0;
    }

    auto plugin::GetFlags(void) const -> uint32_t
    {
        return static_cast<uint32_t>(Ashita::PluginFlags::UseCommands | Ashita::PluginFlags::UseDirect3D);
    }

    auto plugin::Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) -> bool
    {
        this->core_ = core;
        this->log_  = logger;
        this->id_   = id;

        if (!thirdparty::handlers::console::initialize(core) ||
            !thirdparty::handlers::keyboard::initialize(core) ||
            !thirdparty::handlers::text::initialize(core))
            return false;

        return true;
    }

    auto plugin::Release(void) -> void
    {
        thirdparty::handlers::console::release();
        thirdparty::handlers::keyboard::release();
        thirdparty::handlers::text::release();
    }

    auto plugin::HandleCommand(int32_t mode, const char* command, bool injected) -> bool
    {
        UNREFERENCED_PARAMETER(mode);
        UNREFERENCED_PARAMETER(injected);

        std::vector<std::string> args{};
        const auto count = Ashita::Commands::GetCommandArgs(command, &args);

        if (::_strnicmp(command, "/sendkey ", 9) == 0)
        {
            if (count < 2)
                return true;

            const auto key = this->core_->GetInputManager()->GetKeyboard()->S2D(args[1].c_str());
            if (key == -1 || key == 0)
                return true;

            auto down = false;
            if (count >= 3)
                down = args[2] == "down" ? true : false;

            std::lock_guard lock{this->mutex_};

            this->keys_.push_back({static_cast<uint8_t>(key), down});

            return true;
        }

        if ((::_strnicmp(command, "/release keys ", 13) == 0) || (::_strnicmp(command, "/releasekeys ", 12) == 0))
        {
            std::lock_guard lock{this->mutex_};

            for (auto x = 0u; x < 256; x++)
                this->keys_.push_back({static_cast<uint8_t>(x), false});

            return true;
        }

        if ((command[0] == '/' && command[1] == '/') || ::_strnicmp(command, "/console ", 9) == 0 || ::_strnicmp(command, "/con ", 5) == 0)
        {
            const char* cmd = nullptr;
            if (command[0] == '/' && command[1] == '/')
                cmd = command + 2;
            else
            {
                cmd = ::strstr(command, " ");
                if (!cmd)
                    return true;

                cmd++;
                if (cmd[0] == '/' && cmd[1] == '/')
                    cmd++;
            }

            thirdparty::handlers::console::command(cmd);
            return true;
        }

        return false;
    }

    auto plugin::Direct3DInitialize(IDirect3DDevice8* device) -> bool
    {
        this->device_ = device;

        return true;
    }

    auto plugin::Direct3DBeginScene(const bool is_rendering_backbuffer) -> void
    {
        if (!is_rendering_backbuffer)
            return;

        if (!this->keys_.empty())
        {
            std::lock_guard lock{this->mutex_};

            for (auto& [k, d] : this->keys_)
                thirdparty::handlers::keyboard::set_key(k, d);

            this->keys_.clear();
        }

        thirdparty::handlers::text::update();
    }

} // namespace thirdparty

__declspec(dllexport) auto __stdcall expCreatePlugin(const char* args) -> IPlugin*
{
    UNREFERENCED_PARAMETER(args);

    return new thirdparty::plugin();
}

__declspec(dllexport) auto __stdcall expGetInterfaceVersion(void) -> double
{
    return ASHITA_INTERFACE_VERSION;
}