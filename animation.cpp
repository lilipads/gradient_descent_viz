#include "animation.h"

void AnimationHelper::setBallPosition(Ball* ball, Point p){
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


void Animation::triggerAnimation(){
    animateStep();
    timer->setInterval(kInterval);
    state = (state + 1) % num_states;
}


void Animation::prepareDetailedAnimation(){
    QColor color = descent->ball_color;
    color.setAlpha(100);
    total_arrow = std::unique_ptr<Arrow>(new Arrow);
    m_graph->addCustomItem(total_arrow.get());
    temporary_ball = std::unique_ptr<Ball>(new Ball(color));
    m_graph->addCustomItem(temporary_ball.get());
}


void GradientDescentAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball
    {
        descent->arrowX->setVisible(false);
        descent->arrowZ->setVisible(false);
        total_arrow->setVisible(false);
        Point p = descent->position();
        AnimationHelper::setBallPosition(descent->ball.get(), p);
        break;
    }
    case 1: // show the x and z direction gradients
    { 
        Point grad(descent->gradX(), descent->gradZ());
        descent->arrowX->setMagnitude(grad.x);
        descent->arrowZ->setMagnitude(grad.z);
        for (Arrow* arrow : {descent->arrowX.get(), descent->arrowZ.get()})
        {
            arrow->setPosition(descent->ball->position());
            arrow->setVisible(true);
        }
        break;
    }
    case 2: // show the composite of gradients
    {
        // TODO: change this to delta
        Point grad(descent->gradX(), descent->gradZ());
        total_arrow->setVector(-QVector3D(grad.x, 0, grad.z));
        total_arrow->setPosition(descent->ball->position());
        total_arrow->setVisible(true);
        break;
    }
    case 3: // draw an imganinary ball of the future position
    {
        Point p = descent->takeGradientStep();
        AnimationHelper::setBallPosition(temporary_ball.get(), p);
        break;
    }
    }
}
