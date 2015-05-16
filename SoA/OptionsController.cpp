#include "stdafx.h"
#include "OptionsController.h"

OptionsController::OptionsController() {
    // Empty
}

OptionsController::~OptionsController() {
    // Empty
}

void OptionsController::setDefault() {
    m_tempCopy = soaOptions;
    m_default = soaOptions; // TODO(Ben): This is wrong
}

void OptionsController::beginContext() {
    m_tempCopy = soaOptions;
}

void OptionsController::loadOptions() {
    // TODO(Ben): Implement
}

void OptionsController::saveOptions() {
    soaOptions = m_tempCopy;
}

void OptionsController::restoreDefault() {
    soaOptions = m_default;
    m_tempCopy = m_default;
}

void OptionsController::registerScripting(vscript::Environment* env) {
    env->setNamespaces("Options");
    env->addCDelegate("setOptionInt", makeDelegate(*this, &OptionsController::setOptionInt));
    env->addCDelegate("setOptionFloat", makeDelegate(*this, &OptionsController::setOptionFloat));
    env->addCDelegate("setOptionBool", makeDelegate(*this, &OptionsController::setOptionBool));
    env->addCRDelegate("getOptionInt", makeRDelegate(*this, &OptionsController::getOptionInt));
    env->addCRDelegate("getOptionFloat", makeRDelegate(*this, &OptionsController::getOptionFloat));
    env->addCRDelegate("getOptionBool", makeRDelegate(*this, &OptionsController::getOptionBool));
    env->addCDelegate("beginContext", makeDelegate(*this, &OptionsController::beginContext));
    env->addCDelegate("save", makeDelegate(*this, &OptionsController::saveOptions));
    env->addCDelegate("load", makeDelegate(*this, &OptionsController::loadOptions));
    env->addCDelegate("restoreDefault", makeDelegate(*this, &OptionsController::restoreDefault));
    env->setNamespaces();
}

void OptionsController::setOptionInt(nString optionName, int val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.i != val) {
        option.value.i = val;
    }
}

void OptionsController::setOptionFloat(nString optionName, f32 val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.f != val) {
        option.value.f = val;
    }
}

void OptionsController::setOptionBool(nString optionName, bool val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.b != val) {
        option.value.b = val;
    }
}

int OptionsController::getOptionInt(nString optionName) {
    return m_tempCopy.get(optionName).value.i;
}

f32 OptionsController::getOptionFloat(nString optionName) {
    return m_tempCopy.get(optionName).value.f;
}

bool OptionsController::getOptionBool(nString optionName) {
    return m_tempCopy.get(optionName).value.b;
}
