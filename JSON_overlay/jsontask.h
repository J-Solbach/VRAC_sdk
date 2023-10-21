#ifndef JSONTASK_H
#define JSONTASK_H

#include "../Strategy/GOAP/task.h"

class Stm;

class JsonTask : public ITask
{
public:

    JsonTask(QString fileName, QString dir);
    virtual ~JsonTask();

    virtual void run() override;
    virtual int cost(const QVariantMap&) const override;
    virtual bool isValid() const override;

private:
    Stm* mStm;

    int mCost;
    QJsonObject mValid;
};

#endif // JSONTASK_H
