// attractor.h

#ifndef ATTRACTOR_H
#define ATTRACTOR_H

#include <vector>
#include <array>
#include <string>
#include <SFML/Graphics.hpp>

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
    std::vector<std::array<float, 3>> angles;
    std::array<float, 4> offsetYs;
    std::string defaultaudio;
    bool xyswap;
    float randrange;
    float maxamplitude;
    sf::Color startColor;
    sf::Color endColor;
};;

#endif // ATTRACTOR_H