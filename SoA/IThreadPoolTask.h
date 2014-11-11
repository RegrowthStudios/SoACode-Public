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

class WorkerData;

class IThreadPoolTask {
public:
    /// Constructor
    /// @param taskId: Optional unique identifier for task type.
    IThreadPoolTask(i32 taskId = 0) : _taskId(taskId) { /* Empty */ };
    virtual ~IThreadPoolTask() { /* Empty */ };

    /// Executes the task
    virtual void execute(WorkerData* workerData) = 0;

    /// Checks if this should be stored in a finished tasks queue
    virtual const bool& shouldAddToFinishedTasks() const { return _shouldAddToFinishedTasks; }

    /// Gets the task ID
    const i32& getTaskId() const { return _taskId; }

protected:
    i32 _taskId;
    bool _shouldAddToFinishedTasks = false; ///< SHould it be stored in a finished tasks queue
};

#endif // ThreadPoolTask_h__