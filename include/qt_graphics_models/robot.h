#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneDragDropEvent>

namespace vrac::qt_graphics::models {

class robot_graphic_item : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    robot_graphic_item() {
        setPos(QPointF(500,500));
        setFlags(QGraphicsItem::ItemIsMovable);
    }

    void setPos(QPointF pos) {
        QGraphicsPixmapItem::setPos(pos - boundingRect().center());
        emit posChanged(pos);
    }
    QPointF pos() { return sceneBoundingRect().center(); }

    int theta() const { return mTheta; }
    void setTheta(int newTheta) {
        mTheta = newTheta;

        setTransformOriginPoint(boundingRect().center());
        setRotation(-mTheta);
    }

    void addItemHandler(QString name, QPointF pos, int radius) {
        mItemHandlers[name] = std::shared_ptr<QGraphicsItem>(new QGraphicsEllipseItem(QRectF(-radius, -radius, radius * 2, radius * 2), this));
        mItemHandlers[name]->setPos(pos);
    }
    QGraphicsItem* getItemHandler(QString name) { return mItemHandlers[name].get(); }

signals :
    void posChanged(QPointF);

    protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        emit posChanged(event->scenePos());
        QGraphicsItem::mouseMoveEvent(event);
    }

public slots:
    void updatePos(QPointF pos, double theta) {
        setPos(pos);
        setTheta(theta);
    }

private:
    int mTheta;
    QMap<QString, std::shared_ptr<QGraphicsItem>> mItemHandlers;
};

}
