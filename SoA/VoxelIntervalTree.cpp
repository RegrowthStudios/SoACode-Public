#include "VoxelIntervalTree.h"

template <typename T>
void VoxelIntervalTree::insert(ui16 index, T data) {
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
        if (newIndex == _tree[node.parent].right && node.parent == grandparent->left) {
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