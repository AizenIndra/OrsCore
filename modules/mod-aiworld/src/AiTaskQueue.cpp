/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "AiTaskQueue.h"
#include "AiLlmClient.h"
#include "AiWorldConfig.h"
#include "Log.h"

AiTaskQueue* AiTaskQueue::instance()
{
    static AiTaskQueue queue;
    return &queue;
}

void AiTaskQueue::Start()
{
    bool expected = false;
    if (!_running.compare_exchange_strong(expected, true))
        return;

    _worker = std::thread([this]() { WorkerLoop(); });
    LOG_INFO("module", ">> AiWorld task queue worker started");
}

void AiTaskQueue::Stop()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _running = false;
    }
    _cv.notify_all();
    if (_worker.joinable())
        _worker.join();
}

bool AiTaskQueue::Enqueue(AiTask task)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_pending.size() >= sAiWorldConfig().GetTaskMaxQueue())
    {
        LOG_DEBUG("module", "AiWorld task queue full, dropping task");
        return false;
    }
    _pending.push_back(std::move(task));
    _cv.notify_one();
    return true;
}

std::optional<AiWorld::DirectorAction> AiTaskQueue::PopCompletedAction()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_completed.empty())
        return std::nullopt;
    AiWorld::DirectorAction action = std::move(_completed.front());
    _completed.pop_front();
    return action;
}

void AiTaskQueue::WorkerLoop()
{
    while (true)
    {
        AiTask task;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [&]() { return !_running || !_pending.empty(); });
            if (!_running && _pending.empty())
                break;
            task = std::move(_pending.front());
            _pending.pop_front();
        }

        if (task.type == AiWorld::AiTaskType::ProposeScenario)
        {
            AiWorld::LlmProposal proposal = sAiLlmClient->ProposeScenario(task.snapshot, task.recentEventsJson);
            if (!proposal.valid)
            {
                sAiLlmClient->NotifyFailure();
                continue;
            }

            sAiLlmClient->NotifySuccess();
            AiWorld::DirectorAction action;
            action.type = proposal.action;
            action.zoneId = task.snapshot.zoneId;
            action.beneficiary = proposal.beneficiary;
            action.botCount = proposal.botCount;
            action.eventTemplateId = proposal.eventTemplateId
                ? proposal.eventTemplateId
                : AiWorld::EVENT_HILLSBRAD_SKIRMISH;
            action.priority = 5;
            action.reason = proposal.reason.empty() ? "llm_proposal" : proposal.reason;

            std::lock_guard<std::mutex> lock(_mutex);
            _completed.push_back(std::move(action));
        }
        else if (task.type == AiWorld::AiTaskType::GenerateDialogue)
        {
            std::string dialogue = sAiLlmClient->GenerateDialogue(task.dialogueContext);
            if (dialogue.empty())
                sAiLlmClient->NotifyFailure();
            else
                sAiLlmClient->NotifySuccess();
            LOG_INFO("module", "AiWorld NPC dialogue: {}", dialogue);
        }
        else if (task.type == AiWorld::AiTaskType::AnalyzeHistory)
        {
            // Reserved: LLM summary of recentEventsJson for GM logs.
            LOG_DEBUG("module", "AiWorld AnalyzeHistory task for zone {}", task.snapshot.zoneId);
        }
    }
}
