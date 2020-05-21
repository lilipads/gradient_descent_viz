#include "gradientdescent.h"

#include <math.h>

GradientDescent::GradientDescent()
    : ball(new QtDataVisualization::QCustom3DItem),
      delta(0., 0.),
      starting_p(4., 7.)
{
    resetPosition();
}

float GradientDescent::f(float x, float z){
//    return x * x + z * z;
    z *= 1.4;
    return -2 * exp(-((x - 1) * (x - 1) + z * z) / .2) -30. * exp(-((x + 1) * (x + 1) + z * z) / .2) + x * x + z * z;
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
    p = starting_p;
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
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Momentum */

    delta.x = momentum * delta.x - learning_rate * gradX();
    delta.z = momentum * delta.z - learning_rate * gradZ();
    return delta;
}


Point AdaGrad::getGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#AdaGrad */

    Point grad(gradX(), gradZ());
    grad_sum_of_squared.x += pow(grad.x, 2);
    grad_sum_of_squared.z += pow(grad.z, 2);
    delta.x = -learning_rate * grad.x / (sqrt(grad_sum_of_squared.x) + 1e-8);
    delta.z = -learning_rate * grad.z / (sqrt(grad_sum_of_squared.z) + 1e-8);
    return delta;
}

Point RMSProp::getGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#RMSProp */

    Point grad(gradX(), gradZ());
    decayed_grad_sum_of_squared.x *= decay_rate;
    decayed_grad_sum_of_squared.x += (1 - decay_rate) * pow(grad.x, 2);
    decayed_grad_sum_of_squared.z *= decay_rate;
    decayed_grad_sum_of_squared.z += (1 - decay_rate) * pow(grad.z, 2);
    delta.x = -learning_rate * grad.x / (sqrt(decayed_grad_sum_of_squared.x) + kEpsilon);
    delta.z = -learning_rate * grad.z / (sqrt(decayed_grad_sum_of_squared.z) + kEpsilon);
    return delta;
}

Point Adam::getGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Adam */

    Point grad(gradX(), gradZ());
    // first moment (momentum)
    decayed_grad_sum.x *= beta1;
    decayed_grad_sum.x += (1 - beta1) * grad.x;
    decayed_grad_sum.z *= beta1;
    decayed_grad_sum.z += (1 - beta1) * grad.z;
    // second moment (rmsprop)
    decayed_grad_sum_of_squared.x *= beta2;
    decayed_grad_sum_of_squared.x += (1 - beta2) * pow(grad.x, 2);
    decayed_grad_sum_of_squared.z *= beta2;
    decayed_grad_sum_of_squared.z += (1 - beta2) * pow(grad.z, 2);

    delta.x = -learning_rate * decayed_grad_sum.x / (sqrt(decayed_grad_sum_of_squared.x) + kEpsilon);
    delta.z = -learning_rate * decayed_grad_sum.z / (sqrt(decayed_grad_sum_of_squared.z) + kEpsilon);
    return delta;
}
