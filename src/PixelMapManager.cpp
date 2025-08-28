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
PixelMapManager::getMapTemplate(const std::string &mapName) const noexcept
{
    if (m_mappings.empty()) return nullptr;

    auto it = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                           [&mapName](const PixelMap &p) -> bool
    { return p.name == mapName; });

    if (it != m_mappings.end()) return it->map;

    return nullptr;
}

std::vector<std::string>
PixelMapManager::mappingNames() noexcept
{
    std::vector<std::string> result;

    for (const auto &p : m_mappings)
        result.push_back(p.name);

    return result;
}
