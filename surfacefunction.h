#ifndef SURFACEFUNCTION_H
#define SURFACEFUNCTION_H

struct Point{
    float x;
    float z;
    Point(float _x, float _z) : x(_x), z(_z) {}
};

class SurfaceFunction
{
public:
    SurfaceFunction();
    float f(float x, float z);
    float gradX();
    float gradZ();
    Point getPosition(){ return p; }
    Point gradientStep();
    void setLearningRate(float lr){ learning_rate = lr; }

private:
    Point p = Point(4., 0.); // current position
    void setCurrentPosition(float x, float z) {p.x = x; p.z = z;}
    float learning_rate = 0.001;
};

#endif // SURFACEFUNCTION_H
