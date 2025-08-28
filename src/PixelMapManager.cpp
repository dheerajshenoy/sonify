#include "PixelMapManager.hpp"

#include <algorithm>
#include <dlfcn.h>
#include <print>

PixelMapManager::~PixelMapManager() noexcept
{
    for (auto &p : m_mappings)
    {

        if (p.map)
        {
            delete p.map;
            p.map = nullptr;
        }
        if (p.handle)
        {
            dlclose(p.handle);
            p.handle = nullptr;
        }
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

void
PixelMapManager::remove(const PixelMap &pm) noexcept
{
    auto it =
        std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                     [&pm](const PixelMap &p) -> bool { return pm == p; });
    if (it != m_mappings.end())
    {
        auto i = std::distance(m_mappings.cbegin(), it);
        _removeFromVec(i);
    }
}

void
PixelMapManager::remove(const std::string &mapName) noexcept
{
    auto it = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                           [&mapName](const PixelMap &p) -> bool
    { return p.name == mapName; });
    if (it != m_mappings.end())
    {
        auto i = std::distance(m_mappings.cbegin(), it);

        _removeFromVec(i);
    }
}

void
PixelMapManager::remove(const char *mapName) noexcept
{
    auto it = std::find_if(m_mappings.cbegin(), m_mappings.cend(),
                           [&mapName](const PixelMap &p) -> bool
    { return mapName == p.name; });
    if (it != m_mappings.end())
    {
        auto i = std::distance(m_mappings.cbegin(), it);
        _removeFromVec(i);
    }
}

void
PixelMapManager::_removeFromVec(unsigned int id) noexcept
{
    if (id < m_mappings.size())
    {
        auto t = m_mappings.at(id);

        if (t.map)
        {
            delete t.map;
            t.map = nullptr;
        }

        if (t.handle)
        {
            dlclose(t.handle);
            t.handle = nullptr;
        }

        m_mappings[id] = m_mappings.back(); // move last element here

        m_mappings.pop_back(); // remove last element
    }
}
