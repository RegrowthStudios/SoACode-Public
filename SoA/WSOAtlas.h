#pragma once
class WSOData;

#include <Vorb/types.h>
/************************************************************************/
/* WSO File Specification                                               */
/* FileSpecs\WSO.txt                                                    */
/************************************************************************/

class WSOAtlas {
public:
    WSOAtlas();
    ~WSOAtlas();

    // Add Data To Atlas (But Memory Is Not Associated With It)
    void add(WSOData* data);

    // Load WSO Data From A File
    void load(const cString file);

    // Destroy All Memory Associated With This Atlas
    void clear();

    // The Number Of Data In The Atlas
    ui32 getSize() const {
        return _data.size();
    }

    // Retrieve Data From The Atlas
    WSOData* get(const size_t& index) const {
        return _data[index];
    }
    WSOData* operator[] (const size_t& index) const {
        return get(index);
    }
    WSOData* get(nString name) const {
        return _mapName.at(name);
    }
    WSOData* operator[] (nString name) const {
        return get(name);
    }
private:
    // Pointers To All The Data
    std::vector<WSOData*> _data;

    // Access Data By A Name
    std::map<nString, WSOData*> _mapName;

    // Pointers To Allocated Memory Blocks
    std::vector<void*> _allocatedMem;
};