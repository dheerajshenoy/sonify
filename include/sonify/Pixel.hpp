#pragma once

typedef struct
{
    unsigned int r, g, b, a;
} RGBA;

typedef struct
{
    RGBA rgba;
    int x, y;
} Pixel;

typedef struct
{
    double h, s, v;
} HSV;
