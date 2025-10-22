#include <bitset>
#include <cmath>
#include <cstdint>

using ComponentType = std::uint8_t;
using Rotation = float_t;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

struct Vec2 {
    uint32_t x;
    uint32_t y;
};

struct Transform {
    Vec2 Position;
    Rotation rotation;
    Vec2 scale;
};