#pragma once
#include "VoxelIntervalTree.h"

template <typename T>
void VoxelIntervalTree<typename T>::clear() {
    std::vector<Node>().swap(_tree);
    std::vector<NodeToAdd>().swap(_nodesToAdd);
    std::vector<ui16>().swap(_nodesToRemove);
    _root = -1;
}

template <typename T>
 T VoxelIntervalTree<typename T>::getData(ui16 index) const {
     return _tree[getInterval(index)].data;
}

//Get the enclosing interval for a given point
template <typename T>
 i16 VoxelIntervalTree<typename T>::getInterval(ui16 index) const {
    int interval = _root;
    while (true) {
        if (interval == -1) {
            printf("HERE BEN YOU DOLT");
            checkValidity();
            for (int i = 0; i < _tree.size(); i++) {
                if (interval >= _tree[i].getStart() && interval < _tree[i].getStart() + _tree[i].length) {
                    
                    printf("\nTHE NODE WORKS AT %d\n", i);
                    break;
                }
            }
            fflush(stdout);
        }
        const Node& node = _tree[interval];
        if (index < node.getStart()) { //check for go left
            interval = node.left;
        } else if (index < node.getStart() + node.length) { //check for at interval
            return interval;
        } else { //go right
            interval = node.right;
        }
    }
}

template <typename T>
 bool VoxelIntervalTree<typename T>::treeInsert(int index, T data, int &newIndex) {
    int interval = _root;
    Node* enclosingInterval = nullptr;
    int enclosingIndex = -1;
    while (true) {
        Node& node = _tree[interval];
    
       
        if (index < (int)node.getStart()) { //go left
            //Check if we are at the leaf
            if (node.left == -1) {
                //check if we are right before the current node               
                if (index == node.getStart() - 1) {

                    if (enclosingInterval) {
                        --(enclosingInterval->length);
                    }

                    //If data is the same, simply combine with the node
                    if (data == node.data) {
                        node.decrementStart();
                        ++(node.length);
                        newIndex = interval;
                        return false;
                    }      

                    node.left = _tree.size();
                    newIndex = node.left;
                    _tree.push_back(Node(data, index, 1));
                    _tree.back().parent = interval;
                    return true;
                }
                //If we get here, we are splitting an interval

                node.left = _tree.size();
                newIndex = node.left;

                //is this check even necesarry?
                if (enclosingInterval) {
                    enclosingInterval->length = index - enclosingInterval->getStart();
                    //We need to add a new node to the tree later
                    _nodesToAdd.push_back(NodeToAdd(node.left, node.getStart() - index - 1, enclosingInterval->data));
                }

                _tree.push_back(Node(data, index, 1));
                _tree.back().parent = interval;
                return true;
            }
            interval = node.left;

        }else if (index < node.getStart() + node.length) { //we are in the nodes interval
            if (node.data == data) {
                newIndex = interval;
                return false;
            } else if (node.getStart() == index) { //check if we are at the start of the interval
                //check for interval replacement
                if (node.length == 1) {
                    node.data = data;
                    newIndex = interval;
                    return false;
                }

                //move the interval over to make room for the new node
                node.incrementStart();
                --(node.length);


                //Check for if we are at leaf
                if (node.left == -1) {
                    node.left = _tree.size();
                    newIndex = node.left;
                    _tree.push_back(Node(data, index, 1));
                    _tree.back().parent = interval;
                    return true;
                }

                //TODO: We may have passed the interval.
                //If we get here, then we must continue along to the left to do the proper insertion
                interval = node.left;
            } else {
                enclosingIndex = interval;
                enclosingInterval = &node;
                //go right
                //Check if we are at the leaf
                if (node.right == -1) {

                    node.right = _tree.size();
                    newIndex = node.right;
                    
                    if (index == node.getStart() + node.length - 1) { //at the edge of the interval
                        --(enclosingInterval->length);
                    } else { //splitting the interval
                        _nodesToAdd.push_back(NodeToAdd(node.right, node.getStart() + node.length - index - 1, enclosingInterval->data));
                        enclosingInterval->length = index - enclosingInterval->getStart();
                    }
                   
                    _tree.push_back(Node(data, index, 1));
                    _tree.back().parent = interval;
                    return true;
                }
                interval = node.right;
            }
        } else { //go right
            //Check if we are at the leaf
            if (node.right == -1) {
                node.right = _tree.size();
                newIndex = node.right;
                _tree.push_back(Node(data, index, 1));
                _tree.back().parent = interval;               
                return true;
            }
            interval = node.right;
        }
    }
}

template <typename T>
 int VoxelIntervalTree<typename T>::getGrandparent(Node& node)
{
    if (node.parent != -1) {
        return _tree[node.parent].parent;
    } else {
        return -1;
    }
}

template <typename T>
 int VoxelIntervalTree<typename T>::getUncle(Node& node, Node** grandParent)
{
    int grandparentIndex = getGrandparent(node);
    if (grandparentIndex == -1) {
        return -1; // No grandparent means no uncle
    }

    Node* g = &_tree[grandparentIndex];
    *grandParent = g;

    if (node.parent == g->left) {
        return g->right;
    } else {
        return g->left;
    }
}

template <typename T>
 void VoxelIntervalTree<typename T>::rotateParentLeft(int index, Node* grandParent) {
    Node& node = _tree[index];
    i16 parent = node.parent, left = node.left;

    grandParent->left = index;
    node.left = parent;
    _tree[parent].right = left;
}

template <typename T>
 void VoxelIntervalTree<typename T>::rotateParentRight(int index, Node* grandParent) {
    Node& node = _tree[index];
    i16 parent = node.parent, right = node.right;

    grandParent->right = index;
    node.right = parent;
    _tree[parent].left = right;
}

template <typename T>
 void VoxelIntervalTree<typename T>::rotateRight(int index) {
 
    Node& node = _tree.at(index);
    Node& left = _tree.at(node.left);
    Node& parent = _tree.at(node.parent);

    i16 right = left.right;
    left.right = index;
    left.parent = node.parent;
    node.parent = node.left;
    node.left = -1;
    if (parent.left == index) {
        parent.left = node.left;
    } else {
        parent.right = node.left;
    }
    node.left = right;
    if (right != -1) {
        _tree[right].parent = index;
    }
 
}

template <typename T>
void VoxelIntervalTree<typename T>::rotateLeft(int index) {

    Node& node = _tree.at(index);
    Node& right = _tree.at(node.right);
    Node& parent = _tree.at(node.parent);
    i16 left = right.left;
    right.left = index;
    right.parent = node.parent;
    node.parent = node.right;
    node.right = -1;
    if (parent.right == index) {
        parent.right = node.right;
    } else {
        parent.left = node.right;
    }

    node.right = left;
    if (left != -1) {
        _tree[left].parent = index;
    }
}
template <typename T>
typename VoxelIntervalTree<typename T>::Node* VoxelIntervalTree<typename T>::insertFirst(T data, ui16 length) {
    _root = 0;
    _tree.push_back(Node(data, 0, length));
    _tree[0].paintBlack();
    if (_tree.back().isRed()) {
        cout << "THATS BAD MKAY\n";
    }
    return &_tree.back();
}

template <typename T>
typename VoxelIntervalTree<typename T>::Node* VoxelIntervalTree<typename T>::insert(ui16 index, T data) {

    int newIndex;
    if (!treeInsert(index, data, newIndex)) {
        return &_tree[newIndex];
    }
    Node* grandparent;

    //Balance the tree
    while (true) {
        Node& node = _tree[newIndex];

        //Case 1: Current node is root
        if (node.parent == -1) {
            node.paintBlack();
            return &node;
        }

        //Case 2: Parent is black.
        if (_tree[node.parent].isRed() == false) return &node;

        //Case 3: Both parent and uncle are red
        int uncleIndex = getUncle(node, &grandparent);
        if (uncleIndex != -1) {
            Node& uncle = _tree[uncleIndex];
            if (uncle.isRed()) {
                _tree[node.parent].paintBlack();
                uncle.paintBlack();
                grandparent->paintRed();
                newIndex = uncle.parent; //now we operate on the grandparent and start over
                continue;
            }
        }
        int dbg = 4;
        int npar = node.parent;
        int nl = node.left;
        int nr = node.right;
        //Case 4: Parent is red, uncle is black.
        if (newIndex == _tree[node.parent].right && node.parent == grandparent->left) {
            //rotateParentLeft(newIndex, grandparent);
            rotateLeft(node.parent);
            newIndex = node.left;
            dbg = 5;
            node = _tree[newIndex];
            grandparent = &_tree[_tree[node.parent].parent];
        } else if (newIndex == _tree[node.parent].left && node.parent == grandparent->right) {
           // rotateParentRight(newIndex, grandparent);
            rotateRight(node.parent);
            newIndex = node.right;
            dbg = 6;
            node = _tree[newIndex];
            grandparent = &_tree[_tree[node.parent].parent];
        }

        //Case 5 Parent is red, Uncle is black.
        _tree[node.parent].paintBlack();
        grandparent->paintRed();
        if (newIndex == _tree[node.parent].left) {
            rotateRight(_tree[node.parent].parent);
        } else {
            rotateLeft(_tree[node.parent].parent);
        }
        break;
    }

    while (_nodesToAdd.size()) {
        NodeToAdd& n = _nodesToAdd.back();
        _tree[n.parent].right = _tree.size();
        i16 parent = n.parent;
        _tree.push_back(Node(n.data, _tree[parent].getStart() + 1, n.length));
        _tree.back().parent = parent;
        _nodesToAdd.pop_back();
    }
    return &_tree[newIndex];
}