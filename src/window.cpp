#include <math.h>

#include <QtWidgets>

#include "learning_rate_schedule.h"
#include "window.h"


Window::Window(QWidget *parent)
 : QWidget(parent),
   graph_container(nullptr),
   control_group(nullptr),
   settings_scroll_area(nullptr),
   workspace_tabs(nullptr)
{
    setWindowTitle(QStringLiteral("Gradient Descent Visualization"));
    setMinimumSize(560, 420);

    Q3DSurface *graph = new Q3DSurface();
    plot_area = new PlotArea(graph);
    graph_container = QWidget::createWindowContainer(graph);
    graph_container->setMinimumSize(280, 210);
    graph_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph_container->setFocusPolicy(Qt::StrongFocus);

    QWidget *visualization_panel = new QWidget;
    QVBoxLayout *visualization_layout = new QVBoxLayout(visualization_panel);
    visualization_layout->setContentsMargins(0, 0, 0, 0);
    visualization_layout->addWidget(graph_container, 1);
    control_group = createControlGroup();
    visualization_layout->addWidget(control_group);

    QWidget *settings = new QWidget;
    QVBoxLayout *settings_layout = new QVBoxLayout(settings);
    settings_layout->addWidget(createFunctionSelector());
    settings_layout->addWidget(createViewTabs());
    settings_layout->addWidget(createGradientDescentGroup());
    settings_layout->addWidget(createMomentumGroup());
    settings_layout->addWidget(createQHMGroup());
    settings_layout->addWidget(createAdaGradGroup());
    settings_layout->addWidget(createRMSPropGroup());
    settings_layout->addWidget(createAdamGroup());
    settings_layout->addWidget(createAdamWGroup());
    settings_layout->addWidget(createQHAdamGroup());
    settings_layout->addStretch();

    settings_scroll_area = new QScrollArea;
    settings_scroll_area->setWidget(settings);
    settings_scroll_area->setWidgetResizable(true);
    settings_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    settings_scroll_area->setFrameShape(QFrame::NoFrame);
    settings_scroll_area->setMinimumWidth(260);
    settings_scroll_area->setMaximumWidth(460);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(visualization_panel);
    splitter->addWidget(settings_scroll_area);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    workspace_tabs = new QTabWidget;
    workspace_tabs->addTab(splitter, QStringLiteral("Optimizer Visualization"));
    LearningRateScheduleWidget *schedule_workspace = new LearningRateScheduleWidget;
    workspace_tabs->addTab(schedule_workspace,
                           QStringLiteral("Learning Rate Schedules"));
    QObject::connect(workspace_tabs, &QTabWidget::currentChanged,
                     [schedule_workspace](int index) {
        schedule_workspace->setActive(index == 1);
    });
    schedule_workspace->setActive(false);

    QHBoxLayout *window_layout = new QHBoxLayout(this);
    window_layout->addWidget(workspace_tabs);

    const QRect available = graph->screen()->availableGeometry();
    resize(available.width() * 9 / 10, available.height() * 9 / 10);
    splitter->setSizes({available.width() * 3 / 4, available.width() / 4});

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
    QShortcut* zoomin = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal), this);
    QObject::connect(zoomin, &QShortcut::activated, plot_area, &PlotArea::cameraZoomIn);
    QShortcut* zoomout = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus), this);
    QObject::connect(zoomout, &QShortcut::activated, plot_area, &PlotArea::cameraZoomOut);

    QShortcut *fullscreen = new QShortcut(Qt::Key_F11, this);
    QObject::connect(fullscreen, &QShortcut::activated,
                     this, &Window::toggleVisualizationFullscreen);
    QShortcut *exit_fullscreen = new QShortcut(Qt::Key_Escape, this);
    QObject::connect(exit_fullscreen, &QShortcut::activated, [this]() {
        if (visualization_fullscreen)
            toggleVisualizationFullscreen();
    });
}


void Window::toggleVisualizationFullscreen(){
    visualization_fullscreen = !visualization_fullscreen;
    control_group->setVisible(!visualization_fullscreen);
    settings_scroll_area->setVisible(!visualization_fullscreen);
    workspace_tabs->tabBar()->setVisible(!visualization_fullscreen);

    if (visualization_fullscreen) {
        previous_window_state = windowState();
        showFullScreen();
        graph_container->setFocus();
    } else if (previous_window_state.testFlag(Qt::WindowMaximized)) {
        showMaximized();
    } else {
        showNormal();
    }
}


QGroupBox *Window::createControlGroup(){
    QGroupBox *groupBox = new QGroupBox();
    QGridLayout *layout = new QGridLayout;
    groupBox->setLayout(layout);

    layout->addWidget(createToggleAnimationButton(), 0, 0);
    layout->addWidget(createRestartAnimationButton(), 0, 1);
    layout->addWidget(createFullscreenButton(), 0, 2, 1, 2);
    layout->addWidget(new QLabel(QStringLiteral("Playback speed:")), 1, 0);
    layout->addWidget(createPlaybackSpeedBox(), 1, 1);
    layout->addWidget(createZoomButton(1), 1, 2);
    layout->addWidget(createZoomButton(0), 1, 3);

    layout->setAlignment(Qt::AlignHCenter);
    groupBox->setFocusPolicy(Qt::NoFocus);
    return groupBox;
}


QPushButton *Window::createFullscreenButton(){
    QPushButton *fullscreen = new QPushButton(QStringLiteral("Fullscreen"));
    fullscreen->setToolTip(QStringLiteral("Show only the visualization (F11)"));
    QObject::connect(fullscreen, &QPushButton::clicked,
                     this, &Window::toggleVisualizationFullscreen);
    return fullscreen;
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

    QObject::connect(box, &QComboBox::currentTextChanged,
                     plot_area, &PlotArea::changeSurface);
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


QGroupBox *Window::createAdamWGroup(){
    AdamW *descent = dynamic_cast<AdamW *>(plot_area->adamw->descent.get());

    QFormLayout *form = new QFormLayout;
    form->addRow(new QLabel(QStringLiteral("Learning Rate:")),
                 createLearningRateBox(descent));
    form->addRow(new QLabel(QStringLiteral("Beta1:")),
                 createDecayBox(descent->beta1));
    form->addRow(new QLabel(QStringLiteral("Beta2:")),
                 createDecayBox(descent->beta2));

    QDoubleSpinBox *weight_decay = new QDoubleSpinBox(this);
    weight_decay->setDecimals(4);
    weight_decay->setRange(0.0, 10.0);
    weight_decay->setSingleStep(0.01);
    weight_decay->setValue(descent->weight_decay);
    QObject::connect(weight_decay,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [descent](double value) { descent->weight_decay = value; });
    form->addRow(new QLabel(QStringLiteral("Weight Decay:")), weight_decay);

    QCheckBox *scaled = new QCheckBox("Use Bias Correction");
    scaled->setChecked(descent->use_bias_correction);
    QObject::connect(scaled, &QCheckBox::clicked, [descent](bool clicked) {
        descent->use_bias_correction = clicked;
    });
    form->addRow(scaled);

    return createDescentGroup(plot_area->adamw.get(), form);
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
    QObject::connect(descentPicker, &QComboBox::currentTextChanged,
                     plot_area, &PlotArea::setDetailedAnimation);

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
