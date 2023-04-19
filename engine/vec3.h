#pragma once
#include <cmath>

#define MPI 3.14159265358979323846

class vec3
{
public:
    vec3() : x(0), y(0), z(0)
    {}

    vec3(float x, float y, float z) : x(x), y(y), z(z)
    {}

    ~vec3()
    {
    }

    vec3(vec3 const& rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;
    }

    vec3 operator+(vec3 const& rhs) const 
    { 
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }
    vec3 operator-(vec3 const& rhs) const 
    { 
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }
    vec3 operator-() const 
    { 
        return {-x, -y, -z};
    }
    vec3 operator*(float const c) const 
    { 
        return {x * c, y * c, z * c};
    }
    vec3 operator*(const vec3& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z};
    }

    float x, y, z;
};

inline float dot(vec3 a, vec3 b)
{

    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Get length of 3D vector
inline float len(vec3 const& v)
{
    return std::sqrtf(dot(v, v));
}

// Get normalized version of v
inline vec3 normalize(const vec3& v)
{
    float l = len(v);
    if (l != 0.f)
        return v * (1.f / l);

    return v;
}

inline vec3 reflect(vec3 v, vec3 n)
{
    return v - n * (2 * dot(v,n));
}

inline vec3 cross(vec3 a, vec3 b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x, };
}