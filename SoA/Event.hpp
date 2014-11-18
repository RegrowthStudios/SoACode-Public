///
/// Events.hpp
///
/// Created by Cristian Zaloj on 8 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// C#-style events
///

#pragma once

#ifndef Events_h__
#define Events_h__

#include <algorithm>

/// This is the main function pointer

/// @author Cristian Zaloj
template<typename... Params>
class IDelegate {
public:
    /// Invoke this function's code
    /// @param sender: The sender that underlies the event
    /// @param p: Additional arguments to function
    virtual void invoke(void* sender, Params... p) = 0;
};

/// Functor object to hold instances of anonymous lambdas

/// @author Cristian Zaloj
template<typename F, typename... Params>
class Delegate : public IDelegate<Params...> {
public:
    /// Empty constructor
    Delegate() {
        // Empty
    }
    /// Copy constructor
    /// @param f: Lambda functor
    Delegate(F& f):
        _f(f) {
        // Empty
    }

    /// Invoke this functor's code
    /// @param sender: The sender that underlies the event
    /// @param p: Additional arguments to function
    virtual void invoke(void* sender, Params... p) override {
        _f(sender, p...);
    }
private:
    F _f; ///< Type-inferenced lambda functor
};

/// Use the compiler to generate a delegate
/// @param f: Lambda functor
/// @return Pointer to delegate created on the heap (CALLER DELETE)
template<typename F, typename... Params>
inline IDelegate<Params...>* createDelegate(F f) {
    return new Delegate<F, Params...>(f);
}

/// An event that invokes methods taking certain arguments

/// @author Cristian Zaloj
template<typename... Params>
class Event {
public:
    /// Create an event with a sender attached to it

    /// @param sender: Owner object sent with each invokation
    Event(void* sender = nullptr):
        _sender(sender) {
        // Empty
    }

    /// Call all bound methods

    /// @param p: Arguments used in function calls
    void send(Params... p) {
        for(auto& f : _funcs) {
            f->invoke(_sender, p...);
        }
    }

    /// Call all bound methods

    /// @param p: Arguments used in function calls
    void operator()(Params... p) {
        this->send(p...);
    }

    /// Add a function to this event

    /// @param f: A subscriber
    /// @return The delegate passed in
    IDelegate<Params...>* add(IDelegate<Params...>* f) {
        if(f == nullptr) return nullptr;
        _funcs.push_back(f);
        return f;
    }

    /// Add a function to this event

    /// @param f: A unknown-type subscriber
    /// @return The newly made delegate (CALLER DELETE)
    template<typename F>
    IDelegate<Params...>* addFunctor(F f) {
        IDelegate<Params...>* d = createDelegate<F, Params...>(f);
        return this->add(d);
    }

    /// Remove a function (just one) from this event

    /// @param f: A subscriber
    void remove(IDelegate<Params...>* f) {
        if(f == nullptr) return;
        _funcs.erase(std::find(_funcs.begin(), _funcs.end(), f));
    }
private:
    void* _sender; ///< Event owner
    std::vector< IDelegate<Params...>* > _funcs; ///< List of bound functions (subscribers)
};

#endif // Events_h__