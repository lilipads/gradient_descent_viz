#ifndef ITEM_H
#define ITEM_H

#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/QCustom3DLabel>
#include <QtDataVisualization/Q3DSurface>


using namespace QtDataVisualization;

const float kArrowYScale = 0.1; // 0.02; // how much we want to scale down the arrow
const float kArrowOffset = 8; // original arrow dimension in y axis
const QVector3D kLabelOffset(0, 20, 0);


class Item : public QCustom3DItem{
public:
    Item() {}

    void setColor(QColor color);
    void addToGraph(Q3DSurface* graph);

protected:
    QCustom3DLabel* m_label = nullptr;
    Q3DSurface* m_graph = nullptr;
};


class LabeledItem : public Item
{
public:
    LabeledItem();

    void setLabel(const QString &text);
    void addToGraph(Q3DSurface* graph);
    void setLabelVisibility(bool visible);
    void setVisible(bool visible);
    void setPosition(const QVector3D& position);

protected:
    bool label_visibility = false;

    void initializeLabel();
};


class Ball : public Item
{
public:
    Ball(QColor color);
};

class Arrow : public LabeledItem{
public:
    Arrow();
    Arrow(QVector3D vector);
    void setVector(QVector3D vector);
    void setMagnitude(const float &magnitude);

    QVector3D vector(){
        /* the xyz coordinates of the rendered vector (starts from the origin)
         * scale down by a constant because when rendering, we don't draw it
         * to the full scale (otherwise it would dominate the screen)
         */

         return direction * m_magnitude * kArrowYScale * kArrowOffset;}
    float magnitude(){return m_magnitude;}

private:
    QVector3D direction = QVector3D(0, 1, 0);
    float m_magnitude = 1.0;

};

class Square : public LabeledItem{
public:
    Square();
    void setArea(const float &area);

};

#endif // ITEM_H
