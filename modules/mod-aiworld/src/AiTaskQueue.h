/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef MOD_AIWORLD_TASK_QUEUE_H
#define MOD_AIWORLD_TASK_QUEUE_H

#include "AiWorldCommon.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

struct AiTask
{
    AiWorld::AiTaskType type = AiWorld::AiTaskType::ProposeScenario;
    AiWorld::ZoneSnapshot snapshot;
    std::string recentEventsJson;
    std::string dialogueContext;
};

class AiTaskQueue
{
public:
    static AiTaskQueue* instance();

    void Start();
    void Stop();

    bool Enqueue(AiTask task);
    // Called from world thread to pull completed LLM proposals.
    std::optional<AiWorld::DirectorAction> PopCompletedAction();

private:
    AiTaskQueue() = default;
    void WorkerLoop();

    std::mutex _mutex;
    std::condition_variable _cv;
    std::deque<AiTask> _pending;
    std::deque<AiWorld::DirectorAction> _completed;
    std::thread _worker;
    std::atomic<bool> _running{ false };
};

#define sAiTaskQueue AiTaskQueue::instance()

#endif
