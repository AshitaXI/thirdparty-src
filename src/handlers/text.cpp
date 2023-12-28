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

#include "text.hpp"

namespace thirdparty::handlers::text
{
    IAshitaCore* core_{nullptr};
    HANDLE mapping_{nullptr};
    LPVOID view_{nullptr};
    map_t* map_{nullptr};

    auto initialize(IAshitaCore* core) -> bool
    {
        text::core_ = core;

        char name[MAX_PATH]{};
        ::sprintf_s(name, MAX_PATH, "WindowerMMFTextHandler_%d", ::GetCurrentProcessId());

        text::mapping_ = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(text::map_t), name);
        if (text::mapping_ == nullptr)
        {
            if (text::core_ && text::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the text MMF handler. (1)");
                text::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        text::view_ = ::MapViewOfFile(text::mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (text::view_ == nullptr)
        {
            ::CloseHandle(text::mapping_);
            text::mapping_ = nullptr;

            if (text::core_ && text::core_->GetChatManager())
            {
                std::ostringstream msg;
                msg << Ashita::Chat::Header("thirdparty")
                    << Ashita::Chat::Error("Failed to initialize the text MMF handler. (2)");
                text::core_->GetChatManager()->Writef(1, false, msg.str().c_str());
            }

            return false;
        }

        text::map_           = static_cast<text::map_t*>(text::view_);
        text::map_->handled_ = true;

        return true;
    }

    auto release(void) -> void
    {
        text::map_ = nullptr;

        if (text::view_ != nullptr)
            ::UnmapViewOfFile(text::view_);
        text::view_ = nullptr;

        if (text::mapping_ != nullptr)
            ::CloseHandle(text::mapping_);
        text::mapping_ = nullptr;
    }

    auto update(void) -> void
    {
        if (text::map_ == nullptr || text::map_->handled_)
            return;

        if (text::core_ == nullptr || text::core_->GetFontManager() == nullptr)
            return;

        const auto& mgr   = text::core_->GetFontManager();
        IFontObject* font = nullptr;

        for (auto x = 0u; x < text::map_->num_commands_; x++)
        {
            switch (text::map_->commands_[x].command_)
            {
                case 0x0001: // Font: Create
                    mgr->Create(text::map_->commands_[x].alias_);
                    break;

                case 0x0002: // Font: Delete
                    mgr->Delete(text::map_->commands_[x].alias_);
                    break;

                case 0x0003: // Font: SetText
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                        font->SetText(text::map_->commands_[x].text_);

                    break;
                }

                case 0x0004: // Font: SetVisibility
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                        font->SetVisible(*reinterpret_cast<bool*>(&text::map_->commands_[x].param1_));

                    break;
                }

                case 0x0005: // Font: SetFont
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                    {
                        font->SetFontFamily(text::map_->commands_[x].param1_);
                        font->SetFontHeight(*reinterpret_cast<int32_t*>(&text::map_->commands_[x].param2_));
                    }

                    break;
                }

                case 0x0006: // Font: SetColor
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                    {
                        font->SetColor(D3DCOLOR_ARGB(
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param1_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param2_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param3_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param4_)));
                    }

                    break;
                }

                case 0x0007: // Font: SetPosition
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                    {
                        font->SetPositionX(*reinterpret_cast<float*>(&text::map_->commands_[x].param1_));
                        font->SetPositionY(*reinterpret_cast<float*>(&text::map_->commands_[x].param2_));
                    }

                    break;
                }

                case 0x0008: // Font: SetBold
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                        font->SetBold(*reinterpret_cast<bool*>(&text::map_->commands_[x].param1_));

                    break;
                }

                case 0x0009: // Font: SetItalic
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                        font->SetItalic(*reinterpret_cast<bool*>(&text::map_->commands_[x].param1_));

                    break;
                }

                case 0x000A: // Font: SetBackgroundColor
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr && font->GetBackground() != nullptr)
                    {
                        font->GetBackground()->SetColor(D3DCOLOR_ARGB(
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param1_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param2_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param3_),
                            *reinterpret_cast<uint8_t*>(&text::map_->commands_[x].param4_)));
                    }

                    break;
                }

                case 0x000B: // Font: SetBackgroundBorderSize
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr && font->GetBackground() != nullptr)
                    {
                        const auto size = static_cast<LONG>(*reinterpret_cast<float*>(&text::map_->commands_[x].param1_));

                        font->GetBackground()->SetBorderSizes({size, size, size, size});
                        font->GetBackground()->SetBorderVisible(size == 0 ? false : true);
                        font->GetBackground()->SetBorderFlags(size == 0 ? Ashita::FontBorderFlags::None : Ashita::FontBorderFlags::All);
                    }

                    break;
                }

                case 0x000C: // Font: SetBackgroundVisibility
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr && font->GetBackground() != nullptr)
                        font->GetBackground()->SetVisible(*reinterpret_cast<bool*>(&text::map_->commands_[x].param1_));

                    break;
                }

                case 0x000D: // Font: SetRightJustified
                {
                    if (font = mgr->Get(text::map_->commands_[x].alias_); font != nullptr)
                        font->SetRightJustified(*reinterpret_cast<bool*>(&text::map_->commands_[x].param1_));

                    break;
                }

                default:
                    break;
            }
        }

        text::map_->num_commands_ = 0;
        text::map_->handled_      = true;
    }

} // namespace thirdparty::handlers::text