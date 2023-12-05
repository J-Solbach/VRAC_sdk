#pragma once

#include <QObject>
#include <set>
#include <functional>
#include <range/v3/all.hpp>

#include "event.h"
#include "transition.h"

class state_signals_slots :  public QObject {
    Q_OBJECT
public:
    explicit state_signals_slots(){
        connect(this, &state_signals_slots::sendEvent, this, &state_signals_slots::onEvent, Qt::QueuedConnection);
    }

signals :
    void sendEvent(Event e);

public slots:
    virtual std::vector<std::string> onEvent(Event e) = 0;
};

template<typename context_t, typename params_t>
class state : public state_signals_slots
{
public:
    using context_type = context_t;
    using params_type = params_t;
    using signal_slot_handler_type = state_signals_slots;

    state(std::string name) : state_signals_slots(), mCheckSum(std::hash<std::string>{}(name)) {
       ;
        setObjectName("[" + name + "]");
        mCheckSum += (rand() %0xFFFF);
    }

    state(const std::string & name,  const params_t & params) : state_signals_slots(), mParams(params), mCheckSum(std::hash<std::string>{}(name)) {
        setObjectName("[" + name + "]");
        mCheckSum += (rand() %0xFFFF);
    }

    const std::vector<transition> &transitions() const { return mTransitions; }
    void set_transitions(std::vector<transition> && newTransitions) { mTransitions = std::move(newTransitions); }

    bool testCheckSum(uint16_t checkSum) {
        if (checkSum == 0xFFFF) return true;

        if (checkSum != mCheckSum ) return false;

        if (mCheckCounter > 0) mCheckCounter--;

        return (mCheckCounter == 0);
    }

    uint16_t checkSum() {
        if (mCheckCounter == 0xFFFF) mCheckCounter = 0;

        mCheckCounter++;
        return mCheckSum;
    }

    virtual std::vector<std::string> onEvent(Event e) override {
        return mTransitions
               | ranges::views::filter([&](const auto & transition){
                     return transition.event == e && testCheckSum(e.checksum);
                    })
                | ranges::views::transform([](const auto & transition) {
                     return transition.target_state;
                    })
                | ranges::to<std::vector>;
    }

    virtual void onEntry(context_t &, Event) = 0;
    virtual void onExit(context_t &, Event) {}

    params_t params() const {return mParams;}

protected:
    std::vector<transition> mTransitions;
    params_t mParams;

    std::size_t mCheckSum = 0xFFFF;
    std::size_t mCheckCounter = 0xFFFF;
};

#include <fmt/format.h>

template<typename context_t, typename params_t>
struct fmt::formatter<state<context_t, params_t>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const state<context_t, params_t> & val, format_context& ctx) {
        // Format val and write the output to ctx.out().
        return fmt::format_to(ctx.out(), "{}", val.objectName().toStdString());
    }
};


