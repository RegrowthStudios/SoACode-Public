#include "stdafx.h"
#include "LoadMonitor.h"

LoadMonitor::LoadMonitor() :
_lock(),
_completionCondition() {
    // Empty
}
LoadMonitor::~LoadMonitor() {
    if (_internalTasks.size() > 0) {
        for (ILoadTask* t : _internalTasks) {
            if (t) delete t;
        }
    }
    for (auto& t : _internalThreads) {
        t.detach();
    }
}

void LoadMonitor::addTask(nString name, ILoadTask* task) {
    _tasks.emplace(name, task);
}
bool LoadMonitor::isTaskFinished(nString task) {
    _lock.lock();
    bool state = isFinished(task);
    _lock.unlock();
    return state;
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
            _completionCondition.wait(uLock, [=] {
#ifdef DEBUG
                printf("CHECK: %s\r\n", name.c_str());
#endif
                return canStart(name);
            });
            uLock.unlock();

#ifdef DEBUG
            printf("BEGIN: %s\r\n", name.c_str());
            task->doWork();
            printf("END: %s\r\n", name.c_str());
#else
            task->doWork();
#endif // DEBUG

            // Notify That This Task Is Completed
            _completionCondition.notify_all();
        });
    }
}
void LoadMonitor::wait() {
    for (auto& t : _internalThreads) {
        t.join();
        t.detach();
    }
    _internalThreads.clear();

    for (ILoadTask* t : _internalTasks) delete t;
    _internalTasks.clear();
}

void LoadMonitor::setDep(nString name, nString dep) {
    auto& kvp = _tasks.find(name);
    if (kvp == _tasks.end()) return;

    kvp->second->dependencies.insert(dep);
}


