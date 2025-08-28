#pragma once

#include "sonify/MapTemplate.hpp"

#include <functional>
#include <string>
#include <vector>

typedef struct
{
    std::string name;
    void *handle;
    MapTemplate *map;
} PixelMap;

namespace sonify
{
    using MapFunc =
        std::function<std::vector<short>(const std::vector<Pixel> &)>;
};

class PixelMapManager
{
public:

    ~PixelMapManager() noexcept;

    std::vector<std::string> mappingNames() noexcept;
    inline std::vector<PixelMap> &mappings() noexcept { return m_mappings; }
    inline void addMap(const PixelMap &p) noexcept { m_mappings.push_back(p); }
    MapTemplate *getMapTemplate(const std::string &mapName) const noexcept;

private:

    std::vector<PixelMap> m_mappings;
};
