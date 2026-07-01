/* ------------------------------------ */
/* AUTHOR   Aurøra                      */
/*    FOR   Syphrena Private Server     */
/* ------------------------------------ */

#include "ScriptMgr.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Config.h"
#include "Chat.h"

class StartGuild : public PlayerScript
{
public:
    StartGuild() : PlayerScript("StartGuild") { }

    void OnLogin(Player* player, bool firstLogin)
    {
        if (firstLogin)
        {
            uint32 GUILD_ID_ALLIANCE = sConfigMgr->GetIntDefault("StartGuild.Alliance", 0);
            uint32 GUILD_ID_HORDE = sConfigMgr->GetIntDefault("StartGuild.Horde", 0);
            Guild* guild = sGuildMgr->GetGuildById(player->GetTeam() == ALLIANCE ? GUILD_ID_ALLIANCE : GUILD_ID_HORDE);

            if (guild)
            {
                ObjectGuid playerGuid = player->GetGUID();
                CharacterDatabaseTransaction trans(nullptr);
                guild->AddMember(trans, playerGuid);
            }
        }
    }
};

void AddStartGuildScripts()
{
    new StartGuild();
}