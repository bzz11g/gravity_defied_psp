#pragma once

#include <vector>
#include <memory>

#include "TimerOrMotoPartOrMenuElem.h"

// Moto component physics data (bike parts: chassis, wheels, rider, etc.)
class MotoComponent {
public:
    // Component radius - used in collision detection
    int radiusF16;
    // Component type / radius index (0, 1, or 2 maps to different radii)
    int radiusIndex;
    // Component mass (inverse) - used in force calculations: force / mass = acceleration
    int inverseMassF16;
    // Component lean influence - only set for back wheel, used in angular velocity calc
    int leanInfluenceF16;
    std::vector<std::unique_ptr<TimerOrMotoPartOrMenuElem>> motoComponents = std::vector<std::unique_ptr<TimerOrMotoPartOrMenuElem>>(6);

    MotoComponent();
    ~MotoComponent() = default;
    void reset();
};
