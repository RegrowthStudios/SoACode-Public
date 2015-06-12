#include "stdafx.h"
#include "OptionsController.h"

#include <fstream>

OptionsController::OptionsController(const nString& filePath /*= "Data/options.ini"*/) :
    m_filePath(filePath) {
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

// TODO(Ben): Better parsing
bool OptionsController::loadOptions() {
    std::ifstream file(m_filePath);
    if (file.fail()) return false;
    nString token;
    nString dummy;
    while (std::getline(file, token, ':')) {
        int id = soaOptions.findID(token);
        if (id != -1) {
            SoaOption& opt = soaOptions.get(id);
            switch (opt.value.type) {
                case OptionValueType::F32:
                    file >> opt.value.f; break;
                case OptionValueType::I32:
                    file >> opt.value.i; break;
                case OptionValueType::BOOL:
                    file >> opt.value.b; break;
                case OptionValueType::CHAR:
                    file >> opt.value.c; break;
                default:
                    file >> dummy; break;
            }
            char nl;
            file.get(nl);
        }
    }
    return true;
}

void OptionsController::saveOptions() {
    soaOptions = m_tempCopy;
    OptionsChange();

    std::ofstream file(m_filePath);
    if (file.fail()) return;

    auto& options = soaOptions.getOptions();
    for (int id = 0; id < (int)options.size(); id++) {
        auto& opt = options[id];
        // Don't duplicated app.config
        if (id != OPT_SCREEN_HEIGHT && id != OPT_SCREEN_WIDTH &&
            id != OPT_FULLSCREEN && id != OPT_BORDERLESS && id != OPT_VSYNC) {
            file << opt.name << ": ";
            switch (opt.value.type) {
                case OptionValueType::F32:
                    file << opt.value.f << '\n'; break;
                case OptionValueType::I32:
                    file << opt.value.i << '\n'; break;
                case OptionValueType::BOOL:
                    file << opt.value.b << '\n'; break;
                case OptionValueType::CHAR:
                    file << opt.value.c << '\n'; break;
                default:
                    break;
            }
        }
    }
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
    env->addCRDelegate("load", makeRDelegate(*this, &OptionsController::loadOptions));
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
