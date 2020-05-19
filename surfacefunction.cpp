#include "surfacefunction.h"

SurfaceFunction::SurfaceFunction(){
    reset();
}

float SurfaceFunction::f(float x, float z){
    return x * x + z * z;
}

float SurfaceFunction::gradX(){
    // use finite difference method
    const float epsilon = 1e-5;
    return (f(p.x + epsilon, p.z) - f(p.x - epsilon, p.z)) / (2 * epsilon);
}

float SurfaceFunction::gradZ(){
    // use finite difference method
    const float epsilon = 1e-5;
    return (f(p.x, p.z + epsilon) - f(p.x, p.z - epsilon)) / (2 * epsilon);
}


Point SurfaceFunction::gradientStep(){
    p.x -= learning_rate * gradX();
    p.z -= learning_rate * gradZ();
    return p;
}

void SurfaceFunction::reset(){
    p = Point(4., 0.);
}
