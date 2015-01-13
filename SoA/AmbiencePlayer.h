///
/// AmbiencePlayer.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Plays ambience tracks with cross-fading
///

#pragma once

#ifndef AmbiencePlayer_h__
#define AmbiencePlayer_h__

#include <Vorb/Random.h>
#include <Vorb/sound/SoundInstance.h>
#include <Vorb/sound/SoundResource.h>
#include <Vorb/VorbPreDecl.inl>

#include "AmbienceStream.h"

DECL_VSOUND(class, Engine)
class AmbienceLibrary;

class AmbiencePlayer {
public:
    struct SoundStream {
    public:
        AmbienceStream stream;
        vsound::Resource resource;
        vsound::Instance instance;
    };
    typedef std::unordered_map<nString, SoundStream> StreamMap;

    void init(vsound::Engine* engine, const AmbienceLibrary* library);
    void dispose();

    void setToTrack(const nString& name, const f32& fadeTime);

    void update(const f32& dt);
private:
    f32 m_timerDisposal = 0.0f;
    vsound::Engine* m_engine = nullptr;
    const AmbienceLibrary* m_lib = nullptr;
    StreamMap m_streams;
    Random m_rand;

    nString currentAmbience = "";
    nString currentTrack = "";
};

#endif // AmbiencePlayer_h__
