/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GUILDMGR_H
#define _GUILDMGR_H

#include "Guild.h"

class GuildMgr
{
private:
    GuildMgr();
    ~GuildMgr();

    typedef std::unordered_map<uint32, Guild*> GuildContainer;

public:
    static GuildMgr* instance();

    Guild* GetGuildByLeader(ObjectGuid guid) const;
    Guild* GetGuildById(uint32 guildId) const;
    Guild* GetGuildByName(std::string_view guildName) const;
    std::string GetGuildNameById(uint32 guildId) const;

    void LoadGuilds();
    void AddGuild(Guild* guild);
    void RemoveGuild(uint32 guildId);

    uint32 GenerateGuildId();
    void SetNextGuildId(uint32 Id) { NextGuildId = Id; }

    void InitAutomaticGuildXPDistribution();
    uint32 GetGuildAutoDistibutionChecker() const { return m_GuildAutoDistributionTimeChecker; };
    time_t GetGuildNextPeriodicUpdateTime() const { return m_NextGuildPeriodicQueueUpdateTime; };
    void SetGuildAutoDistibutionChecker(uint32 val) { m_GuildAutoDistributionTimeChecker = val; }
    void SetGuildNextPeriodicUpdateTime(time_t val) { m_NextGuildPeriodicQueueUpdateTime = val; }

    void DistributeGuildXP();
    void ResetTimes();

    template<class Fn>
    void ForEachGuild(Fn&& fn) const
    {
        for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
            fn(itr->second);
    }

protected:
    uint32 NextGuildId;
    GuildContainer GuildStore;

    uint32 m_GuildAutoDistributionTimeChecker = 0;
    time_t m_NextGuildPeriodicQueueUpdateTime = 0;
};

#define sGuildMgr GuildMgr::instance()

#endif
