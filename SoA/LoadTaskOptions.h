#pragma once
#include "Options.h"
#include "LoadMonitor.h"

// Sample Dependency Task
class LoadTaskOptions : public ILoadTask {
    virtual void load() {
        initializeOptions();
        loadOptions();
    }
};