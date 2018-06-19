#pragma once

#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <Vorb/types.h>

// Interface For A Loading Task
class ILoadTask {
public:
    ILoadTask()
        : _isFinished(false) {
        // Empty
    }

    bool isFinished() const {
        return _isFinished;
    }

    virtual void load() = 0;
    void doWork() {
        load();
        _isFinished = true;
    }

    std::unordered_set<nString> dependencies;
private:
    // Loading State
    volatile bool _isFinished;
};

// Loading Task Closure Wrapper
template<typename F>
class LoadFunctor : public ILoadTask {
public:
    LoadFunctor() {
        // Empty
    }
    LoadFunctor(F& f) 
        : _f(f) {
        // Empty
    }

    void load() {
        // Call The Closure
        _f();
    }
private:
    F _f;
};

// Make Use Of Compiler Type Inference For Anonymous Type Closure Wrapper
template<typename F>
inline ILoadTask* makeLoader(F f) {
    return new LoadFunctor<F>(f);
}

class LoadMonitor {
public:
    LoadMonitor();
    ~LoadMonitor();

    // Add A Task To This List (No Copy Or Delete Will Be Performed On Task Pointer)
    void addTask(nString name, ILoadTask* task);
    template<typename F>
    void addTaskF(nString name, F taskF) {
        ILoadTask* t = makeLoader(taskF);
        _internalTasks.push_back(t);
        addTask(name, t);
    }
    
    // Make A Loading Task Dependant On Another (Blocks Until Dependency Completes)
    void setDep(nString name, nString dep);

    // Fires Up Every Task In A Separate Thread
    void start();
    // Blocks On Current Thread Until All Child Threads Have Completed
    void wait();

    // Checks If A Task Is Finished
    bool isTaskFinished(nString task);
private:
    // Is A Task Finished (False If Task Does Not Exist
    bool isFinished(nString task);
    // Check Dependencies Of A Task To See If It Can Begin
    bool canStart(nString task);

    // Tasks Mapped By Name
    std::unordered_map<nString, ILoadTask*> _tasks;
    
    // Thread For Each Task
    std::vector<std::thread> _internalThreads;

    // Functor Wrapper Tasks That Must Be Deallocated By This Monitor
    std::vector<ILoadTask*> _internalTasks;

    // Monitor Lock
    std::mutex _lock;
    std::condition_variable _completionCondition;
};