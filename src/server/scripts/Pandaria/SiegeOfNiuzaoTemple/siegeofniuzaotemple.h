#ifndef SIEGEOFNIUZAOTEMPLE_H_
#define SIEGEOFNIUZAOTEMPLE_H_

#include "CreatureAIImpl.h"

#define SNTScriptName "instance_siegeofniuzaotemple"
#define DataHeader "SNT"

uint32 const EncounterCount = 4;

enum SNTDataTypes
{
    // Encounter States/Boss GUIDs
    DATA_VIZIER_JINBAK          = 0,
    DATA_SIKTHIK_WARDEN         = 1,
    DATA_GENERAL_PAVALAK        = 2,
    DATA_WING_LEADER_NERONOK    = 3,
};

enum SNTGameObjectIds
{
    GO_HARDENED_RESIN           = 213174, // after jinbak
    GO_DOOR                     = 212921, // after Sikthik Warden
    GO_TEMPLE_INVIS_DOOR        = 213251, // before neronok bridge
    GO_FIRE_WALL                = 210097, // door Sikthik Warden
};

enum SNTCreaturesIds
{
    // Boss
    NPC_VIZIER_JINBAK           = 61567,
    NPC_SIKTHIK_WARDEN          = 62795,
    NPC_GENERAL_PAVALAK         = 61485,
    NPC_WING_LEADER_NERONOK     = 62205,

    // Trash
    NPC_SAP_PUDDLE_2            = 61965,
};


template <class AI, class T>
inline AI* GetSiegeOfNiuzaoTempleAI(T* obj)
{
    return GetInstanceAI<AI>(obj, SNTScriptName);
}

#define RegisterSkyreachCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetSiegeOfNiuzaoTempleAI)

#endif