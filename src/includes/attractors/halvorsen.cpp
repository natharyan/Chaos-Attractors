#include "halvorsen.h"

HalvorsenAttractor::HalvorsenAttractor(float dt) : Attractor() {
    this->dt = dt;
    defdt = 0.00035f;
    scale = 40.0f;
    offsetX = 0.0f;
    offsetY = 0.0f;
    angles = {{9.79f, -0.82f, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    offsetYs = {0,0,0,0};
    defaultaudio = "audio/Debussy - Clair De Lune 2009.mp3";
    xyswap = false;
    randrange = 0.02f;
    maxamplitude = 2500.0f;
}

std::vector<float> HalvorsenAttractor::step(const std::vector<float>& point) const {
    float dx = (-a * point[0] - 4 * point[1] - 4 * point[2] - point[1] * point[1]) * dt;
    float dy = (-a * point[1] - 4 * point[2] - 4 * point[0] - point[2] * point[2]) * dt;
    float dz = (-a * point[2] - 4 * point[0] - 4 * point[1] - point[0] * point[0]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float HalvorsenAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.000008f * amplitude;
}