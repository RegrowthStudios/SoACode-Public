#pragma once

template<typename T>
class PtrRecycler {
public:
    PtrRecycler() {}
    ~PtrRecycler() {
        freeAll();
    }

    T* create() {
        T* data;
        if (_recycled.size() > 0) {
            // Get A Recycled Data
            data = _recycled[_recycled.size() - 1];

            // Update Lists
            _recycled.pop_back();
            *(_recycledChecks[(ui32)data]) = 0;
        } else {
            // Create A New Data Segment
            PtrBind* bind = new PtrBind();
            data = &bind->data;
            bind->recycleCheck = 0;

            // Add The Data Checks
            _allocated.push_back(bind);
            _recycledChecks[(ui32)data] = &bind->recycleCheck;
        }
        return data;
    }
    void recycle(T* data) {
        i32* recycleCheck = _recycledChecks[(ui32)data];

        // Make Sure It Hasn't Already Been Recycled
        if (*recycleCheck == 0) {
            _recycled.push_back(data);
            *recycleCheck = 1;
        }
    }

    void freeAll() {
        if (_allocated.size() > 0) {
            // Free All Allocated Memory
            for (i32 i = _allocated.size() - 1; i >= 0; i--) {
                delete _allocated[i];
            }
            
            // Empty Out The Lists
            _allocated.swap(std::vector<PtrBind*>());
            _recycledChecks.swap(std::map<ui32, i32*>());
            _recycled.swap(std::vector<T*>());
        }
    }
private:
    struct PtrBind {
    public:
        PtrBind() : data(T()), recycleCheck(0) {}

        T data;
        i32 recycleCheck;
    };

    std::vector<PtrBind*> _allocated;
    std::map<ui32, i32*> _recycledChecks;
    std::vector<T*> _recycled;
};