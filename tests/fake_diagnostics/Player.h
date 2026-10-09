#pragma once
#include <cstdint>

struct Player
{
    std::uint32_t GuildId = 0;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint32_t ZoneId = 0;
    std::uint32_t AreaId = 0;
    std::uint32_t PhaseMask = 0;
    std::uint32_t GetGuildId() const { return GuildId; }
    std::uint32_t GetMapId() const { return MapId; }
    std::uint32_t GetInstanceId() const { return InstanceId; }
    std::uint32_t GetZoneId() const { return ZoneId; }
    std::uint32_t GetAreaId() const { return AreaId; }
    std::uint32_t GetPhaseMask() const { return PhaseMask; }
};
