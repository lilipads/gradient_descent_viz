#include "gradient_descent.h"

#include <math.h>

const double kDivisionEpsilon = 1e-12;
const double kFiniteDiffEpsilon = 1e-12;
const double kConvergenceEpsilon = 1e-2;

Function::FunctionName GradientDescent::function_name = Function::local_minimum;


GradientDescent::GradientDescent()
    : starting_p(0., 0.),
      m_delta(0., 0.)
{
    resetPositionAndComputeGradient();
}


double GradientDescent::f(double x, double z){
    switch (function_name){
    case Function::local_minimum:{
        z *= 1.4;
        return -2 * exp(-((x - 1) * (x - 1) + z * z) / .2) -
                6. * exp(-((x + 1) * (x + 1) + z * z) / .2) +
                x * x + z * z;
    }
    case Function::global_minimum:{
        return x * x + z * z;
    }
    case Function::saddle_point:{
        return sin(x) + z * z;
    }
    case Function::ecliptic_bowl:{
        x /= 2.;
        z /= 2.;
        return -exp(-(x * x + 5 * z * z)) + x * x + 0.5 * z * z;
    }
    case Function::hills:{
        z *= 1.4;
        return  2 * exp(-((x - 1) * (x - 1) + z * z) / .2) +
                6. * exp(-((x + 1) * (x + 1) + z * z) / .2) -
                2 * exp(-((x - 1) * (x  - 1) + (z + 1) * (z + 1)) / .2) +
                x * x + z * z;
    }
    case Function::plateau:{
        x *= 10;
        z *= 10;
        double r = sqrt(z * z + x * x) + 0.01;
        return -sin(r) / r + 0.01 * r * r;
    }
    }
    return 0.;
}


void GradientDescent::computeGradient(){
    // use finite difference method
    grad.x = (f(p.x + kFiniteDiffEpsilon, p.z) -
            f(p.x - kFiniteDiffEpsilon, p.z)) / (2 * kFiniteDiffEpsilon);

    grad.z = (f(p.x, p.z + kFiniteDiffEpsilon) -
            f(p.x, p.z - kFiniteDiffEpsilon)) / (2 * kFiniteDiffEpsilon);
}

void GradientDescent::resetPositionAndComputeGradient(){
    is_converged = false;
    m_delta = Point(0, 0);
    resetState();
    setPositionAndComputeGradient(starting_p.x, starting_p.z);
}


void GradientDescent::setPositionAndComputeGradient(double x, double z){
   /* set position and dirty gradient */

   p.x = x;
   p.z = z;
   computeGradient();
}

Point GradientDescent::takeGradientStep(){
    /* take a gradient step. return the new position
     * side effects:
     * - update delta to the step just taken
     * - update position to new position.
     * - update grad to gradient of the new position
     */

    if (abs(gradX()) < kConvergenceEpsilon &&
         abs(gradZ()) < kConvergenceEpsilon){
         is_converged = true;
     }
    if (is_converged) return p;

    updateGradientDelta();
    setPositionAndComputeGradient(p.x + m_delta.x, p.z + m_delta.z);
    return p;
}

void VanillaGradientDescent::updateGradientDelta(){
    m_delta.x = -learning_rate * grad.x;
    m_delta.z = -learning_rate * grad.z;
}

void Momentum::updateGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Momentum */

    m_delta.x = decay_rate * m_delta.x - learning_rate * grad.x;
    m_delta.z = decay_rate * m_delta.z - learning_rate * grad.z;
}

void QHM::updateGradientDelta()
{
    /* https://arxiv.org/abs/1810.06801v4 - paper on QHM and QHADAM */

    momentum.x = decay_rate * momentum.x + ( 1 - decay_rate ) * grad.x;
    momentum.z = decay_rate * momentum.z + (1 - decay_rate) * grad.z;

    // we need to denormalize the learning rate by 1/(1-decay_rate) to correct
    // for the fact that the momentum term is scaled by decay_rate here, but not
    // in the momentum implementation. see section 7.1 of the paper.
    auto adjusted_learning_rate = learning_rate / ( 1 - decay_rate );

    m_delta.x = -adjusted_learning_rate
            * ( ( 1 - discount_factor ) * grad.x + discount_factor * momentum.x );
    m_delta.z = -adjusted_learning_rate
            * ( ( 1 - discount_factor ) * grad.z + discount_factor * momentum.z );
}

void QHM::resetState()
{
    momentum = Point( 0, 0 );
}

void AdaGrad::updateGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#AdaGrad */

    grad_sum_of_squared.x += pow(grad.x, 2);
    grad_sum_of_squared.z += pow(grad.z, 2);
    m_delta.x = -learning_rate * grad.x / (sqrt(grad_sum_of_squared.x) + kDivisionEpsilon);
    m_delta.z = -learning_rate * grad.z / (sqrt(grad_sum_of_squared.z) + kDivisionEpsilon);
}


void AdaGrad::resetState(){
    grad_sum_of_squared = Point(0, 0);
}


void RMSProp::updateGradientDelta(){
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#RMSProp */

    decayed_grad_sum_of_squared.x *= decay_rate;
    decayed_grad_sum_of_squared.x += (1 - decay_rate) * pow(grad.x, 2);
    decayed_grad_sum_of_squared.z *= decay_rate;
    decayed_grad_sum_of_squared.z += (1 - decay_rate) * pow(grad.z, 2);
    m_delta.x = -learning_rate * grad.x / (sqrt(decayed_grad_sum_of_squared.x) + kDivisionEpsilon);
    m_delta.z = -learning_rate * grad.z / (sqrt(decayed_grad_sum_of_squared.z) + kDivisionEpsilon);
}


void RMSProp::resetState(){
    decayed_grad_sum_of_squared = Point(0, 0);
}

void Adam::baseCompute( Point &scaled_decayed_grad_sum, Point &scaled_decayed_grad_sum_sq )
{
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

    if (use_bias_correction) {
        scaled_decayed_grad_sum.x = decayed_grad_sum.x / ( 1 - beta1_pow );
        scaled_decayed_grad_sum.z = decayed_grad_sum.z / ( 1 - beta1_pow );
        scaled_decayed_grad_sum_sq.x = decayed_grad_sum_of_squared.x / ( 1 - beta2_pow );
        scaled_decayed_grad_sum_sq.z = decayed_grad_sum_of_squared.z / ( 1 - beta2_pow );
        beta1_pow *= beta1;
        beta2_pow *= beta2;
    } else {
        scaled_decayed_grad_sum.x = decayed_grad_sum.x;
        scaled_decayed_grad_sum.z = decayed_grad_sum.z;
        scaled_decayed_grad_sum_sq.x = decayed_grad_sum_of_squared.x;
        scaled_decayed_grad_sum_sq.z = decayed_grad_sum_of_squared.z;
    }
}

void Adam::updateGradientDelta()
{
    /* https://en.wikipedia.org/wiki/Stochastic_gradient_descent#Adam */

    Point grad_sum, grad_sum_sq;
    baseCompute( grad_sum, grad_sum_sq );

    m_delta.x = -learning_rate * grad_sum.x
            / ( sqrt( grad_sum_sq.x ) + kDivisionEpsilon );
    m_delta.z = -learning_rate * grad_sum.z
            / ( sqrt( grad_sum_sq.z ) + kDivisionEpsilon );
}


void Adam::resetState(){
    decayed_grad_sum_of_squared = Point(0, 0);
    decayed_grad_sum = Point(0, 0);
    beta1_pow = beta1;
    beta2_pow = beta2;
}

void QHAdam::updateGradientDelta()
{
    /* https://arxiv.org/abs/1810.06801v4 - paper on QHM and QHADAM */
    Point grad_sum, grad_sum_sq;
    baseCompute( grad_sum, grad_sum_sq );

    m_delta.x = -learning_rate
            * ( ( 1 - discount_factor ) * grad.x + discount_factor * grad_sum.x )
            / ( sqrt( ( 1 - squared_discount_factor ) * pow( grad.x, 2 )
                      + squared_discount_factor * grad_sum_sq.x )
                + kDivisionEpsilon );
    m_delta.z = -learning_rate
            * ( ( 1 - discount_factor ) * grad.z + discount_factor * grad_sum.z )
            / ( sqrt( ( 1 - squared_discount_factor ) * pow( grad.z, 2 )
                      + squared_discount_factor * grad_sum_sq.z )
                + kDivisionEpsilon );
}
