///
/// ComponentTable.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 10 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ComponentTable_h__
#define ComponentTable_h__

#include "ComponentTableBase.h"

namespace vorb {
    namespace core {
        template<typename T>
        class ComponentTable : public ComponentTableBase {
        public:
            ComponentTable(T defaultData) : ComponentTableBase(),
                _defaultData(defaultData) {
                // Empty
            }
        
            virtual void addComponent(ui64 eID) override {
                _components.insert(std::pair<ui64, T>(eID, _defaultData));
            }
            virtual void removeComponent(ui64 eID) override {
                _components.erase(_components.find(eID));
            }
        
            T& get(ui64 eID) {
                return _defaultData.at(eID);
            }
        protected:
            std::map<ui64, T> _components;
            T _defaultData;
        };
    }
}
namespace vcore = vorb::core;

#endif // ComponentTable_h__