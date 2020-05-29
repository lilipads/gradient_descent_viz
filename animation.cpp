#include "animation.h"

const float simpleAnimationArrowScale = 0.2;


void Animation::triggerSimpleAnimation(int animation_speedup,
     bool show_gradient, bool show_momentum, bool show_gradient_squared){
    if (descent->isConverged()) return;
    Point p;
    for (int i = 0; i < animation_speedup; i++)
        p = descent->takeGradientStep();
    if (!m_visible) return;
    ball->setPositionOnSurface(p.x, p.z);
    if (show_gradient) animateGradient();
    if (has_momentum && show_momentum) animateMomentum();
    if (has_gradient_squared && show_gradient_squared) animateGradientSquared();
}


void Animation::animateGradient(){
    if (arrowX == nullptr)
        arrowX = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(-1, 0, 0), Qt::black));
    if (arrowZ == nullptr)
        arrowZ = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(0, 0, -1), Qt::black));
    arrowX->setMagnitude(descent->gradX() * simpleAnimationArrowScale);
    arrowZ->setMagnitude(descent->gradZ() * simpleAnimationArrowScale);
    arrowX->setPosition(ball->position());
    arrowZ->setPosition(ball->position());
}


void Animation::animateMomentum(){
    if (momentumArrowX == nullptr)
        momentumArrowX = std::unique_ptr<Arrow>(
                    new Arrow(m_graph, QVector3D(-1, 0, 0), momentum_color));
    if (momentumArrowZ == nullptr)
        momentumArrowZ = std::unique_ptr<Arrow>(
                    new Arrow(m_graph, QVector3D(0, 0, -1), momentum_color));
    momentumArrowX->setMagnitude(momentum().x * simpleAnimationArrowScale);
    momentumArrowZ->setMagnitude(momentum().z * simpleAnimationArrowScale);
    momentumArrowX->setPosition(ball->position());
    momentumArrowZ->setPosition(ball->position());
}


void Animation::animateGradientSquared(){
    if (squareX == nullptr){
        squareX = std::unique_ptr<Square>(new Square(m_graph));
        QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
        QQuaternion y_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
        squareX->setRotation(z_rotation * y_rotation);
    }
    if (squareZ == nullptr)
        squareZ = std::unique_ptr<Square>(new Square(m_graph));

    squareX->setArea(gradSumOfSquared().x * pow(simpleAnimationArrowScale, 2));
    squareZ->setArea(gradSumOfSquared().z * pow(simpleAnimationArrowScale, 2));
    squareX->setPosition(ball->position());
    squareZ->setPosition(ball->position());
}


void Animation::cleanupGradient(){
    arrowX = nullptr;
    arrowZ = nullptr;
}


void Animation::cleanupMomentum(){
    momentumArrowX = nullptr;
    momentumArrowZ = nullptr;
}


void Animation::cleanupGradientSquared(){
    squareX = nullptr;
    squareZ = nullptr;
}


void Animation::cleanupAll(){
    ball->setVisible(false);
    if (temporary_ball != nullptr) temporary_ball = nullptr;
    if (total_arrow != nullptr) total_arrow = nullptr;
    cleanupGradient();
    cleanupMomentum();
    cleanupGradientSquared();
    detailed_animation_prepared = false;
}


void Animation::setVisible(bool visible){ 
    if (visible != m_visible){
        m_visible = visible;
        ball->setVisible(visible);

        if (arrowX != nullptr) arrowX->setVisible(visible);
        if (arrowZ != nullptr) arrowZ->setVisible(visible);
        if (momentumArrowX != nullptr) momentumArrowX->setVisible(visible);
        if (momentumArrowZ != nullptr) momentumArrowZ->setVisible(visible);
        if (squareX != nullptr) squareX->setVisible(visible);
        if (squareZ != nullptr) squareZ->setVisible(visible);
    }
}

void Animation::triggerDetailedAnimation(){
    if (!detailed_animation_prepared){
        prepareDetailedAnimation();
    }
    animateStep();
    if (!in_initial_state)
        timer->setInterval(interval());
    state = (state + 1) % num_states;
}


void Animation::prepareDetailedAnimation(){
    timer->stop();
    ball->setVisible(true);
    arrowX = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(-1, 0, 0), Qt::black));
    arrowX->setMagnitude(0);
    arrowX->setLabel("gradient in x");
    arrowX->setVisible(false);
    arrowZ = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(0, 0, -1), Qt::black));
    arrowZ->setMagnitude(0);
    arrowZ->setLabel("gradient in z");
    arrowZ->setVisible(false);
    total_arrow = std::unique_ptr<Arrow>(new Arrow(m_graph));
    total_arrow->setLabel("gradient step");
    total_arrow->setVisible(false);
    QColor color = ball_color;
    color.setAlpha(100);
    temporary_ball = std::unique_ptr<Ball>(new Ball(m_graph, color, f));
    detailed_animation_prepared = true;
    timer->start(15);
}


void Animation::restartAnimation(){
    descent->resetPositionAndComputeGradient();
    state = 0;
    ball->setVisible(m_visible);
    in_initial_state = true;
    detailed_animation_prepared = false;
}

void GradientDescentAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball
    {
        in_initial_state = false;
        temporary_ball->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        total_arrow->setVisible(false);
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);;
        break;
    }
    case 1: // show the x and z direction gradients
    { 
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x);
        arrowZ->setMagnitude(grad.z);
        for (Arrow* arrow : {arrowX.get(), arrowZ.get()})
        {
            arrow->setPosition(ball->position());
            arrow->setVisible(true);
        }
        break;
    }
    case 2: // show the composite of gradients
    {
        descent->takeGradientStep();
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);
        break;
    }
    case 3: // draw an imaginary ball of the future position
    {
        ball->setPositionOnSurface(descent->position().x, descent->position().z);
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        break;
    }
    }
}


void MomentumAnimation::prepareDetailedAnimation(){
    Animation::prepareDetailedAnimation();
    momentumArrowX = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(-1, 0, 0), ball_color));
    momentumArrowX->setLabel("momentum x");
    momentumArrowX->setMagnitude(0);
    momentumArrowX->setPosition(ball->position());
    momentumArrowX->setLabelVisibility(false);

    momentumArrowZ = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(0, 0, -1), ball_color));
    momentumArrowZ->setLabel("momentum z");
    momentumArrowZ->setMagnitude(0);
    momentumArrowZ->setPosition(ball->position());
    momentumArrowZ->setLabelVisibility(false);
}


void MomentumAnimation::animateStep(){
    switch(state){
    case 0: // the ball and momentum arrows
    {
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);

        momentumArrowX->setLabel("momentum x");
        momentumArrowZ->setLabel("momentum z");
        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        momentumArrowX->setLabelVisibility(!in_initial_state);
        momentumArrowZ->setLabelVisibility(!in_initial_state);

        break;
    }
    case 1: // decay the momentum
    {
        momentumArrowX->setMagnitude(momentumArrowX->magnitude() *
                                     dynamic_cast<Momentum*> (descent.get()) ->decay_rate);
        momentumArrowZ->setMagnitude(momentumArrowZ->magnitude() *
                                     dynamic_cast<Momentum*> (descent.get()) ->decay_rate);
        momentumArrowX->setLabel("decay momentum");

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        momentumArrowX->setLabelVisibility(!in_initial_state);
        momentumArrowZ->setLabelVisibility(false);

        in_initial_state = false;

        break;
    }
    case 2: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x);
        arrowZ->setMagnitude(grad.z);
        // if in the same direction, then start the arrow at the tip of the momentum arrow
        if (momentumArrowX->magnitude() * grad.x > 0){
            arrowX->setPosition(ball->position() + momentumArrowX->renderedVectorInPlotUnit());
        }
        else{
            arrowX->setPosition(ball->position());
        }

        if (momentumArrowZ->magnitude() * grad.z > 0){
            arrowZ->setPosition(ball->position() + momentumArrowZ->renderedVectorInPlotUnit());
        }
        else{
            arrowZ->setPosition(ball->position());
        }

        momentumArrowX->setLabelVisibility(false);
        momentumArrowZ->setLabelVisibility(false);
        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        break;
    }
    case 3: // show the composite of gradients
    {
        descent->takeGradientStep();
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) / descent->learning_rate);
        total_arrow->setPosition(ball->position());

        total_arrow->setVisible(true);
        break;
    }
    case 4: // show momentum for next iteration
    {
        Point delta = descent->delta();
        momentumArrowX->setMagnitude(-delta.x / descent->learning_rate);
        momentumArrowZ->setMagnitude(-delta.z / descent->learning_rate);
        momentumArrowX->setLabel("momentum for next iteration");

        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        break;
    }
    case 5: // draw an imaginary ball of the future position
    {
        ball->setPositionOnSurface(descent->position().x, descent->position().z);
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        break;
    }
    }
}


void AdaGradAnimation::prepareDetailedAnimation(){
    Animation::prepareDetailedAnimation();
    squareX = std::unique_ptr<Square>(new Square(m_graph));
    squareX->setLabel("sum of gradient squared in x");
    squareX->setArea(0);
    QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
    QQuaternion y_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
    squareX->setRotation(z_rotation * y_rotation);
    squareX->setVisible(false);

    squareZ = std::unique_ptr<Square>(new Square(m_graph));
    squareZ->setLabel("sum of gradient squared in z");
    squareZ->setArea(0);
    squareZ->setVisible(false);
}


void AdaGradAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball and sum of squares
    {
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);

        squareX->setLabel("sum of gradient_x^2");
        squareZ->setLabel("sum of gradient_z^2");
        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        squareX->setLabelVisibility(!in_initial_state);
        squareZ->setLabelVisibility(!in_initial_state);
        break;
    }
    case 1: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * arrowScale);
        arrowZ->setMagnitude(grad.z * arrowScale);
        arrowX->setLabel("gradient in X");
        arrowX->setPosition(ball->position());
        arrowZ->setPosition(ball->position());

        squareX->setLabelVisibility(false);
        squareZ->setLabelVisibility(false);
        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        in_initial_state = false;
        break;
    }
    case 2: // show sum of squares updating
    {
        descent->takeGradientStep();
        squareX->setArea(dynamic_cast<AdaGrad*> (descent.get())->gradSumOfSquared().x);
        squareZ->setArea(dynamic_cast<AdaGrad*> (descent.get())->gradSumOfSquared().z);
        squareX->setLabel("add onto current gradient^2");
        squareX->setVisible(true);
        squareZ->setVisible(true);
        arrowX->setLabelVisibility(false);
        arrowZ->setLabelVisibility(false);
        break;
    }
    case 3: // show delta arrows shrink wrt gradient arrows
    {
        arrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        arrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        arrowX->setLabel("divide by the side of the square");
        break;
    }
    case 4: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate * arrowScale);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);
        squareX->setLabelVisibility(false);
        arrowX->setLabelVisibility(false);

        break;
    }
    case 5: // draw an imaginary ball of the future position
    {
        ball->setPositionOnSurface(descent->position().x, descent->position().z);
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        break;
    }
    }
}



void RMSPropAnimation::prepareDetailedAnimation(){
    Animation::prepareDetailedAnimation();
    squareX = std::unique_ptr<Square>(new Square(m_graph));
    squareX->setLabel("sum of gradient squared in x");
    squareX->setArea(0);
    QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
    QQuaternion y_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
    squareX->setRotation(z_rotation * y_rotation);
    squareX->setVisible(false);

    squareZ = std::unique_ptr<Square>(new Square(m_graph));
    squareZ->setLabel("sum of gradient squared in z");
    squareZ->setArea(0);
    squareZ->setVisible(false);
}


void RMSPropAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball and sum of squares
    {
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);

        squareX->setLabel("sum of gradient_x^2");
        squareZ->setLabel("sum of gradient_z^2");
        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        squareX->setLabelVisibility(!in_initial_state);
        squareZ->setLabelVisibility(!in_initial_state);
        break;
    }
    case 1: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * arrowScale);
        arrowZ->setMagnitude(grad.z * arrowScale);
        arrowX->setLabel("gradient in X");
        arrowX->setPosition(ball->position());
        arrowZ->setPosition(ball->position());

        squareX->setLabelVisibility(false);
        squareZ->setLabelVisibility(false);
        arrowX->setVisible(true);
        arrowZ->setVisible(true);

        break;
    }
    case 2: // show sum of squares decaying
    {
        squareX->setArea(squareX->area() * dynamic_cast<RMSProp*> (descent.get())->decay_rate);
        squareZ->setArea(squareZ->area() * dynamic_cast<RMSProp*> (descent.get())->decay_rate);
        squareX->setLabel("decay gradient^2");
        squareX->setLabelVisibility(!in_initial_state);
        squareX->setVisible(true);
        squareZ->setVisible(true);
        arrowX->setLabelVisibility(false);
        arrowZ->setLabelVisibility(false);
        in_initial_state = false;
        break;
    }
    case 3: // show sum of squares updating
    {
        descent->takeGradientStep();
        squareX->setArea(dynamic_cast<RMSProp*> (descent.get())->decayedGradSumOfSquared().x);
        squareZ->setArea(dynamic_cast<RMSProp*> (descent.get())->decayedGradSumOfSquared().z);
        squareX->setLabel("add on current gradient^2");
        break;
    }
    case 4: // show delta arrows shrink wrt gradient arrows
    {
        arrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        arrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        arrowX->setLabel("divide by the side of the square");
        break;
    }
    case 5: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate * arrowScale);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);
        squareX->setLabelVisibility(false);
        arrowX->setLabelVisibility(false);

        break;
    }
    case 6: // draw an imaginary ball of the future position
    {
        ball->setPositionOnSurface(descent->position().x, descent->position().z);
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        break;
    }
    }
}


void AdamAnimation::prepareDetailedAnimation(){
    Animation::prepareDetailedAnimation();
    momentumArrowX = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(-1, 0, 0), momentum_color));
    momentumArrowX->setMagnitude(0);

    momentumArrowZ = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(0, 0, -1), momentum_color));
    momentumArrowZ->setMagnitude(0);

    squareX = std::unique_ptr<Square>(new Square(m_graph));
    squareX->setArea(0);
    QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
    QQuaternion y_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
    squareX->setRotation(z_rotation * y_rotation);
    squareX->setVisible(false);

    squareZ = std::unique_ptr<Square>(new Square(m_graph));
    squareZ->setArea(0);
    squareZ->setVisible(false);
}



void AdamAnimation::animateStep(){
    switch(state){
    case 0: // the ball and momentum arrows
    {
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);

        momentumArrowZ->setLabel("momentum z");
        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());
        squareZ->setLabel("sum of gradient_z^2");
        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        momentumArrowZ->setLabelVisibility(!in_initial_state);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        squareZ->setLabelVisibility(!in_initial_state);

        break;
    }
    case 1: // decay the momentum
    {
        Point p = descent->position();
        ball->setPositionOnSurface(p.x, p.z);

        momentumArrowX->setMagnitude(momentumArrowX->magnitude() *
                                     dynamic_cast<Adam*> (descent.get()) ->beta1);
        momentumArrowZ->setMagnitude(momentumArrowZ->magnitude() *
                                     dynamic_cast<Adam*> (descent.get()) ->beta1);
        momentumArrowZ->setLabel("decay momentum");
        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        momentumArrowZ->setLabelVisibility(!in_initial_state);
        squareZ->setLabelVisibility(false);

        break;
    }
    case 2: // show sum of squares decaying
    {
        squareX->setArea(squareX->area() * dynamic_cast<Adam*> (descent.get())->beta2);
        squareZ->setArea(squareZ->area() * dynamic_cast<Adam*> (descent.get())->beta2);
        squareZ->setLabel("decay gradient^2");

        squareZ->setLabelVisibility(!in_initial_state);
        squareX->setVisible(true);
        squareZ->setVisible(true);
        momentumArrowZ->setLabelVisibility(false);
        in_initial_state = false;
        break;
    }
    case 3: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x);
        arrowZ->setMagnitude(grad.z);
        // if in the same direction, then start the arrow at the tip of the momentum arrow
        if (momentumArrowX->magnitude() * grad.x > 0){
            arrowX->setPosition(ball->position() + momentumArrowX->renderedVectorInPlotUnit());
        }
        else{
            arrowX->setPosition(ball->position());
        }

        if (momentumArrowZ->magnitude() * grad.z > 0){
            arrowZ->setPosition(ball->position() + momentumArrowZ->renderedVectorInPlotUnit());
        }
        else{
            arrowZ->setPosition(ball->position());
        }

        squareZ->setLabelVisibility(false);
        momentumArrowZ->setLabelVisibility(false);
        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        arrowZ->setLabelVisibility(true);
        break;
    }
    case 4: // update momentum
    {
        descent->takeGradientStep();
        momentumArrowX->setMagnitude(dynamic_cast<Adam*> (descent.get())->decayedGradSum().x);
        momentumArrowZ->setMagnitude(dynamic_cast<Adam*> (descent.get())->decayedGradSum().z);
        momentumArrowZ->setLabel("add gradient to momentum");

        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        break;
    }
    case 5: // update sum of squares
    {
        squareX->setArea(dynamic_cast<Adam*> (descent.get())->decayedGradSumOfSquared().x);
        squareZ->setArea(dynamic_cast<Adam*> (descent.get())->decayedGradSumOfSquared().z);
        squareZ->setLabel("add on current gradient^2");
        momentumArrowZ->setLabelVisibility(false);
        break;
    }
    case 6: // show delta arrows shrink wrt gradient arrows
    {
        momentumArrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        momentumArrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        momentumArrowZ->setLabel("divide momentum by the side of the square");
        squareZ->setLabelVisibility(false);
        break;
    }
    case 7: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) / descent->learning_rate);
        total_arrow->setPosition(ball->position());

        momentumArrowZ->setLabelVisibility(false);
        total_arrow->setVisible(true);
        break;
    }
    case 8: // draw an imaginary ball of the future position
    {
        ball->setPositionOnSurface(descent->position().x, descent->position().z);
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        break;
    }
    }
}

