#pragma once
#include "stdafx.h"
#include <map>

//Implementation of a specialized interval tree based on a red-black tree
//Red black tree: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree

//NOTE: THIS INTERVAL TREE CANNOT HAVE MORE THAN 32768 INTERVALS
template <typename T>
class IntervalTree {
public:

    IntervalTree() : _root(0) {
    
    }
    
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

    inline void clear() {
        std::vector<Node>().swap(_tree);
        _root = 0;
        _tree.push_back(Node(0, 32768));
    } 
    inline T getData(ui16 index) const {
        return _tree[getInterval(index)].data;
    }

    //Get the enclosing interval for a given point
    inline i16 getInterval(ui16 index) {
        int interval = _root;
        while (true) {
            Node& node = _tree[interval];
            if (index < node.start) { //check for go left
                interval = node.left;
            } else if (index < node.start + node.length) { //check for at interval
                return interval;
            } else { //go right
                interval = node.right;
            }
        }
    }

    void insert(ui16 index, T data) {
        int newIndex = treeInsert(index, data);
        if (newIndex == -1) return;

        Node* grandparent;
        
        //Balance the tree
        while (true) {
            Node& node = _tree[newIndex];

            //Case 1: Current node is root
            if (node.parent == -1) paintBlack(node);

            //Case 2: Parent is black. 
            if ((_tree[node.parent].length & COLOR_BIT) == 0) return;

            //Case 3: Both parent and uncle are red
            int uncleIndex = getUncle(index, grandparent);
            if (uncleIndex != -1) {
                Node& uncle = _tree[getUncle(index)];
                if (isRed(uncle)) {
                    paintBlack(node.parent);
                    paintBlack(uncle);
                    paintRed(grandparent);
                    newIndex = uncle.parent; //now we operate on the grandparent and start over
                    continue;
                }
            }

            //Case 4: Parent is red, uncle is black. 
            if (newIndex == _tree[node.parent].right && node.parent = grandparent->left) {
                rotateParentLeft(newIndex, grandparent);
                newIndex = node.left;
                node = _tree[newIndex];
                grandparent = &_tree[_tree[node.parent].parent];
            } else if (newIndex == _tree[node.parent].left && node.parent == grandparent->right) {
                rotateParentRight(newIndex, grandparent);
                newIndex = node.right;
                node = _tree[newIndex];
                grandparent = &_tree[_tree[node.parent].parent];
            }

            //Case 5 Parent is red, Uncle is black.
            colorBlack(node.parent);
            colorRed(grandparent);
            if (newIndex == _tree[node.parent].left) {
                rotateRight(_tree[node.parent].parent);
            } else {
                rotateLeft(_tree[node.parent].parent);
            }
            return;
        }
    }

    Node& operator[](int index) { return _tree[index]; }
    int size() const { return _tree.size(); }
    
private:

    inline int treeInsert(int index, T data) {
        int interval = _root;
        while (true) {
            Node& node = _tree[interval];
            if (index < node.start) { //go left
                //Check if we are at the leaf
                if (node.left == -1) {
                    //We are currently in node.parent's interval, check if data is the same
                    if (node.parent != -1 && _tree[node.parent].data == data) { //if the data is the same, nothing happens
                        return -1;
                    } else if (index == node.start - 1 && data == node.data) {
                        --node.start;
                        ++node.length;
                        return -1;
                    }
                    node.left = _tree.size();
                    _tree.insert(Node(data, 1));
                    _tree.back().parent = node.left;
                    return node.right;
                }
                interval = node.left;
            } else if (index > node.start) { //go right
                //Check if we are at the leaf
                if (node.right == -1) {
                    //We are currently in node's interval, check if data is the same.
                    if (node.data == data) { //if data is the same, nothing happens
                        return -1;
                    }
                    node.right = _tree.size();
                    _tree.insert(Node(data, 1));
                    _tree.back().parent = node.right;
                    return node.right;
                }
                interval = node.right;
            } else { //we are at the start of a current interval
                if (node.data == data) { //do nothing if data is the same
                    return -1;
                }
                if ((node.length & LENGTH_MASK) == 1) { //if we are replacing a single block, just change the data
                    node.data = data;
                    return -1;
                }
                //move the interval over to make room for the new node
                ++node.start;
                --node.length;
                //We are in node.parent's interval, so check to see if we are the same data
                if (node.parent != -1 && _tree[node.parent].data == data) {
                    ++(_tree[node.parent].length); //increase the length by 1 and return
                    return -1;
                }

                //Check for if we are at root
                if (node.left == -1) {
                    node.left = _tree.size();
                    _tree.insert(Node(data, 1));
                    _tree.back().parent = node.left;
                    return node.left;
                }
                //If we get here, then we must continue along to the left to do the proper insertion
                interval = node.left;
            }
        }
    }

    inline int getGrandparent(Node& node)
    {
        if (node.parent != -1) {
            return _tree[node.parent].parent;
        } else {
            return -1;
        }
    }

    inline int getUncle(Node& node, Node* grandParent)
    {
        int grandparentIndex = grandparent(node);
        if (grandparentIndex == -1) {
            return -1; // No grandparent means no uncle
        }

        grandParent = &_tree[grandparentIndex];

        if (node.parent == grandParent->left) {
            return grandParent->right;
        } else {
            return grandParent->left;
        }
    }

    inline void paintRed(Node* node) {
        node->length |= COLOR_BIT;
    }

    inline void paintBlack(int index) { _tree[index].length &= LENGTH_MASK; }

    inline void paintBlack(Node& node) { node.length &= LENGTH_MASK; }

    inline bool isRed(Node& node) { return (node.length & COLOR_BIT) != 0; }

    inline void rotateParentRight(int index, Node* grandParent) {
        Node& node = _tree[index];
        i16 parent = node.parent, left = node.left;

        grandParent->left = index;
        node.left = parent;
        _tree[parent].right = left;
    }

    inline void rotateParentLeft(int index, Node* grandParent) {
        Node& node = _tree[index];
        i16 parent = node.parent, right = node.right;

        grandParent->right = index;
        node.right = parent;
        _tree[parent].left = right;
    }

    inline void rotateRight(int index) {

        Node& node = _tree[index];
        Node& left = _tree[node.left];
        Node& parent = _tree[node.parent];

        i16 right = left.right;
        left.right = index;
        left.parent = node.parent;
        if (parent.left == index) {
            parent.left = node.left;
        } else {
            parent.right = node.left;
        }
        node.left = right;
        _tree[right].parent = index;
    }

    inline void rotateLeft(int index) {

        Node& node = _tree[index];
        Node& right = _tree[node.right];
        Node& parent = _tree[node.parent];

        i16 left = right.left;
        right.left = index;
        right.parent = node.parent;
        if (parent.right == index) {
            parent.right = node.right;
        } else {
            parent.left = node.right;
        }
        node.right = left;
        _tree[left].parent = index;
    }

    int _root;
    std::vector <Node> _tree;
};