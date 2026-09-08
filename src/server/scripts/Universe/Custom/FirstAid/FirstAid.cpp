/* ------------------------------------ */
/* AUTHOR   Aurøra                      */
/*    FOR   Syphrena Private Server     */
/* ------------------------------------ */

//By leewheel 2026-09-08
//[First Aid action bar icon normalization / Normalisation des icones de Premiers soins sur les barres d'actions]
//Problem: A brand new character gets many "First Aid" icons on its action bars after first login.
//Root cause: On first login this script learns First Aid (45542). The core then walks down the spell
//rank chain (3273 -> 3274 -> 7924 -> 10846 -> 27028 -> 45542) recursively and sends SMSG_LEARNED_SPELL /
//SMSG_SUPERCEDED_SPELL packets. The client auto-places each learned icon into the first empty action
//bar slots and reports them back through CMSG_SET_ACTION_BUTTON (persisted in character_action),
//which piles up many First Aid icons on the bars and in the DB.
//Fix (additive, previous behaviour kept):
//  1) Keep guaranteeing the character knows First Aid (learn it when missing, as before);
//  2) On every login normalize the action bars: remove every First Aid icon except on slot 11;
//  3) Keep at most ONE First Aid icon, placed on the LAST slot of the FIRST action bar (button 11);
//     for a brand new character (first login) force the placement on slot 11; for older characters,
//     if slot 11 is occupied by something of the player's own, do not steal it;
//  4) When something changed, immediately resend the whole action bar (SMSG_ACTION_BUTTONS) so the
//     client display is overwritten with the clean layout.
//End By leewheel

#include "Player.h"
#include "ScriptMgr.h"

class syphrena_first_aid : public PlayerScript
{
private:
    bool isEnabled = true;
    uint32 firstAid = 45542; /* First Aid / Premiers soins */

    //By leewheel 2026-09-08 First Aid rank chain IDs (low to high): 3273/3274/7924/10846/27028/45542
    //Id de la chaine des rangs de Premiers soins (bas -> haut): 3273/3274/7924/10846/27028/45542
    static bool IsFirstAidChainSpell(uint32 spellId)
    {
        switch (spellId)
        {
        case 3273:
        case 3274:
        case 7924:
        case 10846:
        case 27028:
        case 45542:
            return true;
        default:
            return false;
        }
    }

    //By leewheel 2026-09-08
    //Normalize the action bars: keep at most one First Aid icon, on the last slot (button 11) of the
    //first action bar. Returns true if anything changed (caller decides whether to resend the bars).
    //Normalise les barres d'actions: garder au plus une icone de Premiers soins, sur le dernier
    //emplacement (bouton 11) de la premiere barre. Retourne vrai si un changement a ete fait.
    bool NormalizeFirstAidActionBar(Player* player, bool forcePlaceAtLastSlot)
    {
        bool changed = false;

        // 1) Remove every First Aid icon except on slot 11 (scans ALL bars, button 0..MAX_ACTION_BUTTONS-1)
        //    Supprimer toute icone de Premiers soins sauf sur le bouton 11 (toutes les barres, 0..MAX_ACTION_BUTTONS-1)
        for (uint8 button = 0; button < MAX_ACTION_BUTTONS; ++button)
        {
            if (button == 11)                            // last slot handled in the next step / bouton 11 traite apres
                continue;

            ActionButton const* ab = player->GetActionButton(button);
            if (!ab)
                continue;

            if (ab->GetType() == ACTION_BUTTON_SPELL && IsFirstAidChainSpell(ab->GetAction()))
            {
                player->removeActionButton(button);
                changed = true;
            }
        }

        // 2) Handle the last slot (button 11) / Traitement du dernier emplacement (bouton 11)
        if (player->HasSpell(firstAid))
        {
            ActionButton const* ab11 = player->GetActionButton(11);

            // Already the wanted state: slot 11 IS the First Aid icon / Deja ideal: le bouton 11 est l'icone
            if (ab11 && ab11->GetType() == ACTION_BUTTON_SPELL && ab11->GetAction() == firstAid)
                return changed;

            if (!ab11)
            {
                // Slot 11 empty -> place First Aid there / Bouton 11 vide -> y placer Premiers soins
                if (player->addActionButton(11, firstAid, ACTION_BUTTON_SPELL))
                    changed = true;
            }
            else if (ab11->GetType() == ACTION_BUTTON_SPELL && IsFirstAidChainSpell(ab11->GetAction()))
            {
                // Slot 11 holds a lower First Aid rank -> upgrade it to the mastered one (45542)
                // Bouton 11 avec un rang inferieur -> le remplacer par le rang maitrise (45542)
                if (player->addActionButton(11, firstAid, ACTION_BUTTON_SPELL))
                    changed = true;
            }
            else if (forcePlaceAtLastSlot)
            {
                // Brand new character (first login): put First Aid on the very last slot as requested.
                // The previous content (usually a default class spell) stays learned in the spellbook.
                // Nouveau personnage (premiere connexion): placer Premiers soins sur le dernier emplacement.
                // L'ancien contenu (sort de classe par defaut) reste appris dans le grimoire.
                if (player->addActionButton(11, firstAid, ACTION_BUTTON_SPELL))
                    changed = true;
            }
            // Not a first login and slot 11 used by the player: respect the layout, do not steal it.
            // Pas une premiere connexion et bouton 11 occupe: respecter la disposition du joueur.
        }

        return changed;
    }
    //End By leewheel

public:
    syphrena_first_aid() : PlayerScript("syphrena_first_aid") {}

    void OnLogin(Player* player, bool firstLogin) {
        if (isEnabled) {
            if (!player->HasSpell(firstAid)) {
                player->LearnSpell(firstAid, false);
            }

            //By leewheel 2026-09-08 normalize the First Aid action bar at every login (force slot 11 for new chars)
            //Normaliser la barre d'actions de Premiers soins a chaque connexion (forcer bouton 11 pour les nouveaux)
            if (NormalizeFirstAidActionBar(player, firstLogin))
                player->SendActionButtons(1);
            //End By leewheel
        }
    }
};

void AddSC_syphrena_first_aid()
{
    new syphrena_first_aid();
}
