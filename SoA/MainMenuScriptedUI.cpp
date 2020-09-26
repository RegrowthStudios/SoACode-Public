#include "stdafx.h"
#include "MainMenuScriptedUI.h"
#include "SoaEngine.h"
#include "SoAState.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/ui/script/ViewScriptEnvironment.h>
#include <Vorb/script/lua/Environment.h>
#include <Vorb/Event.hpp>
#include <Vorb/ui/KeyStrings.h>

#define ON_TARGET_CHANGE_NAME "onTargetChange"

MainMenuScriptedUI::MainMenuScriptedUI() {
    // Empty
}

MainMenuScriptedUI::~MainMenuScriptedUI() {
    // Empty
}

void MainMenuScriptedUI::init(vui::IGameScreen* ownerScreen, const vui::GameWindow* window, vio::IOManager* iom, vg::TextureCache* textureCache, vg::SpriteFont* defaultFont /*= nullptr*/, vg::SpriteBatch* spriteBatch /*= nullptr*/) {
    MainMenuScreen* mainMenuScreen = ((MainMenuScreen*)ownerScreen);
    m_inputMapper = mainMenuScreen->m_inputMapper;

    mainMenuScreen->m_mainMenuSystemViewer->TargetChange += makeDelegate(this, &MainMenuScriptedUI::onTargetChange);

    ScriptedUI::init(ownerScreen, window, iom, textureCache, defaultFont, spriteBatch); 
}

void MainMenuScriptedUI::prepareScriptEnv(ViewEnv* viewEnv) {
    vui::ScriptedUI<vscript::lua::Environment>::prepareScriptEnv(viewEnv);
    vscript::lua::Environment* env = static_cast<vscript::lua::Environment*>(viewEnv->getEnv());

    SoaEngine::optionsController.registerScripting<vscript::lua::Environment>(env);

    // Controls menu stuff
    env->setNamespaces("Controls");
    env->addCDelegate("size",                makeDelegate(this, &MainMenuScriptedUI::getNumInputs));
    env->addCDelegate("getInput",            makeDelegate(this, &MainMenuScriptedUI::getInput));
    env->addCDelegate("getKey",              makeDelegate(this, &MainMenuScriptedUI::getKey));
    env->addCDelegate("getDefaultKey",       makeDelegate(this, &MainMenuScriptedUI::getDefaultKey));
    env->addCDelegate("getKeyString",        makeDelegate(this, &MainMenuScriptedUI::getKeyString));
    env->addCDelegate("getDefaultKeyString", makeDelegate(this, &MainMenuScriptedUI::getDefaultKeyString));
    env->addCDelegate("getName",             makeDelegate(this, &MainMenuScriptedUI::getName));

    env->setNamespaces();
    env->addCDelegate("newGame", makeDelegate(this, &MainMenuScriptedUI::newGame));

    env->setNamespaces("Game");
    env->addCDelegate("exit", makeDelegate(this, &MainMenuScriptedUI::onExit));

    // TODO(Ben): Expose and use ECS instead???
    env->setNamespaces("Space");
    env->addCDelegate("getTargetBody",       makeDelegate(this, &MainMenuScriptedUI::getTargetBody));
    env->addCDelegate("getBodyName",         makeDelegate(this, &MainMenuScriptedUI::getBodyName));
    env->addCDelegate("getBodyParentName",   makeDelegate(this, &MainMenuScriptedUI::getBodyName));
    env->addCDelegate("getBodyTypeName",     makeDelegate(this, &MainMenuScriptedUI::getBodyTypeName));
    env->addCDelegate("getBodyMass",         makeDelegate(this, &MainMenuScriptedUI::getBodyMass));
    env->addCDelegate("getBodyDiameter",     makeDelegate(this, &MainMenuScriptedUI::getBodyDiameter));
    env->addCDelegate("getBodyRotPeriod",    makeDelegate(this, &MainMenuScriptedUI::getBodyRotPeriod));
    env->addCDelegate("getBodyOrbPeriod",    makeDelegate(this, &MainMenuScriptedUI::getBodyOrbPeriod));
    env->addCDelegate("getBodyAxialTilt",    makeDelegate(this, &MainMenuScriptedUI::getBodyAxialTilt));
    env->addCDelegate("getBodyEccentricity", makeDelegate(this, &MainMenuScriptedUI::getBodyEccentricity));
    env->addCDelegate("getBodyInclination",  makeDelegate(this, &MainMenuScriptedUI::getBodyInclination));
    env->addCDelegate("getBodySemiMajor",    makeDelegate(this, &MainMenuScriptedUI::getBodySemiMajor));
    env->addCDelegate("getGravityAccel",     makeDelegate(this, &MainMenuScriptedUI::getGravityAccel));
    env->addCDelegate("getVolume",           makeDelegate(this, &MainMenuScriptedUI::getVolume));
    env->addCDelegate("getAverageDensity",   makeDelegate(this, &MainMenuScriptedUI::getAverageDensity));
    env->setNamespaces();
}

size_t MainMenuScriptedUI::getNumInputs() {
    return m_inputMapper->getInputLookup().size();
}

InputMapper::InputID MainMenuScriptedUI::getInput(int index) {
    // This is slow, but that is ok.
    auto it = m_inputMapper->getInputLookup().begin();
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

void MainMenuScriptedUI::onTargetChange(Sender, vecs::EntityID id) {
    // TODO(Ben): Race condition???
    for (auto& view : m_views) {
        if (view.second.viewport->isEnabled()) {
            vscript::IEnvironment<vscript::lua::Environment>* env = view.second.viewEnv->getEnv();
            Delegate<void, vecs::EntityID> del = env->template getScriptDelegate<void, vecs::EntityID>(ON_TARGET_CHANGE_NAME);
            if (del != NilDelegate<void, vecs::EntityID>) del(id);
        }
    }
}

void MainMenuScriptedUI::newGame() {
    ((MainMenuScreen*)m_ownerScreen)->m_newGameClicked = true;
}

vecs::EntityID MainMenuScriptedUI::getTargetBody() {
    return ((MainMenuScreen*)m_ownerScreen)->m_mainMenuSystemViewer->getTargetBody();
}

nString MainMenuScriptedUI::getBodyName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return state->spaceSystem->namePosition.getFromEntity(entity).name;
}

nString MainMenuScriptedUI::getBodyParentName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    auto parentOID = state->spaceSystem->orbit.getFromEntity(entity).parentOrbId;
    if (parentOID == 0) return "None";
    auto parentNpID = state->spaceSystem->orbit.get(parentOID).npID;
    return state->spaceSystem->namePosition.get(parentNpID).name;
}

nString MainMenuScriptedUI::getBodyTypeName(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    auto t = state->spaceSystem->orbit.getFromEntity(entity).type;
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
    return (f32)state->spaceSystem->sphericalGravity.getFromEntity(entity).mass;
}

f32 MainMenuScriptedUI::getBodyDiameter(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->sphericalGravity.getFromEntity(entity).radius * 2.0f;
}

f32 MainMenuScriptedUI::getBodyRotPeriod(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->axisRotation.getFromEntity(entity).period;
}

f32 MainMenuScriptedUI::getBodyOrbPeriod(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).t;
}

f32 MainMenuScriptedUI::getBodyAxialTilt(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->axisRotation.getFromEntity(entity).tilt;
}

f32 MainMenuScriptedUI::getBodyEccentricity(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).e;
}

f32 MainMenuScriptedUI::getBodyInclination(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).i;
}

f32 MainMenuScriptedUI::getBodySemiMajor(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).a;
}

f32 MainMenuScriptedUI::getGravityAccel(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    auto& sgCmp = state->spaceSystem->sphericalGravity.getFromEntity(entity);
    f32 rad = (f32)(sgCmp.radius * M_PER_KM);
    return (f32)(M_G * sgCmp.mass / (rad * rad));
}

f32 MainMenuScriptedUI::getVolume(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    // TODO(Ben): Handle oblateness
    auto& sgCmp = state->spaceSystem->sphericalGravity.getFromEntity(entity);
    f32 rad = (f32)(sgCmp.radius * M_PER_KM);
    return (f32)(4.0 / 3.0 * M_PI * rad * rad * rad);
}

f32 MainMenuScriptedUI::getAverageDensity(vecs::EntityID entity) {
    SoaState* state = ((MainMenuScreen*)m_ownerScreen)->m_soaState;
    // TODO(Ben): This is a double lookup
    f32 volume = getVolume(entity);
    return (f32)(state->spaceSystem->sphericalGravity.getFromEntity(entity).mass / volume);
}
