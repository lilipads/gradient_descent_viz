#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

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

    float f(float x, float z);
    float gradX();
    float gradZ();
    Point getPosition(){ return p; }
    void setLearningRate(float lr){ learning_rate = lr; }
    void reset();

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

#endif // GRADIENTDESCENT_H
