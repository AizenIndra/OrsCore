#ifndef _ANTICHEAT_H
#define _ANTICHEAT_H

#include "Common.h"

class Player;
struct MovementInfo;

class AC_GAME_API Anticheat
{
public:
    Anticheat(Player* player);
    ~Anticheat();

    void update(uint32 p_time);
    void punish(uint8 method);

    void setReloadModelsDisplayTimer();

    void resetFallingData(float z);
    void startWaitingLandOrSwimOpcode();
    void updateFallInformationIfNeed(float newZ);
    bool isWaitingLandOrSwimOpcode() const { return m_antiNoFallDmg; }
    bool isUnderLastChanceForLandOrSwimOpcode() const { return m_antiNoFallDmgLastChance; }
    void setSuccessfullyLanded() { m_antiNoFallDmgLastChance = false; m_antiNoFallDmg = false; }

    void setSkipOnePacketForASH(bool blinked) { m_skipOnePacketForASH = blinked; }
    bool isSkipOnePacketForASH() const { return m_skipOnePacketForASH; }
    void setJumpingbyOpcode(bool jump) { m_isjumping = jump; }
    bool isJumpingbyOpcode() const { return m_isjumping; }
    void setCanFlybyServer(bool canfly) { m_canfly = canfly; }
    bool isCanFlybyServer() const { return m_canfly; }

    bool underACKmount() const { return m_ACKmounted; }
    bool underACKRootUpd() const { return m_rootUpd; }

    void setUnderACKmount();
    void setRootACKUpd(uint32 delay);

    void setLastMoveClientTimestamp(uint32 timestamp) { lastMoveClientTimestamp = timestamp; }
    void setLastMoveServerTimestamp(uint32 timestamp) { lastMoveServerTimestamp = timestamp; }
    uint32 getLastMoveClientTimestamp() const { return lastMoveClientTimestamp; }
    uint32 getLastMoveServerTimestamp() const { return lastMoveServerTimestamp; }
    void updateMovementInfo(MovementInfo const& movementInfo);

    bool checkOnFlyHack();
    bool checkMovementInfo(MovementInfo const& movementInfo, bool jump);

    std::string getDescriptionACForLogs(uint8 type, float param1 = 0.f, float param2 = 0.f) const;
    std::string getPositionACForLogs() const;

private:
    Player* pPlayer = nullptr;

    bool m_skipOnePacketForASH; // Used for skip 1 movement packet after charge or blink
    bool m_isjumping;           // Used for jump-opcode in movementhandler
    bool m_canfly;              // Used for access at fly flag - handled restricted access
    bool m_ACKmounted;
    bool m_rootUpd;
    bool m_antiNoFallDmg;
    bool m_antiNoFallDmgLastChance;
    uint32 m_mountTimer;
    uint32 m_rootUpdTimer;
    uint32 m_flyhackTimer;
    uint32 m_antiNoFallDmgTimer;
    uint32 m_reloadModelsDisplayTimer;

    // Timestamp on client clock of the moment the most recently processed movement packet was SENT by the client
    uint32 lastMoveClientTimestamp;
    // Timestamp on server clock of the moment the most recently processed movement packet was RECEIVED from the client
    uint32 lastMoveServerTimestamp;
};

#endif // _ANTICHEAT_H
