/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_FACTION_SCORE_H
#define MOD_AIWORLD_FACTION_SCORE_H

#include "AiWorldCommon.h"
#include <mutex>

class AiFactionScore
{
public:
    static AiFactionScore* instance();

    void Load();
    void Flush();

    void AddKill(AiWorld::FactionSide side, int32 amount = 1);
    void AddCapture(AiWorld::FactionSide side, int32 amount = 5);
    void AddEventWin(AiWorld::FactionSide side, int32 amount = 10);

    [[nodiscard]] AiWorld::FactionScoreRecord Get(AiWorld::FactionSide side) const;
    [[nodiscard]] int32 GetHourDelta() const; // Alliance hour - Horde hour

private:
    AiFactionScore() = default;

    void EnsureWindow();

    mutable std::mutex _mutex;
    AiWorld::FactionScoreRecord _alliance;
    AiWorld::FactionScoreRecord _horde;
    uint32 _hourBucket = 0;
    uint32 _dayBucket = 0;
};

#define sAiFactionScore AiFactionScore::instance()

#endif
