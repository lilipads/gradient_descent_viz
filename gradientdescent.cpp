#include "gradientdescent.h"

GradientDescent::GradientDescent()
    : ball(new QtDataVisualization::QCustom3DItem)
{
    resetPosition();
}

float GradientDescent::f(float x, float z){
    return x * x + z * z;
}

float GradientDescent::gradX(){
    // use finite difference method
    const float epsilon = 1e-5;
    return (f(p.x + epsilon, p.z) - f(p.x - epsilon, p.z)) / (2 * epsilon);
}

float GradientDescent::gradZ(){
    // use finite difference method
    const float epsilon = 1e-5;
    return (f(p.x, p.z + epsilon) - f(p.x, p.z - epsilon)) / (2 * epsilon);
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
    delta_x = momentum * delta_x - learning_rate * gradX();
    delta_z = momentum * delta_z - learning_rate * gradZ();
    Point delta(delta_x, delta_z);
    return delta;
}
