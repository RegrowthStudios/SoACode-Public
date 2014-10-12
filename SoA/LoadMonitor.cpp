#include "stdafx.h"
#include "LoadMonitor.h"

LoadMonitor::LoadMonitor() :
_lock(),
_completionCondition() {
    // Empty
}
LoadMonitor::~LoadMonitor() {
    for (ILoadTask* t : _internalTasks) delete t;
    _internalTasks.clear();
}

void LoadMonitor::addTask(nString name, ILoadTask* task) {
    _tasks.emplace(name, task);
}

bool LoadMonitor::isFinished(nString task) {
    auto& kvp = _tasks.find(task);
    if (kvp == _tasks.end()) return false;
    return kvp->second->isFinished();
}
bool LoadMonitor::canStart(nString task) {
    auto& kvp = _tasks.find(task);
    if (kvp == _tasks.end()) return false;

    // Check All Dependencies
    for (auto& dep : kvp->second->dependencies) {
        if (!isFinished(dep)) return false;
    }
    return true;
}

void LoadMonitor::start() {
    LoadMonitor* monitor = this;
    for (auto& kvp : _tasks) {
        ILoadTask* task = kvp.second;
        nString name = kvp.first;
        _internalThreads.emplace_back([=] () {
            // Wait For Dependencies To Complete
            std::unique_lock<std::mutex> uLock(monitor->_lock);
            _completionCondition.wait(uLock, [=] { return canStart(name); });

#ifdef DEBUG
            printf("BEGIN: %s\r\n", name.c_str());
            task->doWork();
            printf("END: %s\r\n", name.c_str());
#else
            task->doWork();
#endif // DEBUG

            // Notify That This Task Is Completed
            uLock.unlock();
            _completionCondition.notify_all();
        });
    }
}
void LoadMonitor::wait() {
    for (auto& t : _internalThreads) t.join();
    for (ILoadTask* t : _internalTasks) delete t;
    _internalTasks.clear();
}

void LoadMonitor::setDep(nString name, nString dep) {
    auto& kvp = _tasks.find(name);
    if (kvp == _tasks.end()) return;

    kvp->second->dependencies.insert(dep);
}

