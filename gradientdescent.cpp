#include "gradientdescent.h"

GradientDescent::GradientDescent(){
    reset();
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

void GradientDescent::reset(){
    p = Point(4., 0.);
}

Point VanillaGradientDescent::gradientStep(){
    p.x -= learning_rate * gradX();
    p.z -= learning_rate * gradZ();
    return p;
}
