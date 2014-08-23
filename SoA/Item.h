#pragma once

enum itemType {
    ITEM_BLOCK, ITEM_WEAPON, ITEM_ARMOR, ITEM_CONSUMABLE, ITEM_MATERIAL, ITEM_MISC
};
const nString itemTypeNames[] = { "Block", "Weapon", "Armor", "Consumable", "Material", "Misc" };
class Item {
public:
    Item() {}
    Item(Item *item) {
        name = item->name;
        type = item->type;
        ID = item->ID;
        bound = item->bound;
        count = item->count;
        value = item->value;
        weight = item->weight;
        maxDurability = item->maxDurability;
        stackable = item->stackable;
    }
    Item(i32 id, i32 Count, nString Name, i32 Type, f32 Weight, f32 MaxDurability, f32 Value) {
        ID = id;
        count = Count;
        name = Name;
        type = Type;
        weight = Weight;
        maxDurability = MaxDurability;
        bound = -1;
        value = Value;
        stackable = 1;
    }

    nString name;
    i32 type;
    i32 ID;
    i32 bound;
    i32 count;
    f32 value;
    f32 weight;
    f32 maxDurability;
    bool stackable;
};