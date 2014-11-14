///
/// FixedSizeArrayRecycler.hpp
/// Vorb Engine
///
/// Created by Benjamin Arnold on 14 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This class will recycle fixed sized arrays of a specific type.
/// This is very useful for chunk data but may have other uses.

#pragma once

#ifndef FixedSizeArrayRecycler_h__
#define FixedSizeArrayRecycler_h__

template<int N, typename T>
class FixedSizeArrayRecylcer {
public:
    /// Constructor
    /// @param maxSize: The maximum number of arrays to hold at any time.
    FixedSizeArrayRecycler(ui32 maxSize) : _maxSize(maxSize) { /* Empty */ }
    ~FixedSizeArrayRecylcer() { destroy(); }

    /// Frees all arrays
    void destroy() {
        for (auto& array : _arrays) {
            delete[] array;
        }
        std::vector<T*>().swap(_arrays);
    }

    /// Gets an array of type T with size N. May allocate new memory
    /// @return Pointer to the array
    T* create() {
        T* rv;
        if (_arrays.size() {
            rv = _arrays.back();
            _arrays.pop_back();
        } else {
            rv = new T[N];
        }
        return rv;
    }

    /// Recycles an array of type T with size N.
    /// Does not check for double recycles, and does not
    /// check that size is actually N, so be careful.
    /// @param array: The pointer to the array to be recycled.
    /// must be of size N.
    void recycle(T* array) {
        /// Only recycle it if there is room for it
        if (_arrays.size() < _maxSize) {
            _arrays.push_back(array);
        } else {
            delete[] array;
        }
    }

    /// getters
    const size_t& getSize() const { return _arrays.size(); }
private:
    ui32 _maxSize; ///< Maximum number of arrays to hold
    // Stack of arrays
    std::vector<T*> _arrays;
};

#endif // FixedSizeArrayRecycler_h__