#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include <memory>

#include <QtDataVisualization/QCustom3DItem>
#include <QColor>

#include "point.h"


namespace Function{
enum FunctionName {local_minimum, global_minimum, saddle_point, ecliptic_bowl,
                  hills, plateau};
}


class GradientDescent {
public:
    GradientDescent();
    virtual ~GradientDescent() {}

    double learning_rate = 0.001;
    static Function::FunctionName function_name;

    // simple getters and setters
    Point position() {return p;}
    void setStartingPosition(double x, double z) {starting_p.x = x; starting_p.z = z;}
    bool isConverged() {return is_converged;};
    double gradX() {return grad.x;};
    double gradZ() {return grad.z;};
    Point delta() {return m_delta;}


    // core methods
    static double f(double x, double z);
    Point takeGradientStep();
    void resetPositionAndComputeGradient();

protected:
    Point p; // current position
    Point starting_p; // starting position
    Point m_delta; // movement in each direction after a gradient step
    Point grad; // gradient at the current position
    bool is_converged = false;

    void setPositionAndComputeGradient(double x, double z);
    void computeGradient();
    virtual void updateGradientDelta() = 0;
    virtual void resetState(){}
};


class VanillaGradientDescent : public GradientDescent {
public:
    VanillaGradientDescent() {}

protected:
     void updateGradientDelta();
};


class Momentum : public GradientDescent {
public:
    Momentum() {}

    double decay_rate = 0.9;

protected:
    void updateGradientDelta();
};

class QHM : public GradientDescent {
public:
    QHM(): momentum( 0., 0.) { }

    double decay_rate = 0.990;     // beta
    double discount_factor = 0.7;  // v

protected:
    void updateGradientDelta() override;
    void resetState() override;
private:
    Point momentum;
};

class AdaGrad : public GradientDescent {
public:
    AdaGrad() : grad_sum_of_squared(0., 0.){}
    Point gradSumOfSquared(){return grad_sum_of_squared;}

protected:
    void updateGradientDelta();
    void resetState();

private:
    Point grad_sum_of_squared;
};

class RMSProp : public GradientDescent {
public:
    RMSProp() : decayed_grad_sum_of_squared(0., 0.){}

    double decay_rate = 0.99;
    Point decayedGradSumOfSquared(){return decayed_grad_sum_of_squared;}

protected:
    void updateGradientDelta();
    void resetState();

private:
    Point decayed_grad_sum_of_squared;
};

class Adam : public GradientDescent {
public:
    Adam()
        : decayed_grad_sum( 0., 0. )
        , decayed_grad_sum_of_squared( 0., 0. )
        , beta1_pow( beta1 )
        , beta2_pow( beta2 )
    { }

    double beta1 = 0.9;
    double beta2 = 0.999;
    bool use_bias_correction = true;

    Point decayedGradSum(){return decayed_grad_sum;}
    Point decayedGradSumOfSquared(){return decayed_grad_sum_of_squared;}

protected:
    void baseCompute( Point &scaled_decayed_grad_sum, Point &scaled_decayed_grad_sum_sq );
    void updateGradientDelta() override;
    void resetState() override;

private:
    Point decayed_grad_sum;
    Point decayed_grad_sum_of_squared;
    double beta1_pow;
    double beta2_pow;
};

class QHAdam : public Adam {
public:
    QHAdam() {}

    double discount_factor = 0.7;         // v1
    double squared_discount_factor = 1.0; // v2

protected:
    void updateGradientDelta() override;
};

#endif // GRADIENTDESCENT_H
