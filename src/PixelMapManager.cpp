#include "PixelMapManager.hpp"

#include <algorithm>
#include <cstring>
#include <dlfcn.h>

PixelMapManager::~PixelMapManager() noexcept
{
    for (const auto &p : m_mappings)
    {
        if (p.handle) dlclose(p.handle);
        if (p.map) delete p.map;
    }
}

MapTemplate *
PixelMapManager::getMapTemplate(const char *mapName) const noexcept
{
    if (m_mappings.empty()) return nullptr;

    auto it = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                           [&mapName](const PixelMap &p) -> bool
    { return p.map && std::strcmp(p.map->name(), mapName) == 0; });

    if (it != m_mappings.end()) return it->map;

    return nullptr;
}
