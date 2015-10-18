#include "stdafx.h"
#include "Item.h"

ItemPack::ItemPack() :
    onItemDataAddition(this) {
    // Add default item
    append(ItemData());
}

ItemID ItemPack::append(ItemData item) {
    const ItemData* curBlock;
    ItemID rv;
    if (curBlock = hasItem(item.id)) {
        rv = curBlock->id;
        item.id = rv;
        // Overwrite block
        *const_cast<ItemData*>(curBlock) = item;
    } else {
        rv = m_itemList.size();
        item.id = rv;
        // Add a new block
        m_itemList.push_back(item);
        // Set the correct index
        m_itemMap[item.name] = rv;
    }
    onItemDataAddition(item.id);
    return rv;
}

void ItemPack::reserveID(const ItemIdentifier& sid, ItemID id) {
    if (id >= m_itemList.size()) m_itemList.resize(id + 1);
    m_itemMap[sid] = id;
    m_itemList[id].id = id;
}
