/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "Channel.h"
#include "Group.h"
#include "Guild.h"
#include "Log.h"
#include "Player.h"

#define SC_LOG_CHAT(TYPE, ...)                       \
    if (lang != LANG_ADDON)                          \
        SC_LOG_DEBUG("chat.log." TYPE, __VA_ARGS__); \
    else                                             \
        SC_LOG_DEBUG("chat.log.addon." TYPE, __VA_ARGS__);

class ChatLogScript : public PlayerScript
{
    public:
        ChatLogScript() : PlayerScript("ChatLogScript") { }

        void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
        {
            switch (type)
            {
                case CHAT_MSG_SAY:
                    SC_LOG_CHAT("say", "Player {} says (language {}): {}",
                        player->GetName(), lang, msg);
                    break;

                case CHAT_MSG_EMOTE:
                    SC_LOG_CHAT("emote", "Player {} emotes: {}",
                        player->GetName(), msg);
                    break;

                case CHAT_MSG_YELL:
                    SC_LOG_CHAT("yell", "Player {} yells (language {}): {}",
                        player->GetName(), lang, msg);
                    break;
            }
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Player* receiver) override
        {
            SC_LOG_CHAT("whisper", "Player {} tells {}: {}",
               player->GetName(), receiver ? receiver->GetName() : "<unknown>", msg);
        }

        void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group) override
        {
            //! NOTE:
            //! LANG_ADDON can only be sent by client in "PARTY", "RAID", "GUILD", "BATTLEGROUND", "WHISPER"
            switch (type)
            {
                case CHAT_MSG_PARTY:
                    SC_LOG_CHAT("party", "Player {} tells group with leader {}: {}",
                        player->GetName(), group ? group->GetLeaderName() : "<unknown>", msg);
                    break;

                case CHAT_MSG_PARTY_LEADER:
                    SC_LOG_CHAT("party", "Leader {} tells group: {}",
                        player->GetName(), msg);
                    break;

                case CHAT_MSG_RAID:
                    SC_LOG_CHAT("raid", "Player {} tells raid with leader {}: {}",
                        player->GetName(), group ? group->GetLeaderName() : "<unknown>", msg);
                    break;

                case CHAT_MSG_RAID_LEADER:
                    SC_LOG_CHAT("raid", "Leader player {} tells raid: {}",
                        player->GetName(), msg);
                    break;

                case CHAT_MSG_RAID_WARNING:
                    SC_LOG_CHAT("raid", "Leader player {} warns raid with: {}",
                        player->GetName(), msg);
                    break;

                case CHAT_MSG_BATTLEGROUND:
                    SC_LOG_CHAT("bg", "Player {} tells battleground with leader {}: {}",
                        player->GetName(), group ? group->GetLeaderName() : "<unknown>", msg);
                    break;

                case CHAT_MSG_BATTLEGROUND_LEADER:
                    SC_LOG_CHAT("bg", "Leader player {} tells battleground: {}",
                        player->GetName(), msg);
                    break;
            }
        }

        void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild) override
        {
            switch (type)
            {
                case CHAT_MSG_GUILD:
                    SC_LOG_CHAT("guild", "Player {} tells guild {}: {}",
                        player->GetName(), guild ? guild->GetName() : "<unknown>", msg);
                    break;

                case CHAT_MSG_OFFICER:
                    SC_LOG_CHAT("guild.officer", "Player {} tells guild {} officers: {}",
                        player->GetName(), guild ? guild->GetName() : "<unknown>", msg);
                    break;
            }
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Channel* channel) override
        {
            bool isSystem = channel &&
                            (channel->HasFlag(CHANNEL_FLAG_TRADE) ||
                             channel->HasFlag(CHANNEL_FLAG_GENERAL) ||
                             channel->HasFlag(CHANNEL_FLAG_CITY) ||
                             channel->HasFlag(CHANNEL_FLAG_LFG));

            if (isSystem)
            {
                SC_LOG_CHAT("system", "Player {} tells channel {}: {}",
                    player->GetName(), channel->GetName(), msg);
            }
            else
            {
                std::string channelName = channel ? channel->GetName() : "<unknown>";
                SC_LOG_CHAT("channel." + channelName, "Player {} tells channel {}: {}",
                    player->GetName(), channelName, msg);
            }
        }
};

void AddSC_chat_log()
{
    new ChatLogScript();
}

#undef SC_LOG_CHAT
