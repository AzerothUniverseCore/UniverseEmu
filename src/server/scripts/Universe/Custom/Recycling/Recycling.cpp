/* ------------------------------------ */
/* AUTHOR   Aurøra                      */
/*    FOR   Syphrena Private Server     */
/* ------------------------------------ */

class syphrena_recycling : public PlayerScript
{
private:
    bool isEnabled = false;
    uint32 airRecycling = 338060; /* Recycling */

public:
    syphrena_recycling() : PlayerScript("syphrena_recycling") {}

    void OnLogin(Player* player, bool /*firstLogin*/) {
        if (isEnabled) {
            if (!player->HasSpell(airRecycling)) {
                player->LearnSpell(airRecycling, false);
            }
        }
    }
};

void AddSC_syphrena_recycling()
{
    new syphrena_recycling();
}
