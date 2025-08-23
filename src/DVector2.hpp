#pragma once

template <typename T> struct DVector2
{
    T x, y;

    bool operator==(const DVector2 &other) noexcept
    {
        return this->x == other.x && this->y == other.y;
    }
};
