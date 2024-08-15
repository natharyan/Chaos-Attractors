#include "lorenz.h"

LorenzAttractor::LorenzAttractor(float dt) : Attractor(){
    this->dt = dt;
    defdt = 0.0005f;
    scale = 17.0f;
    offsetX = 0.0f;
    offsetY = 300.0f;
    angles = {{M_PI / 2, 0, 0}, {0, 0, 0}, {M_PI, 0, 0}, {3 * M_PI / 2, 0, 0}};
    offsetYs = {580, 10, 10, -580};
    defaultaudio = "audio/Debussy - Dances for Harp and Orchestra Danse profane.mp3";
    xyswap = false;
    randrange = 0.2f;
    maxamplitude = 3500.0f;
}

std::vector<float> LorenzAttractor::step(const std::vector<float>& point) const {
    float dx = sigma * (point[1] - point[0]) * dt;
    float dy = (point[0] * (rho - point[2]) - point[1]) * dt;
    float dz = (point[0] * point[1] - beta * point[2]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float LorenzAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.000007f * amplitude;
}