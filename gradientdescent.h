#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include <memory>

#include <QtDataVisualization/QCustom3DItem>
#include <QColor>

#include "item.h"

struct Point {
    double x = 0.;
    double z = 0.;
    Point() : x(0.), z(0.) {}
    Point(double _x, double _z) : x(_x), z(_z) {}
};

class GradientDescent {
public:
    GradientDescent();
    virtual ~GradientDescent() {}

    const char* name;
    double learning_rate = 0.01;

    // TODO: move these to the animation class
    // visual elements
    QColor ball_color;
    std::unique_ptr<Ball> ball;


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
    VanillaGradientDescent() {
        ball_color = Qt::cyan;
        name = "&Gradient Descent";
    }

protected:
     void updateGradientDelta();
};

class Momentum : public GradientDescent {
public:
    Momentum() {
        ball_color = Qt::magenta;
        name = "&Momentum";
    }

    double decay_rate = 0.8;

protected:
    void updateGradientDelta();
};

class AdaGrad : public GradientDescent {
public:
    AdaGrad() : grad_sum_of_squared(0., 0.){
        ball_color = Qt::white;
        name = "&AdaGrad";
        learning_rate = 1.;
    }
    Point gradSumOfSquared(){return grad_sum_of_squared;}

protected:
    void updateGradientDelta();
    void resetState();

private:
    Point grad_sum_of_squared;
};

class RMSProp : public GradientDescent {
public:
    RMSProp() : decayed_grad_sum_of_squared(0., 0.){
        ball_color = Qt::green;
        name = "&RMSProp";
    }

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
    Adam() : decayed_grad_sum(0., 0.),
        decayed_grad_sum_of_squared(0., 0.)
    {
        ball_color = Qt::blue;
        name = "&Adam";
    }

    double beta1 = 0.9;
    double beta2 = 0.999;
    Point decayedGradSum(){return decayed_grad_sum;}
    Point decayedGradSumOfSquared(){return decayed_grad_sum_of_squared;}

protected:
    void updateGradientDelta();
    void resetState();

private:
    Point decayed_grad_sum;
    Point decayed_grad_sum_of_squared;
};

#endif // GRADIENTDESCENT_H
