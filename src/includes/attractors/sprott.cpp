#include "sprott.h"

SprottAttractor::SprottAttractor(float dt) : Attractor(){
    this->dt = dt;
    defdt = 0.000005f;
    scale = 180.0f;
    offsetX = -20.0f;
    offsetY = 10.0f;
    // angles = {M_PI, M_PI / 2 + M_PI/16, 3 * M_PI / 2 + M_PI/16, 0};
    angles = {{M_PI / 2, M_PI, 0}, {0, M_PI / 2, 0}, {0, 3 * M_PI / 2, 0}, {0, 0, 0}};
    offsetYs = {0,0,0,0};
    defaultaudio = "audio/Ravel - Pavane pour une infante défunte - M. 19.mp3";
    xyswap = true;
    randrange = 0.02f;
    maxamplitude = 2500.0f;
    startColor = sf::Color(128, 0, 32);
    endColor = sf::Color(245,245,220);
}

std::vector<float> SprottAttractor::step(const std::vector<float>& point) const {
    float dx = (point[1]) * dt;
    float dy = (-point[0] + point[1] * point[2]) * dt;
    float dz = (1 - point[1] * point[1]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float SprottAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.00002f * amplitude;
}