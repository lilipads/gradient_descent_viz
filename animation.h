#ifndef ANIMATION_H
#define ANIMATION_H

#include <memory>

#include <QtCore/QTimer>
#include <QtDataVisualization/QCustom3DItem>

#include "gradientdescent.h"

using namespace  QtDataVisualization;

const auto f = GradientDescent::f;
const float kBallYOffset = 10.f;
const float kArrowOffset = 0.4;
const float stepX = 16. / 49;
const float stepZ = 16. / 49;
const int kInterval = 1000; // seconds in between steps

namespace AnimationHelper {
void setBallPosition(QCustom3DItem* ball, Point p);
void setArrowGeometry(GradientDescent* descent, Point grad);
// void drawArrow(QCustom3DItem* arrow, QVector3D origin, QVector3D direction, double magnitude);
}


class Animation
{
public:
    Animation(QTimer* _timer, GradientDescent* _descent)
        : timer(_timer), descent(_descent){}

    void triggerAnimation();

protected:
    int num_states;
    int state = 0;
    QTimer* timer;
    GradientDescent* descent;
    QCustom3DItem* temporary_ball;

    virtual void animateStep() = 0;
};


class GradientDescentAnimation : public Animation
{
public:
    GradientDescentAnimation(
            QTimer* _timer, GradientDescent* _descent)
        : Animation(_timer, _descent)
    {
        num_states = 3;
    };


    void animateStep();
};

#endif // ANIMATION_H
