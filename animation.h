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
const float stepX = 16. / 49;
const float stepZ = 16. / 49;
const int kInterval = 1000; // seconds in between steps

namespace AnimationHelper {
void setBallPosition(Ball* ball, Point p);
void setXZArrows(GradientDescent* descent, Point grad);
}


class Animation
{
public:
    Animation(Q3DSurface* _graph, QTimer* _timer, GradientDescent* _descent)
        : m_graph(_graph), timer(_timer), descent(_descent){}

    void triggerAnimation();
    void prepareDetailedAnimation();

protected:
    int num_states;
    int state = 0;

    Q3DSurface* m_graph;
    QTimer* timer;
    GradientDescent* descent;
    std::unique_ptr<Ball> temporary_ball;
    std::unique_ptr<Arrow> total_arrow;


    virtual void animateStep() = 0;
};


class GradientDescentAnimation : public Animation
{
public:
    GradientDescentAnimation(
            Q3DSurface* _graph, QTimer* _timer, GradientDescent* _descent)
        : Animation(_graph, _timer, _descent)
    {
        num_states = 4;
    };


    void animateStep();
};

class MomentumAnimation : public Animation
{
public:
    MomentumAnimation(
            Q3DSurface* _graph, QTimer* _timer, GradientDescent* _descent)
        : Animation(_graph, _timer, _descent)
    {
        num_states = 4;
    };

    void prepareDetailedAnimation();

    void animateStep();

protected:
    std::unique_ptr<Arrow> momentumX;
    std::unique_ptr<Arrow> momentumZ;

};

#endif // ANIMATION_H
