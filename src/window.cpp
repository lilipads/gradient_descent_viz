#include <math.h>

#include <QtWidgets>

#include "window.h"


Window::Window(QWidget *parent)
 : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Gradient Descent Visualization"));

    Q3DSurface *graph = new Q3DSurface();
    plot_area = new PlotArea(graph);
    QWidget *graph_container = QWidget::createWindowContainer(graph);

    QSize screenSize = graph->screen()->size();
    graph_container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.5));
    graph_container->setMaximumSize(screenSize);
    graph_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph_container->setFocusPolicy(Qt::StrongFocus);


    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QVBoxLayout *vLayoutLeft = new QVBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();

    // things on the left
    hLayout->addLayout(vLayoutLeft);
    vLayoutLeft->addWidget(graph_container, 1);
    vLayoutLeft->addWidget(createControlGroup());
    hLayout->addLayout(vLayout);

    // things on the right
    vLayout->addWidget(createFunctionSelector());
    vLayout->addWidget(createViewTabs());
    // widgets to tune gradient parameters
    vLayout->addWidget(createGradientDescentGroup());
    vLayout->addWidget(createMomentumGroup());
    vLayout->addWidget(createQHMGroup());
    vLayout->addWidget(createAdaGradGroup());
    vLayout->addWidget(createRMSPropGroup());
    vLayout->addWidget(createAdamGroup());
    vLayout->addWidget( createQHAdamGroup(), 1, Qt::AlignTop );

    setupKeyboardShortcuts();
}


void Window::setupKeyboardShortcuts(){
    QShortcut* left = new QShortcut(Qt::Key_Left, this);
    QObject::connect(left, &QShortcut::activated, [=](){plot_area->moveCamera(-1, 0);});
    QShortcut* right = new QShortcut(Qt::Key_Right, this);
    QObject::connect(right, &QShortcut::activated, [=](){plot_area->moveCamera(1, 0);});
    QShortcut* up = new QShortcut(Qt::Key_Up, this);
    QObject::connect(up, &QShortcut::activated, [=](){plot_area->moveCamera(0, 1);});
    QShortcut* down = new QShortcut(Qt::Key_Down, this);
    QObject::connect(down, &QShortcut::activated, [=](){plot_area->moveCamera(0, -1);});
    QShortcut* zoomin = new QShortcut(Qt::CTRL + Qt::Key_Equal, this);
    QObject::connect(zoomin, &QShortcut::activated, plot_area, &PlotArea::cameraZoomIn);
    QShortcut* zoomout = new QShortcut(Qt::CTRL + Qt::Key_Minus, this);
    QObject::connect(zoomout, &QShortcut::activated, plot_area, &PlotArea::cameraZoomOut);
}


QGroupBox *Window::createControlGroup(){
    QGroupBox *groupBox = new QGroupBox();
    QHBoxLayout *layout= new QHBoxLayout;
    groupBox->setLayout(layout);

    layout->addWidget(createToggleAnimationButton());
    layout->addWidget(createRestartAnimationButton());
    layout->addWidget(new QLabel(QStringLiteral("Playback speed:")));
    layout->addWidget(createPlaybackSpeedBox());
    layout->addWidget(createZoomButton(1));
    layout->addWidget(createZoomButton(0));

    layout->setAlignment(Qt::AlignHCenter);
    groupBox->setFocusPolicy(Qt::NoFocus);
    return groupBox;
}


QPushButton *Window::createZoomButton(int is_zoomout){
    // is_zoomout: 1 for zoomout; 0 for zoom in.
    QPushButton* zoom = new QPushButton();
    if (is_zoomout)
        zoom->setIcon(QIcon(QPixmap(":/icons/zoomout.png")));
    else
        zoom->setIcon(QIcon(QPixmap(":/icons/zoomin.png")));
    zoom->setIconSize(QSize(20, 20));
    zoom->setFlat(true);
    zoom->setStyleSheet("QPushButton {background-color:transparent; border-radius: 2px;"
                        "padding-left: 2px; padding-right: 2px;}");
    zoom->setCursor(Qt::PointingHandCursor);
    auto slot = is_zoomout? &PlotArea::cameraZoomOut : &PlotArea::cameraZoomIn;
    QObject::connect(zoom, &QPushButton::clicked, plot_area, slot);
    return zoom;
}


QPushButton *Window::createToggleAnimationButton(){
    // toggle animation button
    QPushButton *toggleAnimationButton = new QPushButton(this);

    toggleAnimationButton->setCheckable(true);
    toggleAnimationButton->setChecked(true);
    toggleAnimationButton->setText(QString(" Pause "));

    QObject::connect(toggleAnimationButton, &QPushButton::toggled,
        [=](bool is_checked){
            if (is_checked){
                toggleAnimationButton->setText(QString(" Pause "));
                plot_area->playAnimation();
            }
            else{
                toggleAnimationButton->setText(QString("Paused"));
                plot_area->pauseAnimation();
            }
        });

    return toggleAnimationButton;
}


QPushButton *Window::createRestartAnimationButton(){
    // restart animation button
    QPushButton *restartAnimationButton = new QPushButton(this);
    restartAnimationButton->setText(QStringLiteral("Restart"));
    QObject::connect(restartAnimationButton, &QPushButton::clicked, plot_area,
                     &PlotArea::resetAnimations);
    return restartAnimationButton;
}


QComboBox *Window::createPlaybackSpeedBox(){
    QComboBox *box = new QComboBox(this);
    box->addItem("0.1x");
    box->addItem("0.2x");
    box->addItem("1x");
    box->addItem("5x");
    box->addItem("10x");
    box->setCurrentIndex(2);

    QObject::connect(box, SIGNAL(currentIndexChanged(int)),
                     plot_area, SLOT(setAnimationSpeed(int)));
    return box;
}


QComboBox *Window::createFunctionSelector(){
    QComboBox *box = new QComboBox(this);
    box->addItem("--Choose a surface--");
    box->addItem("Local Minimum");
    box->addItem("Global Minimum");
    box->addItem("Saddle Point");
    box->addItem("Ecliptic Bowl");
    box->addItem("Hills");
    box->addItem("Plateau");

    QObject::connect(box, SIGNAL(currentIndexChanged(QString)),
                     plot_area, SLOT(changeSurface(QString)));
    return box;
}


QGroupBox *Window::createDescentGroup(Animation* animation,
                                      QFormLayout* layout){
    QGroupBox *groupBox = new QGroupBox(animation->name);
    groupBox->setCheckable(true);
    groupBox->setChecked(true);

    QObject::connect(groupBox, &QGroupBox::clicked,
                     [=](const bool& visible){animation->setVisible(visible);});

    groupBox->setStyleSheet(QString("QGroupBox::title {font: 10pt; border-radius: 5px; background: %1;}"
                                    ).arg(animation->ball_color.name()));

    groupBox->setLayout(layout);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setLabelAlignment(Qt::AlignLeft);
    return groupBox;
}


QGroupBox *Window::createGradientDescentGroup(){
    VanillaGradientDescent* descent = dynamic_cast<VanillaGradientDescent*> (
                plot_area->gradient_descent->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));

    return createDescentGroup(plot_area->gradient_descent.get(), form);
}


QGroupBox *Window::createMomentumGroup(){
    Momentum* descent = dynamic_cast<Momentum*> (plot_area->momentum->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));
    form->addRow(new QLabel(QStringLiteral("Decay rate:")),
                 createDecayBox(descent->decay_rate));

    return createDescentGroup(plot_area->momentum.get(), form);
}

QGroupBox *Window::createQHMGroup()
{
    QHM *descent = dynamic_cast<QHM *>( plot_area->qhm->descent.get() );

    QFormLayout *form = new QFormLayout;
    form->addRow(
            new QLabel( QStringLiteral( "Learning Rate:" ) ),
            createLearningRateBox( descent ) );
    form->addRow(
            new QLabel( QStringLiteral( "Decay rate:" ) ),
            createDecayBox( descent->decay_rate ) );
    form->addRow(
            new QLabel( QStringLiteral( "Discount factor:" ) ),
            createDecayBox( descent->discount_factor ) );

    return createDescentGroup( plot_area->qhm.get(), form );
}

QGroupBox *Window::createAdaGradGroup(){
    AdaGrad* descent = dynamic_cast<AdaGrad*> (plot_area->ada_grad->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));

    return createDescentGroup(plot_area->ada_grad.get(), form);
}


QGroupBox *Window::createRMSPropGroup(){
    RMSProp* descent = dynamic_cast<RMSProp*> (plot_area->rms_prop->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));
    form->addRow(new QLabel(QStringLiteral("Decay rate:")),
                 createDecayBox(descent->decay_rate));

    return createDescentGroup(plot_area->rms_prop.get(), form);
}


QGroupBox *Window::createAdamGroup(){
    Adam* descent = dynamic_cast<Adam*> (plot_area->adam->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));
    form->addRow(new QLabel(QStringLiteral("Beta1:")),
                 createDecayBox(descent->beta1));
    form->addRow(new QLabel(QStringLiteral("Beta2:")),
                 createDecayBox(descent->beta2));

    QCheckBox *scaled = new QCheckBox( "Use Bias Correction" );
    auto &bias_val = descent->use_bias_correction;
    scaled->setChecked( bias_val );
    QObject::connect( scaled, &QCheckBox::clicked, [&]( bool clicked ) {
                bias_val = clicked;
            } );
    form->addRow( scaled );

    return createDescentGroup(plot_area->adam.get(), form);
}


QGroupBox *Window::createQHAdamGroup()
{
    QHAdam *descent = dynamic_cast<QHAdam *>( plot_area->qhadam->descent.get() );

    QFormLayout *form = new QFormLayout;
    form->addRow(
            new QLabel( QStringLiteral( "Learning Rate:" ) ),
            createLearningRateBox( descent ) );
    form->addRow(
            new QLabel( QStringLiteral( "Beta1:" ) ),
            createDecayBox( descent->beta1 ) );
    form->addRow(
            new QLabel( QStringLiteral( "Beta2:" ) ),
            createDecayBox( descent->beta2 ) );
    form->addRow(
            new QLabel( QStringLiteral( "Discount Factor:" ) ),
            createDecayBox( descent->discount_factor ) );
    form->addRow(
            new QLabel( QStringLiteral( "Sq Discount Factor:" ) ),
            createDecayBox( descent->squared_discount_factor ) );

    QCheckBox *scaled = new QCheckBox( "Use Bias Correction" );
    auto &bias_val = descent->use_bias_correction;
    scaled->setChecked( bias_val );
    QObject::connect( scaled, &QCheckBox::clicked, [&]( bool clicked ) {
                bias_val = clicked;
            } );
    form->addRow( scaled );

    return createDescentGroup( plot_area->qhadam.get(), form );
}

QLayout *Window::createLearningRateBox(GradientDescent* descent){
    // learning rate spin box
    QFormLayout *hbox = new QFormLayout;

    QSpinBox *learningRateBox = new QSpinBox(this);
    learningRateBox->setRange(-10, 10);
    double x = log(descent->learning_rate) / log(10.);
    learningRateBox->setValue(nearbyint(x));
    QObject::connect(learningRateBox,
        QOverload<int>::of(&QSpinBox::valueChanged),
        [=](const int &newValue) {
            descent->learning_rate = pow(10, newValue);
        });

    hbox->addRow(new QLabel(QStringLiteral("1e")), learningRateBox);
    hbox->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    return hbox;
}


QDoubleSpinBox *Window::createDecayBox(double& val){
    QDoubleSpinBox *decayRateBox = new QDoubleSpinBox(this);
    decayRateBox->setDecimals(3);
    decayRateBox->setRange(0.0, 2.0);
    decayRateBox->setValue(val);
    decayRateBox->setSingleStep(0.1);
    QObject::connect(decayRateBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [&](const double &newValue ) {val = newValue;});
    return decayRateBox;
}

QTabWidget *Window::createViewTabs(){
    QTabWidget* tab = new QTabWidget;
    // TODO: say it's scaled down
    QCheckBox* gradient = new QCheckBox("Gradient Arrows");
    gradient->setToolTip("Gradient at that point on the surface (only depends on position;\n"
                         "independent of the gradient descent method");
    QObject::connect(gradient, &QCheckBox::clicked, plot_area, &PlotArea::setShowGradient);
    QCheckBox* adjusted_gradient = new QCheckBox("Adjusted Gradient Arrows");
    adjusted_gradient->setToolTip("Some methods adjust the gradient by adding on momentum or\n"
                                  "dividing by some numbers (e.g. sum of squares). The adjusted\n"
                                  "gradient shows the calculation result of each gradient method\n"
                                  "for a gradient step.");
    QObject::connect(adjusted_gradient, &QCheckBox::clicked, plot_area, &PlotArea::setShowAdjustedGradient);
    QCheckBox* momentum = new QCheckBox("Momentum Arrows");
    momentum->setToolTip("Momentum is some form of decayed sum of gradients.\n"
                         "Momentum Descent and Adam Descent use momentum.");
    QObject::connect(momentum, &QCheckBox::clicked, plot_area, &PlotArea::setShowMomentum);
    QCheckBox* squaredGrad = new QCheckBox("Sum of Gradient Squared");
    squaredGrad->setToolTip("AdaGrad sums up the square of gradients in each direction across\n"
                            "all steps. RMSProp and Adam do the summing with some decay factor.\n"
                            "The area of the squares represent the magnitude of the sum.");
    QObject::connect(squaredGrad, &QCheckBox::clicked, plot_area, &PlotArea::setShowGradientSquared);
    QCheckBox* path = new QCheckBox("Path");
    QObject::connect(path, &QCheckBox::clicked, plot_area, &PlotArea::setShowPath);


    QWidget* overview_tab = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout;
    overview_tab->setLayout(vbox);
    vbox->addWidget(gradient);
    vbox->addWidget(adjusted_gradient);
    vbox->addWidget(momentum);
    vbox->addWidget(squaredGrad);
    vbox->addWidget(path);
    tab->addTab(overview_tab, "Overview");

    QComboBox* descentPicker = new QComboBox;
    descentPicker->addItem("Choose a method");
    for (auto animation : plot_area->all_animations)
        descentPicker->addItem(animation->name);
    QObject::connect(descentPicker, SIGNAL(currentIndexChanged(QString)),
                     plot_area, SLOT(setDetailedAnimation(QString)));

    QLabel* messageBox = new QLabel;
    messageBox->setWordWrap(true);
    messageBox->setMinimumHeight(70);
    messageBox->setAlignment(Qt::AlignTop);
    QObject::connect(plot_area, &PlotArea::updateMessage,
                     messageBox,&QLabel::setText);

    QWidget* step_by_step_tab = new QWidget();
    QVBoxLayout* vbox2 = new QVBoxLayout;
    step_by_step_tab->setLayout(vbox2);
    vbox2->addWidget(descentPicker);
    vbox2->addWidget(messageBox);
    tab->addTab(step_by_step_tab, "Step-by-Step");


    QObject::connect(tab, &QTabWidget::currentChanged,
                     plot_area, &PlotArea::setAnimationMode);
    // when switching to overview, clear prevoius selection and reset to "Choose your method"
    QObject::connect(tab, &QTabWidget::currentChanged,
                     [=](int idx){if (idx == 1) {
            descentPicker->setCurrentIndex(0);
            messageBox->setText("");
        }});
    return tab;
}
