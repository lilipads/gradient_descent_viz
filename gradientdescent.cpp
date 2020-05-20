#include "gradientdescent.h"

GradientDescent::GradientDescent()
    : ball(new QtDataVisualization::QCustom3DItem),
      delta(0., 0.)
{
    resetPosition();
}

float GradientDescent::f(float x, float z){
    return x * x + z * z;
}

float GradientDescent::gradX(){
    // use finite difference method
    return (f(p.x + kFiniteDiffEpsilon, p.z) -
            f(p.x - kFiniteDiffEpsilon, p.z)) / (2 * kFiniteDiffEpsilon);
}

float GradientDescent::gradZ(){
    // use finite difference method
    return (f(p.x, p.z + kFiniteDiffEpsilon) -
            f(p.x, p.z - kFiniteDiffEpsilon)) / (2 * kFiniteDiffEpsilon);
}

void GradientDescent::resetPosition(){
    p = Point(4., 0.);
    is_converged = false;
}

Point GradientDescent::gradientStep(){
    if (is_converged) return p;

    Point delta = getGradientDelta();
    p.x += delta.x;
    p.z += delta.z;
    if (abs(delta.x) < kConvergenceEpsilon &&
            abs(delta.z) < kConvergenceEpsilon){
        is_converged = true;
    }
    return p;
}

Point VanillaGradientDescent::getGradientDelta(){
    Point delta;
    delta.x = -learning_rate * gradX();
    delta.z = -learning_rate * gradZ();
    return delta;
}

Point Momentum::getGradientDelta(){
    delta.x = momentum * delta.x - learning_rate * gradX();
    delta.z = momentum * delta.z - learning_rate * gradZ();
    return delta;
}
