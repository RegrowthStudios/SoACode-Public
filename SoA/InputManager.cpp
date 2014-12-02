#include "stdafx.h"
#include "InputManager.h"

#include <SDL\SDL_keyboard.h>
#include <SDL\SDL_mouse.h>

#include "global.h"
#include "FileSystem.h"

#include "GameManager.h"
#include "Inputs.h"
#include "Keg.h"

#include <string>
#include <sstream>
#include <stdio.h>

KEG_ENUM_INIT_BEGIN(AxisType, InputManager::AxisType, e)
e->addValue("dualKey", InputManager::AxisType::DUAL_KEY);
e->addValue("joystickAxis", InputManager::AxisType::JOYSTICK_AXIS);
e->addValue("joystickButton", InputManager::AxisType::JOYSTICK_BUTTON);
e->addValue("none", InputManager::AxisType::NONE);
e->addValue("singleKey", InputManager::AxisType::SINGLE_KEY);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR2(Axis, InputManager::Axis)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("type", Keg::Value::custom("AxisType", offsetof(InputManager::Axis, type), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("defaultPositiveKey", Keg::Value::basic(Keg::BasicType::UI32, offsetof(InputManager::Axis, defaultPositiveKey)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("defaultNegativeKey", Keg::Value::basic(Keg::BasicType::UI32, offsetof(InputManager::Axis, defaultNegativeKey)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("positiveKey", Keg::Value::basic(Keg::BasicType::UI32, offsetof(InputManager::Axis, positiveKey)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("negativeKey", Keg::Value::basic(Keg::BasicType::UI32, offsetof(InputManager::Axis, negativeKey))); ///< The actual negative key.
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("joystickAxis", Keg::Value::basic(Keg::BasicType::I32, offsetof(InputManager::Axis, joystickAxis)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("joystickButton", Keg::Value::basic(Keg::BasicType::I32, offsetof(InputManager::Axis, joystickButton)));
KEG_TYPE_INIT_END

InputManager::InputManager() :
_defaultConfigLocation(DEFAULT_CONFIG_LOCATION) {
    for (unsigned int i = 0; i < 256; i++) {
        _currentKeyStates[i] = false;
        _previousKeyStates[i] = false;
    }
}

InputManager::~InputManager() {
    for (unsigned int i = 0; i < _axes.size(); i++) {
        delete _axes[i];
    }
}

float InputManager::getAxis(const int axisID) {
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

bool InputManager::getKey(const int axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return _currentKeyStates[axis->positiveKey];
}

bool InputManager::getKeyDown(const int axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return _currentKeyStates[axis->positiveKey] && !_previousKeyStates[axis->positiveKey];
}

bool InputManager::getKeyUp(const int axisID) {
    // Check Input
    if (axisID < 0 || axisID >= _axes.size()) return false;

    Axis* axis = _axes.at(axisID);
    return !_currentKeyStates[axis->positiveKey] && _previousKeyStates[axis->positiveKey];
}

i32 InputManager::createAxis(const nString& axisName, ui32 defaultPositiveKey, ui32 defaultNegativeKey) {
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

int InputManager::createAxis(const std::string &axisName, unsigned int defaultPositiveKey) {
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
    axis->negativeKey = (SDLKey)0;
    axis->defaultNegativeKey = (SDLKey)0;
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

void InputManager::loadAxes(const std::string &location) {
    IOManager ioManager; // TODO: Pass in a real boy
    nString data;
    ioManager.readFileToString(location.c_str(), data);
   
    if (data.length() == 0) {
        fprintf(stderr, "Failed to load %s", location.c_str());
        throw 33;
    }

    YAML::Node node = YAML::Load(data.c_str());
    if (node.IsNull() || !node.IsMap()) {
        perror(location.c_str());
        throw 34;
    }

    // Manually parse yml file
    for (auto& kvp : node) {
        Axis* curAxis = new Axis();
        curAxis->name = kvp.first.as<nString>();
        Keg::parse((ui8*)curAxis, kvp.second, Keg::getGlobalEnvironment(), &KEG_GLOBAL_TYPE(Axis));
     
        if (curAxis->type == AxisType::SINGLE_KEY) {
            curAxis->upEvent = Event<ui32>();
            curAxis->downEvent = Event<ui32>();
        }
        _axisLookup[curAxis->name] = _axes.size();
        _axes.push_back(curAxis);
    }
}

void InputManager::loadAxes() {
    loadAxes(_defaultConfigLocation);
}

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
    for (auto iter = _previousKeyStates.begin(); iter != _previousKeyStates.end(); iter++) {
        iter->second = _currentKeyStates[iter->first];
    }
}

void InputManager::pushEvent(const SDL_Event& inputEvent) {
    switch (inputEvent.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (inputEvent.button.button == SDL_BUTTON_LEFT) {
            _currentKeyStates[SDL_BUTTON_LEFT] = true;
        } else if (inputEvent.button.button == SDL_BUTTON_RIGHT) {
            _currentKeyStates[SDL_BUTTON_RIGHT] = true;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (inputEvent.button.button == SDL_BUTTON_LEFT) {
            _currentKeyStates[SDL_BUTTON_LEFT] = false;
        } else if (inputEvent.button.button == SDL_BUTTON_RIGHT) {
            _currentKeyStates[SDL_BUTTON_RIGHT] = false;
        }
        break;
    case SDL_MOUSEWHEEL:
        break;
    case SDL_KEYDOWN:
        _currentKeyStates[inputEvent.key.keysym.sym] = true;
        break;
    case SDL_KEYUP:
        _currentKeyStates[inputEvent.key.keysym.sym] = false;
        break;
    }
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

void InputManager::saveAxes(const string &filePath) {

}

void InputManager::saveAxes() {
    saveAxes(_defaultConfigLocation);
}

unsigned int InputManager::getNegativeKey(const int axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return UINT32_MAX;
    return _axes.at(axisID)->negativeKey;
}

void InputManager::setNegativeKey(const int axisID, unsigned int key) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    _axes.at(axisID)->negativeKey = key;
}

unsigned int InputManager::getPositiveKey(const int axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return UINT32_MAX;
    return _axes.at(axisID)->positiveKey;
}

void InputManager::setPositiveKey(const int axisID, unsigned int key) {
    _axes.at(axisID)->positiveKey = key;
}

void InputManager::setPositiveKeyToDefault(const int axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    Axis* axis = _axes.at(axisID);
    axis->positiveKey = axis->defaultPositiveKey;
}

void InputManager::setNegativeKeyToDefault(const int axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return;
    Axis* axis = _axes.at(axisID);
    axis->negativeKey = axis->defaultNegativeKey;
}

InputManager::AxisType InputManager::getAxisType(const int axisID) {
    if(axisID < 0 || axisID >= _axes.size()) return AxisType::NONE;
    return _axes.at(axisID)->type;
}