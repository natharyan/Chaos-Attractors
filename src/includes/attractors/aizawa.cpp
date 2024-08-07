#include "aizawa.h"

AizawaAttractor::AizawaAttractor(float a, float b, float c, float d, float e, float f, float dt)
    : a(a), b(b), c(c), d(d), e(e), f(f), dt(dt) {}

std::vector<float> AizawaAttractor::step(const std::vector<float>& point) const {
    float dx = ((point[2] - b) * point[0] - d * point[1]) * dt;
    float dy = (d * point[0] + (point[2] - b) * point[1]) * dt;
    float dz = (c + a * point[2] - point[2] * point[2] * point[2] / 3 - point[0] * point[0]+ f * point[2] * point[0] * point[0] * point[0]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}