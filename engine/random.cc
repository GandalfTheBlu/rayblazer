#include "random.h"

float RandomFloat(uint32_t x)
{
    constexpr uint32_t max = -1;

    x += 123987 * x;
    x |= (x << 11);
    x ^= x << 13;
    x |= (x >> 11);
    x ^= x >> 17;
    x |= (x << 3);
    x ^= x << 5;

    return (float)x / max;
}

float RandomFloatNTP(uint32_t x)
{
    return RandomFloat(x) * 2.f - 1.f;
}
