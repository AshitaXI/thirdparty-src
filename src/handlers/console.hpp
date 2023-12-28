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

#ifndef THIRDPARTY_HANDLERS_CONSOLE_HPP_INCLUDED
#define THIRDPARTY_HANDLERS_CONSOLE_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"

namespace thirdparty::handlers::console
{
    struct arg_t
    {
        char arg_[2048];
    };

    struct command_t
    {
        int32_t arg_count_;
        arg_t args_[8];
    };

    struct map_t
    {
        uint32_t last_update_;
        command_t command_;
    };

    auto initialize(IAshitaCore* core) -> bool;
    auto release(void) -> void;
    auto command(const char* command) -> void;

} // namespace thirdparty::handlers::console

#endif // THIRDPARTY_HANDLERS_CONSOLE_HPP_INCLUDED