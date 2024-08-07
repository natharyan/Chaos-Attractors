#include "lorenz.h"

LorenzAttractor::LorenzAttractor(float sigma, float rho, float beta, float dt)
    : sigma(sigma), rho(rho), beta(beta), dt(dt) {}



std::vector<float> LorenzAttractor::step(const std::vector<float>& point) const {
    float dx = sigma * (point[1] - point[0]) * dt;
    float dy = (point[0] * (rho - point[2]) - point[1]) * dt;
    float dz = (point[0] * point[1] - beta * point[2]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float LorenzAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.000007f * amplitude;
}