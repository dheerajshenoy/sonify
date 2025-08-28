#pragma once

#include "sonify/MapTemplate.hpp"

#include <functional>
#include <string>
#include <vector>

struct PixelMap
{
    std::string name;
    void *handle;
    MapTemplate *map;

    bool operator==(const PixelMap &other) const noexcept
    {
        return (name == other.name) && (handle == other.handle) &&
               (map == other.map);
    }
};

namespace sonify
{
    using MapFunc =
        std::function<std::vector<short>(const std::vector<Pixel> &)>;
};

class PixelMapManager
{
public:

    ~PixelMapManager() noexcept;

    [[nodiscard]] std::vector<std::string> mappingNames() noexcept;
    [[nodiscard]] inline std::vector<PixelMap> &mappings() noexcept
    {
        return m_mappings;
    }
    inline void addMap(const PixelMap &p) noexcept { m_mappings.push_back(p); }

    [[nodiscard]] MapTemplate *
    getMapTemplate(const std::string &mapName) const noexcept;

    void remove(const PixelMap &p) noexcept;
    void remove(const std::string &mapName) noexcept;
    void remove(const char *mapName) noexcept;

private:

    void _removeFromVec(unsigned int id) noexcept;
    std::vector<PixelMap> m_mappings;
};
