#pragma once
#include <map>
#include <vector>

//Implementation of a specialized interval tree based on a red-black tree
//Red black tree: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree

//NOTE: THIS INTERVAL TREE CANNOT HAVE MORE THAN 32768 INTERVALS
template <typename T>
class VoxelIntervalTree {
public:

    VoxelIntervalTree() : _root(0) {}

#define COLOR_BIT 0x8000
#define LENGTH_MASK 0x7FFF
    struct Node {
        Node(T Data, ui16 Length) : start(0), length(Length | COLOR_BIT), left(-1), right(-1), parent(0), data(Data) {}
        ui16 start;
        ui16 length; //also stores color
        i16 left;
        i16 right;
        i16 parent;
        T data;
    };

    void clear();

    T getData(ui16 index) const;
    //Get the enclosing interval for a given point
    i16 getInterval(ui16 index) const;

    Node& insert(ui16 index, T data);

    inline Node& operator[](int index) { return _tree[index]; }
    inline int size() const { return _tree.size(); }

private:

    bool treeInsert(int index, T data, int &newIndex);

    int getGrandparent(Node& node);

    int getUncle(Node& node, Node** grandParent);

    inline void paintRed(Node* node) { node->length |= COLOR_BIT; }

    inline void paintBlack(int index) { _tree[index].length &= LENGTH_MASK; }

    inline void paintBlack(Node& node) { node.length &= LENGTH_MASK; }

    inline bool isRed(Node& node) { return (node.length & COLOR_BIT) != 0; }

    void rotateParentRight(int index, Node* grandParent);

    void rotateParentLeft(int index, Node* grandParent);

    void rotateRight(int index);

    void rotateLeft(int index);

    int _root;
    std::vector <Node> _tree;
};

#include "VoxelIntervalTree.inl"