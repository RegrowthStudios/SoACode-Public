#pragma once
#include <FMOD/fmod.hpp> //FMOD studio
#include <FMOD/fmod_errors.h>

#include "Constants.h"

// TODO: Remove This
using namespace std;

enum SOUND_TYPE {S_DEFAULT, S_MUSIC_CAVE, S_MUSIC_WIND, S_MUSIC_RANDOM, S_RANDOM_NIGHT, S_RANDOM_DAY, S_LAST};

class FmodSound {
public:
    FmodSound(){};
    FmodSound(FMOD::Sound *Sound)
    {
        sound = Sound;
    }

    bool isStream, is3D;
    float volume;
    float minDistance;
    FMOD::Sound *sound;
};


class SoundEngine
{
public:
    SoundEngine(){
        _isInitialized = 0;
        _isOn = 1;
        _lastFrameTicks = 0;
        _lastSongTicks = 0;
        _lastRandomTicks = 0;
        _songFrequency = 10000;
        _randomFrequency = 12000;
        _currentSongChannel = NULL;
        _currentRandomChannel = NULL;
    };
    ~SoundEngine()
    {
        if (_isInitialized){
            _system->release(); //TODO:: release sounds
        }
    }
    void Initialize();
    void LoadAllSounds();
    void update(const f64v3 &listenerPos = f64v3(0.0), const f32v3 &listenerDir = f32v3(0.0f, 0.0f, 1.0f), const f32v3 &listenerUp = f32v3(0.0f, 1.0f, 0.0f));
    bool AddSound(const char *name, const char *filename, SOUND_TYPE soundType = S_DEFAULT, bool stream = 0, bool is3D = 1, float minDistance = 1.0f);
    bool RemoveSound(const char *name);
    void PlayExistingSound(const char *name, bool loop, float volume, bool song, glm::dvec3 position = glm::dvec3(0.0), int callBackType = 0);
    void SetSound (bool s);
    bool GetSound ();

    void SetMusicVolume(float v);
    void SetEffectVolume(float v);

private:
    bool _isInitialized;
    bool _isOn; //is sound on?

    FMOD_VECTOR _listenerPosition, _listenerDirection, _listenerUp;

    FMOD_RESULT _result;
    FMOD::Channel *_currentSongChannel;
    FMOD::Channel *_currentRandomChannel;
    FMOD::System *_system;

    unsigned int _lastFrameTicks;
    unsigned int _lastSongTicks;
    unsigned int _lastRandomTicks;
    unsigned int _songFrequency;
    unsigned int _randomFrequency;

    vector <string> _soundTypes[S_LAST];

    map <string, FmodSound> _soundMap;
    FMOD::ChannelGroup *_songGroup, *_effectGroup;
};