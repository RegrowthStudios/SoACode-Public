#include "stdafx.h"
#include "SoaOptions.h"

#include <SDL/SDL.h>
#include <Vorb/io/IOManager.h>

#include "FileSystem.h"
#include "GameManager.h"
#include "InputMapper.h"

std::vector<ui32v2> SCREEN_RESOLUTIONS;

void SoaOptions::addOption(int id, const nString& name, OptionValue defaultValue, SoaOptionFlags flags) {

}

void SoaOptions::addOption(const nString& name, OptionValue defaultValue, SoaOptionFlags flags) {

}

void SoaOptions::addStringOption(const nString& name, const nString& defaultValue, const nString& value) {

}

bool SoaOptions::removeOption(int id) {

}

bool SoaOptions::removeOption(const nString& name) {

}

SoaOption* SoaOptions::get(int id) {

}

SoaOption* SoaOptions::get(const nString& name) {

}

SoaStringOption* SoaOptions::getStringOption(const nString& name) {

}

void SoaOptions::dispose() {

}
