#include "VoxelIntervalTree.h"
#include "Constants.h"

enum class VoxelStorageState { FLAT_ARRAY, INTERVAL_TREE };

template <typename T>
class SmartVoxelContainer {
public:

    friend class Chunk;

    SmartVoxelContainer() : _dataArray(nullptr), _accessCount(0), _quietFrames(0), _state(VoxelStorageState::FLAT_ARRAY) {}

    inline T get(ui16 index) const {
       // _accessCount++;
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            return _dataTree.getData(index);
        } else { //_state == FLAT_ARRAY
            return _dataArray[index];
        }
    }

    inline void set(ui16 index, T value) {
        _accessCount++;
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            _dataTree.insert(index, value);
        } else { //_state == FLAT_ARRAY
            _dataArray[index] = value;
        }
    }

    inline void init(VoxelStorageState state) {
        _state = state;
        if (_state == VoxelStorageState::FLAT_ARRAY) {
            _dataArray = new T[CHUNK_SIZE];
        }
    }

    //Creates the tree using a sorted array of data. The number of voxels should add up to 32768
    inline void initFromSortedArray(VoxelStorageState state, const std::vector <typename VoxelIntervalTree<T>::LightweightNode>& data) {
        _state = state;
        _accessCount = 0;
        _quietFrames = 0;
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

#define QUIET_FRAMES_UNTIL_COMPRESS 10
#define ACCESS_COUNT_UNTIL_DECOMPRESS 20

    inline void update() {
        if (_accessCount >= ACCESS_COUNT_UNTIL_DECOMPRESS) {
            _quietFrames = 0;
        } else {
            _quietFrames++;
        }

        if (_state == VoxelStorageState::INTERVAL_TREE) {
            if (_quietFrames == 0) {
                _dataArray = new T[CHUNK_SIZE];
                uncompressIntoBuffer(_dataArray);
                // Free memory
                _dataTree.clear();
                // Set the new state
                _state = VoxelStorageState::FLAT_ARRAY;
            }
        } else {
            if (_quietFrames >= QUIET_FRAMES_UNTIL_COMPRESS) {
                std::vector<VoxelIntervalTree<T>::LightweightNode> dataVector;
                dataVector.reserve(CHUNK_WIDTH / 2);
                dataVector.emplace_back(0, 1, _dataArray[0]);
                for (int i = 1; i < CHUNK_SIZE; i++) {
                    if (_dataArray[i] == dataVector.back().data) {
                        dataVector.back().length++;
                    } else {
                        dataVector.emplace_back(i, 1, _dataArray[i]);
                    }
                }
                // Free memory
                delete[] _dataArray;
                _dataArray = nullptr;
                // Set new state
                _state = VoxelStorageState::INTERVAL_TREE;
                // Create the tree
                _dataTree.createFromSortedArray(dataVector);
            }
        }
        _accessCount = 0;
    }

    inline void clear() {
        if (_state == VoxelStorageState::INTERVAL_TREE) {
            _dataTree.clear();
        } else {
            delete[] _dataArray;
            _dataArray = nullptr;
        }
    }

    inline void uncompressIntoBuffer(T* buffer) { _dataTree.uncompressIntoBuffer(buffer); }

    VoxelStorageState getState() { return _state; }

    T* getDataArray() { return _dataArray; }

public: //TEMP SHOULD BE PRIVATE
     VoxelIntervalTree<T> _dataTree;
    T* _dataArray;
    int _accessCount; ///< Number of times the container was accessed this frame
    int _quietFrames; ///< Number of frames since we have had heavy updates

    VoxelStorageState _state;
};