#pragma once
#include <thread>

template<typename T>
class Thread {
public:
    Thread(i32(*func)(T)) :
        _func(func),
        _isFinished(false),
        _exitCode(0),
        _argCopy({}) {
        _threadFunction = [] (Thread<T>* thread) {
            thread->_exitCode = (*thread->_func)(thread->_argCopy);
            thread->_isFinished = true;
        };
    }
    Thread() : Thread(nullptr) {}

    void start(const T& arg) {
        _argCopy = arg;
        _thread = std::thread(_threadFunction, this);
    }
    void setFunction(i32(*func)(T)) {
        _func = func;
    }

    const bool getIsFinished() const {
        return _isFinished;
    }
    const i32 getErrorCode() const {
        return _exitCode;
    }
private:
    void(*_threadFunction)(Thread<T>*);
    i32(*_func)(T);
    std::thread _thread;

    T _argCopy;
    bool _isFinished;
    i32 _exitCode;
};

typedef i32(*ThreadVoidFunction)();

class ThreadVoid {
public:
    ThreadVoid(ThreadVoidFunction func) :
        _func(func),
        _isFinished(false),
        _exitCode(0) {
        _threadFunction = [] (ThreadVoid* thread) {
            thread->_exitCode = thread->_func();
            thread->_isFinished = true;
        };
    }
    ThreadVoid() : ThreadVoid(nullptr) {}

    void start() {
        _thread = std::thread(_threadFunction, this);
    }
    void setFunction(ThreadVoidFunction func) {
        _func = func;
    }

    const bool getIsFinished() const {
        return _isFinished;
    }
    const i32 getErrorCode() const {
        return _exitCode;
    }
private:
    void(*_threadFunction)(ThreadVoid*);
    ThreadVoidFunction _func;
    std::thread _thread;

    bool _isFinished;
    i32 _exitCode;
};