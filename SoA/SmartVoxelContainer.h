#include "VoxelIntervalTree.h"
#include "Constants.h"

enum class VoxelStorageState { FLAT_ARRAY, INTERVAL_TREE };

template <typename T>
class SmartVoxelContainer {
public:

    friend class Chunk;

    SmartVoxelContainer() : _state(VoxelStorageState::INTERVAL_TREE), _dataArray(nullptr) {}

    inline T get(ui16 index) const {
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            return _dataTree.getData(index);
        } else { //_state == FLAT_ARRAY
            return _dataArray[index];
        }
    }

    inline void set(ui16 index, T value) {
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            _dataTree.insert(index, value);
        } else { //_state == FLAT_ARRAY
            _dataArray[index] = value;
        }
    }

    //Creates the tree using a sorted array of data. The number of voxels should add up to 32768
    inline void initFromSortedArray(const std::vector <typename VoxelIntervalTree<T>::LightweightNode>& data) {
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            _dataTree.createFromSortedArray(data);
        } else {
            _dataArray = new T[CHUNK_SIZE];
            int index = 0;
            for (int i = 0; i < data.size(); i++) {
                for (int j = 0; j < data[i].length; j++) {
                    _dataArray[index++] = data[i].data;
                }
            }
        }
    }

    inline void clear() {
        _dataTree.clear();
    }

    void uncompressIntoBuffer(T* buffer) { _dataTree.uncompressIntoBuffer(buffer); }

    VoxelStorageState getState() { return _state; }

    T* getDataArray() { return _dataArray; }

private:
    VoxelIntervalTree<T> _dataTree;
    T* _dataArray;

    VoxelStorageState _state;
};