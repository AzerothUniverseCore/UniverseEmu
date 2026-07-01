/* ------------------------------------ */
/* AUTHOR   Azgath                      */
/*    FOR   Az'gath Private Server      */
/* ------------------------------------ */

class azgath_gift_mount : public PlayerScript
{
private:
    bool isEnabled = true;
    uint32 flySpell = 34090; /* Maître cavalier */
    uint32 mountSpell = 150034; /* Wyrm éternel ensorcelé */
    uint32 petSpell = 10716; /* Wyrm éternel ensorcelé */

public:
    azgath_gift_mount() : PlayerScript("azgath_gift_mount") {}

    void OnLogin(Player* player, bool /*firstLogin*/) {
        if (isEnabled) {
            if (!player->HasSpell(flySpell)) {
                player->LearnSpell(flySpell, false);
            }

            if (!player->HasSpell(mountSpell)) {
                player->LearnSpell(mountSpell, false);
            }

            if (!player->HasSpell(petSpell)) {
                player->LearnSpell(petSpell, false);
            }
        }
    }
};

void AddSC_azgath_gift_mount()
{
    new azgath_gift_mount();
}
