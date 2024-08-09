#ifndef LORENZ_H
#define LORENZ_H
#include <vector>
#include <cmath>
#include <string>
#include "base_attractor.h"

class LorenzAttractor : public Attractor{
public:
    LorenzAttractor(float dt);
    float dt;
    std::vector<float> step(const std::vector<float>& point) const override;
    float speedfactor(float dt, float amplitude) const override;

private:
    float sigma = 10.0f;
    float rho = 28.0f;
    float beta = 8.0f / 3.0f;
};

#endif