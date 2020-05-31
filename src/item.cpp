#include <math.h>

#include "item.h"

void Item::setColor(QColor color){
    QImage pointColor = QImage(2, 2, QImage::Format_ARGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}


void Item::addToGraph(Q3DSurface *graph){
    if (m_graph == nullptr){
        m_graph = graph;
        m_graph->addCustomItem(this);
    }
}


QVector3D Item::plotScalingVector(){
    return QVector3D(m_graph->axisX()->max() - m_graph->axisX()->min(), 0,
                     m_graph->axisZ()->max() - m_graph->axisZ()->min());
}


Ball::Ball(Q3DSurface* graph, QColor color, double (*_f) (double, double))
    : f(_f)
{
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    setColor(color);
    addToGraph(graph);
}


void Ball::setPositionOnSurface(Point p){
    float yOffset =  (m_graph->axisY()->max() - m_graph->axisY()->min()) /
            kBallRadiusPerGraph;
    setPosition(QVector3D(p.x, f(p.x, p.z) + yOffset, p.z));
}

Arrow::Arrow(Q3DSurface* graph) : Item(graph){
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(Qt::black);
}


Arrow::Arrow(Q3DSurface* graph, QVector3D vector): Arrow(graph) {
    setVector(vector);
}


Arrow::Arrow(Q3DSurface* graph, QVector3D vector, QColor color) {
    m_graph = graph;
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(color);
    setVector(vector);
    m_graph->addCustomItem(this);
}


void Arrow::setVector(QVector3D vector){
    /* draw an arrow representing the vector (direction and magnitude) */

    direction = vector.normalized();
    // z times -1 because oddly, the default xyz axis in Q3DSurface has Z axis flipped
    QQuaternion rotation = QQuaternion::rotationTo(
                QVector3D(0, 1, 0), direction * QVector3D(1, 1, -1));
    setRotation(rotation);
    setMagnitude(vector.length());
}

void Arrow::setMagnitude(const float &magnitude){
    // if magnitude is negative, arrow extends in the other direction
    float unitPlotPerGraph = (direction * plotScalingVector()).length();

    m_magnitude = magnitude;

    float magnitude_in_unit_arrow = magnitude * kUnitItemPerGraph /
            unitPlotPerGraph * kItemScale;
    // if the arrow is really small, make sure its tip still extends outside of the ball
    float min_magnitude = kBallRadiusPerGraph / (0.1 * kUnitItemPerGraph) * 1.2;
    if (abs(magnitude_in_unit_arrow) < min_magnitude)
        magnitude_in_unit_arrow = min_magnitude * magnitude_in_unit_arrow / abs(magnitude_in_unit_arrow);
    setScaling(QVector3D(0.1, 0.1 * magnitude_in_unit_arrow, 0.1));
}


Square::Square(Q3DSurface* graph) : Item(){
    setMeshFile(QStringLiteral(":/mesh/plane.obj"));
    setScaling(QVector3D(0.1, 0.1, 0.1));
    QColor color = Qt::white;
    color.setAlpha(150);
    setColor(color);
    addToGraph(graph);
}


Square::Square(Q3DSurface* graph, QString direction) : Square(graph){
    QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
    if (direction == "x"){  
        QQuaternion x_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
        setRotation(z_rotation * x_rotation);
    } else{
        setRotation(z_rotation);
    }
    m_is_x_direction = (direction == "x");
}


void Square::setArea(const float &area){
    float unitPlotPerGraph = plotScalingVector().length();
    float scale = sqrt(area) * kUnitItemPerGraph / unitPlotPerGraph * kItemScale;
    setScaling(QVector3D(scale, 1, scale) * 0.1);
    m_area = area;
}


void Square::setArea(const float &area, const bool &is_positive){
    setArea(area);
    if (area != 0 && is_positive != m_is_positive){
        m_is_positive = is_positive;
        flipDirection();
    }
}


void Square::flipDirection(){
    QQuaternion z_rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, 90);
    QQuaternion x_rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, 180);
    if (m_is_positive){
        if (m_is_x_direction){
            QQuaternion x_rotation2 = QQuaternion::fromAxisAndAngle(1, 0, 0, 90);
            setRotation(z_rotation * x_rotation2);
        } else{
            setRotation(z_rotation * x_rotation);
        }
    } else{
        if (m_is_x_direction){
            QQuaternion x_rotation3 = QQuaternion::fromAxisAndAngle(1, 0, 0, -90);
            setRotation(z_rotation * x_rotation3);
        } else{
            setRotation(z_rotation);
        }
    }
}


int Line::layer = 1;


Line::Line(Q3DSurface* graph, QColor color, double (*_f) (double, double))
    : m_graph(graph), f(_f)
{
    line_proxy = new QSurfaceDataProxy;
    setDataProxy(line_proxy);
    setDrawMode(QSurface3DSeries::DrawSurface);
    setBaseColor(color);
    setColorStyle(Q3DTheme::ColorStyleUniform);
    m_graph->addSeries(this);
    layer++;
    this_layer = layer;
    x_range = m_graph->axisX()->max() - m_graph->axisX()->min();
    y_range = m_graph->axisY()->max() - m_graph->axisY()->min();
    z_range = m_graph->axisZ()->max() - m_graph->axisZ()->min();
}


void Line::addPoint(Point p){
    // to make it computationally efficient, don't add crosslines
    // when they are too close to each other
    if (crosslines.size() >= 2){
       Point last_point = crosslines.back().center;
       Point second_to_last_point = crosslines[crosslines.size() - 2].center;
       if (abs(second_to_last_point.z - last_point.z) < z_range * kLineStepSize &&
            abs(second_to_last_point.x - last_point.x) < x_range * kLineStepSize) {
            crosslines.pop_back();
            need_to_replace_last_row = true;
       }
    }


    CrossLine new_crossline(p);
    if (crosslines.empty()){
        new_crossline.left.x = p.x - kLineHalfWidth * x_range;
        new_crossline.right.x = p.x + kLineHalfWidth * x_range;
    } else{
        // calculate the left and right edge of the previous point
        // make them perpendicular to the direction of the line between the last two
        // points. This way, the ribbon appears to have equal width regardless of its
        // orientation.
        Point last_point = crosslines.back().center;
        QVector2D normal(p.x - last_point.x, p.z - last_point.z);
        normal.normalize();

        new_crossline.left.x = p.x - kLineHalfWidth * x_range * normal.y();
        new_crossline.left.z = p.z + kLineHalfWidth * z_range * normal.x();
        new_crossline.right.x = p.x + kLineHalfWidth * x_range * normal.y();
        new_crossline.right.z = p.z - kLineHalfWidth * z_range * normal.x();
    }
    crosslines.push_back(new_crossline);
}


QSurfaceDataArray* Line::getDataArray(){
    if (crosslines.empty()) return nullptr;
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    int end_idx = crosslines.size();
    dataArray->reserve(end_idx);
    for (int idx = 0; idx < end_idx; idx++){
        *dataArray << getDataRow(idx);
    }
    return dataArray;
}


QSurfaceDataRow* Line::getDataRow(int idx){
    QSurfaceDataRow* new_row = new QSurfaceDataRow(2);
    int index = 0;
    for (Point p : {crosslines[idx].left, crosslines[idx].right}) {
        float y = f(p.x, p.z) + kLineLayerHeight * y_range * (layer - this_layer + 1);
        (*new_row)[index++].setPosition(QVector3D(p.x, y, p.z));
    }
    return new_row;
}


QSurfaceDataRow* Line::getDataRow(){
    return getDataRow(crosslines.size() - 1);
}

void Line::render(){
    /* render in the most efficient way:
     * if there are small changes, just rewrite the last few rows;
     * if there are major changes, rewrite the whole data array so that
     * the proxy doesn't issue new requests to render everytime we write a row
     */
    m_visible = true;
    int data_size = crosslines.size();
    int rendered_data_size = line_proxy->rowCount();
    Q_ASSERT(rendered_data_size <= data_size);

    if (rendered_data_size == 0){
        line_proxy->resetArray(getDataArray());
    } else if (need_to_replace_last_row){
        need_to_replace_last_row = false;
        if (data_size - rendered_data_size){
            line_proxy->setRow(rendered_data_size - 1, getDataRow());
        } else if (data_size - rendered_data_size <= 5){
            line_proxy->removeRows(rendered_data_size - 1, 1);
            for (int i = rendered_data_size; i < data_size; i++){
                line_proxy->addRow(getDataRow(i));
            }
        } else{
            line_proxy->resetArray(getDataArray());
        }
    } else{
        if (data_size - rendered_data_size <= 5){
            for (int i = rendered_data_size; i < data_size; i++){
                line_proxy->addRow(getDataRow(i));
            }
        } else{
            line_proxy->resetArray(getDataArray());
        }
    }
}


void Line::erase(){
    // clears the line on the screen and the internally stored data
    line_proxy->resetArray(nullptr);
    crosslines = {};
    // Note: fragile code. This assumes everytime the graph
    // changes range, this erase function is called (through reset).
    x_range = m_graph->axisX()->max() - m_graph->axisX()->min();
    y_range = m_graph->axisY()->max() - m_graph->axisY()->min();
    z_range = m_graph->axisZ()->max() - m_graph->axisZ()->min();
}


void Line::setVisible(bool visible){
    if (visible == m_visible) return;
    m_visible = visible;
    visible ? render() : line_proxy->resetArray(nullptr);
}
