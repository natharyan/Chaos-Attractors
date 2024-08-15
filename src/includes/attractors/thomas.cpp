#include "thomas.h"

ThomasAttractor::ThomasAttractor(float dt) : Attractor() {
    this->dt = dt;
    defdt = 0.00035f;
    scale = 140.0f;
    offsetX = 0.0f;
    offsetY = 0.0f;
    angles = {{-0.61f, 2.349f, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    offsetYs = {0,0,0,0};
    defaultaudio = "audio/Debussy - Clair De Lune 2009.mp3";
    xyswap = false;
    randrange = 0.02f;
    maxamplitude = 2500.0f;
}

std::vector<float> ThomasAttractor::step(const std::vector<float>& point) const {
    float dx = (sin(point[1]) - b * point[0]) * dt;
    float dy = (sin(point[2]) - b * point[1]) * dt;
    float dz = (sin(point[0]) - b * point[2]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float ThomasAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.00013f * amplitude;
}