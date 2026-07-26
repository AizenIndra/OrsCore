#include "Player.h"
#include "ScriptMgr.h"

class Login_script : public PlayerScript
{
public:
    Login_script() : PlayerScript("Login_script", {
        PLAYERHOOK_ON_MAP_CHANGED,
        PLAYERHOOK_ON_LOGIN
    }) { }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!player)
            return;

        player->VerifiedRankBuff(player->GetMap());
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        player->RankControlOnLogin();
        player->LoadPvPRank();
        player->VerifiedRankBuff(player->GetMap());
    }
};

void AddSC_Login_script()
{
    new Login_script();
}
