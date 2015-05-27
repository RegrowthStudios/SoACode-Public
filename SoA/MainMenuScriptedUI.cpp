#include "stdafx.h"
#include "MainMenuScriptedUI.h"
#include "SoaEngine.h"
#include "SoAState.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/ui/FormScriptEnvironment.h>
#include <Vorb/script/Environment.h>
#include <Vorb/Events.hpp>
#include <Vorb/ui/Form.h>
#include <Vorb/ui/KeyStrings.h>

#define ON_TARGET_CHANGE_NAME "onTargetChange"

MainMenuScriptedUI::MainMenuScriptedUI() {
    // Empty
}

MainMenuScriptedUI::~MainMenuScriptedUI() {
    // Empty
}

void MainMenuScriptedUI::init(const nString& startFormPath, vui::IGameScreen* ownerScreen, const vui::GameWindow* window, const f32v4& destRect, vg::SpriteFont* defaultFont /*= nullptr*/) {
    MainMenuScreen* mainMenuScreen = ((MainMenuScreen*)ownerScreen);
    m_inputMapper = mainMenuScreen->m_inputMapper;

    mainMenuScreen->m_mainMenuSystemViewer->TargetChange += makeDelegate(*this, &MainMenuScriptedUI::onTargetChange);

    ScriptedUI::init(startFormPath, ownerScreen, window, destRect, defaultFont); 
}

void MainMenuScriptedUI::registerScriptValues(vui::FormScriptEnvironment* newFormEnv) {
    vui::ScriptedUI::registerScriptValues(newFormEnv);
    vscript::Environment* env = newFormEnv->getEnv();

    SoaEngine::optionsController.registerScripting(env);

    // Controls menu stuff
    env->setNamespaces("Controls");
    env->addCRDelegate("size", makeRDelegate(*this, &MainMenuScriptedUI::getNumInputs));
    env->addCRDelegate("getInput", makeRDelegate(*this, &MainMenuScriptedUI::getInput));
    env->addCRDelegate("getKey", makeRDelegate(*this, &MainMenuScriptedUI::getKey));
    env->addCRDelegate("getDefaultKey", makeRDelegate(*this, &MainMenuScriptedUI::getDefaultKey));
    env->addCRDelegate("getKeyString", makeRDelegate(*this, &MainMenuScriptedUI::getKeyString));
    env->addCRDelegate("getDefaultKeyString", makeRDelegate(*this, &MainMenuScriptedUI::getDefaultKeyString));
    env->addCRDelegate("getName", makeRDelegate(*this, &MainMenuScriptedUI::getName));
    env->setNamespaces();

    env->setNamespaces("Game");
    env->addCDelegate("exit", makeDelegate(*this, &MainMenuScriptedUI::onExit));
    env->setNamespaces();

    // TODO(Ben): Expose and use ECS instead???
    env->setNamespaces("Space");
    env->addCRDelegate("getTargetBody", makeRDelegate(*this, &MainMenuScriptedUI::getTargetBody));
    env->addCRDelegate("getBodyName", makeRDelegate(*this, &MainMenuScriptedUI::getBodyName));
    env->addCRDelegate("getBodyParentName", makeRDelegate(*this, &MainMenuScriptedUI::getBodyName));
    env->addCRDelegate("getBodyTypeName", makeRDelegate(*this, &MainMenuScriptedUI::getBodyTypeName));
    env->addCRDelegate("getBodyMass", makeRDelegate(*this, &MainMenuScriptedUI::getBodyMass));
    env->addCRDelegate("getBodyDiameter", makeRDelegate(*this, &MainMenuScriptedUI::getBodyDiameter));
    env->addCRDelegate("getBodyRotPeriod", makeRDelegate(*this, &MainMenuScriptedUI::getBodyRotPeriod));
    env->addCRDelegate("getBodyOrbPeriod", makeRDelegate(*this, &MainMenuScriptedUI::getBodyOrbPeriod));
    env->addCRDelegate("getBodyAxialTilt", makeRDelegate(*this, &MainMenuScriptedUI::getBodyAxialTilt));
    env->addCRDelegate("getBodyEccentricity", makeRDelegate(*this, &MainMenuScriptedUI::getBodyEccentricity));
    env->addCRDelegate("getBodyInclination", makeRDelegate(*this, &MainMenuScriptedUI::getBodyInclination));
    env->addCRDelegate("getBodySemiMajor", makeRDelegate(*this, &MainMenuScriptedUI::getBodySemiMajor));
    env->setNamespaces();
}

size_t MainMenuScriptedUI::getNumInputs() {
    return m_inputMapper->getInputLookup().size();
}

InputMapper::InputID MainMenuScriptedUI::getInput(int index) {
    // This is slow, but that is ok.
    auto& it = m_inputMapper->getInputLookup().begin();
    std::advance(it, index);
    return it->second;
}

VirtualKey MainMenuScriptedUI::getKey(InputMapper::InputID id) {
    return m_inputMapper->getKey(id);
}

VirtualKey MainMenuScriptedUI::getDefaultKey(InputMapper::InputID id) {
    return m_inputMapper->get(id).defaultKey;
}

nString MainMenuScriptedUI::getKeyString(InputMapper::InputID id) {
    return nString(VirtualKeyStrings[m_inputMapper->getKey(id)]);
}

nString MainMenuScriptedUI::getDefaultKeyString(InputMapper::InputID id) {
    return nString(VirtualKeyStrings[m_inputMapper->get(id).defaultKey]);
}

nString MainMenuScriptedUI::getName(InputMapper::InputID id) {
    return m_inputMapper->get(id).name;
}

void MainMenuScriptedUI::onExit(int code) {
    ((MainMenuScreen*)m_ownerScreen)->onQuit(this, code);
}

void MainMenuScriptedUI::onTargetChange(Sender s, vecs::EntityID id) {
    // TODO(Ben): Race condition???
    for (auto& it : m_forms) {
        if (it.first->isEnabled()) {
            vscript::Environment* env = it.second->getEnv();
            const vscript::Function& f = (*env)[ON_TARGET_CHANGE_NAME];
            if (!f.isNil()) f(id);
        }
    }
}

vecs::EntityID MainMenuScriptedUI::getTargetBody() {
    return ((MainMenuScreen*)m_ownerScreen)->m_mainMenuSystemViewer->getTargetBody();
}

nString MainMenuScriptedUI::getBodyName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return state->spaceSystem->m_namePositionCT.getFromEntity(entity).name;
}

nString MainMenuScriptedUI::getBodyParentName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    auto parentOID = state->spaceSystem->m_orbitCT.getFromEntity(entity).parentOrbId;
    if (parentOID == 0) return "None";
    return state->spaceSystem->m_namePositionCT.get(parentOID).name;
}

nString MainMenuScriptedUI::getBodyTypeName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    auto t = state->spaceSystem->m_orbitCT.getFromEntity(entity).type;
    nString n;
    switch (t) {
        case SpaceObjectType::BARYCENTER:
            n = "Barycenter"; break;
        case SpaceObjectType::STAR: // TODO(Ben): Spectral classes
            n = "Star"; break;
        case SpaceObjectType::PLANET:
            n = "Planet"; break;
        case SpaceObjectType::DWARF_PLANET:
            n = "Dwarf Planet"; break;
        case SpaceObjectType::MOON:
            n = "Moon"; break;
        case SpaceObjectType::DWARF_MOON:
            n = "Dwarf Moon"; break;
        case SpaceObjectType::ASTEROID:
            n = "Asteroid"; break;
        case SpaceObjectType::COMET:
            n = "Comet"; break;
        default:
            n = "UNKNOWN"; break;
    }
    return n;
}

f32 MainMenuScriptedUI::getBodyMass(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_sphericalGravityCT.getFromEntity(entity).mass;
}

f32 MainMenuScriptedUI::getBodyDiameter(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_sphericalGravityCT.getFromEntity(entity).radius * 2.0f;
}

f32 MainMenuScriptedUI::getBodyRotPeriod(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_axisRotationCT.getFromEntity(entity).period;
}

f32 MainMenuScriptedUI::getBodyOrbPeriod(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_orbitCT.getFromEntity(entity).t;
}

f32 MainMenuScriptedUI::getBodyAxialTilt(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_axisRotationCT.getFromEntity(entity).tilt;
}

f32 MainMenuScriptedUI::getBodyEccentricity(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_orbitCT.getFromEntity(entity).e;
}

f32 MainMenuScriptedUI::getBodyInclination(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_orbitCT.getFromEntity(entity).i;
}

f32 MainMenuScriptedUI::getBodySemiMajor(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->m_orbitCT.getFromEntity(entity).a;
}