#pragma once

#include <Vorb/types.h>
#include <Vorb/Events.hpp>

enum class ItemType {
    NONE, BLOCK, WEAPON, ARMOR, CONSUMABLE, MATERIAL, USABLE, MISC
};

typedef nString ItemIdentifier;
typedef ui32 ItemID;

const nString ITEM_TYPE_STRINGS[] = { "None", "Block", "Weapon", "Armor", "Consumable", "Material", "Usable", "Misc" };

// One or more items. All items are ItemStacks.
struct ItemData {
    ItemIdentifier name = "?";
    ItemType type = ItemType::NONE;
    ItemID id = 0;
    f32 value = 0.0f;
    f32 weight = 0.0f;
    ui32 maxCount = 1; ///< If this is 1, this can only be a single item.
    // Certain types don't need certain data
    union {
        ui32 maxDurability;
        ui32 blockID;
    };
};

// Container for all item types
// TODO(Ben): Save and load
class ItemPack {
public:
    ItemPack();

    ItemID append(ItemData item);

    void reserveID(const ItemIdentifier& sid, ItemID id);

    const ItemData* hasItem(ItemID id) const {
        if (id >= m_itemList.size()) {
            return nullptr;
        } else {
            return &m_itemList[id];
        }
    }
    const ItemData* hasItem(const ItemIdentifier& sid) const {
        auto v = m_itemMap.find(sid);
        if (v == m_itemMap.end()) {
            return nullptr;
        } else {
            return &m_itemList[v->second];
        }
    }

    size_t size() const {
        return m_itemList.size();
    }

    /************************************************************************/
    /* ItemData accessors                                                   */
    /************************************************************************/
    ItemData& operator[](const size_t& index) {
        return m_itemList[index];
    }
    const ItemData& operator[](const size_t& index) const {
        return m_itemList[index];
    }
    ItemData& operator[](const ItemIdentifier& sid) {
        return m_itemList[m_itemMap.at(sid)];
    }
    const ItemData& operator[](const ItemIdentifier& sid) const {
        return m_itemList[m_itemMap.at(sid)];
    }
    ui16 getItemDataIndex(const ItemIdentifier& sid) const {
        return m_itemMap.at(sid);
    }

    const std::unordered_map<ItemIdentifier, ui32>& getItemDataMap() const { return m_itemMap; }
    const std::vector<ItemData>& getItemDataList() const { return m_itemList; }

    Event<ui16> onItemDataAddition; ///< Signaled when a block is loaded

private:
    // TODO(Ben): worry about runtime resizing
    std::unordered_map<ItemIdentifier, ui32> m_itemMap; ///< Item indices organized by identifiers
    std::vector<ItemData> m_itemList; ///< Item data list
};

struct ItemStack {
    ItemID id = 0;
    ui32 count = 0;
    ui32 durability = 0;
    ItemPack* pack = nullptr; ///< Flyweight
};