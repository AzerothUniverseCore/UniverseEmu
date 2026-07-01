#include "ScriptMgr.h"

class at_fatal_boundary : public AreaTriggerScript
{
public:
    at_fatal_boundary() : AreaTriggerScript("at_fatal_boundary") {}

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (player->IsAlive())
        {
            player->KillSelf(false);
            return true;
        }

        return false;
    }
};

void AddSC_minelevationtrigger()
{
    new at_fatal_boundary();
}
