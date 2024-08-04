#include "animation.h"

void Animation::triggerSimpleAnimation(int animation_speedup,
     bool show_gradient, bool show_adjusted_gradient,
     bool show_momentum, bool show_gradient_squared, bool show_path){
    if (descent->isConverged()) {
        if (m_visible) path->setVisible(show_path);
        return;
    }
    Point p;
    for (int i = 0; i < animation_speedup; i++)
        p = descent->takeGradientStep();
    path->addPoint(descent->position());

    if (!m_visible) return;

    ball->setPositionOnSurface(p);
    this->show_path = show_path;
    if (show_path) path->render();
    if (show_gradient) animateGradient();
    if (show_adjusted_gradient) animateAdjustedGradient();
    if (has_momentum && show_momentum) animateMomentum();
    if (has_gradient_squared && show_gradient_squared) animateGradientSquared();
    if (descent->isConverged()) {
        cleanupAllButPath();
        ball->setVisible(true);
    }
}


void Animation::setVisible(bool visible){
    if (visible != m_visible){
        m_visible = visible;
        ball->setVisible(visible);

        if (path != nullptr) path->setVisible(visible && show_path);
        if (arrowX != nullptr) arrowX->setVisible(visible);
        if (arrowZ != nullptr) arrowZ->setVisible(visible);
        if (adjustedArrowX != nullptr) adjustedArrowX->setVisible(visible);
        if (adjustedArrowZ != nullptr) adjustedArrowZ->setVisible(visible);
        if (momentumArrowX != nullptr) momentumArrowX->setVisible(visible);
        if (momentumArrowZ != nullptr) momentumArrowZ->setVisible(visible);
        if (squareX != nullptr) squareX->setVisible(visible);
        if (squareZ != nullptr) squareZ->setVisible(visible);
    }
}


void Animation::animateGradient(){
    if (arrowX == nullptr)
        arrowX = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(-1, 0, 0), kGradientColor));
    if (arrowZ == nullptr)
        arrowZ = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(0, 0, -1), kGradientColor));
    arrowX->setMagnitude(descent->gradX() * kSimpleAnimationArrowScale);
    arrowZ->setMagnitude(descent->gradZ() * kSimpleAnimationArrowScale);
    arrowX->setPosition(ball->position());
    arrowZ->setPosition(ball->position());
}


void Animation::animateAdjustedGradient(){
    if (adjustedArrowX == nullptr)
        adjustedArrowX = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(-1, 0, 0), Qt::black));
    if (adjustedArrowZ == nullptr)
        adjustedArrowZ = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(0, 0, -1), Qt::black));
    adjustedArrowX->setMagnitude(-descent->delta().x / descent->learning_rate * kSimpleAnimationArrowScale);
    adjustedArrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * kSimpleAnimationArrowScale);
    adjustedArrowX->setPosition(ball->position());
    adjustedArrowZ->setPosition(ball->position());
}


void Animation::animateMomentum(){
    if (momentumArrowX == nullptr)
        momentumArrowX = std::unique_ptr<Arrow>(
                    new Arrow(m_graph, QVector3D(-1, 0, 0), kMomentumColor));
    if (momentumArrowZ == nullptr)
        momentumArrowZ = std::unique_ptr<Arrow>(
                    new Arrow(m_graph, QVector3D(0, 0, -1), kMomentumColor));
    momentumArrowX->setMagnitude(momentum().x * kSimpleAnimationArrowScale);
    momentumArrowZ->setMagnitude(momentum().z * kSimpleAnimationArrowScale);
    momentumArrowX->setPosition(ball->position());
    momentumArrowZ->setPosition(ball->position());
}


void Animation::animateGradientSquared(){
    if (squareX == nullptr)
        squareX = std::unique_ptr<Square>(new Square(m_graph, "x"));
    if (squareZ == nullptr)
        squareZ = std::unique_ptr<Square>(new Square(m_graph, "z"));

    squareX->setArea(gradSumOfSquared().x * pow(kSimpleAnimationArrowScale, 2),
                     signbit(descent->gradX()));
    squareZ->setArea(gradSumOfSquared().z * pow(kSimpleAnimationArrowScale, 2),
                     signbit(descent->gradZ()));
    squareX->setPosition(ball->position());
    squareZ->setPosition(ball->position());
}


void Animation::cleanupGradient(){
    arrowX = nullptr;
    arrowZ = nullptr;
}


void Animation::cleanupAdjustedGradient(){
    adjustedArrowX = nullptr;
    adjustedArrowZ = nullptr;
}


void Animation::cleanupMomentum(){
    momentumArrowX = nullptr;
    momentumArrowZ = nullptr;
}


void Animation::cleanupGradientSquared(){
    squareX = nullptr;
    squareZ = nullptr;
}


void Animation::cleanupPath(){
    path->setVisible(false);
}


void Animation::cleanupAllButPath(){
    ball->setVisible(false);
    if (temporary_ball != nullptr) temporary_ball = nullptr;
    if (total_arrow != nullptr) total_arrow = nullptr;
    cleanupGradient();
    cleanupAdjustedGradient();
    cleanupMomentum();
    cleanupGradientSquared();
    detailed_animation_prepared = false;
}


void Animation::cleanupAll(){
    cleanupAllButPath();
    cleanupPath();
}

void Animation::initializeMomentumArrows(){
    momentumArrowX = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(-1, 0, 0), kMomentumColor));
    momentumArrowX->setMagnitude(0);

    momentumArrowZ = std::unique_ptr<Arrow>(
                new Arrow(m_graph, QVector3D(0, 0, -1), kMomentumColor));
    momentumArrowZ->setMagnitude(0);
}


void Animation::initializeSquares(){
    squareX = std::unique_ptr<Square>(new Square(m_graph, "x"));
    squareX->setArea(0);
    squareX->setVisible(false);

    squareZ = std::unique_ptr<Square>(new Square(m_graph, "z"));
    squareZ->setArea(0);
    squareZ->setVisible(false);
}


QString Animation::triggerDetailedAnimation(int animation_speedup){
    if (!detailed_animation_prepared){
        prepareDetailedAnimation();
    }
    QString message = animateStep();
    if (!in_initial_state)
        timer->setInterval(interval() / animation_speedup);
    state = (state + 1) % num_states;
    return in_initial_state ? "" :message;
}


void Animation::prepareDetailedAnimation(){
    timer->stop();
    ball->setVisible(true);

    arrowX = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(-1, 0, 0), kGradientColor));
    arrowX->setMagnitude(0);
    arrowX->setVisible(false);

    arrowZ = std::unique_ptr<Arrow>(new Arrow(m_graph, QVector3D(0, 0, -1), kGradientColor));
    arrowZ->setMagnitude(0);
    arrowZ->setVisible(false);

    total_arrow = std::unique_ptr<Arrow>(new Arrow(m_graph));
    total_arrow->setVisible(false);
    QColor color = ball_color;
    color.setAlpha(100);
    temporary_ball = std::unique_ptr<Ball>(new Ball(m_graph, color, f));

    if (has_momentum) initializeMomentumArrows();
    if (has_gradient_squared) initializeSquares();
    detailed_animation_prepared = true;
    timer->start(15);
}


void Animation::resetAnimation(){
    descent->resetPositionAndComputeGradient();
    state = 0;
    ball->setPositionOnSurface(descent->position());
    ball->setVisible(m_visible);
    if (path != nullptr) path->erase();
    else path = std::unique_ptr<Line>(new Line(m_graph, ball_color, f));
    path->addPoint(descent->position());
    in_initial_state = true;
    detailed_animation_prepared = false;

}


QString GradientDescentAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball
    {
        in_initial_state = false;
        temporary_ball->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        total_arrow->setVisible(false);
        ball->setPositionOnSurface(descent->position());
        break;
    }
    case 1: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * kSimpleAnimationArrowScale);
        arrowZ->setMagnitude(grad.z * kSimpleAnimationArrowScale);
        for (Arrow* arrow : {arrowX.get(), arrowZ.get()})
        {
            arrow->setPosition(ball->position());
            arrow->setVisible(true);
        }
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 2: // show the composite of gradients
    {
        descent->takeGradientStep();
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate * kSimpleAnimationArrowScale);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);
        return "The black arrow shows the total gradient.";
    }
    case 3: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
    }
    }
    return "";
}


QString MomentumAnimation::animateStep(){
    switch(state){
    case 0: // the ball and momentum arrows
    {
        ball->setPositionOnSurface(descent->position());

        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        return "Magenta arrows show momentum in x and y directions.";
    }
    case 1: // decay the momentum
    {
        float decay_rate = dynamic_cast<Momentum*> (descent.get()) ->decay_rate;
        momentumArrowX->setMagnitude(momentumArrowX->magnitude() * decay_rate);
        momentumArrowZ->setMagnitude(momentumArrowZ->magnitude() * decay_rate);

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);

        return QString("Momentum decays by x %1.").arg(decay_rate);
    }
    case 2: // show the x and z direction gradients
    {
        in_initial_state = false;
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * kSimpleAnimationArrowScale);
        arrowZ->setMagnitude(grad.z * kSimpleAnimationArrowScale);
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

        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 3: // add the gradient to the momentum
    {
        descent->takeGradientStep();
        Point delta = descent->delta();
        momentumArrowX->setMagnitude(-delta.x / descent->learning_rate * kSimpleAnimationArrowScale);
        momentumArrowZ->setMagnitude(-delta.z / descent->learning_rate * kSimpleAnimationArrowScale);

        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        return "Add the gradient (cyan) onto the momentum (magenta).";
    }
    case 4: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(
                    QVector3D(delta.x, 0, delta.z) / descent->learning_rate * kSimpleAnimationArrowScale);
        total_arrow->setPosition(ball->position());

        total_arrow->setVisible(true);
        return "The black arrow shows the total adjusted gradient (composite of the x & y momentum).";
    }

    case 5: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
    }
    }
    return "";
}


QString QHMAnimation::animateStep()
{
    switch ( state ) {
    case 0: // the ball and momentum arrows
    {
        ball->setPositionOnSurface( descent->position() );

        momentumArrowX->setPosition( ball->position() );
        momentumArrowZ->setPosition( ball->position() );

        temporary_ball->setVisible( false );
        total_arrow->setVisible( false );
        return "Magenta arrows show momentum in x and y directions.";
    }
    case 1: // decay the momentum
    {
        float decay_rate
                = dynamic_cast<QHM *>( descent.get() )->decay_rate;
        momentumArrowX->setMagnitude(
                momentumArrowX->magnitude() * decay_rate );
        momentumArrowZ->setMagnitude(
                momentumArrowZ->magnitude() * decay_rate );

        temporary_ball->setVisible( false );
        total_arrow->setVisible( false );

        return QString( "Momentum decays by x %1." ).arg( decay_rate );
    }
    case 2: // show the x and z direction gradients
    {
        in_initial_state = false;
        Point grad( descent->gradX(), descent->gradZ() );
        arrowX->setMagnitude( grad.x * kSimpleAnimationArrowScale );
        arrowZ->setMagnitude( grad.z * kSimpleAnimationArrowScale );
        // if in the same direction, then start the arrow at the tip of the
        // momentum arrow
        if ( momentumArrowX->magnitude() * grad.x > 0 ) {
            arrowX->setPosition(
                    ball->position()
                    + momentumArrowX->renderedVectorInPlotUnit() );
        } else {
            arrowX->setPosition( ball->position() );
        }

        if ( momentumArrowZ->magnitude() * grad.z > 0 ) {
            arrowZ->setPosition(
                    ball->position()
                    + momentumArrowZ->renderedVectorInPlotUnit() );
        } else {
            arrowZ->setPosition( ball->position() );
        }

        arrowX->setVisible( true );
        arrowZ->setVisible( true );
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 3: // add the gradient to the momentum
    {
        descent->takeGradientStep();
        Point delta = descent->delta();
        momentumArrowX->setMagnitude(
                -delta.x / descent->learning_rate
                * kSimpleAnimationArrowScale );
        momentumArrowZ->setMagnitude(
                -delta.z / descent->learning_rate
                * kSimpleAnimationArrowScale );

        arrowX->setVisible( false );
        arrowZ->setVisible( false );
        return "Add the gradient (cyan) onto the momentum (magenta).";
    }
    case 4: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(
                QVector3D( delta.x, 0, delta.z ) / descent->learning_rate
                * kSimpleAnimationArrowScale );
        total_arrow->setPosition( ball->position() );

        total_arrow->setVisible( true );
        return "The black arrow shows the total adjusted gradient (composite of the x & y momentum).";
    }

    case 5: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition( QVector3D(
                descent->position().x, ball->position().y(),
                descent->position().z ) );

        temporary_ball->setVisible( true );
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
    }
    }
    return "";
}


QString AdaGradAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball and sum of squares
    {
        ball->setPositionOnSurface(descent->position());

        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        return "The squares represent the accumulated sum of gradient^2.";
    }
    case 1: // show the x and z direction gradients
    {
        in_initial_state = false;

        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * arrowScale);
        arrowZ->setMagnitude(grad.z * arrowScale);
        arrowX->setPosition(ball->position());
        arrowZ->setPosition(ball->position());

        arrowX->setVisible(true);
        arrowZ->setVisible(true);

        return "The cyan arrows show gradients in x and y directions.";
    }
    case 2: // show sum of squares updating
    {
        descent->takeGradientStep();
        squareX->setArea(dynamic_cast<AdaGrad*> (descent.get())->gradSumOfSquared().x,
                         signbit(descent->gradX()));
        squareZ->setArea(dynamic_cast<AdaGrad*> (descent.get())->gradSumOfSquared().z,
                         signbit(descent->gradZ()));
        squareX->setVisible(true);
        squareZ->setVisible(true);
        return "Square the gradient and add on to the square in each direction.";
    }
    case 3: // show delta arrows shrink wrt gradient arrows
    {
        arrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        arrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        return "Divide the gradient by the length of the side of the square in each direction.";
    }
    case 4: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate * arrowScale);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);

        return "The black arrow shows the total adjusted gradient.";
    }
    case 5: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
    }
    }
    return "";
}


QString RMSPropAnimation::animateStep(){
    switch(state){
    case 0: // just show the ball and sum of squares
    {
        ball->setPositionOnSurface(descent->position());

        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        return "The squares represent the decayed sum of gradient^2.";
    }
    case 1: // show the x and z direction gradients
    {
        Point grad(descent->gradX(), descent->gradZ());
        arrowX->setMagnitude(grad.x * arrowScale);
        arrowZ->setMagnitude(grad.z * arrowScale);
        arrowX->setPosition(ball->position());
        arrowZ->setPosition(ball->position());

        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 2: // show sum of squares decaying
    {
        float decay_rate =  dynamic_cast<RMSProp*> (descent.get())->decay_rate;
        squareX->setArea(squareX->area() * decay_rate, signbit(descent->gradX()));
        squareZ->setArea(squareZ->area() * decay_rate, signbit(descent->gradZ()));
        squareX->setVisible(true);
        squareZ->setVisible(true);
        return QString("Shrink the squares by x %1").arg(decay_rate);
    }
    case 3:
    {
        in_initial_state = false;
        descent->takeGradientStep();
        squareX->setArea(dynamic_cast<RMSProp*> (descent.get())->decayedGradSumOfSquared().x,
                         signbit(descent->gradX()));
        squareZ->setArea(dynamic_cast<RMSProp*> (descent.get())->decayedGradSumOfSquared().z,
                         signbit(descent->gradZ()));
        return QString("Squares grow by %1 x gradient^2").arg(
                    1-dynamic_cast<RMSProp*> (descent.get())->decay_rate);
    }
    case 4: // show delta arrows shrink wrt gradient arrows
    {
        arrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        arrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        return "Divide the gradient by the length of the side of the square in each direction.";
    }
    case 5: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) /
                               descent->learning_rate * arrowScale);
        total_arrow->setPosition(ball->position());
        total_arrow->setVisible(true);
        return "The black arrow shows the total adjusted gradient.";
    }
    case 6: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
        }
    }
    return "";
}


QString AdamAnimation::animateStep(){
    switch(state){
    case 0: // the ball and momentum arrows
    {
        ball->setPositionOnSurface(descent->position());

        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());
        squareX->setPosition(ball->position());
        squareZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        return "Arrows are momentum; squares are decayed sum of gradient^2.";
    }
    case 1: // decay the momentum
    {
        ball->setPositionOnSurface(descent->position());
        float beta1 = dynamic_cast<Adam*> (descent.get()) ->beta1;
        momentumArrowX->setMagnitude(momentumArrowX->magnitude() * beta1);
        momentumArrowZ->setMagnitude(momentumArrowZ->magnitude() * beta1);
        momentumArrowX->setPosition(ball->position());
        momentumArrowZ->setPosition(ball->position());

        temporary_ball->setVisible(false);
        total_arrow->setVisible(false);
        return QString("Decay the momentum by beta1 (%1).").arg(beta1);
    }
    case 2: // show sum of squares decaying
    {
        float beta2 = dynamic_cast<Adam*> (descent.get())->beta2;
        squareX->setArea(squareX->area() * beta2, signbit(descent->gradX()));
        squareZ->setArea(squareZ->area() * beta2, signbit(descent->gradZ()));

        squareX->setVisible(true);
        squareZ->setVisible(true);

        return QString("Decay the squares by beta2 (%1).").arg(beta2);
    }
    case 3: // show the x and z direction gradients
    {
        in_initial_state = false;
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

        arrowX->setVisible(true);
        arrowZ->setVisible(true);
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 4: // update momentum
    {
        descent->takeGradientStep();
        momentumArrowX->setMagnitude(dynamic_cast<Adam*> (descent.get())->decayedGradSum().x);
        momentumArrowZ->setMagnitude(dynamic_cast<Adam*> (descent.get())->decayedGradSum().z);

        arrowX->setVisible(false);
        arrowZ->setVisible(false);
        return "Momentums grow by (1 - beta1) x gradient.";
    }
    case 5: // update sum of squares
    {
        squareX->setArea(dynamic_cast<Adam*> (descent.get())->decayedGradSumOfSquared().x,
                         signbit(descent->gradX()));
        squareZ->setArea(dynamic_cast<Adam*> (descent.get())->decayedGradSumOfSquared().z,
                         signbit(descent->gradZ()));
        return "Squares grow by (1 - beta2) x gradient^2.";
    }
    case 6: // show delta arrows shrink wrt gradient arrows
    {
        momentumArrowX->setMagnitude(-descent->delta().x / descent->learning_rate * arrowScale);
        momentumArrowZ->setMagnitude(-descent->delta().z / descent->learning_rate * arrowScale);
        return "Divide the momentum by the length of the side of the square in each direction.";
    }
    case 7: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(QVector3D(delta.x, 0, delta.z) / descent->learning_rate);
        total_arrow->setPosition(ball->position());

        total_arrow->setVisible(true);
        return "The black arrow shows the total adjusted gradient.";
    }
    case 8: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition(QVector3D(
                                        descent->position().x,
                                        ball->position().y(),
                                        descent->position().z));

        temporary_ball->setVisible(true);
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
        }
    }
    return "";
}


QString QHAdamAnimation::animateStep()
{
    switch ( state ) {
    case 0: // the ball and momentum arrows
    {
        ball->setPositionOnSurface( descent->position() );

        momentumArrowX->setPosition( ball->position() );
        momentumArrowZ->setPosition( ball->position() );
        squareX->setPosition( ball->position() );
        squareZ->setPosition( ball->position() );

        temporary_ball->setVisible( false );
        total_arrow->setVisible( false );
        arrowX->setVisible( false );
        arrowZ->setVisible( false );
        return "Arrows are momentum; squares are decayed sum of gradient^2.";
    }
    case 1: // decay the momentum
    {
        ball->setPositionOnSurface( descent->position() );
        float beta1 = dynamic_cast<QHAdam *>( descent.get() )->beta1;
        momentumArrowX->setMagnitude( momentumArrowX->magnitude() * beta1 );
        momentumArrowZ->setMagnitude( momentumArrowZ->magnitude() * beta1 );
        momentumArrowX->setPosition( ball->position() );
        momentumArrowZ->setPosition( ball->position() );

        temporary_ball->setVisible( false );
        total_arrow->setVisible( false );
        return QString( "Decay the momentum by beta1 (%1)." ).arg( beta1 );
    }
    case 2: // show sum of squares decaying
    {
        float beta2 = dynamic_cast<QHAdam *>( descent.get() )->beta2;
        squareX->setArea(
                squareX->area() * beta2, signbit( descent->gradX() ) );
        squareZ->setArea(
                squareZ->area() * beta2, signbit( descent->gradZ() ) );

        squareX->setVisible( true );
        squareZ->setVisible( true );

        return QString( "Decay the squares by beta2 (%1)." ).arg( beta2 );
    }
    case 3: // show the x and z direction gradients
    {
        in_initial_state = false;
        Point grad( descent->gradX(), descent->gradZ() );
        arrowX->setMagnitude( grad.x );
        arrowZ->setMagnitude( grad.z );
        // if in the same direction, then start the arrow at the tip of the
        // momentum arrow
        if ( momentumArrowX->magnitude() * grad.x > 0 ) {
            arrowX->setPosition(
                    ball->position()
                    + momentumArrowX->renderedVectorInPlotUnit() );
        } else {
            arrowX->setPosition( ball->position() );
        }

        if ( momentumArrowZ->magnitude() * grad.z > 0 ) {
            arrowZ->setPosition(
                    ball->position()
                    + momentumArrowZ->renderedVectorInPlotUnit() );
        } else {
            arrowZ->setPosition( ball->position() );
        }

        arrowX->setVisible( true );
        arrowZ->setVisible( true );
        return "The cyan arrows show gradients in x and y directions.";
    }
    case 4: // update momentum
    {
        descent->takeGradientStep();
        momentumArrowX->setMagnitude(
                dynamic_cast<QHAdam *>( descent.get() )->decayedGradSum().x );
        momentumArrowZ->setMagnitude(
                dynamic_cast<QHAdam *>( descent.get() )->decayedGradSum().z );

        arrowX->setVisible( false );
        arrowZ->setVisible( false );
        return "Momentums grow by (1 - beta1) x gradient.";
    }
    case 5: // update sum of squares
    {
        squareX->setArea(
                dynamic_cast<QHAdam *>( descent.get() )
                        ->decayedGradSumOfSquared()
                        .x,
                signbit( descent->gradX() ) );
        squareZ->setArea(
                dynamic_cast<QHAdam *>( descent.get() )
                        ->decayedGradSumOfSquared()
                        .z,
                signbit( descent->gradZ() ) );
        return "Squares grow by (1 - beta2) x gradient^2.";
    }
    case 6: // show delta arrows shrink wrt gradient arrows
    {
        momentumArrowX->setMagnitude(
                -descent->delta().x / descent->learning_rate * arrowScale );
        momentumArrowZ->setMagnitude(
                -descent->delta().z / descent->learning_rate * arrowScale );
        return "Divide the momentum by the length of the side of the square in each direction.";
    }
    case 7: // show the composite of gradients
    {
        Point delta = descent->delta();
        total_arrow->setVector(
                QVector3D( delta.x, 0, delta.z ) / descent->learning_rate );
        total_arrow->setPosition( ball->position() );

        total_arrow->setVisible( true );
        return "The black arrow shows the total adjusted gradient.";
    }
    case 8: // draw an imaginary ball of the future position
    {
        temporary_ball->setPosition( QVector3D(
                descent->position().x, ball->position().y(),
                descent->position().z ) );

        temporary_ball->setVisible( true );
        return "The ball takes a step in the direction of the black arrow, scaled by the learning rate.";
    }
    }
    return "";
}