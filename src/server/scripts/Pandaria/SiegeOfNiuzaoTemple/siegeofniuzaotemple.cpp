#include "siegeofniuzaotemple.h"

// Sap Spray 61964
class npc_sap_spray : public CreatureScript
{
public:
    npc_sap_spray() : CreatureScript("npc_sap_spray") {}

    enum Spells
    {
        SPELL_SAP_SPRAY = 120586,
        SPELL_SAP_PUDDLE = 120591,
    };

    struct npc_sap_sprayAI : public ScriptedAI
    {
        npc_sap_sprayAI(Creature* creature) : ScriptedAI(creature) {}

        bool activated;

        void InitializeAI() override
        {
            activated = false;
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (!activated && me->SelectNearestPlayer(10.0f))
            {
                activated = true;

                if (Creature* puddle = GetClosestCreatureWithEntry(me, NPC_SAP_PUDDLE_2, 20.0f, true))
                {
                    me->CastSpell(me, SPELL_SAP_SPRAY, false);
                    puddle->CastSpell(puddle, SPELL_SAP_PUDDLE, false);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sap_sprayAI(creature);
    }
};

void AddSC_siegeofniuzaotemple()
{
    new npc_sap_spray();
};