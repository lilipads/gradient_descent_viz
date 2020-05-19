#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include <memory>
#include <QtDataVisualization/QCustom3DItem>

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

    float f(float x, float z);
    float gradX();
    float gradZ();
    Point getPosition(){ return p; }
    void setLearningRate(float lr){ learning_rate = lr; }
    void resetPosition();

    virtual Point gradientStep() = 0;

protected:
    Point p; // current position
    void setCurrentPosition(float x, float z) {p.x = x; p.z = z;}
    float learning_rate = 0.01;
};

class VanillaGradientDescent : public GradientDescent {
public:
    Point gradientStep();
};

class Momentum : public GradientDescent {
public:
    Point gradientStep();
    void setMomemtum(float m) {momentum = m;}
private:
    float delta_x = 0.;
    float delta_z = 0.;
    float momentum = 0.8;
};

#endif // GRADIENTDESCENT_H
