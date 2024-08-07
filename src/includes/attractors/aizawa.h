#ifndef AIZAWA_H
#define AIZAWA_H

#include <vector>

class AizawaAttractor {
public:
    AizawaAttractor(float a, float b, float c, float d, float e, float f, float dt);
    std::vector<float> step(const std::vector<float>& point) const;
    float a, b, c, d, e, f, dt;
};

#endif