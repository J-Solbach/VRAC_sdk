#ifndef GAMEELEMENT_H
#define GAMEELEMENT_H

#include <QGraphicsItem>

class GameElement : public QGraphicsItem
{
public:
    explicit GameElement(QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {}
    GameElement(std::string name) {mName = name;}
    QPointF entryPoint() const { return scenePos();}

    virtual QRectF boundingRect() const override { return QRectF();}

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = nullptr)  override {}

    const std::string &name() const { return mName; }

private:
    QPointF mEntryPoint;
    std::string mName;
};




#endif // GAMEELEMENT_H
