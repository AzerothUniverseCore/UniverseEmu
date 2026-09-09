/* ------------------------------------ */
/* AUTHOR   Aurora                       */
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
//  2) On every login normalize the action bars, wherever the icon(s) currently are:
//       - exactly one icon anywhere -> leave it on that slot untouched (the player may have
//         dragged it off slot 11 on purpose), only refresh it if it still shows an old rank;
//       - two or more icons (the actual duplicate-spam bug) -> keep the first one found, drop
//         the rest, refresh the kept one to the mastered rank;
//       - no icon at all -> only place one on slot 11 for a brand new character's very first
//         login (this OVERWRITES whatever default class spell is on slot 11 - required so the
//         client succeeds the incoming rank-chain packets onto that slot in place instead of
//         spawning duplicate icons); an older character who removed it on purpose is left alone.
//     Slot 11 is therefore only ever a *default* for brand-new characters, never a place the
//     script forces an existing character's icon back onto after they move it.
//  3) When something changed, immediately resend the whole action bar (SMSG_ACTION_BUTTONS) so the
//     client display is overwritten with the clean layout.
//2026-09-08 update 1: fixed a regression where moving the icon off slot 11 and relogging would
//silently move it back to slot 11 every time (step 2 used to strip every non-11 slot
//unconditionally, including the player's own single, deliberately-moved icon).
//2026-09-08 update 2: fixed a regression from update 1 where brand new characters got duplicate
//First Aid icons again on character creation, because step "no icon at all" had been made
//conditional on slot 11 already being empty - it almost never is (a default class spell sits
//there), so the preemptive seed never fired. Slot 11 is now overwritten unconditionally again on
//a character's very first login, exactly like the original script, while the slot-11-snap-back
//fix from update 1 (steps 2 and 3 above) is kept intact.
//End By leewheel

#include "Player.h"
#include "ScriptMgr.h"
#include <vector>

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

        // 1) Find every existing First Aid icon, on ANY slot (the player may have moved it).
        //    Trouver toutes les icones de Premiers soins existantes, sur N'IMPORTE quel bouton
        //    (le joueur a pu la deplacer).
        std::vector<uint8> firstAidButtons;
        for (uint8 button = 0; button < MAX_ACTION_BUTTONS; ++button)
        {
            ActionButton const* ab = player->GetActionButton(button);
            if (ab && ab->GetType() == ACTION_BUTTON_SPELL && IsFirstAidChainSpell(ab->GetAction()))
                firstAidButtons.push_back(button);
        }

        // 2) Exactly one icon already present: this IS the player's own placement, wherever it
        //    is on the bars. Never move it - only refresh it in place if it still shows an older
        //    rank than the mastered spell (45542).
        //    Une seule icone deja presente: c'est le choix du joueur, ou qu'elle soit sur les
        //    barres. Ne jamais la deplacer - juste la rafraichir sur place si elle affiche encore
        //    un rang inferieur au rang maitrise (45542).
        if (firstAidButtons.size() == 1)
        {
            uint8 button = firstAidButtons[0];
            ActionButton const* ab = player->GetActionButton(button);
            if (player->HasSpell(firstAid) && ab->GetAction() != firstAid)
            {
                if (player->addActionButton(button, firstAid, ACTION_BUTTON_SPELL))
                    changed = true;
            }

            return changed;
        }

        // 3) Two or more copies (the original duplicate-spam bug): keep only the first one found
        //    (lowest slot number), drop every other copy, and refresh the kept one to the
        //    mastered rank. This never touches slot 11 specifically - it keeps whichever slot the
        //    surviving icon happens to be on.
        //    Deux icones ou plus (bug de doublons d'origine): garder uniquement la premiere
        //    trouvee (bouton le plus bas), retirer toutes les autres, et la rafraichir au rang
        //    maitrise. Cela ne touche pas specifiquement le bouton 11 - le bouton conserve est
        //    celui ou l'icone survivante se trouvait deja.
        if (firstAidButtons.size() > 1)
        {
            for (size_t i = 1; i < firstAidButtons.size(); ++i)
            {
                player->removeActionButton(firstAidButtons[i]);
                changed = true;
            }

            uint8 keptButton = firstAidButtons[0];
            ActionButton const* ab = player->GetActionButton(keptButton);
            if (player->HasSpell(firstAid) && (!ab || ab->GetAction() != firstAid))
            {
                if (player->addActionButton(keptButton, firstAid, ACTION_BUTTON_SPELL))
                    changed = true;
            }

            return changed;
        }

        // 4) No copy anywhere: only place one on slot 11 for a brand new character's very first
        //    login, OVERWRITING whatever default class spell already sits there. This is required:
        //    seeding slot 11 before the recursive First Aid rank-chain (3273 -> ... -> 45542) learn
        //    packets reach the client is what makes the client auto-succeed that slot's icon in
        //    place instead of auto-placing every learned rank into a new empty slot, which is what
        //    caused the original duplicate-icon pile-up on character creation. Older characters who
        //    removed every copy of their own First Aid icon on purpose are left alone (forcePlaceAtLastSlot
        //    is only true on the character's very first login).
        //    Aucune icone nulle part: on en place une sur le bouton 11 uniquement lors de la toute
        //    premiere connexion d'un nouveau personnage, en ECRASANT le sort de classe par defaut qui
        //    s'y trouve deja. C'est necessaire: pre-remplir le bouton 11 avant que les paquets
        //    d'apprentissage de la chaine de rangs de Premiers soins (3273 -> ... -> 45542) n'arrivent
        //    au client est ce qui permet au client de faire succeder l'icone de ce bouton sur place au
        //    lieu de placer chaque rang appris dans un nouveau bouton vide, ce qui causait l'empilement
        //    de doublons a la creation du personnage. Les personnages plus anciens qui ont retire leur
        //    propre icone expres ne sont pas modifies (forcePlaceAtLastSlot n'est vrai qu'a la toute
        //    premiere connexion).
        if (forcePlaceAtLastSlot && player->HasSpell(firstAid))
        {
            if (player->addActionButton(11, firstAid, ACTION_BUTTON_SPELL))
                changed = true;
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
