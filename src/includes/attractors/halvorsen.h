#ifndef HALVORSEN_H
#define HALVORSEN_H
#include <vector>
#include <cmath>
#include <string>
#include "base_attractor.h"

class HalvorsenAttractor : public Attractor{
public:
    HalvorsenAttractor(float dt);
    float dt;
    std::vector<float> step(const std::vector<float>& point) const override;
    float speedfactor(float dt, float amplitude) const override;

private:
    float a = 1.89f;
};

#endif