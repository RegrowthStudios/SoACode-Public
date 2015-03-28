#include "stdafx.h"
#include "InputMapper.h"

#include <SDL\SDL_keyboard.h>
#include <SDL\SDL_mouse.h>
#include <Vorb/io/Keg.h>

#include "global.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Inputs.h"

#include "VirtualKeyKegDef.inl"

struct InputKegArray {
    Array<VirtualKey> defaultKey;
    Array<VirtualKey> key;
};
KEG_TYPE_DECL(InputKegArray);
KEG_TYPE_DEF(InputKegArray, InputKegArray, kt) {
    using namespace keg;
    kt.addValue("defaultKey", Value::array(offsetof(InputKegArray, defaultKey), Value::custom(0, "VirtualKey", true)));
    kt.addValue("key", Value::array(offsetof(InputKegArray, key), Value::custom(0, "VirtualKey", true)));
}
InputMapper::InputMapper() {
    // Empty
}

InputMapper::~InputMapper() {
    // Empty
}

bool InputMapper::getInputState(const InputID id) {
    // Check Input
    if (id < 0 || id >= m_inputs.size()) return false;
    return m_keyStates[m_inputs.at(id).key];
}

InputMapper::InputID InputMapper::createInput(const nString& inputName, VirtualKey defaultKey) {
    InputID id = getInputID(inputName);
    if (id >= 0) return id;
    id = m_inputs.size();
    m_inputLookup[inputName] = id;
    m_inputs.emplace_back(inputName, defaultKey, this);
    return id;
}

InputMapper::InputID InputMapper::getInputID(const nString& inputName) const {
    auto iter = m_inputLookup.find(inputName);

    if (iter != m_inputLookup.end()) {
        return iter->second;
    } else {
        return -1;
    }
}

void InputMapper::loadInputs(const nString &location /* = INPUTMAPPER_DEFAULT_CONFIG_LOCATION */) {
    vio::IOManager iom; //TODO PASS IN
    nString data;

    // If the file doesn't exist, just make it with defaults
    if (!iom.fileExists(location)) {
        saveInputs(location);
        return;
    }

    iom.readFileToString(location.c_str(), data);
   
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
        InputKegArray kegArray;
        
        keg::parse((ui8*)&kegArray, value, reader, keg::getGlobalEnvironment(), &KEG_GET_TYPE(InputKegArray));
        
        // TODO(Ben): Somehow do multikey support
        // Right now its only using the first key
        InputID id = m_inputs.size();
        m_inputs.emplace_back(name, kegArray.defaultKey[0], this);
        if (kegArray.key.size()) {
            m_inputs.back().key = kegArray.key[0];
        } else {
            m_inputs.back().key = kegArray.defaultKey[0];
        }

        m_inputLookup[name] = id;

    });
    reader.forAllInMap(node, f);
    delete f;
    reader.dispose();
}

void InputMapper::startInput() {
    vui::InputDispatcher::mouse.onButtonDown += makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp += makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &InputMapper::onKeyDown);
    vui::InputDispatcher::key.onKeyUp += makeDelegate(*this, &InputMapper::onKeyUp);
}
void InputMapper::stopInput() {
    vui::InputDispatcher::mouse.onButtonDown -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::mouse.onButtonUp -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
    vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &InputMapper::onKeyDown);
    vui::InputDispatcher::key.onKeyUp -= makeDelegate(*this, &InputMapper::onKeyUp);
}

void InputMapper::subscribe(const InputID id, EventType eventType, Listener f) {
    if (id < 0 || id >= m_inputs.size()) return;
    switch (eventType) {
    case UP:
        m_inputs[id].upEvent.add(f);
    case DOWN:
        m_inputs[id].downEvent.add(f);
    }
}

void InputMapper::unsubscribe(const InputID id, EventType eventType, Listener f) {
    if (id < 0 || id >= m_inputs.size()) return;
    switch(eventType) {
    case UP:
        m_inputs[id].upEvent.remove(f);
    case DOWN:
        m_inputs[id].downEvent.remove(f);
    }
}

void InputMapper::saveInputs(const nString &filePath /* = DEFAULT_CONFIG_LOCATION */) {
    //TODO(Ben): Implement
   // vio::IOManager iom;
    // Just build the data string manually then write it
   
   /* bool tmp;
    keg::Enum enm;
    nString data = "";
    for (auto& input : m_inputs) {
        data += input->name + ":\n";
        data += "  defaultKey:\n";
        data += "    - " + keg::getEnum(tmp, .getValue()
    }*/
}

VirtualKey InputMapper::getKey(const InputID inputID) {
    if (inputID < 0 || inputID >= m_inputs.size()) return VKEY_HIGHEST_VALUE;
    return m_inputs.at(inputID).key;
}

void InputMapper::setKey(const InputID inputID, VirtualKey key) {
    m_inputs.at(inputID).key = key;
}

void InputMapper::setKeyToDefault(const InputID inputID) {
    // TODO(Ben): Change the key map here too
    if(inputID < 0 || inputID >= m_inputs.size()) return;
    Input& input = m_inputs.at(inputID);
    input.key = input.defaultKey;
}

void InputMapper::onMouseButtonDown(Sender, const vui::MouseButtonEvent& e) {
    switch (e.button) {
    case vui::MouseButton::LEFT:
        m_keyStates[SDL_BUTTON_LEFT] = true;
        break;
    case vui::MouseButton::RIGHT:
        m_keyStates[SDL_BUTTON_RIGHT] = true;
        break;
    default:
        break;
    }
}
void InputMapper::onMouseButtonUp(Sender, const vui::MouseButtonEvent& e) {
    switch (e.button) {
    case vui::MouseButton::LEFT:
        m_keyStates[SDL_BUTTON_LEFT] = false;
        break;
    case vui::MouseButton::RIGHT:
        m_keyStates[SDL_BUTTON_RIGHT] = false;
        break;
    default:
        break;
    }
}
void InputMapper::onKeyDown(Sender, const vui::KeyEvent& e) {
    m_keyStates[e.keyCode] = true;
}
void InputMapper::onKeyUp(Sender, const vui::KeyEvent& e) {
    m_keyStates[e.keyCode] = false;
}
