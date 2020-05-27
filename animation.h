#ifndef ANIMATION_H
#define ANIMATION_H

#include <memory>

#include <QtCore/QTimer>
#include <QtDataVisualization/QCustom3DItem>

#include "surface.h"
#include "gradientdescent.h"

using namespace  QtDataVisualization;

const auto f = GradientDescent::f;
const float kBallYOffset = 10.f;
const float stepX = 4. / 49;
const float stepZ = 4. / 49;
const int kInterval = 1000; // seconds in between steps

namespace AnimationHelper {
void setBallPositionOnSurface(Ball* ball, Point p);
void setXZArrows(GradientDescent* descent, Point grad);
}


class Animation
{
public:
    Animation(Surface* _graph, QTimer* _timer)
        : m_graph(_graph),
          timer(_timer) {}

    void triggerDetailedAnimation();
    virtual void triggerSimpleAnimation(int animation_speedup,
        bool show_gradient, bool show_momentum, bool show_gradient_squared);
    virtual void prepareDetailedAnimation();
    std::unique_ptr<GradientDescent> descent;

protected:
    int num_states;
    int state = 0;
    bool in_initial_state = true;
    bool has_momentum = false;
    bool has_gradient_squared = false;

    Surface* m_graph;
    QTimer* timer;

    std::unique_ptr<Ball> temporary_ball = nullptr;
    std::unique_ptr<Arrow> arrowX = nullptr;
    std::unique_ptr<Arrow> arrowZ = nullptr;
    std::unique_ptr<Arrow> total_arrow = nullptr;

    std::unique_ptr<Arrow> momentumArrowX = nullptr;
    std::unique_ptr<Arrow> momentumArrowZ = nullptr;
    std::unique_ptr<Square> squareX = nullptr;
    std::unique_ptr<Square> squareZ = nullptr;

    virtual void animateStep() = 0;
    virtual int interval(){return kInterval;}

    void animateGradient();
    void animateMomentum();
    void animateGradientSquared();
    virtual Point momentum(){return Point();};
    virtual Point gradSumOfSquared(){return Point();};

};


class GradientDescentAnimation : public Animation
{
public:
    GradientDescentAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 4;
        descent = std::unique_ptr<GradientDescent>(new VanillaGradientDescent);
    };


    void animateStep();

protected:

};

class MomentumAnimation : public Animation
{
public:
    MomentumAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 6;
        descent = std::unique_ptr<GradientDescent>(new Momentum);
        has_momentum = true;
    };

    void prepareDetailedAnimation();
    void animateStep();

protected:
    Point momentum(){return Point(-descent->delta().x / descent->learning_rate,
                                  -descent->delta().z / descent->learning_rate);}
};


class AdaGradAnimation : public Animation
{
public:
    AdaGradAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 6;
        descent = std::unique_ptr<GradientDescent>(new AdaGrad);
        has_gradient_squared = true;
    };

    void prepareDetailedAnimation();
    void animateStep();

protected:
    // scale up the arrow, otherwise you can't see because adagrad moves so slow
    const float arrowScale = 1;

    Point gradSumOfSquared() {
        return dynamic_cast<AdaGrad*> (descent.get())->gradSumOfSquared();}
};


class RMSPropAnimation : public Animation
{
public:
    RMSPropAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 7;
        descent = std::unique_ptr<GradientDescent>(new RMSProp);
        has_gradient_squared = true;
    };

    void prepareDetailedAnimation();
    void animateStep();

protected:
    const float arrowScale = 1;

    Point gradSumOfSquared(){
        return dynamic_cast<RMSProp*> (descent.get())->decayedGradSumOfSquared();}
};


class AdamAnimation : public Animation
{
public:
    AdamAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 9;
        descent = std::unique_ptr<GradientDescent>(new Adam);
        has_momentum = true;
        has_gradient_squared = true;
    };

    void prepareDetailedAnimation();
    void animateStep();

protected:
    // scale up the arrow, otherwise you can't see because adagrad moves so slow
    const float arrowScale = 1;
    int interval() {return 2000;}

    Point momentum() {
        return dynamic_cast<Adam*> (descent.get())->decayedGradSum();} 
    Point gradSumOfSquared(){
        return dynamic_cast<Adam*> (descent.get())->decayedGradSumOfSquared();}

};

#endif // ANIMATION_H
