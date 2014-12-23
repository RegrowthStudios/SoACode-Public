#include "stdafx.h"
#include "InputManager.h"

#include <sstream>

#include <SDL\SDL_keyboard.h>
#include <SDL\SDL_mouse.h>

#include "global.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Inputs.h"

InputManager::InputManager() :
_defaultConfigLocation(DEFAULT_CONFIG_LOCATION) {
    _iniKeys["type"] = 0;
    _iniKeys["defaultPositiveKey"] = 1;
    _iniKeys["defaultNegativeKey"] = 2;
    _iniKeys["positiveKey"] = 3;
    _iniKeys["negativeKey"] = 4;
    _iniKeys["joystickButton"] = 5;
    _iniKeys["joystickAxis"] = 6;
    memset(_currentKeyStates, 0, sizeof(_currentKeyStates));
    memset(_previousKeyStates, 0, sizeof(_previousKeyStates));
}

InputManager::~InputManager() {
    for (unsigned int i = 0; i < _axes.size(); i++) {
        delete _axes[i];
    }
}

f32 InputManager::getAxis(const i32 axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return 0.0f;
    
    Axis* axis = _axes.at(axisID);
    float result = 0;
    switch (axis->type) {
    case AxisType::DUAL_KEY:
        //std::cout << axis->name << " " << axis->positiveKey << " " << axis->negativeKey << " " << currentKeyStates[axis->positiveKey].pr << " " << currentKeyStates[axis->negativeKey].pr << std::endl;
        result += _currentKeyStates[axis->positiveKey] ? 1.0f : 0;
        result -= _currentKeyStates[axis->negativeKey] ? 1.0f : 0;
        break;
    case AxisType::SINGLE_KEY:
        result += _currentKeyStates[axis->positiveKey] ? +1 : 0;
        break;
    case AxisType::JOYSTICK_AXIS:
        //TODO: implement
        break;
    case AxisType::JOYSTICK_BUTTON:
        //TODO: implement
        break;
    }
    return result;
}

bool InputManager::getKey(const i32 axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return _currentKeyStates[axis->positiveKey];
}

bool InputManager::getKeyDown(const i32 axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return _currentKeyStates[axis->positiveKey] && !_previousKeyStates[axis->positiveKey];
}

bool InputManager::getKeyUp(const i32 axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return !_currentKeyStates[axis->positiveKey] && _previousKeyStates[axis->positiveKey];
}

i32 InputManager::createAxis(const nString& axisName, VirtualKey defaultPositiveKey, VirtualKey defaultNegativeKey) {
    i32 id = getAxisID(axisName);
    if (id >= 0) return id;
    id = _axes.size();
    _axisLookup[axisName] = id;
    Axis *axis = new Axis();
    axis->name = axisName;
    axis->type = AxisType::DUAL_KEY;
    axis->defaultPositiveKey = defaultPositiveKey;
    axis->positiveKey = defaultPositiveKey;
    axis->defaultNegativeKey = defaultNegativeKey;
    axis->negativeKey = defaultNegativeKey;
    //Not applicable for this type of axis
    axis->joystick = NULL;
    axis->joystickAxis = -1;
    axis->joystickButton = -1;
    _axes.push_back(axis);
    return id;
}

i32 InputManager::createAxis(const nString& axisName, VirtualKey defaultPositiveKey) {
    i32 id = getAxisID(axisName);
    if (id >= 0) return id;
    id = _axes.size();
    _axisLookup[axisName] = id;
    Axis *axis = new Axis();
    axis->name = axisName;
    axis->type = AxisType::SINGLE_KEY;
    axis->defaultPositiveKey = defaultPositiveKey;
    axis->positiveKey = defaultPositiveKey;
    axis->upEvent = Event<ui32>(this);
    axis->downEvent = Event<ui32>(this);
    //Not applicable for this type of axis
    axis->negativeKey = VKEY_UNKNOWN;
    axis->defaultNegativeKey = VKEY_UNKNOWN;
    axis->joystick = NULL;
    axis->joystickAxis = -1;
    axis->joystickButton = -1;
    _axes.push_back(axis);
    return id;
}

i32 InputManager::getAxisID(const nString& axisName) const {
    auto iter = _axisLookup.find(axisName);

    if (iter != _axisLookup.end()) {
        return iter->second;
    } else {
        return -1;
    }
}

void InputManager::loadAxes(const nString& location) {
    std::vector<std::vector<IniValue>> iniValues;
    std::vector<nString> iniSections;
    if (fileManager.loadIniFile(location, iniValues, iniSections))  return;

    IniValue* iniVal;
    int iniKey;
    Axis* curAxis;
    for (unsigned int i = 1; i < iniSections.size(); i++) {
        if (getAxisID(iniSections[i]) >= 0) continue;
        curAxis = new Axis;
        curAxis->name = iniSections[i];
        for (unsigned int j = 0; j < iniValues[i].size(); j++) {
            iniVal = &iniValues[i][j];
            iniKey = getIniKey(iniVal->key);

            switch (iniKey) {
            case 0: // Type
                curAxis->type = (AxisType)iniVal->getInt();
                break;
            case 1: // Default Positive Key
                curAxis->defaultPositiveKey = (VirtualKey)iniVal->getInt();
                break;
            case 2: // Default Negative Key
                curAxis->defaultNegativeKey = (VirtualKey)iniVal->getInt();
                break;
            case 3: // Positive Key
                curAxis->positiveKey = (VirtualKey)iniVal->getInt();
                break;
            case 4: // Negative Key
                curAxis->negativeKey = (VirtualKey)iniVal->getInt();
                break;
            case 5: // Joystick Button
                curAxis->joystickButton = iniVal->getInt();
                break;
            case 6: // Joystick Axis
                curAxis->joystickAxis = iniVal->getInt();
                break;
            }
        }
        if(curAxis->type == AxisType::SINGLE_KEY) {
            curAxis->upEvent = Event<ui32>();
            curAxis->downEvent = Event<ui32>();
        }
        _axisLookup[curAxis->name] = _axes.size();
        _axes.push_back(curAxis);
        curAxis = NULL;
    }
}

void InputManager::loadAxes() {
    loadAxes(_defaultConfigLocation);
}

// TODO: Remove this
void InputManager::update() {
    for(Axis* axis : _axes) {
        switch(axis->type) {
        case SINGLE_KEY:
            if(!_previousKeyStates[axis->positiveKey] && _currentKeyStates[axis->positiveKey]) //Up
                axis->upEvent(axis->positiveKey);
            else if(_previousKeyStates[axis->positiveKey] && !_currentKeyStates[axis->positiveKey]) //Down
                axis->downEvent(axis->positiveKey);
            break;
        default:
            break;
        }
    }
    memcpy(_previousKeyStates, _currentKeyStates, sizeof(_previousKeyStates));
}

void InputManager::startInput() {
    m_inputHooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonDown, [=] (void* sender, const vui::MouseButtonEvent& e) {
        switch (e.button) {
        case vui::MouseButton::LEFT:
            _currentKeyStates[SDL_BUTTON_LEFT] = true;
            break;
        case vui::MouseButton::RIGHT:
            _currentKeyStates[SDL_BUTTON_RIGHT] = true;
            break;
        default:
            break;
        }
    });
    m_inputHooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonUp, [=] (void* sender, const vui::MouseButtonEvent& e) {
        switch (e.button) {
        case vui::MouseButton::LEFT:
            _currentKeyStates[SDL_BUTTON_LEFT] = false;
            break;
        case vui::MouseButton::RIGHT:
            _currentKeyStates[SDL_BUTTON_RIGHT] = false;
            break;
        default:
            break;
        }
    });
    m_inputHooks.addAutoHook(&vui::InputDispatcher::key.onKeyDown, [=] (void* sender, const vui::KeyEvent& e) {
        _currentKeyStates[e.keyCode] = true;
    });
    m_inputHooks.addAutoHook(&vui::InputDispatcher::key.onKeyUp, [=] (void* sender, const vui::KeyEvent& e) {
        _currentKeyStates[e.keyCode] = false;
    });
}

void InputManager::stopInput() {
    m_inputHooks.dispose();
}

IDelegate<ui32>* InputManager::subscribe(const i32 axisID, EventType eventType, IDelegate<ui32>* f) {
    if(axisID < 0 || axisID >= _axes.size() || f == nullptr || _axes[axisID]->type != AxisType::SINGLE_KEY) return nullptr;
    switch(eventType) {
    case UP:
        return _axes[axisID]->upEvent.add(f);
    case DOWN:
        std::cout << "subscribing" << axisID << std::endl;
        return _axes[axisID]->downEvent.add(f);
    }
    return nullptr;
}

void InputManager::unsubscribe(const i32 axisID, EventType eventType, IDelegate<ui32>* f) {
    if(axisID < 0 || axisID >= _axes.size() || f == nullptr || _axes[axisID]->type != AxisType::SINGLE_KEY) return;
    switch(eventType) {
    case UP:
        _axes[axisID]->upEvent.remove(f);
    case DOWN:
        _axes[axisID]->downEvent.remove(f);
    }
}

void InputManager::saveAxes(const nString &filePath) {
    std::vector<nString> iniSections;
    std::vector<std::vector<IniValue>> iniValues;

    iniSections.push_back("");
    iniValues.push_back(std::vector<IniValue>());
    for (unsigned int i = 0; i < _axes.size(); i++) {
        Axis* axis = _axes[i];
        iniSections.push_back(axis->name);
        std::vector<IniValue> values;

        values.push_back(IniValue("type", std::to_string(axis->type)));
        values.push_back(IniValue("defaultPositiveKey", std::to_string(axis->defaultPositiveKey)));
        values.push_back(IniValue("defaultNegativeKey", std::to_string(axis->defaultNegativeKey)));
        values.push_back(IniValue("positiveKey", std::to_string(axis->positiveKey)));
        values.push_back(IniValue("negativeKey", std::to_string(axis->negativeKey)));
        values.push_back(IniValue("joystickButton", std::to_string(axis->joystickAxis)));
        values.push_back(IniValue("joystickAxis", std::to_string(axis->joystickButton)));

        iniValues.push_back(values);
    }
    fileManager.saveIniFile(filePath, iniValues, iniSections);
}

void InputManager::saveAxes() {
    saveAxes(_defaultConfigLocation);
}

i32 InputManager::getIniKey(const nString &val) {
    auto iter = _iniKeys.find(val);
    if (iter != _iniKeys.end()) {
        return iter->second;
    } else {
        return -1;
    }
}

ui32 InputManager::getNegativeKey(const i32 axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return UINT32_MAX;
    return _axes.at(axisID)->negativeKey;
}

void InputManager::setNegativeKey(const i32 axisID, VirtualKey key) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    _axes.at(axisID)->negativeKey = (VirtualKey)key;
}

ui32 InputManager::getPositiveKey(const i32 axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return UINT32_MAX;
    return _axes.at(axisID)->positiveKey;
}

void InputManager::setPositiveKey(const i32 axisID, VirtualKey key) {
    _axes.at(axisID)->positiveKey = key;
}

void InputManager::setPositiveKeyToDefault(const i32 axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    Axis* axis = _axes.at(axisID);
    axis->positiveKey = axis->defaultPositiveKey;
}

void InputManager::setNegativeKeyToDefault(const i32 axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    Axis* axis = _axes.at(axisID);
    axis->negativeKey = axis->defaultNegativeKey;
}

InputManager::AxisType InputManager::getAxisType(const i32 axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return AxisType::NONE;
    return _axes.at(axisID)->type;
}

