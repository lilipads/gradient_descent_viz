#include "animation.h"

void AnimationHelper::setBallPosition(QCustom3DItem* ball, Point p){
    const float cutoff = 15;
    float y = f(p.x, p.z);
    // hack: if the graph has a hole that's too deep, we can't see the ball
    // hardcode to lift the ball up
    if (f(p.x + stepX, p.z) - y > cutoff ||
        f(p.x, p.z + stepZ) - y > cutoff){
        y = std::max(f(p.x + stepX, p.z),
                f(p.x, p.z + stepZ) - y) - cutoff - 10;
    }
    else{
        // to make the ball look like it's above the surface
        y += kBallYOffset;
    }
    ball->setPosition(QVector3D(p.x, y, p.z));
}


void AnimationHelper::setArrowGeometry(GradientDescent* descent, Point grad){
    // scale
    descent->arrowX->setScaling(QVector3D(0.1f, 0.1f * grad.x, 0.1f));
    descent->arrowZ->setScaling(QVector3D(0.1f, 0.1f * grad.z, 0.1f));
    // translate
    QVector3D ball_position = descent->ball->position();
    descent->arrowX->setPosition(
                QVector3D(ball_position.x() - grad.x * kArrowOffset,
                          ball_position.y(),
                          ball_position.z()));
    descent->arrowZ->setPosition(
                QVector3D(ball_position.x(),
                          ball_position.y(),
                          ball_position.z() - grad.z * kArrowOffset));
}


void Animation::triggerAnimation(){
    animateStep();
    timer->setInterval(kInterval);
    state = (state + 1) % num_states;
}


void Animation::prepareDetailedAnimation(){
    QColor color = descent->ball_color;
    color.setAlpha(100);
    temporary_ball = std::unique_ptr<Ball>(new Ball(color));
    m_graph->addCustomItem(temporary_ball.get());
}


void GradientDescentAnimation::animateStep(){
    switch(state){
    case 0:
    {
        Point p = descent->getPosition();
        AnimationHelper::setBallPosition(descent->ball.get(), p);
        // TODO: make arrows invisible
        break;
    }
    case 1:
    {
        Point grad(descent->gradX(), descent->gradZ());
        AnimationHelper::setArrowGeometry(descent, grad);
        // TODO: make arrows visible
        break;
    }
    // TODO: draw the composite gradient arrow
    case 2:{
        Point p = descent->takeGradientStep();
        AnimationHelper::setBallPosition(temporary_ball.get(), p);
        break;
    }
    }
}
