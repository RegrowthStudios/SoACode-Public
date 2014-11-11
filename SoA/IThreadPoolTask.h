///
/// IThreadPoolTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file provides a thread pool task interface
///

#pragma once

#ifndef ThreadPoolTask_h__
#define ThreadPoolTask_h__

class IThreadPoolTask {
public:
    IThreadPoolTask() { /* Empty */ };
    virtual ~IThreadPoolTask() { /* Empty */ };
    /// Executes the task
    virtual void execute() = 0;

    /// Checks if this should be stored in a finished tasks queue
    virtual const bool& shouldAddToFinishedTasks() const { return _shouldAddToFinishedTasks; }

protected:
    bool _shouldAddToFinishedTasks = false; ///< SHould it be stored in a finished tasks queue
};

#endif // ThreadPoolTask_h__