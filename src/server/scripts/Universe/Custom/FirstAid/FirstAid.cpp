/* ------------------------------------ */
/* AUTHOR   Aurøra                      */
/*    FOR   Syphrena Private Server     */
/* ------------------------------------ */

class syphrena_first_aid : public PlayerScript
{
private:
    bool isEnabled = true;
    uint32 firstAid = 45542; /* First Aid */

public:
    syphrena_first_aid() : PlayerScript("syphrena_first_aid") {}

    void OnLogin(Player* player, bool /*firstLogin*/) {
        if (isEnabled) {
            if (!player->HasSpell(firstAid)) {
                player->LearnSpell(firstAid, false);
            }
        }
    }
};

void AddSC_syphrena_first_aid()
{
    new syphrena_first_aid();
}
