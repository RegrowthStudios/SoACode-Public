#include "stdafx.h"
#include "MainMenuScriptContext.h"
#include "SoaEngine.h"
#include "SoAState.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/ui/script/ViewScriptContext.hpp>
#include <Vorb/script/lua/Environment.h>
#include <Vorb/Event.hpp>
#include <Vorb/ui/KeyStrings.h>

void MainMenuScriptContext::injectInto(vscript::lua::Environment* env, const vui::GameWindow* window, InputMapper* inputMapper, MainMenuScreen* ownerScreen) {
    vui::UIScriptContext::injectInto(env, window);

    vui::ViewScriptContext::injectInto(env, window);

    SoaEngine::optionsController.registerScripting<vscript::lua::Environment>(env);

    // Controls menu stuff
    env->setNamespaces("Controls");
    env->addCDelegate("size", makeFunctor([inputMapper]() {
        return MainMenuScriptContext::impl::getNumInputs(inputMapper);
    }));
    env->addCDelegate("getInput", makeFunctor([inputMapper](int index) {
        return MainMenuScriptContext::impl::getInput(inputMapper, index);
    }));
    env->addCDelegate("getKey", makeFunctor([inputMapper](InputMapper::InputID id) {
        return MainMenuScriptContext::impl::getKey(inputMapper, id);
    }));
    env->addCDelegate("getDefaultKey", makeFunctor([inputMapper](InputMapper::InputID id) {
        return MainMenuScriptContext::impl::getDefaultKey(inputMapper, id);
    }));
    env->addCDelegate("getKeyString", makeFunctor([inputMapper](InputMapper::InputID id) {
        return MainMenuScriptContext::impl::getKeyString(inputMapper, id);
    }));
    env->addCDelegate("getDefaultKeyString", makeFunctor([inputMapper](InputMapper::InputID id) {
        return MainMenuScriptContext::impl::getDefaultKeyString(inputMapper, id);
    }));
    env->addCDelegate("getName", makeFunctor([inputMapper](InputMapper::InputID id) {
        return MainMenuScriptContext::impl::getName(inputMapper, id);
    }));

    env->setNamespaces();
    env->addCDelegate("newGame", makeDelegate(&MainMenuScriptContext::impl::newGame));

    env->setNamespaces("Game");
    env->addCDelegate("exit", makeFunctor([ownerScreen](int code) {
        return MainMenuScriptContext::impl::onExit(ownerScreen, code);
    }));

    // TODO(Ben): Expose and use ECS instead???
    env->setNamespaces("Space");
    env->addCDelegate("getTargetBody",       makeDelegate(&MainMenuScriptContext::impl::getTargetBody));
    env->addCDelegate("getBodyName",         makeDelegate(&MainMenuScriptContext::impl::getBodyName));
    env->addCDelegate("getBodyParentName",   makeDelegate(&MainMenuScriptContext::impl::getBodyName));
    env->addCDelegate("getBodyTypeName",     makeDelegate(&MainMenuScriptContext::impl::getBodyTypeName));
    env->addCDelegate("getBodyMass",         makeDelegate(&MainMenuScriptContext::impl::getBodyMass));
    env->addCDelegate("getBodyDiameter",     makeDelegate(&MainMenuScriptContext::impl::getBodyDiameter));
    env->addCDelegate("getBodyRotPeriod",    makeDelegate(&MainMenuScriptContext::impl::getBodyRotPeriod));
    env->addCDelegate("getBodyOrbPeriod",    makeDelegate(&MainMenuScriptContext::impl::getBodyOrbPeriod));
    env->addCDelegate("getBodyAxialTilt",    makeDelegate(&MainMenuScriptContext::impl::getBodyAxialTilt));
    env->addCDelegate("getBodyEccentricity", makeDelegate(&MainMenuScriptContext::impl::getBodyEccentricity));
    env->addCDelegate("getBodyInclination",  makeDelegate(&MainMenuScriptContext::impl::getBodyInclination));
    env->addCDelegate("getBodySemiMajor",    makeDelegate(&MainMenuScriptContext::impl::getBodySemiMajor));
    env->addCDelegate("getGravityAccel",     makeDelegate(&MainMenuScriptContext::impl::getGravityAccel));
    env->addCDelegate("getVolume",           makeDelegate(&MainMenuScriptContext::impl::getVolume));
    env->addCDelegate("getAverageDensity",   makeDelegate(&MainMenuScriptContext::impl::getAverageDensity));
    env->setNamespaces();

    env->setNamespaces("Space", "onTargetChange");
    env->addCDelegate("subscribe", makeFunctor([ownerScreen, env](nString name) {
        ownerScreen->getMainMenuSystemViewer()->TargetChange.add(env->template getScriptDelegate<void, Sender, ui32>(name), true);
    }));
    env->addCDelegate("unsubscribe",  makeFunctor([ownerScreen, env](nString name) {
        ownerScreen->getMainMenuSystemViewer()->TargetChange.remove(env->template getScriptDelegate<void, Sender, ui32>(name));
    }));
    env->setNamespaces();
}

size_t MainMenuScriptContext::impl::getNumInputs(InputMapper* inputMapper) {
    return inputMapper->getInputLookup().size();
}

InputMapper::InputID MainMenuScriptContext::impl::getInput(InputMapper* inputMapper, int index) {
    // This is slow, but that is ok.
    auto it = inputMapper->getInputLookup().begin();
    std::advance(it, index);
    return it->second;
}

VirtualKey MainMenuScriptContext::impl::getKey(InputMapper* inputMapper, InputMapper::InputID id) {
    return inputMapper->getKey(id);
}

VirtualKey MainMenuScriptContext::impl::getDefaultKey(InputMapper* inputMapper, InputMapper::InputID id) {
    return inputMapper->get(id).defaultKey;
}

nString MainMenuScriptContext::impl::getKeyString(InputMapper* inputMapper, InputMapper::InputID id) {
    return nString(VirtualKeyStrings[inputMapper->getKey(id)]);
}

nString MainMenuScriptContext::impl::getDefaultKeyString(InputMapper* inputMapper, InputMapper::InputID id) {
    return nString(VirtualKeyStrings[inputMapper->get(id).defaultKey]);
}

nString MainMenuScriptContext::impl::getName(InputMapper* inputMapper, InputMapper::InputID id) {
    return inputMapper->get(id).name;
}

void MainMenuScriptContext::impl::onExit(MainMenuScreen* ownerScreen, int code) {
    ownerScreen->onQuit(nullptr, code);
}

void MainMenuScriptContext::impl::newGame(MainMenuScreen* ownerScreen) {
    ownerScreen->requestNewGame();
}

vecs::EntityID MainMenuScriptContext::impl::getTargetBody(MainMenuScreen* ownerScreen) {
    return ownerScreen->getMainMenuSystemViewer()->getTargetBody();
}

nString MainMenuScriptContext::impl::getBodyName(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return state->spaceSystem->namePosition.getFromEntity(entity).name;
}

nString MainMenuScriptContext::impl::getBodyParentName(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    auto parentOID = state->spaceSystem->orbit.getFromEntity(entity).parentOrbId;
    if (parentOID == 0) return "None";
    auto parentNpID = state->spaceSystem->orbit.get(parentOID).npID;
    return state->spaceSystem->namePosition.get(parentNpID).name;
}

nString MainMenuScriptContext::impl::getBodyTypeName(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
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

f32 MainMenuScriptContext::impl::getBodyMass(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->sphericalGravity.getFromEntity(entity).mass;
}

f32 MainMenuScriptContext::impl::getBodyDiameter(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->sphericalGravity.getFromEntity(entity).radius * 2.0f;
}

f32 MainMenuScriptContext::impl::getBodyRotPeriod(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->axisRotation.getFromEntity(entity).period;
}

f32 MainMenuScriptContext::impl::getBodyOrbPeriod(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).t;
}

f32 MainMenuScriptContext::impl::getBodyAxialTilt(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->axisRotation.getFromEntity(entity).tilt;
}

f32 MainMenuScriptContext::impl::getBodyEccentricity(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).e;
}

f32 MainMenuScriptContext::impl::getBodyInclination(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).i;
}

f32 MainMenuScriptContext::impl::getBodySemiMajor(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    return (f32)state->spaceSystem->orbit.getFromEntity(entity).a;
}

f32 MainMenuScriptContext::impl::getGravityAccel(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    auto& sgCmp = state->spaceSystem->sphericalGravity.getFromEntity(entity);
    f32 rad = (f32)(sgCmp.radius * M_PER_KM);
    return (f32)(M_G * sgCmp.mass / (rad * rad));
}

f32 MainMenuScriptContext::impl::getVolume(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    // TODO(Ben): Handle oblateness
    auto& sgCmp = state->spaceSystem->sphericalGravity.getFromEntity(entity);
    f32 rad = (f32)(sgCmp.radius * M_PER_KM);
    return (f32)(4.0 / 3.0 * M_PI * rad * rad * rad);
}

f32 MainMenuScriptContext::impl::getAverageDensity(MainMenuScreen* ownerScreen, vecs::EntityID entity) {
    SoaState* state = ownerScreen->getSoAState();
    // TODO(Ben): This is a double lookup
    f32 volume = getVolume(ownerScreen, entity);
    return (f32)(state->spaceSystem->sphericalGravity.getFromEntity(entity).mass / volume);
}
