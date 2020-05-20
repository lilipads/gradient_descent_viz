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
    float learning_rate = 0.01;
    void resetPosition();

    virtual Point gradientStep() = 0;

protected:
    Point p; // current position
    void setCurrentPosition(float x, float z) {p.x = x; p.z = z;}

};

class VanillaGradientDescent : public GradientDescent {
public:
    VanillaGradientDescent(){ ball_color = Qt::cyan; }
    Point gradientStep();
};

class Momentum : public GradientDescent {
public:
    Momentum(){ ball_color = Qt::magenta; }
    float momentum = 0.8;

    Point gradientStep();

private:
    float delta_x = 0.;
    float delta_z = 0.;
};

#endif // GRADIENTDESCENT_H
