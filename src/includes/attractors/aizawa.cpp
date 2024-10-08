#include "aizawa.h"

AizawaAttractor::AizawaAttractor(float dt) : Attractor(){
    this->dt = dt;
    defdt = 0.0000005f;
    scale = 300.0f;
    offsetX = 0.0f;
    offsetY = -140.0f;
    // angles = {M_PI, M_PI / 2 + M_PI/16, 3 * M_PI / 2 + M_PI/16, 0};
    angles = {{M_PI / 2, M_PI, 0}, {0, M_PI / 2, 0}, {0, 3 * M_PI / 2, 0}, {0, 0, 0}};
    offsetYs = {0,0,0,0};
    defaultaudio = "audio/Debussy - 2 Arabesques, CD 74, L. 66_ No. 1, Andantino con moto.mp3";
    xyswap = true;
    randrange = 0.008f;
    maxamplitude = 1600.0f;
    startColor = sf::Color(224, 255, 255);
    endColor = sf::Color(239, 204, 144);
}

std::vector<float> AizawaAttractor::step(const std::vector<float>& point) const {
    float dx = ((point[2] - b) * point[0] - d * point[1]) * dt;
    float dy = (d * point[0] + (point[2] - b) * point[1]) * dt;
    float dz = (c + a * point[2] - point[2] * point[2] * point[2] / 3 - (point[0] * point[0] + point[1] * point[1]) * (1 + e * point[2]) + f * point[2] * point[0] * point[0] * point[0]) * dt;
    return {point[0] + dx, point[1] + dy, point[2] + dz};
}

float AizawaAttractor::speedfactor(float dt, float amplitude) const {
    return dt + 0.00002f * amplitude;
}