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

#ifndef THIRDPARTY_HANDLERS_KEYBOARD_HPP_INCLUDED
#define THIRDPARTY_HANDLERS_KEYBOARD_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"

namespace thirdparty::handlers::keyboard
{
    struct command_t
    {
        uint32_t command_;
        char param1_[8];
        char param2_[8];
        char param3_[8];
        char param4_[8];
        char text_[2048];
    };

    struct map_t
    {
        uint32_t num_commands_;
        bool handled_;
        command_t commands_[32];
    };

    auto initialize(IAshitaCore* core) -> bool;
    auto release(void) -> void;
    auto update(void) -> void;
    auto set_key(const uint8_t key, const bool down) -> void;
    auto set_key_state(const uint8_t key, const bool down) -> void;
    auto inject_command(const uint8_t key, const bool down, const bool use_virtual_key) -> void;
    auto inject_data(const DIDEVICEOBJECTDATA data) -> void;

    auto __stdcall handle_device_data(IDirectInputDevice8A* device, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags, DWORD dwOriginalCount) -> HRESULT;
    auto __stdcall handle_device_state(IDirectInputDevice8A* device, DWORD cbData, LPVOID lpvData) -> HRESULT;

} // namespace thirdparty::handlers::keyboard

#endif // THIRDPARTY_HANDLERS_KEYBOARD_HPP_INCLUDED