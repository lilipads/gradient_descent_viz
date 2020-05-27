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
    virtual void triggerSimpleAnimation(int animation_speedup);
    virtual void prepareDetailedAnimation();
    std::unique_ptr<GradientDescent> descent;

protected:
    int num_states;
    int state = 0;

    Surface* m_graph;
    QTimer* timer;

    std::unique_ptr<Ball> temporary_ball;
    std::unique_ptr<Arrow> arrowX;
    std::unique_ptr<Arrow> arrowZ;
    std::unique_ptr<Arrow> total_arrow;

    virtual void animateStep() = 0;
    virtual int interval(){return kInterval;}
    bool in_initial_state = true;
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
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Arrow> momentumArrowX;
    std::unique_ptr<Arrow> momentumArrowZ;
};


class AdaGradAnimation : public Animation
{
public:
    AdaGradAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 6;
        descent = std::unique_ptr<GradientDescent>(new AdaGrad);
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Square> squareX;
    std::unique_ptr<Square> squareZ;
    // scale up the arrow, otherwise you can't see because adagrad moves so slow
    const float arrowScale = 1;
};


class RMSPropAnimation : public Animation
{
public:
    RMSPropAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 7;
        descent = std::unique_ptr<GradientDescent>(new RMSProp);
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Square> squareX;
    std::unique_ptr<Square> squareZ;
    const float arrowScale = 1;
};


class AdamAnimation : public Animation
{
public:
    AdamAnimation(Surface* _graph, QTimer* _timer)
        : Animation(_graph, _timer)
    {
        num_states = 9;
        descent = std::unique_ptr<GradientDescent>(new Adam);
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Arrow> momentumArrowX;
    std::unique_ptr<Arrow> momentumArrowZ;
    std::unique_ptr<Square> squareX;
    std::unique_ptr<Square> squareZ;
    // scale up the arrow, otherwise you can't see because adagrad moves so slow
    const float arrowScale = 1;
    int interval() {return 2000;}
};

#endif // ANIMATION_H
