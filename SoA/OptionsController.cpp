#include "stdafx.h"
#include "OptionsController.h"

OptionsController::OptionsController(const nString& filePath /*= "Data/options.ini"*/) {
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
    OptionsChange();
}

void OptionsController::restoreDefault() {
    soaOptions = m_default;
    m_tempCopy = m_default;
    OptionsChange();
}

void OptionsController::registerScripting(vscript::Environment* env) {
    env->setNamespaces("Options");
    env->addCDelegate("setInt", makeDelegate(*this, &OptionsController::setInt));
    env->addCDelegate("setFloat", makeDelegate(*this, &OptionsController::setFloat));
    env->addCDelegate("setBool", makeDelegate(*this, &OptionsController::setBool));
    env->addCRDelegate("getInt", makeRDelegate(*this, &OptionsController::getInt));
    env->addCRDelegate("getFloat", makeRDelegate(*this, &OptionsController::getFloat));
    env->addCRDelegate("getBool", makeRDelegate(*this, &OptionsController::getBool));
    env->addCDelegate("beginContext", makeDelegate(*this, &OptionsController::beginContext));
    env->addCDelegate("save", makeDelegate(*this, &OptionsController::saveOptions));
    env->addCDelegate("load", makeDelegate(*this, &OptionsController::loadOptions));
    env->addCDelegate("restoreDefault", makeDelegate(*this, &OptionsController::restoreDefault));
    env->setNamespaces();
}

void OptionsController::setInt(nString optionName, int val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.i != val) {
        option.value.i = val;
    }
}

void OptionsController::setFloat(nString optionName, f32 val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.f != val) {
        option.value.f = val;
    }
}

void OptionsController::setBool(nString optionName, bool val) {
    auto& option = m_tempCopy.get(optionName);
    if (option.value.b != val) {
        option.value.b = val;
    }
}

int OptionsController::getInt(nString optionName) {
    return m_tempCopy.get(optionName).value.i;
}

f32 OptionsController::getFloat(nString optionName) {
    return m_tempCopy.get(optionName).value.f;
}

bool OptionsController::getBool(nString optionName) {
    return m_tempCopy.get(optionName).value.b;
}
