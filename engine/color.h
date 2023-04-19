#pragma once

struct Color
{
    float r = 0;
    float g = 0;
    float b = 0;

    void operator+=(Color const& rhs)
    {
        this->r += rhs.r;
        this->g += rhs.g;
        this->b += rhs.b;
    }

    Color operator+(Color const& rhs) const
    {
        return {this->r + rhs.r,
                this->g + rhs.g,
                this->b + rhs.b};
    }

    Color operator*(Color const& rhs) const
    {
        return {this->r * rhs.r,
                this->g * rhs.g,
                this->b * rhs.b};
    }

    Color operator*(float rhs) const
    {
        return {
            r * rhs,
            g * rhs,
            b * rhs
        };
    }

    Color& operator*=(float rhs)
    {
        r *= rhs;
        g *= rhs;
        b *= rhs;
        return *this;
    }
};
