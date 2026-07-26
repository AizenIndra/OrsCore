/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiFactionScore.h"
#include "AiWorldMemory.h"
#include "GameTime.h"

AiFactionScore* AiFactionScore::instance()
{
    static AiFactionScore score;
    return &score;
}

void AiFactionScore::Load()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _alliance = sAiWorldMemory->GetFactionScore(AiWorld::FactionSide::Alliance);
    _horde = sAiWorldMemory->GetFactionScore(AiWorld::FactionSide::Horde);
    _alliance.side = AiWorld::FactionSide::Alliance;
    _horde.side = AiWorld::FactionSide::Horde;

    uint32 now = uint32(GameTime::GetGameTime().count());
    _hourBucket = now / 3600;
    _dayBucket = now / 86400;
}

void AiFactionScore::Flush()
{
    std::lock_guard<std::mutex> lock(_mutex);
    uint32 now = uint32(GameTime::GetGameTime().count());
    _alliance.updatedAt = now;
    _horde.updatedAt = now;
    sAiWorldMemory->SetFactionScore(_alliance);
    sAiWorldMemory->SetFactionScore(_horde);
}

void AiFactionScore::EnsureWindow()
{
    uint32 now = uint32(GameTime::GetGameTime().count());
    uint32 hour = now / 3600;
    uint32 day = now / 86400;
    if (hour != _hourBucket)
    {
        _alliance.windowHour = 0;
        _horde.windowHour = 0;
        _hourBucket = hour;
    }
    if (day != _dayBucket)
    {
        _alliance.windowDay = 0;
        _horde.windowDay = 0;
        _dayBucket = day;
    }
}

void AiFactionScore::AddKill(AiWorld::FactionSide side, int32 amount)
{
    std::lock_guard<std::mutex> lock(_mutex);
    EnsureWindow();
    AiWorld::FactionScoreRecord& rec = (side == AiWorld::FactionSide::Alliance) ? _alliance : _horde;
    if (side != AiWorld::FactionSide::Alliance && side != AiWorld::FactionSide::Horde)
        return;
    rec.windowHour += amount;
    rec.windowDay += amount;
    rec.total += amount;
}

void AiFactionScore::AddCapture(AiWorld::FactionSide side, int32 amount)
{
    AddKill(side, amount);
}

void AiFactionScore::AddEventWin(AiWorld::FactionSide side, int32 amount)
{
    AddKill(side, amount);
}

AiWorld::FactionScoreRecord AiFactionScore::Get(AiWorld::FactionSide side) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (side == AiWorld::FactionSide::Alliance)
        return _alliance;
    if (side == AiWorld::FactionSide::Horde)
        return _horde;
    return {};
}

int32 AiFactionScore::GetHourDelta() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _alliance.windowHour - _horde.windowHour;
}
