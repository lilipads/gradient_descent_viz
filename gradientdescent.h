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

    float f(float x, float z);
    float gradX();
    float gradZ();
    Point getPosition(){ return p; }
    float learning_rate = 0.001;
    void resetPosition();

    Point gradientStep();

protected:
    Point p; // current position
    bool is_converged = false;

    void setCurrentPosition(float x, float z) {p.x = x; p.z = z;}
    virtual Point getGradientDelta() = 0;

private:
    const float kConvergenceEpsilon = 1e-3;
    void checkConvergence();
};

class VanillaGradientDescent : public GradientDescent {
public:
    VanillaGradientDescent(){ ball_color = Qt::cyan; }

protected:
     Point getGradientDelta();
};

class Momentum : public GradientDescent {
public:
    Momentum(){ ball_color = Qt::magenta; }
    float momentum = 0.8;

protected:
    Point getGradientDelta();

private:
    float delta_x = 0.;
    float delta_z = 0.;
};

#endif // GRADIENTDESCENT_H
