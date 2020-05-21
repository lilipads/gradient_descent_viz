#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include <memory>
#include <QtDataVisualization/QCustom3DItem>
#include <QColor>

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

    std::unique_ptr<QtDataVisualization::QCustom3DItem> ball;
    QColor ball_color;
    const char* name;
    double learning_rate = 0.01;

    // simple getters and setters
    Point getPosition() {return p;}
    void setStartingPosition(double x, double z) {starting_p.x = x; starting_p.z = z;}
    bool isConverged() {return is_converged;};

    // core methods
    double f(double x, double z);
    double gradX();
    double gradZ();
    Point gradientStep();
    void resetPosition();

protected:
    Point p; // current position
    Point starting_p; // starting position
    Point delta; // movement in each direction after a gradient step
    bool is_converged = false;

    void setCurrentPosition(double x, double z) {p.x = x; p.z = z;}
    virtual Point getGradientDelta(Point grad) = 0;
};

class VanillaGradientDescent : public GradientDescent {
public:
    VanillaGradientDescent() {
        ball_color = Qt::cyan;
        name = "&Gradient Descent";
    }

protected:
     Point getGradientDelta(Point grad);
};

class Momentum : public GradientDescent {
public:
    Momentum() {
        ball_color = Qt::magenta;
        name = "&Momentum";
    }

    double momentum = 0.8;

protected:
    Point getGradientDelta(Point grad);

};

class AdaGrad : public GradientDescent {
public:
    AdaGrad() : grad_sum_of_squared(0., 0.){
        ball_color = Qt::white;
        name = "&AdaGrad";
        learning_rate = 1.;
    }

protected:
    Point getGradientDelta(Point grad);

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

protected:
    Point getGradientDelta(Point grad);

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

protected:
    Point getGradientDelta(Point grad);

private:
    Point decayed_grad_sum;
    Point decayed_grad_sum_of_squared;
};

#endif // GRADIENTDESCENT_H
