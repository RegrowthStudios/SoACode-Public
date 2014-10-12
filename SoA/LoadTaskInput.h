#pragma once
#include "LoadMonitor.h"

// Sample IO Task
class LoadTaskInput : public ILoadTask {
    virtual void load();
};