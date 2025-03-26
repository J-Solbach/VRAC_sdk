#pragma once

#include <QObject>
#include <QTimer>

class strategy_manager_signals : public QObject {
    Q_OBJECT
public:
    explicit strategy_manager_signals(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void do_funny_action();
    void go_back_home();
    void end();

public slots:
    virtual void jack_pulled() = 0;
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
        go_back_home_timer.setInterval(std::chrono::seconds(85));
        funny_action_timer.setInterval(std::chrono::seconds(98));
        strategy_ticker.setInterval(std::chrono::milliseconds(10));

        game_timer.setSingleShot(true);
        go_back_home_timer.setSingleShot(true);
        funny_action_timer.setSingleShot(true);

        connect(&game_timer,        &QTimer::timeout, this, &strategyManager::end);
        connect(&go_back_home_timer,  &QTimer::timeout, this, &strategyManager::go_back_home);
        connect(&funny_action_timer, &QTimer::timeout, this, &strategyManager::do_funny_action);
        connect(&strategy_ticker,   &QTimer::timeout, this, &strategyManager::update);
    }

    virtual ~strategyManager() {
        delete strategy;
    }

    void set_do_funny_action_time(std::chrono::seconds seconds) { funny_action_timer.setInterval(std::chrono::seconds(seconds)); }
    void set_go_back_home_time(std::chrono::seconds seconds) { go_back_home_timer.setInterval(std::chrono::seconds(seconds)); }
    void set_strategy_ticker_period(std::chrono::seconds seconds) { strategy_ticker.setInterval(seconds); }
    bool has_game_ended() const { return game_ended; }

    void reset() {
        game_timer.stop();
        go_back_home_timer.stop();
        funny_action_timer.stop();
        strategy_ticker.stop();
    }

    virtual void jack_pulled() override {
        if(game_timer.isActive()) return;

        game_timer.start();
        go_back_home_timer.start();
        funny_action_timer.start();
    }

    virtual void update() override {
        strategy->update();
    }

    void start() {
        strategy_ticker.start(std::chrono::milliseconds(10));
        strategy->start();
    }

    strategy_type *getStrategy() const
    {
        return strategy;
    }

private:
    QTimer game_timer;
    QTimer go_back_home_timer;
    QTimer funny_action_timer;
    QTimer strategy_ticker;

    strategy_type *strategy;

    bool game_ended;
};


