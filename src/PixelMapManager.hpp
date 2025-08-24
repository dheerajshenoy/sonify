#pragma once

#include "sonify/MapTemplate.hpp"

#include <functional>
#include <vector>

typedef struct
{
    void *handle;
    MapTemplate *map;
} PixelMap;

namespace sonify
{
    using MapFunc =
        std::function<std::vector<short>(const std::vector<Pixel> &)>;
}

class PixelMapManager
{
public:

    ~PixelMapManager() noexcept;

    inline std::vector<PixelMap> &mappings() noexcept { return m_mappings; }
    inline void addMap(const PixelMap &p) noexcept
    {
        m_mappings.emplace_back(p);
    }

    sonify::MapFunc getMapFunc(const char *mapName) const noexcept;

private:

    std::vector<PixelMap> m_mappings;
};
