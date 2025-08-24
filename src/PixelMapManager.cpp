#include "PixelMapManager.hpp"

#include <algorithm>
#include <dlfcn.h>

PixelMapManager::~PixelMapManager() noexcept
{
    for (const auto &p : m_mappings)
    {
        if (p.handle) dlclose(p.handle);
        if (p.map) delete p.map;
    }
}

sonify::MapFunc
PixelMapManager::getMapFunc(const char *mapName) const noexcept
{
    if (m_mappings.empty()) return nullptr;

    auto _ = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                          [this, mapName](const PixelMap &p) -> bool
    { return p.map->name() == mapName; });

    return nullptr;
}
