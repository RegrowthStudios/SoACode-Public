#pragma once
#include "stdafx.h"
#include <map>

//Very rough implementation of a specialized interval tree
//TODO: Replace std::map with a custom red-black tree implementation or something similar

template <typename T>
class IntervalTree {
public:
    
    struct Node {
        Node(T Data, ui16 Length) : data(Data), length(Length) {}
        T data;
        ui16 length;
    };

    inline void clear() {
        std::map<ui16, Node>().swap(_tree);
    }

    inline T getData(ui16 index) const {
        typename std::map<ui16, Node>::const_iterator it = _tree.lower_bound(index);

        if (it == _tree.end()) {
          
            it--;
            assert(it != _tree.end());
        
            return it->second.data;
        } else {
            if (it->first != index) {
                return (--it)->second.data;
            }
            return it->second.data;
        }
    }

    inline typename std::map<ui16, Node>::iterator getInterval(ui16 index) {
        typename std::map<ui16, Node>::iterator it = _tree.lower_bound(index);
        if (it->first != index) {
            return --it;
        }
        return it;
    }

    inline typename std::map<ui16, Node>::iterator insert(std::pair<ui16, Node> pair) {
        auto npair = _tree.insert(pair);
        assert(npair.second == true);
        return npair.first;
    }

    typename std::map<ui16, Node>::iterator insert(ui16 index, T data) {
        //Find containing interval
        typename std::map<ui16, Node>::iterator it = _tree.lower_bound(index);
        if (it == _tree.end() || it->first != index) {
            --it;
        }
        //Its already in the tree
        if (it->second.data == data) {
            return it;
        }

        //If the interval is only 1, just change its value
        if (it->second.length == 1) {
            it->second.data = data;
            return it;
        }

        //Check if its at the right edge of the interval
        if (index == it->first + it->second.length - 1) {

            typename std::map<ui16, Node>::iterator next = it;
            next++;
            //if it should add to the next interval
            if (next != _tree.end() && data == next->second.data) {
                it->second.length--;
                it = _tree.insert(std::pair<ui16, Node>(index, Node(data, next->second.length + 1))).first;
                _tree.erase(next);
                return it;
            } else { //else we should add an interval and modify left interval
                //Modify left interval
                it->second.length--;
                //Insert new interval
                return _tree.insert(std::pair<ui16, Node>(index, Node(data, 1))).first;
            }
        } else if (index == it->first) { //if its at the beginning of the interval
            //Insert right interval
            _tree.insert(std::pair<ui16, Node>(index + 1, Node(it->second.data, it->second.length - 1))).first;
            _tree.erase(it);
            _tree.insert(std::pair<ui16, Node>(index, Node(data, 1))).first;
        } else { //else its in the middle of the interval
            //Insert right interval
            _tree.insert(std::pair<ui16, Node>(index + 1, Node(it->second.data, it->second.length - (index - it->first) - 1))).first;
            //Modify left interval
            it->second.length = index - it->first;
            //Insert new middle interval
            return _tree.insert(std::pair<ui16, Node>(index, Node(data, 1))).first;
        }
        return _tree.end();
    }

    inline typename std::map<ui16, Node>::iterator begin() { return _tree.begin(); }
    inline typename std::map<ui16, Node>::iterator end() { return _tree.end(); }

private:
    typename std::map <ui16, Node> _tree;
};