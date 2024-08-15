#ifndef SPROTT_H
#define SPROTT_H

#include <vector>
#include "base_attractor.h"

class SprottAttractor : public Attractor {

public:
    SprottAttractor(float dt);
    float dt;
    std::vector<float> step(const std::vector<float>& point) const override;
    float speedfactor(float dt, float amplitude) const override;

private:
    float a = 2.07f;
    float b = 1.79f;
};

#endif