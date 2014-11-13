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

namespace vorb {
    namespace core {

        class WorkerData;

        class IThreadPoolTask {
        public:
            /// Constructor
            /// @param taskId: Optional unique identifier for task type.
            IThreadPoolTask(bool shouldAddToFinishedTasks = false, i32 taskId = -1) :
                _shouldAddToFinishedTasks(shouldAddToFinishedTasks),
                _taskId(taskId) {
                /* Empty */
            }

            virtual ~IThreadPoolTask() { /* Empty */ }

            /// Executes the task
            virtual void execute(WorkerData* workerData) = 0;

            /// Checks if this should be stored in a finished tasks queue
            virtual const bool& shouldAddToFinishedTasks() const { return _shouldAddToFinishedTasks; }

            /// Setters
            void setIsFinished(bool isFinished) { _isFinished = isFinished; }
            void setShouldAddToFinishedtasks(bool shouldAdd) { _shouldAddToFinishedTasks = shouldAdd; }

            /// Getters
            const i32& getTaskId() const { return _taskId; }
            const bool& getIsFinished() const { return _isFinished; }

        protected:
            i32 _taskId;
            bool _isFinished = false;
            bool _shouldAddToFinishedTasks; ///< SHould it be stored in a finished tasks queue
        };
    }
}

// Namespace alias
namespace vcore = vorb::core;

#endif // ThreadPoolTask_h__