#ifndef STRATEGYMANAGER_H
#define STRATEGYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QThread>

class strategy_manager_signals : public QObject {
    Q_OBJECT
public:
    explicit strategy_manager_signals(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void doFunnyAction();
    void goBackHome();
    void end();

public slots:
    virtual void jackPulled() = 0;
};

template<typename strategy_type>
class StrategyManager : public strategy_manager_signals
{
public:

    StrategyManager(strategy_type && strategy, QObject *parent = nullptr) : strategy_manager_signals(parent), strategy(strategy)
    {
        mGameTimer.setInterval(std::chrono::seconds(100));
        mGoBackHomeTimer.setInterval(std::chrono::seconds(90));
        mFunnyActionTimer.setInterval(std::chrono::seconds(98));

        mGameTimer.setSingleShot(true);
        mGoBackHomeTimer.setSingleShot(true);
        mFunnyActionTimer.setSingleShot(true);

        connect(&mGameTimer, &QTimer::timeout, this, &StrategyManager::end);
        connect(&mGoBackHomeTimer, &QTimer::timeout, this, &StrategyManager::goBackHome);
        connect(&mFunnyActionTimer, &QTimer::timeout, this, &StrategyManager::doFunnyAction);
    }

    void setFunnyActionTimer(int seconds) { mFunnyActionTimer.setInterval(std::chrono::seconds(seconds)); }
    void setGoBackHomeTimer(int seconds) { mGoBackHomeTimer.setInterval(std::chrono::seconds(seconds)); }
    bool gameEnded() const { return mGameEnded; }

    void reset() {
        mGameTimer.stop();
        mGoBackHomeTimer.stop();
        mFunnyActionTimer.stop();
    }

    virtual void jackPulled() override
    {
        mGameTimer.start();
        mGoBackHomeTimer.start();
        mFunnyActionTimer.start();
        //GameState::get()->setState("hasJack", false);
    }

    void update() {
        strategy.update();
    }

private:
    QTimer mGameTimer;
    QTimer mGoBackHomeTimer;
    QTimer mFunnyActionTimer;

    strategy_type strategy;

    bool mGameEnded;
};

#endif // STRATEGYMANAGER_H
