#pragma once

#include <QObject>
#include <QMap>
#include <QDebug>

#include "subscriber.h"
#include "Utils/Singleton.h"

class Subscriber;

class MessagePublisher: public QObject, public Singleton<MessagePublisher>
{
    Q_OBJECT
    friend class Singleton<MessagePublisher>;
    public :
    MessagePublisher()
    {

    }
    ~MessagePublisher()
    {
        for (auto dump : mDumpSubscribers) {
            if (dump != nullptr) delete dump;
        }

        for (auto sub : mSubscribers) {
            if (sub != nullptr) delete sub;
        }
    }

    void subscribe(Subscriber *newSub)
    {
        mSubscribers.append(newSub);
    }
    void subscribeAsDump(Subscriber *dump) {mDumpSubscribers.append(dump);}
    void sendMessage(const QString targetNode, int id, const QByteArray data = QByteArray())
    {
        qDebug() << "Test message";
        for (auto dump : mDumpSubscribers) {
            dump->onMessage(id, data);
        }

        for (auto sub : mSubscribers) {
            if (!sub->nodes().contains(targetNode)) continue;
            sub->onMessage(id, data);
        }
    }

private:
    QVector<Subscriber*> mSubscribers;
    QVector<Subscriber*> mDumpSubscribers;
};

#define sendMessageTo MessagePublisher::get()->sendMessage
