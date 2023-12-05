#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneDragDropEvent>

class Robot : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    Robot() { setFlags(QGraphicsItem::ItemIsMovable); }

    void setPos(QPointF pos) { QGraphicsPixmapItem::setPos(pos - boundingRect().center()); }
    QPointF pos() { return sceneBoundingRect().center(); }

    int theta() const { return mTheta; }
    void setTheta(int newTheta) { mTheta = newTheta; }

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

private:
    int mTheta;
    QMap<QString, std::shared_ptr<QGraphicsItem>> mItemHandlers;
};
