#include "stdafx.h"
#include "Sound.h"

#include <SDL/SDL_timer.h>

#include "Errors.h"
#include "GameManager.h"
#include "global.h"

#define ERRCHECK(result) if ((result) != FMOD_OK){ pError(("FMOD error! (" + std::to_string(result) + ") " + FMOD_ErrorString((result)) + "\n").c_str()); }

bool songPlaying = 0;
GLuint songStartTicks = 0;

//loads a sound file
bool SoundEngine::AddSound(const char *name, const char *filename, SOUND_TYPE soundType, bool stream, bool is3D, float minDistance) {

    auto it = _soundMap.find(name);
    if (it != _soundMap.end()) {
        pError((nString("FMOD sound ") + name + " already loaded!").c_str());
        return 0;
    }

    FmodSound newFmodSound;
    FMOD::Sound *newSound;

    FMOD_MODE mode = 0;
    if (is3D) mode |= FMOD_3D;

    newFmodSound.minDistance = minDistance;
    newFmodSound.volume = 1.0f;
    newFmodSound.isStream = stream;
    newFmodSound.is3D = is3D;

    if (stream) {
        _result = _system->createStream(filename, mode, 0, &newSound);
        ERRCHECK(_result);
    } else {
        _result = _system->createSound(filename, mode, 0, &newSound);
        ERRCHECK(_result);
    }
    newFmodSound.sound = newSound;
    _soundMap[name] = newFmodSound;
    _soundTypes[soundType].push_back(name);

    return 1;
}

FMOD_RESULT F_API SongDoneCallback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2) {
    if (type == FMOD_CHANNEL_CALLBACKTYPE::FMOD_CHANNEL_CALLBACKTYPE_END) {
        songStartTicks = SDL_GetTicks();
        songPlaying = 0;
    }
    return FMOD_OK;
}

void SoundEngine::PlayExistingSound(const char *name, bool loop, float volume, bool song, glm::dvec3 position, int callBackType) {
    auto it = _soundMap.find(name);
    if (it == _soundMap.end()) {
        pError(((nString)"Fmod sound " + name + "was not playable!").c_str());
        return;
    }

    FmodSound *sound = &(it->second);
    //sound->volume = volume;
    if (_isOn) {
        FMOD::Channel *channel;

        _result = _system->playSound(FMOD_CHANNELINDEX::FMOD_CHANNEL_FREE, sound->sound, true, &channel);
        ERRCHECK(_result);

        _result = channel->setVolume(volume);        // Set the volume while it is paused.
        ERRCHECK(_result);

        if (loop) channel->setMode(FMOD_LOOP_NORMAL);

        if (sound->is3D) {
            _result = channel->set3DMinMaxDistance(sound->minDistance, 20000);
            ERRCHECK(_result);

            _listenerPosition.x = position.x*0.5;
            _listenerPosition.y = position.y*0.5;
            _listenerPosition.z = -position.z*0.5;
            _result = channel->set3DAttributes(&_listenerPosition, NULL);
            ERRCHECK(_result);
        }

        if (song) {
            _result = channel->setChannelGroup(_songGroup);
            ERRCHECK(_result);
        } else {
            _result = channel->setChannelGroup(_effectGroup);
            ERRCHECK(_result);
        }

        switch (callBackType) {
        case 1:
            _result = channel->setCallback(SongDoneCallback);
            ERRCHECK(_result);
            break;
        }

        _result = channel->setPaused(false);        // This is where the sound really starts.
        ERRCHECK(_result);
    }
}


void SoundEngine::SetSound(bool s) {
    _isOn = s;
}

//tells whether the sound is on or off
bool SoundEngine::GetSound() {
    return _isOn;
}

void SoundEngine::Initialize() {
    //create the sound system. If fails, sound is set to impossible
    _result = FMOD::System_Create(&_system);        // Create the main system object.
    ERRCHECK(_result);

    unsigned int version;

    _result = _system->getVersion(&version);
    ERRCHECK(_result);

    if (version < FMOD_VERSION) {
        printf("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
        pError("FMOD version error. See Command Prompt.");
    }

    _result = _system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
    ERRCHECK(_result);

    //Set the distance units. (meters/feet etc).
    _result = _system->set3DSettings(1.0, 1.0f, 1.0f);
    ERRCHECK(_result);

    _result = _system->createChannelGroup("SONGS", &_songGroup);
    ERRCHECK(_result);
    _result = _system->createChannelGroup("SOUNDS", &_effectGroup);
    ERRCHECK(_result);

    _isInitialized = true;
}

void SoundEngine::update(const glm::dvec3 &listenerPos, const glm::vec3 &listenerDir, const glm::vec3 &listenerUp) {
    _listenerPosition.x = listenerPos.x*0.5;
    _listenerPosition.y = listenerPos.y*0.5;
    _listenerPosition.z = -listenerPos.z*0.5;
    _listenerDirection.x = listenerDir.x;
    _listenerDirection.y = listenerDir.y;
    _listenerDirection.z = -listenerDir.z;
    _listenerUp.x = listenerUp.x;
    _listenerUp.y = listenerUp.y;
    _listenerUp.z = -listenerUp.z;

    _system->set3DListenerAttributes(0, &_listenerPosition, NULL, &_listenerDirection, &_listenerUp);

    _system->update();

    if (songPlaying == 0) {
        //if (GameManager::gameState == GameStates::MAINMENU) {
        //    if (SDL_GetTicks() - songStartTicks >= 0) { //play the song once every half minute
        //        PlayExistingSound("TitleMusic", 0, 1.0f, 1, glm::dvec3(0.0), 1);
        //        songPlaying = 1;
        //    }
        //} else {
        //    if (SDL_GetTicks() - songStartTicks >= 30000) { //play the song once every half minute
        //        PlayExistingSound("ThemeMusic", 0, 1.0f, 1, glm::dvec3(0.0), 1);
        //        songPlaying = 1;
        //    }
        //}
    }

    _lastFrameTicks = SDL_GetTicks();
}

void SoundEngine::LoadAllSounds() {
    //effects
    AddSound("PlaceBlock", "Sounds/Effects/Block/standardvariation1.ogg", S_DEFAULT, false, true, 2.0f);
    AddSound("BreakBlock", "Sounds/Effects/Block/Breakblock.ogg", S_DEFAULT, false, true, 2.0f);
    AddSound("Explosion", "Sounds/Effects/Block/Nitro1.ogg", S_DEFAULT, false, true, 8.0f);

    //music
    AddSound("CaveMusic", "Sounds/Music/cavemusic1.ogg", S_MUSIC_CAVE, true, false);
    AddSound("ThemeMusic", "Sounds/Music/SOATheme.ogg", S_MUSIC_RANDOM, true, false);
    AddSound("TitleMusic", "Sounds/Music/Andromeda Menu.ogg", S_MUSIC_RANDOM, true, false);
    AddSound("WindSong", "Sounds/Music/windsong.ogg", S_MUSIC_WIND, true, false);

    //menu
    AddSound("MenuRollover", "Sounds/Effects/UI/menurollover.ogg", S_MUSIC_WIND, true, false);
}

void SoundEngine::SetMusicVolume(float v) {
    _songGroup->setVolume(v);
}

void SoundEngine::SetEffectVolume(float v) {
    _effectGroup->setVolume(v);
}