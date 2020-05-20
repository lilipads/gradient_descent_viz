#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include <memory>
#include <QtDataVisualization/QCustom3DItem>
#include <QColor>

struct Point {
    float x;
    float z;
    Point() : x(0.), z(0.) {}
    Point(float _x, float _z) : x(_x), z(_z) {}
};

class GradientDescent {
public:
    GradientDescent();
    virtual ~GradientDescent() {}

    std::unique_ptr<QtDataVisualization::QCustom3DItem> ball;
    QColor ball_color;
    const char* name;

    float f(float x, float z);
    float gradX();
    float gradZ();
    Point getPosition(){ return p; }
    float learning_rate = 0.001;
    void resetPosition();

    Point gradientStep();

protected:
    Point p; // current position
    Point delta;
    bool is_converged = false;

    void setCurrentPosition(float x, float z) {p.x = x; p.z = z;}
    virtual Point getGradientDelta() = 0;

private:
    const float kFiniteDiffEpsilon = 1e-5;
    const float kConvergenceEpsilon = 1e-5;
    void checkConvergence();
};

class VanillaGradientDescent : public GradientDescent {
public:
    VanillaGradientDescent() {
        ball_color = Qt::cyan;
        name = "&Gradient Descent";
    }

protected:
     Point getGradientDelta();
};

class Momentum : public GradientDescent {
public:
    Momentum() {
        ball_color = Qt::magenta;
        name = "&Momentum";
    }

    float momentum = 0.8;

protected:
    Point getGradientDelta();

};

class AdaGrad : public GradientDescent {
public:
    AdaGrad() : grad_sum_of_squared(0., 0.){
        ball_color = Qt::darkYellow;
        name = "&AdaGrad";
    }

protected:
    Point getGradientDelta();

private:
    Point grad_sum_of_squared;
};

#endif // GRADIENTDESCENT_H
