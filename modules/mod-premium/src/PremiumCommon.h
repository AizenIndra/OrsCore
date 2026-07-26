/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Ported from OrstetCore premium/VIP.
 */

#ifndef MOD_PREMIUM_COMMON_H
#define MOD_PREMIUM_COMMON_H

#include "Define.h"
#include <ctime>

namespace Premium
{
enum StringId : uint32
{
    STRING_VIP_MODE = 12001,
    STRING_NOT_VIP = 12170,
    STRING_CMD_DISABLED = 12171,
    STRING_ERROR = 12172,
    STRING_BG = 12173,
    STRING_STEALTH = 12174,
    STRING_DEAD = 12175,
    STRING_CHANGE_RACE = 12176,
    STRING_CUSTOMIZE = 12177,
    STRING_GROUP = 12178,
    STRING_TARGET_NOT_VIP = 12179,
    STRING_TIME_LEFT = 12180,
    STRING_TIME_NEAR_END = 12181,
    STRING_TIME_EXPIRED = 12182,
    STRING_TARGET_TIME_LEFT = 12183,
    STRING_IN_DEV = 12184
};

struct AccountRecord
{
    bool active = false;
    time_t endTime = 0;
    uint8 chatTextColor = 1;
};
} // namespace Premium

#endif
