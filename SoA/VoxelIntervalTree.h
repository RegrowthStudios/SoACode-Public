#pragma once
#include <map>
#include <vector>
#include "types.h"

//Implementation of a specialized interval tree based on a red-black tree
//Red black tree: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree

//NOTE: THIS INTERVAL TREE CANNOT HAVE MORE THAN 32768 INTERVALS
template <typename T>
class VoxelIntervalTree {
public:

    VoxelIntervalTree() : _root(-1) {}

#define COLOR_BIT 0x8000
#define START_MASK 0x7FFF

    //Node for createFromArray
    struct LightweightNode {
        LightweightNode(ui16 Start, ui16 Length, T Data) : start(Start), length(Length), data(Data) {}
        ui16 start;
        ui16 length;
        T data;
    };

    struct Node {
        Node() {}
        Node(T Data, ui16 start, ui16 Length) : data(Data), _start(start | COLOR_BIT), length(Length), left(-1), right(-1), parent(-1) {}

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

    bool checkTreeValidity() const {
        int tot = 0;
        for (size_t i = 0; i < _tree.size(); i++) {
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

        if (_tree[_root].isRed()) {
            std::cout << "ROOT IS RED!\n";
            fflush(stdout);
        }
        checkValidRB();
        return true;
    }

    bool checkValidRB() const {
        //    if (_tree[_root].isRed()) {
        //        std::cout << "ROOT IS RED!\n";
        //        fflush(stdout);
        //    }
        if (_tree[_root].parent != -1) {
            std::cout << "ROOT HAS A PARENT!\n";
            fflush(stdout);
        }
        for (int i = 0; i < size(); i++) {
            if (_tree[i].parent == i) {
                std::cout << "Node is own parent!";
            }
            if (_tree[i].parent == i) {
                std::cout << "Node is own parent!";
            }
            if (_tree[i].left == i) {
                std::cout << "Node is own left!";
            }
            if (_tree[i].right == i) {
                std::cout << "Node is own right!";
            }
            if (_tree[i].parent != -1) {
                if (_tree[_tree[i].parent].left != i && _tree[_tree[i].parent].right != i) {
                    std::cout << "Node is disconnected from parent!\n";
                }
            }
        }

        return true;
    }

    void clear();

    void createFromSortedArray(const std::vector <LightweightNode>& data) {
        _tree.resize(data.size());
        for (int i = 0; i < _tree.size(); i++) {
            _tree[i].setStart(data[i].start);
            _tree[i].length = data[i].length;
            _tree[i].data = data[i].data;
        }
        _root = arrayToRedBlackTree(0, data.size() - 1, -1, true);
    }

    T getData(ui16 index) const;
    //Get the enclosing interval for a given point
    i16 getInterval(ui16 index) const;
    //Begin the tree
    Node* insertFirst(T data, ui16 length);

    Node* insert(ui16 index, T data);

    void uncompressTraversal(int index, int& bufferIndex, T* buffer);
    void uncompressIntoBuffer(T* buffer);

    inline Node& operator[](int index) { return _tree[index]; }
    inline int size() const { return _tree.size(); }

private:

    int arrayToRedBlackTree(int i, int j, int parent, bool isBlack) {
        if (i > j) return -1;

        int x = (j + i) / 2;

        Node& node = _tree[x];
        node.parent = parent;

        if (isBlack) {
            node.paintBlack();
        } else{
            node.paintRed();
        }

        if (i < j) {
            node.left = arrayToRedBlackTree(i, x - 1, x, !isBlack);
            node.right = arrayToRedBlackTree(x + 1, j, x, !isBlack);
        } else {
            node.left = -1;
            node.right = -1;
        }
        return x;
    }

    bool treeInsert(int index, T data, int &newIndex);

    int getGrandparent(Node* node);

    int getUncle(Node* node, Node** grandParent);

    void rotateParentRight(int index, Node* grandParent);

    void rotateParentLeft(int index, Node* grandParent);

    void rotateRight(int index);

    void rotateLeft(int index);


    int _root;
    std::vector <Node> _tree;

    struct NodeToAdd {
        NodeToAdd(i16 Parent, ui16 Length, T Data) : start(Parent), length(Length), data(Data) {}
        i16 start;
        ui16 length;
        T data;
    };

    std::vector <NodeToAdd> _nodesToAdd;
    std::vector <ui16> _nodesToRemove;
};

#include "VoxelIntervalTree.inl"