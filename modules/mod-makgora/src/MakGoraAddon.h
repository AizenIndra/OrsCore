/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_MAKGORA_ADDON_H
#define MOD_MAKGORA_ADDON_H

#include <string_view>

class Player;

namespace MakGoraAddon
{
void Send(Player* player, std::string_view payload);
void SendState(Player* player);
bool HandleIncoming(Player* player, std::string& msg);
} // namespace MakGoraAddon

#endif
