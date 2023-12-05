#ifndef STRATEGYMANAGER_H
#define STRATEGYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QDebug>

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
    virtual void update() = 0;
};

template<typename strategy_type>
class strategyManager : public strategy_manager_signals
{
public:

    strategyManager(strategy_type * strategy, QObject *parent = nullptr) :
        strategy_manager_signals(parent),
        strategy(strategy)
    {
        game_timer.setInterval(std::chrono::seconds(100));
        go_back_home_timer.setInterval(std::chrono::seconds(90));
        funny_action_timer.setInterval(std::chrono::seconds(98));
        strategy_ticker.setInterval(std::chrono::milliseconds(10));

        game_timer.setSingleShot(true);
        go_back_home_timer.setSingleShot(true);
        funny_action_timer.setSingleShot(true);

        connect(&game_timer,        &QTimer::timeout, this, &strategyManager::end);
        connect(&go_back_home_timer,  &QTimer::timeout, this, &strategyManager::goBackHome);
        connect(&funny_action_timer, &QTimer::timeout, this, &strategyManager::doFunnyAction);
        connect(&strategy_ticker,   &QTimer::timeout, this, &strategyManager::update);

        strategy_ticker.start(std::chrono::milliseconds(10));
        strategy->start();
    }

    void setFunnyActionTimer(std::chrono::seconds seconds) { funny_action_timer.setInterval(std::chrono::seconds(seconds)); }
    void setGoBackHomeTimer(std::chrono::seconds seconds) { go_back_home_timer.setInterval(std::chrono::seconds(seconds)); }
    void setstrategyTicker(std::chrono::seconds seconds) { strategy_ticker.setInterval(seconds); }
    bool gameEnded() const { return game_ended; }

    void reset() {
        game_timer.stop();
        go_back_home_timer.stop();
        funny_action_timer.stop();
        strategy_ticker.stop();
    }

    virtual void jackPulled() override {
        game_timer.start();
        go_back_home_timer.start();
        funny_action_timer.start();
    }

    virtual void update() override{
        strategy->update();
    }

private:
    QTimer game_timer;
    QTimer go_back_home_timer;
    QTimer funny_action_timer;
    QTimer strategy_ticker;

    strategy_type *strategy;

    bool game_ended;
};

#endif // STRATEGYMANAGER_H
