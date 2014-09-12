#pragma once
#include <map>
#include <vector>

//Implementation of a specialized interval tree based on a red-black tree
//Red black tree: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree

//NOTE: THIS INTERVAL TREE CANNOT HAVE MORE THAN 32768 INTERVALS
template <typename T>
class VoxelIntervalTree {
public:

    VoxelIntervalTree() : _root(-1) {}

#define COLOR_BIT 0x8000
#define START_MASK 0x7FFF
    struct Node {
        Node(T Data, ui16 start, ui16 Length) : data(Data), _start(start | COLOR_BIT), length(Length), left(-1), right(-1), parent(0) {}

        inline void incrementStart() { ++_start; }
        inline void decrementStart() { --_start; }
        inline ui16 getStart() const { return _start & START_MASK; }
        inline void setStart(ui16 Start) { _start = (_start & COLOR_BIT) | Start; }
        inline void paintRed() { _start |= COLOR_BIT; }
        inline void paintBlack() { _start &= START_MASK; }
        inline bool isRed() const { return (_start & COLOR_BIT) != 0; }

        ui16 length;
        i16 left;
        i16 right;
        i16 parent;
    private:
        ui16 _start; //also stores color
    public:
        T data;
    };

    bool checkValidity() const {
        int tot = 0;
        for (int i = 0; i < _tree.size(); i++) {
            if (_tree[i].length > 32768) {
                std::cout << "AHA";
                fflush(stdout);
            }
            tot += _tree[i].length;
        }
        if (tot != 32768) {
            std::cout << "rofl";
            fflush(stdout);
        }
        return false;
    }

    void clear();

    T getData(ui16 index) const;
    //Get the enclosing interval for a given point
    i16 getInterval(ui16 index) const;
    //Begin the tree
    Node* insertFirst(T data, ui16 length);

    Node* insert(ui16 index, T data);

    inline Node& operator[](int index) { return _tree[index]; }
    inline int size() const { return _tree.size(); }

private:

    bool treeInsert(int index, T data, int &newIndex);

    int getGrandparent(Node& node);

    int getUncle(Node& node, Node** grandParent);

    void rotateParentRight(int index, Node* grandParent);

    void rotateParentLeft(int index, Node* grandParent);

    void rotateRight(int index);

    void rotateLeft(int index);

    int _root;
    std::vector <Node> _tree;

    struct NodeToAdd {
        NodeToAdd(i16 Parent, ui16 Length, T Data) : parent(Parent), length(Length), data(Data) {}
        i16 parent;
        ui16 length;
        T data;
    };
    
    std::vector <NodeToAdd> _nodesToAdd;
    std::vector <ui16> _nodesToRemove;
};

#include "VoxelIntervalTree.inl"