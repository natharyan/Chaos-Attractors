#ifndef LORENZ_H
#define LORENZ_H
#include <vector>

class LorenzAttractor{
public:
    LorenzAttractor(float sigma, float rho, float beta, float dt);
    std::vector<float> step(const std::vector<float>& point) const;
    float sigma, rho, beta, dt;
    float speedfactor(float dt, float ampltitude) const;
};

#endif