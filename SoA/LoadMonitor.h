#pragma once
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <thread>

class ILoadTask {
public:
    ILoadTask()
        : _isFinished(false) {
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
    bool _isFinished;
};

template<typename F>
class LoadFunctor : public ILoadTask {
public:
    LoadFunctor() {
    }
    LoadFunctor(F& f) {
        _f = f;
    }

    void load() {
        _f();
    }
private:
    F _f;
};

template<typename F>
inline ILoadTask* makeLoader(F f) {
    return new LoadFunctor<F>(f);
}

class LoadMonitor {
public:
    LoadMonitor();
    ~LoadMonitor();

    //************************************
    // Method:    addTask
    // FullName:  LoadMonitor::addTask
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Parameter: nString name Task Name Used For Dependency Lookups
    // Parameter: ILoadTask * task Pointer To A Task (No Copy Or Delete Will Be Performed On It)
    //************************************
    void addTask(nString name, ILoadTask* task);
    template<typename F>
    void addTaskF(nString name, F taskF) {
        ILoadTask* t = makeLoader(taskF);
        _internalTasks.push_back(t);
        addTask(name, t);
    }
    void setDep(nString name, nString dep);

    void start();
    void wait();
private:
    bool isFinished(nString task);
    bool canStart(nString task);

    std::unordered_map<nString, ILoadTask*> _tasks;
    std::vector<ILoadTask*> _internalTasks;
    std::vector<std::thread> _internalThreads;
    std::mutex _lock;
    std::condition_variable_any _completionCondition;
};