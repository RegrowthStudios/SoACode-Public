#include "stdafx.h"
#include "SoaOptions.h"

#include <SDL/SDL.h>
#include <Vorb/io/IOManager.h>

#include "GameManager.h"
#include "InputMapper.h"

SoaOptions soaOptions;

SoaOptions::SoaOptions() {
    m_options.reserve(OPT_NUM_OPTIONS);
}

SoaOptions::~SoaOptions() {
    // Empty
}

void SoaOptions::addOption(int id, const nString& name, OptionValue defaultValue, SoaOptionFlags flags) {
    if (id >= (int)m_options.size()) {
        m_options.resize(id + 1);
        m_options[id].id = id;
        m_options[id].name = name;
        m_options[id].defaultValue = defaultValue;
        m_options[id].value = defaultValue;
        m_options[id].flags = flags;
        m_optionsLookup[name] = id;
    }
}

void SoaOptions::addOption(const nString& name, OptionValue defaultValue, SoaOptionFlags flags) {
    m_options.emplace_back();
    SoaOption& option = m_options.back();
    option.id = m_options.size() - 1;
    option.name = name;
    option.defaultValue = defaultValue;
    option.value = defaultValue;
    option.flags = flags;
    m_optionsLookup[name] = option.id;
}

void SoaOptions::addStringOption(const nString& name, const nString& defaultValue) {
    SoaStringOption option;
    option.name = name;
    option.defaultValue = defaultValue;
    option.value = defaultValue;
    m_stringOptionsLookup[name] = option;
}

int SoaOptions::findID(const nString& name) {
    auto& it = m_optionsLookup.find(name);
    if (it == m_optionsLookup.end()) return -1;
    return it->second;
}

SoaOption* SoaOptions::find(const nString& name) {
    auto& it = m_optionsLookup.find(name);
    if (it == m_optionsLookup.end()) return nullptr;
    return &m_options[it->second];
}

SoaOption& SoaOptions::get(int id) {
    return m_options.at(id);
}

SoaOption& SoaOptions::get(const nString& name) {
    return m_options.at(m_optionsLookup[name]);
}

SoaStringOption& SoaOptions::getStringOption(const nString& name) {
    return m_stringOptionsLookup[name];
}

void SoaOptions::dispose() {
    std::vector<SoaOption>().swap(m_options);
    std::map<nString, int>().swap(m_optionsLookup);
    std::map<nString, SoaStringOption>().swap(m_stringOptionsLookup);
}
