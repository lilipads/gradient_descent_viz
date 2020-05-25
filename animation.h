#ifndef ANIMATION_H
#define ANIMATION_H

#include <memory>

#include <QtCore/QTimer>
#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/Q3DSurface>

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
    Animation(Q3DSurface* _graph, QTimer* _timer, GradientDescent* _descent)
        : descent(_descent),
          m_graph(_graph),
          timer(_timer) {}

    void triggerAnimation();
    void prepareDetailedAnimation();

protected:
    int num_states;
    int state = 0;

    GradientDescent* descent;
    Q3DSurface* m_graph;
    QTimer* timer;
    std::unique_ptr<Ball> temporary_ball;
    std::unique_ptr<Arrow> arrowX;
    std::unique_ptr<Arrow> arrowZ;
    std::unique_ptr<Arrow> total_arrow;

    virtual void animateStep() = 0;
};


class GradientDescentAnimation : public Animation
{
public:
    GradientDescentAnimation(
            Q3DSurface* _graph, QTimer* _timer, VanillaGradientDescent* _descent)
        : Animation(_graph, _timer, _descent)
    {
        num_states = 4;
    };


    void animateStep();

protected:

};

class MomentumAnimation : public Animation
{
public:
    MomentumAnimation(
            Q3DSurface* _graph, QTimer* _timer, Momentum* _descent)
        : Animation(_graph, _timer, _descent)
    {
        num_states = 6;
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Arrow> momentumArrowX;
    std::unique_ptr<Arrow> momentumArrowZ;
    bool in_initial_state = true;

};


class AdaGradAnimation : public Animation
{
public:
    AdaGradAnimation(
            Q3DSurface* _graph, QTimer* _timer, AdaGrad* _descent)
        : Animation(_graph, _timer, _descent)
    {
        num_states = 4;
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Square> squareX;
    std::unique_ptr<Square> squareZ;
    bool in_initial_state = true;

};

#endif // ANIMATION_H
