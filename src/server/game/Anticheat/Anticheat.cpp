#include "Anticheat.h"
#include "AccountMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "Language.h"
#include "Log.h"
#include "Player.h"
#include "Realm.h"
#include "Vehicle.h"
#include "World.h"
#include "WorldSession.h"

Anticheat::Anticheat(Player* player)
{
    pPlayer = player;
    m_skipOnePacketForASH = false;
    m_isjumping = false;
    m_canfly = false;
    m_ACKmounted = false;
    m_rootUpd = false;
    m_antiNoFallDmg = false;
    m_antiNoFallDmgLastChance = false;

    m_mountTimer = 0;
    m_rootUpdTimer = 0;
    m_flyhackTimer = 0;
    m_antiNoFallDmgTimer = 0;
    m_reloadModelsDisplayTimer = 0;

    lastMoveClientTimestamp = 0;
    lastMoveServerTimestamp = 0;
}

Anticheat::~Anticheat()
{
    m_skipOnePacketForASH = false;
    m_isjumping = false;
    m_canfly = false;
    m_ACKmounted = false;
    m_rootUpd = false;
    m_antiNoFallDmg = false;
    m_antiNoFallDmgLastChance = false;

    m_mountTimer = 0;
    m_rootUpdTimer = 0;
    m_flyhackTimer = 0;
    m_antiNoFallDmgTimer = 0;
    m_reloadModelsDisplayTimer = 0;

    lastMoveClientTimestamp = 0;
    lastMoveServerTimestamp = 0;
    pPlayer = nullptr;
}

void Anticheat::update(uint32 p_time)
{
    if (m_flyhackTimer > 0)
    {
        if (p_time >= m_flyhackTimer)
        {
            if (!checkOnFlyHack() && sWorld->getBoolConfig(CONFIG_AFH_KICK_ENABLED))
                pPlayer->GetSession()->KickPlayer("AFH kicked by flyhackTimer");

            m_flyhackTimer = sWorld->getIntConfig(CONFIG_ANTICHEAT_FLYHACK_TIMER);
        }
        else
            m_flyhackTimer -= p_time;
    }

    if (m_reloadModelsDisplayTimer > 0)
    {
        if (p_time >= m_reloadModelsDisplayTimer)
        {
            pPlayer->RemoveAura(54844);
            m_reloadModelsDisplayTimer = 0;
        }
        else
            m_reloadModelsDisplayTimer -= p_time;
    }

    if (m_ACKmounted && m_mountTimer > 0)
    {
        if (p_time >= m_mountTimer)
        {
            m_mountTimer = 0;
            m_ACKmounted = false;
        }
        else
            m_mountTimer -= p_time;
    }

    if (m_rootUpd && m_rootUpdTimer > 0)
    {
        if (p_time >= m_rootUpdTimer)
        {
            m_rootUpdTimer = 0;
            m_rootUpd = false;
        }
        else
            m_rootUpdTimer -= p_time;
    }

    if (m_antiNoFallDmg && m_antiNoFallDmgTimer > 0)
    {
        if (p_time >= m_antiNoFallDmgTimer)
        {
            m_antiNoFallDmgTimer = 0;
            m_antiNoFallDmg = false;
            m_antiNoFallDmgLastChance = true;
        }
        else
            m_antiNoFallDmgTimer -= p_time;
    }
}

void Anticheat::punish(uint8 method)
{
    switch (method)
    {
        case 1:
        {
            LOG_INFO("anticheat", "MovementHandler::NoFallingDamage by Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
            ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_NOFALLINGDMG, pPlayer->GetSession()->GetAccountId(), pPlayer->GetName().c_str());
            AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                pPlayer->GetName(),
                getDescriptionACForLogs(9),
                getPositionACForLogs(),
                int32(realm.Id.Realm));
            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_NOFALLINGDMG_KICK_ENABLED))
                pPlayer->GetSession()->KickPlayer("Kicked by anticheat::NoFallingDamage");
            break;
        }
        case 2:
        {
            LOG_INFO("anticheat", "MovementHandler::DOUBLE_JUMP by Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
            ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_DOUBLE_JUMP, pPlayer->GetName().c_str());
            AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                pPlayer->GetName(),
                getDescriptionACForLogs(6),
                getPositionACForLogs(),
                int32(realm.Id.Realm));
            if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_DOUBLEJUMP_ENABLED))
                pPlayer->GetSession()->KickPlayer("Kicked by anticheat::DOUBLE_JUMP");
            break;
        }
        case 3:
        {
            LOG_INFO("anticheat", "MovementHandler::Fake_Jumper by Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
            ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_JUMPER_FAKE, pPlayer->GetName().c_str());
            AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                pPlayer->GetName(),
                getDescriptionACForLogs(7),
                getPositionACForLogs(),
                int32(realm.Id.Realm));
            if (sWorld->getBoolConfig(CONFIG_FAKEJUMPER_KICK_ENABLED))
                pPlayer->GetSession()->KickPlayer("Kicked by anticheat::Fake_Jumper");
            break;
        }
        case 4:
        {
            LOG_INFO("anticheat", "MovementHandler::Fake_flying mode (using MOVEMENTFLAG_FLYING flag doesn't restricted) by Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
            ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_JUMPER_FLYING, pPlayer->GetName().c_str());
            AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                pPlayer->GetName(),
                getDescriptionACForLogs(8),
                getPositionACForLogs(),
                int32(realm.Id.Realm));
            if (sWorld->getBoolConfig(CONFIG_FAKEFLYINGMODE_KICK_ENABLED))
                pPlayer->GetSession()->KickPlayer("Kicked by anticheat::Fake_flying mode");
            break;
        }
        default:
            break;
    }
}

void Anticheat::resetFallingData(float z)
{
    pPlayer->UpdateLastZ(z);

    if (isWaitingLandOrSwimOpcode())
        m_antiNoFallDmg = false;
    if (isUnderLastChanceForLandOrSwimOpcode())
        m_antiNoFallDmgLastChance = false;
}

void Anticheat::startWaitingLandOrSwimOpcode()
{
    m_antiNoFallDmgTimer = 3000;
    m_antiNoFallDmg = true;
}

void Anticheat::updateFallInformationIfNeed(float newZ)
{
    pPlayer->UpdateLastZ(newZ);
}

void Anticheat::setUnderACKmount()
{
    m_mountTimer = 3000;
    m_ACKmounted = true;
}

void Anticheat::setRootACKUpd(uint32 delay)
{
    m_rootUpdTimer = 1500 + delay;
    m_rootUpd = true;
}

void Anticheat::setReloadModelsDisplayTimer()
{
    m_reloadModelsDisplayTimer = 500;
    m_flyhackTimer = 3000;
}

void Anticheat::updateMovementInfo(MovementInfo const& movementInfo)
{
    setLastMoveClientTimestamp(movementInfo.time);
    setLastMoveServerTimestamp(GameTime::GetGameTimeMS().count());
}

bool Anticheat::checkOnFlyHack()
{
    if (isCanFlybyServer())
        return true;

    if (pPlayer->ToUnit()->IsFalling() || pPlayer->IsFalling())
        return true;

    if (pPlayer->IsFlying() && !pPlayer->CanFly())
    {
        LOG_INFO("anticheat", "Player::CheckMovementInfo :  FlyHack Detected for Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
        LOG_INFO("anticheat", "Player::========================================================");
        LOG_INFO("anticheat", "Player IsFlying but CanFly is false");

        ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_AFH_CANFLYWRONG, pPlayer->GetName().c_str());
        AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
            pPlayer->GetName(),
            getDescriptionACForLogs(1),
            getPositionACForLogs(),
            int32(realm.Id.Realm));
        return false;
    }

    if (pPlayer->IsFlying() || pPlayer->IsLevitating() || pPlayer->IsInFlight())
        return true;

    if (pPlayer->GetTransport() || pPlayer->GetVehicle() || pPlayer->GetVehicleKit())
        return true;

    if (pPlayer->HasAuraType(SPELL_AURA_CONTROL_VEHICLE))
        return true;

    if (pPlayer->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
        return true;

    if (pPlayer->HasUnitState(UNIT_STATE_IGNORE_ANTISPEEDHACK))
        return true;

    if (underACKmount())
        return true;

    if (isSkipOnePacketForASH())
        return true;

    Position npos = pPlayer->GetPosition();
    float pz = npos.GetPositionZ();
    if (!pPlayer->IsInWater() && pPlayer->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
    {
        float waterlevel = pPlayer->GetMap()->GetWaterLevel(npos.GetPositionX(), npos.GetPositionY());
        bool hovergaura = pPlayer->HasAuraType(SPELL_AURA_WATER_WALK) || pPlayer->HasAuraType(SPELL_AURA_HOVER);
        if (waterlevel && (pz - waterlevel) <= (hovergaura ? pPlayer->GetCollisionHeight() + 1.5f + pPlayer->GetHoverHeight() : pPlayer->GetCollisionHeight() + pPlayer->GetHoverHeight()))
            return true;

        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  FlyHack Detected for Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
        LOG_INFO("anticheat", "Player::========================================================");
        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  Player has a MOVEMENTFLAG_SWIMMING, but not in water");

        ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_AFK_SWIMMING, pPlayer->GetName().c_str());
        AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
            pPlayer->GetName(),
            getDescriptionACForLogs(2),
            getPositionACForLogs(),
            int32(realm.Id.Realm));
        return false;
    }
    else
    {
        if (pPlayer->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
            return true;

        float z = pPlayer->GetMap()->GetHeight(pPlayer->GetPhaseMask(), npos.GetPositionX(), npos.GetPositionY(), pz + pPlayer->GetCollisionHeight() + 0.5f, true, 50.0f);
        float diff = pz - z;
        if (diff > 6.8f)
            if (diff > 6.8f + pPlayer->GetHoverHeight())
            {
                float waterlevel = pPlayer->GetMap()->GetWaterLevel(npos.GetPositionX(), npos.GetPositionY());
                if (waterlevel && waterlevel + pPlayer->GetCollisionHeight() + pPlayer->GetHoverHeight() > pz)
                    return true;

                float cx, cy, cz;
                pPlayer->GetTheClosestPoint(cx, cy, cz, 0.5, pz, 6.8f);
                if (pz - cz > 6.8f)
                {
                    float hitX = cx;
                    float hitY = cy;
                    float hitZ = cz;
                    MapCollisionData const& collision = pPlayer->GetMap()->GetMapCollisionData();
                    float const startX = pPlayer->GetPositionX();
                    float const startY = pPlayer->GetPositionY();
                    float const startZ = pPlayer->GetPositionZ() + pPlayer->GetCollisionHeight();
                    float const destZ = cz + pPlayer->GetCollisionHeight();
                    float const modifyDist = -pPlayer->GetCollisionHeight();

                    bool const staticHit = collision.GetStaticTree().GetObjectHitPos(
                        startX, startY, startZ, cx, cy, destZ, hitX, hitY, hitZ, modifyDist);
                    if (staticHit)
                        cz = hitZ;

                    hitX = cx;
                    hitY = cy;
                    hitZ = cz;
                    bool const dynamicHit = collision.GetDynamicTree().GetObjectHitPos(
                        pPlayer->GetPhaseMask(), startX, startY, startZ, cx, cy, destZ, hitX, hitY, hitZ, modifyDist);
                    if (dynamicHit)
                        cz = hitZ;

                    if (pz - cz > 6.8f)
                    {
                        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  FlyHack Detected for Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
                        LOG_INFO("anticheat", "Player::========================================================");
                        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  playerZ = {}", pz);
                        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  normalZ = {}", z);
                        LOG_INFO("anticheat", "Player::CheckOnFlyHack :  checkz = {}", cz);
                        ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_AFH, pPlayer->GetName().c_str());
                        AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                            pPlayer->GetName(),
                            getDescriptionACForLogs(3, pz, z),
                            getPositionACForLogs(),
                            int32(realm.Id.Realm));
                        return false;
                    }
                }
            }
    }

    return true;
}

bool Anticheat::checkMovementInfo(MovementInfo const& movementInfo, bool jump)
{
    if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_SPEEDHACK_ENABLED))
        return true;

    uint32 oldctime = getLastMoveClientTimestamp();
    if (oldctime)
    {
        if (pPlayer->ToUnit()->IsFalling() || pPlayer->IsInFlight())
            return true;

        bool vehicle = false;
        if (pPlayer->GetVehicleKit() && pPlayer->GetVehicleKit()->GetBase())
            vehicle = true;

        if (pPlayer->GetVehicle())
            return true;

        if (!pPlayer->IsControlledByPlayer())
            return true;

        if (pPlayer->HasUnitState(UNIT_STATE_IGNORE_ANTISPEEDHACK))
            return true;

        if (isSkipOnePacketForASH())
        {
            setSkipOnePacketForASH(false);
            return true;
        }

        bool transportflag = movementInfo.GetMovementFlags() & MOVEMENTFLAG_ONTRANSPORT || pPlayer->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        float x, y, z;
        Position npos;

        if (!transportflag)
            npos = movementInfo.pos;
        else
            npos = movementInfo.transport.pos;

        if (transportflag)
        {
            if (pPlayer->GetTransOffsetX() == 0.f)
                return true;

            x = pPlayer->GetTransOffsetX();
            y = pPlayer->GetTransOffsetY();
            z = pPlayer->GetTransOffsetZ();
        }
        else
            pPlayer->GetPosition(x, y, z);

        if (sWorld->getBoolConfig(CONFIG_ANTICHEAT_IGNORE_CONTROL_MOVEMENT_ENABLED))
        {
            if (pPlayer->HasUnitState(UNIT_STATE_ROOT) && !underACKRootUpd())
            {
                bool unrestricted = npos.GetPositionX() != x || npos.GetPositionY() != y;
                if (unrestricted)
                {
                    LOG_INFO("anticheat", "CheckMovementInfo :  Ignore controll Hack detected for Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
                    ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_MOVE_UNDER_CONTROL, pPlayer->GetSession()->GetAccountId(), pPlayer->GetName().c_str());
                    AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                        pPlayer->GetName(),
                        getDescriptionACForLogs(4),
                        getPositionACForLogs(),
                        int32(realm.Id.Realm));
                    return false;
                }
            }
        }

        float flyspeed = 0.f;
        float distance, runspeed, difftime, normaldistance, delay, diffPacketdelay;
        uint32 ptime;
        std::string mapname = pPlayer->GetMap()->GetMapName();

        distance = sqrt((npos.GetPositionY() - y) * (npos.GetPositionY() - y) + (npos.GetPositionX() - x) * (npos.GetPositionX() - x));

        if (!jump && !pPlayer->CanFly() && !pPlayer->isSwimming() && !transportflag)
        {
            float diffz = fabs(movementInfo.pos.GetPositionZ() - z);
            float tanangle = distance / diffz;

            if (movementInfo.pos.GetPositionZ() > z &&
                diffz > 1.87f &&
                tanangle < 0.57735026919f)
            {
                LOG_INFO("anticheat", "Player::CheckMovementInfo :  Climb-Hack detected for Account id : {}, Player {}, diffZ = {}, distance = {}, angle = {}, Map = {}, mapId = {}, X = {}, Y = {}, Z = {}",
                    pPlayer->GetSession()->GetAccountId(), pPlayer->GetName(), diffz, distance, tanangle, mapname, pPlayer->GetMapId(), x, y, z);
                ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_WALLCLIMB, pPlayer->GetSession()->GetAccountId(), pPlayer->GetName().c_str(), diffz, distance, tanangle, mapname.c_str(), pPlayer->GetMapId(), x, y, z);
                AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
                    pPlayer->GetName(),
                    getDescriptionACForLogs(5, diffz, distance),
                    getPositionACForLogs(),
                    int32(realm.Id.Realm));
                return false;
            }
        }

        uint32 oldstime = getLastMoveServerTimestamp();
        uint32 stime = GameTime::GetGameTimeMS().count();
        uint32 ping;
        ptime = movementInfo.time;

        if (!vehicle)
            runspeed = pPlayer->GetSpeed(MOVE_RUN);
        else
            runspeed = pPlayer->GetVehicleKit()->GetBase()->GetSpeed(MOVE_RUN);

        if (pPlayer->isSwimming())
        {
            if (!vehicle)
                runspeed = pPlayer->GetSpeed(MOVE_SWIM);
            else
                runspeed = pPlayer->GetVehicleKit()->GetBase()->GetSpeed(MOVE_SWIM);
        }

        if (pPlayer->IsFlying() || pPlayer->CanFly())
        {
            if (!vehicle)
                flyspeed = pPlayer->GetSpeed(MOVE_FLIGHT);
            else
                flyspeed = pPlayer->GetVehicleKit()->GetBase()->GetSpeed(MOVE_FLIGHT);
        }

        if (flyspeed > runspeed)
            runspeed = flyspeed;

        delay = ptime - oldctime;
        diffPacketdelay = 10000000 - delay;

        if (oldctime > ptime)
        {
            LOG_INFO("anticheat", "oldctime > ptime");
            delay = 0;
        }
        diffPacketdelay = diffPacketdelay * 0.0000000001f;
        difftime = delay * 0.001f + diffPacketdelay;

        normaldistance = (runspeed * difftime) + 0.002f;
        if (underACKmount())
            normaldistance += 20.0f;
        if (distance < normaldistance)
            return true;

        ping = uint32(diffPacketdelay * 10000.f);

        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  SpeedHack Detected for Account id : {}, Player {}", pPlayer->GetSession()->GetAccountId(), pPlayer->GetName());
        LOG_INFO("anticheat", "Unit::========================================================");
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldX = {}", x);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldY = {}", y);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  newX = {}", npos.GetPositionX());
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  newY = {}", npos.GetPositionY());
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  packetdistance = {}", distance);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  available distance = {}", normaldistance);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldStime = {}", oldstime);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  oldCtime = {}", oldctime);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  serverTime = {}", stime);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  packetTime = {}", ptime);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  diff delay between old ptk and current pkt = {}", diffPacketdelay);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  FullDelay = {}", delay / 1000.f);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  difftime = {}", difftime);
        LOG_INFO("anticheat", "Unit::CheckMovementInfo :  ping = {}", ping);

        ChatHandler(nullptr).SendGMText(LANG_GM_ANNOUNCE_ASH, pPlayer->GetName().c_str(), normaldistance, distance);
        AccountMgr::RecordAntiCheatLog(pPlayer->GetSession()->GetAccountId(),
            pPlayer->GetName(),
            getDescriptionACForLogs(0, distance, normaldistance),
            getPositionACForLogs(),
            int32(realm.Id.Realm));
    }
    else
        return true;

    return false;
}

std::string Anticheat::getDescriptionACForLogs(uint8 type, float param1, float param2) const
{
    std::string str = "";
    switch (type)
    {
        case 0:
        {
            str = fmt::format("AntiSpeedHack: distance from packet =  {}, available distance = {}", param1, param2);
            break;
        }
        case 1:
        {
            str = "AntiFlyHack: Player IsFlying but CanFly is false";
            break;
        }
        case 2:
        {
            str = "AntiFlyHack: Player has a MOVEMENTFLAG_SWIMMING, but not in water";
            break;
        }
        case 3:
        {
            str = fmt::format("AntiFlyHack: Player::CheckOnFlyHack : playerZ = {}, but normalZ = {}", param1, param2);
            break;
        }
        case 4:
        {
            str = "Ignore controll Hack detected";
            break;
        }
        case 5:
        {
            str = fmt::format("Climb-Hack detected , diffZ =  {}, distance = {}", param1, param2);
            break;
        }
        case 6:
        {
            str = "Double-jump detected";
            break;
        }
        case 7:
        {
            str = "FakeJumper detected";
            break;
        }
        case 8:
        {
            str = "FakeFlying mode detected";
            break;
        }
        case 9:
        {
            str = "NoFallingDamage mode detected";
            break;
        }
        default:
            break;
    }
    return str;
}

std::string Anticheat::getPositionACForLogs() const
{
    uint32 areaId = pPlayer->GetAreaId();
    std::string areaName = "Unknown";
    std::string zoneName = "Unknown";
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
    {
        int locale = pPlayer->GetSession()->GetSessionDbcLocale();
        areaName = area->area_name[locale];
        if (area->zone != 0)
            if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->zone))
                zoneName = zone->area_name[locale];
    }

    return fmt::format("Map: {} ({}) Area: {} ({}) Zone: {} XYZ: {} {} {}", pPlayer->GetMapId(), pPlayer->FindMap() ? pPlayer->FindMap()->GetMapName() : "Unknown", areaId, areaName.c_str(), zoneName.c_str(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ());
}