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
    if (kvp == _tasks.end()) {
        fprintf(stderr, "LoadMonitor Warning: dependency %s does not exist\n", task.c_str());
        return false;
    }
    return kvp->second->isFinished();
}
bool LoadMonitor::canStart(nString task) {
    // Check that the dependency exists
    auto& kvp = _tasks.find(task);
    if (kvp == _tasks.end()) {
        fprintf(stderr, "LoadMonitor Warning: task %s does not exist\n", task.c_str());
        return false;
    }
    // Check all dependencies
    for (auto& dep : kvp->second->dependencies) {
        if (!isFinished(dep)) {
            return false;
        }
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
                return monitor->canStart(name);
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
            uLock.lock();
            _completionCondition.notify_all();
            uLock.unlock();
        });
    }
}
void LoadMonitor::wait() {
    // Wait for all threads to complete
    for (auto& t : _internalThreads) {
        t.join();
        t.detach();
    }

    _internalThreads.clear();

    // Free all tasks
    for (ILoadTask* t : _internalTasks) delete t;
    _internalTasks.clear();
}

void LoadMonitor::setDep(nString name, nString dep) {
    // Check that the task exists
    auto& kvp = _tasks.find(name);
    if (kvp == _tasks.end()) {
        fprintf(stderr, "LoadMonitor Warning: Task %s doesn't exist.\n",name.c_str());
        return;
    }

    // Check that the dependency exists
    auto& dvp = _tasks.find(dep);
    if (dvp == _tasks.end()) {
        fprintf(stderr, "LoadMonitor Warning: Dependency %s doesn't exist.\n", dep.c_str());
        return;
    }

    // Add the dependency
    kvp->second->dependencies.insert(dep);
}


