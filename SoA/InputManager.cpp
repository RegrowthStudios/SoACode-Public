#include "stdafx.h"
#include "InputManager.h"

#include <SDL\SDL_keyboard.h>
#include <SDL\SDL_mouse.h>
#include <Vorb/io/Keg.h>

#include "global.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Inputs.h"

KEG_ENUM_DECL(AxisType);
KEG_ENUM_DEF(AxisType, InputManager::AxisType, e) {
    e.addValue("dualKey", InputManager::AxisType::DUAL_KEY);
    e.addValue("joystickAxis", InputManager::AxisType::JOYSTICK_AXIS);
    e.addValue("joystickButton", InputManager::AxisType::JOYSTICK_BUTTON);
    e.addValue("none", InputManager::AxisType::NONE);
    e.addValue("singleKey", InputManager::AxisType::SINGLE_KEY);
}

KEG_TYPE_DECL(Axis);
KEG_TYPE_DEF(Axis, InputManager::Axis, kt) {
    using namespace keg;
    kt.addValue("type", Value::custom(offsetof(InputManager::Axis, type), "AxisType", true));
    kt.addValue("defaultPositiveKey", Value::basic(offsetof(InputManager::Axis, defaultPositiveKey), BasicType::UI32));
    kt.addValue("defaultNegativeKey", Value::basic(offsetof(InputManager::Axis, defaultNegativeKey), BasicType::UI32));
    kt.addValue("positiveKey", Value::basic(offsetof(InputManager::Axis, positiveKey), BasicType::UI32));
    kt.addValue("negativeKey", Value::basic(offsetof(InputManager::Axis, negativeKey), BasicType::UI32)); ///< The actual negative key.
    kt.addValue("joystickAxis", Value::basic(offsetof(InputManager::Axis, joystickAxis), BasicType::I32));
    kt.addValue("joystickButton", Value::basic(offsetof(InputManager::Axis, joystickButton), BasicType::I32));
}
InputManager::InputManager() :
_defaultConfigLocation(DEFAULT_CONFIG_LOCATION) {
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

void InputManager::loadAxes(const std::string &location) {
    vio::IOManager ioManager; //TODO PASS IN
    nString data;
    ioManager.readFileToString(location.c_str(), data);
   
    if (data.length() == 0) {
        fprintf(stderr, "Failed to load %s", location.c_str());
        throw 33;
    }

    keg::YAMLReader reader;
    reader.init(data.c_str());
    keg::Node node = reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        perror(location.c_str());
        reader.dispose();
        throw 34;
    }

    // Manually parse yml file
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&] (Sender, const nString& name, keg::Node value) {
        Axis* curAxis = new Axis();
        curAxis->name = name;
        keg::parse((ui8*)curAxis, value, reader, keg::getGlobalEnvironment(), &KEG_GET_TYPE(Axis));
     
        if (curAxis->type == AxisType::SINGLE_KEY) {
            curAxis->upEvent = Event<ui32>(curAxis);
            curAxis->downEvent = Event<ui32>(curAxis);
        }
        _axisLookup[curAxis->name] = _axes.size();
        _axes.push_back(curAxis);
    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();
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
                axis->downEvent(axis->positiveKey);
            else if(_previousKeyStates[axis->positiveKey] && !_currentKeyStates[axis->positiveKey]) //Down
                axis->upEvent(axis->positiveKey);
            break;
        default:
            break;
        }
    }
    memcpy(_previousKeyStates, _currentKeyStates, sizeof(_previousKeyStates));
}

void InputManager::startInput() {
    vui::InputDispatcher::mouse.onButtonDown += makeDelegate(*this, &InputManager::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp += makeDelegate(*this, &InputManager::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &InputManager::onKeyDown);
    vui::InputDispatcher::key.onKeyUp += makeDelegate(*this, &InputManager::onKeyUp);
}
void InputManager::stopInput() {
    vui::InputDispatcher::mouse.onButtonDown -= makeDelegate(*this, &InputManager::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp -= makeDelegate(*this, &InputManager::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &InputManager::onKeyDown);
    vui::InputDispatcher::key.onKeyUp -= makeDelegate(*this, &InputManager::onKeyUp);
}

InputManager::Listener* InputManager::subscribe(const i32 axisID, EventType eventType, InputManager::Listener* f) {
    if(axisID < 0 || axisID >= _axes.size() || f == nullptr || _axes[axisID]->type != AxisType::SINGLE_KEY) return nullptr;
    switch (eventType) {
    case UP:
        _axes[axisID]->upEvent.add(*f);
        return f;
    case DOWN:
        std::cout << "subscribing" << axisID << std::endl;
        _axes[axisID]->downEvent.add(*f);
        return f;
    }
    return nullptr;
}

void InputManager::unsubscribe(const i32 axisID, EventType eventType, InputManager::Listener* f) {
    if(axisID < 0 || axisID >= _axes.size() || f == nullptr || _axes[axisID]->type != AxisType::SINGLE_KEY) return;
    switch(eventType) {
    case UP:
        _axes[axisID]->upEvent.remove(*f);
    case DOWN:
        _axes[axisID]->downEvent.remove(*f);
    }
}

void InputManager::saveAxes(const nString &filePath) {

}

void InputManager::saveAxes() {
    saveAxes(_defaultConfigLocation);
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

void InputManager::onMouseButtonDown(Sender, const vui::MouseButtonEvent& e) {
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
}
void InputManager::onMouseButtonUp(Sender, const vui::MouseButtonEvent& e) {
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
}
void InputManager::onKeyDown(Sender, const vui::KeyEvent& e) {
    _currentKeyStates[e.keyCode] = true;
}
void InputManager::onKeyUp(Sender, const vui::KeyEvent& e) {
    _currentKeyStates[e.keyCode] = false;
}
