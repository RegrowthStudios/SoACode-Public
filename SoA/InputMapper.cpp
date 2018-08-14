#include "stdafx.h"
#include "InputMapper.h"

#include <Vorb/io/Keg.h>
#include <Vorb/io/IOManager.h>

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
    memset(m_keyStates, 0, sizeof(m_keyStates));
}

InputMapper::~InputMapper() {
    stopInput();
}

bool InputMapper::getInputState(const InputID id) {
    // Check Input
    if (id < 0 || id >= (int)m_inputs.size()) return false;
    return m_keyStates[m_inputs.at(id).key];
}

InputMapper::InputID InputMapper::createInput(const nString& inputName, VirtualKey defaultKey) {
    InputID id = getInputID(inputName);
    if (id >= 0) return id;
    id = m_inputs.size();
    m_inputLookup[inputName] = id;
    m_inputs.emplace_back(id, inputName, defaultKey, this);
    m_keyCodeMap[m_inputs.back().key].push_back(id);
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

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        perror(location.c_str());
        context.reader.dispose();
        throw 34;
    }

    // Manually parse yml file
    auto f = makeFunctor([&] (Sender, const nString& name, keg::Node value) {
        InputKegArray kegArray;
        
        keg::parse((ui8*)&kegArray, value, context, &KEG_GET_TYPE(InputKegArray));
        
        // TODO(Ben): Somehow do multikey support
        // Right now its only using the first key
        InputID id = m_inputs.size();
        m_inputs.emplace_back(id, name, kegArray.defaultKey[0], this);
        m_keyCodeMap[m_inputs.back().key].push_back(id);

        if (kegArray.key.size()) {
            m_inputs.back().key = kegArray.key[0];
        } else {
            m_inputs.back().key = kegArray.defaultKey[0];
        }

        m_inputLookup[name] = id;

    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
}

void InputMapper::startInput() {
    if (!m_receivingInput) {
        vui::InputDispatcher::mouse.onButtonDown += makeDelegate(*this, &InputMapper::onMouseButtonDown);
        vui::InputDispatcher::mouse.onButtonUp += makeDelegate(*this, &InputMapper::onMouseButtonDown);
        vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &InputMapper::onKeyDown);
        vui::InputDispatcher::key.onKeyUp += makeDelegate(*this, &InputMapper::onKeyUp);
        m_receivingInput = true;
    }
}
void InputMapper::stopInput() {
    if (m_receivingInput) {
        vui::InputDispatcher::mouse.onButtonDown -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
        vui::InputDispatcher::mouse.onButtonUp -= makeDelegate(*this, &InputMapper::onMouseButtonDown);
        vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &InputMapper::onKeyDown);
        vui::InputDispatcher::key.onKeyUp -= makeDelegate(*this, &InputMapper::onKeyUp);
        m_receivingInput = false;
    }
}

void InputMapper::saveInputs(const nString &filePath VORB_UNUSED /* = INPUTMAPPER_DEFAULT_CONFIG_LOCATION */) {
    //TODO(Ben): Implement and remove VORB_UNUSED tag.
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

VirtualKey InputMapper::getKey(const InputID id) {
    if (id < 0 || id >= (int)m_inputs.size()) return VKEY_HIGHEST_VALUE;
    return m_inputs.at(id).key;
}

void InputMapper::setKey(const InputID id, VirtualKey key) {
    // Need to remove old key state
    VirtualKey oldKey = m_inputs.at(id).key;
    auto it = m_keyCodeMap.find(oldKey);
    auto& vec = it->second;
    for (size_t i = 0; i < vec.size(); i++) {
        // Remove the input from the vector keyed on VirtualKey
        if (vec[i] == id) {
            vec[i] = vec.back();
            vec.pop_back();
            break;
        }
    }
    // Set new key
    m_keyCodeMap[key].push_back(id);
    m_inputs[id].key = key;
}

void InputMapper::setKeyToDefault(const InputID id) {
    if (id < 0 || id >= (int)m_inputs.size()) return;
    setKey(id, m_inputs.at(id).defaultKey);
}

void InputMapper::onMouseButtonDown(Sender, const vui::MouseButtonEvent& e) {
    ui32 code = VKEY_HIGHEST_VALUE + (ui32)e.button;
    if (!m_keyStates[code]) {
        m_keyStates[code] = true;
        // TODO(Ben): input mapping for mouse
        //auto& it = m_keyCodeMap.find((VirtualKey)e.keyCode);
        //if (it != m_keyCodeMap.end()) {
        //    // Call all events mapped to that virtual key
        //    for (auto& id : it->second) {
        //        m_inputs[id].downEvent(e.keyCode);
        //    }
        //}
    }
}

void InputMapper::onMouseButtonUp(Sender, const vui::MouseButtonEvent& e) {
    ui32 code = VKEY_HIGHEST_VALUE + (ui32)e.button;
    m_keyStates[code] = false;
    // TODO(Ben): input mapping for mouse
    //auto& it = m_keyCodeMap.find((VirtualKey)e.keyCode);
    //if (it != m_keyCodeMap.end()) {
    //    // Call all events mapped to that virtual key
    //    for (auto& id : it->second) {
    //        m_inputs[id].upEvent(e.keyCode);
    //    }
    //} 
}

void InputMapper::onKeyDown(Sender, const vui::KeyEvent& e) {
    if (!m_keyStates[e.keyCode]) {
        m_keyStates[e.keyCode] = true;
        auto it = m_keyCodeMap.find((VirtualKey)e.keyCode);
        if (it != m_keyCodeMap.end()) {
            // Call all events mapped to that virtual key
            for (auto& id : it->second) {
                m_inputs[id].downEvent(e.keyCode);
            }
        }
    }
}

void InputMapper::onKeyUp(Sender, const vui::KeyEvent& e) {
    m_keyStates[e.keyCode] = false;
    auto it = m_keyCodeMap.find((VirtualKey)e.keyCode);
    if (it != m_keyCodeMap.end()) {
        // Call all events mapped to that virtual key
        for (auto& id : it->second) {
            m_inputs[id].upEvent(e.keyCode);
        }
    } 
}
