// attractor.h

#ifndef ATTRACTOR_H
#define ATTRACTOR_H

#include <vector>
#include <array>
#include <string>

class Attractor {
public:
    virtual ~Attractor() = default;
    virtual std::vector<float> step(const std::vector<float>& point) const = 0;
    virtual float speedfactor(float dt, float amplitude) const = 0;

    float dt;
    float defdt;
    float scale;
    float offsetX;
    float offsetY;
    std::array<float, 4> angles;
    std::array<float, 4> offsetYs;
    std::string defaultaudio;
    bool xyswap;
    float randrange;
    float maxamplitude;
};

#endif // ATTRACTOR_H