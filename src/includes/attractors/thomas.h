#ifndef THOMAS_H
#define THOMAS_H
#include <vector>
#include <cmath>
#include <string>
#include "base_attractor.h"

class ThomasAttractor : public Attractor{
public:
    ThomasAttractor(float dt);
    float dt;
    std::vector<float> step(const std::vector<float>& point) const override;
    float speedfactor(float dt, float amplitude) const override;

private:
    float b = 0.208186f;
};

#endif