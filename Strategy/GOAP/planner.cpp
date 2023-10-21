#include "planner.h"

Planner::Planner()
{

}

Plan Planner::getPlan(Goal *goal, const QVariantMap& blackBoard)
{
    QVariantMap desiredState = goal->desiredState();

    if (desiredState.isEmpty()) return Plan();

    return findBestPlan(desiredState, blackBoard);
}

Plan Planner::findBestPlan(QVariantMap &desiredState, const QVariantMap& blackBoard)
{
    Node rootNode {.task = nullptr, .state = desiredState};

    if (buildPlans(rootNode, blackBoard))
    {
        QVector<Plan> plans = gatherAllPlans(rootNode, blackBoard);
        return findCheapestPlan(plans, blackBoard);
    }

    return Plan();
}

Plan Planner::findCheapestPlan(QVector<Plan>& plans, const QVariantMap& blackBoard)
{
    Plan cheapest;
    int cheapestCost = 0xFFFF;

    for (auto plan : plans)
    {
        int totalCost = 0;
        for (auto task : plan)
        {
            if (task != nullptr) totalCost += task->cost(blackBoard);
        }

        if (cheapest.isEmpty() || totalCost < cheapestCost
            || (totalCost == cheapestCost && plan.size() < cheapest.size()))
        {
            cheapest = plan;
        }
    }

    return cheapest;
}

QVector<Plan> Planner::gatherAllPlans(const Node &node, const QVariantMap &blackBoard) const
{
    QVector<Plan> plans;

    if (node.childrens.isEmpty())
    {
        plans.push_back({node.task});
        return plans;
    }

    for (const auto& children : node.childrens)
    {
        for (auto childPlan : gatherAllPlans(children, blackBoard))
        {
            if (node.task != nullptr) childPlan.push_back(node.task);

            plans.push_back(childPlan);
        }
    }

    return plans;
}

bool Planner::buildPlans(Node &step, const QVariantMap &blackBoard)
{
    bool hasFollowUp = false;
    QVariantMap state = step.state;

    for (const auto& key : state.keys())
    {
        if (blackBoard.contains(key) && blackBoard[key] == step.state[key])
        {
            state.remove(key);
        }
    }

    if (state.isEmpty())
    {
        // A PATH HAS BEEN FOUND for this branch. Stopping here.
        return true;
    }

    for (const auto& task : mTaskPool)
    {
        if (!task->isValid()) continue;

        bool shouldUseTask = false;
        const QVariantMap& effects = task->effects();
        QVariantMap desiredState = state;

        //check if at least one effect is part of the desiredState
        for (const auto& key : desiredState.keys())
        {
            if (effects.contains(key) && effects[key] == desiredState[key])
            {
                // If task effects are part of the desired state, removing it from desired
                desiredState.remove(key);
                shouldUseTask = true;
            }
        }

        // not usefull
        if (!shouldUseTask) continue;

        desiredState.insert(task->preconditions());

        Node newStep = {
            .task = task,
            .state = desiredState
        };

        if (desiredState.isEmpty() || buildPlans(newStep, blackBoard))
        {
            step.childrens.push_back(std::move(newStep));
            hasFollowUp = true;
        }

    }
    return hasFollowUp;
}

void Planner::setTaskPool(const QVector<ITask *> &newTaskPool)
{
    mTaskPool = newTaskPool;
}
