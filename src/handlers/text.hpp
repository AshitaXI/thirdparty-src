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

#ifndef THIRDPARTY_HANDLERS_TEXT_HPP_INCLUDED
#define THIRDPARTY_HANDLERS_TEXT_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"

namespace thirdparty::handlers::text
{
    struct command_t
    {
        uint32_t command_;
        char alias_[256];
        char param1_[256];
        char param2_[256];
        char param3_[256];
        char param4_[256];
        char text_[256];
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

} // namespace thirdparty::handlers::text

#endif // THIRDPARTY_HANDLERS_TEXT_HPP_INCLUDED