#include "gradientdescent.h"

#include <math.h>

const double kDivisionEpsilon = 1e-12;
const double kFiniteDiffEpsilon = 1e-12;
const double kConvergenceEpsilon = 1e-2;


GradientDescent::GradientDescent()
    : arrowX(new Arrow(QVector3D(-1, 0, 0))),
      arrowZ(new Arrow(QVector3D(0, 0, -1))),
      starting_p(4., 7.),
      delta(0., 0.)
{
    resetPosition();
}

double GradientDescent::f(double x, double z){
//    return x * x + z * z;
    z *= 1.4;
    return -2 * exp(-((x - 1) * (x - 1) + z * z) / .2) -6. * exp(-((x + 1) * (x + 1) + z * z) / .2) + x * x + z * z;
}

void GradientDescent::calculateGradient(){
    // use finite difference method
    grad.x = (f(p.x + kFiniteDiffEpsilon, p.z) -
            f(p.x - kFiniteDiffEpsilon, p.z)) / (2 * kFiniteDiffEpsilon);

    grad.z = (f(p.x, p.z + kFiniteDiffEpsilon) -
            f(p.x, p.z - kFiniteDiffEpsilon)) / (2 * kFiniteDiffEpsilon);
}

void GradientDescent::resetPosition(){
    setPosition(starting_p.x, starting_p.z);
    is_converged = false;
}


void GradientDescent::setPosition(double x, double z){
   p.x = x;
   p.z = z;
   calculateGradient();
}

Point GradientDescent::takeGradientStep(){
    if (abs(gradX()) < kConvergenceEpsilon &&
         abs(gradZ()) < kConvergenceEpsilon){
         is_converged = true;
     }
    if (is_converged) return p;

    Point delta = getGradientDelta(grad);
    p.x += delta.x;
    p.z += delta.z;

    return p;
}

Point VanillaGradientDescent::getGradientDelta(Point grad){
    delta.x = -learning_rate * grad.x;
    delta.z = -learning_rate * grad.z;
    return delta;
}

Point Momentum::getGradientDelta(Point grad){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Momentum */

    delta.x = momentum * delta.x - learning_rate * grad.x;
    delta.z = momentum * delta.z - learning_rate * grad.z;
    return delta;
}


Point AdaGrad::getGradientDelta(Point grad){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#AdaGrad */

    grad_sum_of_squared.x += pow(grad.x, 2);
    grad_sum_of_squared.z += pow(grad.z, 2);
    delta.x = -learning_rate * grad.x / (sqrt(grad_sum_of_squared.x) + 1e-8);
    delta.z = -learning_rate * grad.z / (sqrt(grad_sum_of_squared.z) + 1e-8);
    return delta;
}

Point RMSProp::getGradientDelta(Point grad){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#RMSProp */

    decayed_grad_sum_of_squared.x *= decay_rate;
    decayed_grad_sum_of_squared.x += (1 - decay_rate) * pow(grad.x, 2);
    decayed_grad_sum_of_squared.z *= decay_rate;
    decayed_grad_sum_of_squared.z += (1 - decay_rate) * pow(grad.z, 2);
    delta.x = -learning_rate * grad.x / (sqrt(decayed_grad_sum_of_squared.x) + kDivisionEpsilon);
    delta.z = -learning_rate * grad.z / (sqrt(decayed_grad_sum_of_squared.z) + kDivisionEpsilon);
    return delta;
}

Point Adam::getGradientDelta(Point grad){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Adam */

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

    delta.x = -learning_rate * decayed_grad_sum.x / (sqrt(decayed_grad_sum_of_squared.x) + kDivisionEpsilon);
    delta.z = -learning_rate * decayed_grad_sum.z / (sqrt(decayed_grad_sum_of_squared.z) + kDivisionEpsilon);
    return delta;
}
