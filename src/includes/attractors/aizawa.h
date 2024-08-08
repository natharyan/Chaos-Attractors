#ifndef AIZAWA_H
#define AIZAWA_H

#include <vector>
#include "base_attractor.h"

class AizawaAttractor : public Attractor {

public:
    AizawaAttractor(float dt);
    float dt;
    std::vector<float> step(const std::vector<float>& point) const override;
    float speedfactor(float dt, float amplitude) const override;

private:
    float a = 0.95f;
    float b = 0.7f;
    float c = 0.6f;
    float d = 3.5f;
    float e = 0.25f;
    float f = 0.1f;
};

#endif