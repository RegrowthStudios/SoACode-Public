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

DECL_VSOUND(class Engine)
class AmbienceLibrary;

/// Mixes ambient music for a game
/// TODO: Make this thread-safe and put it on another thread with begin/end
class AmbiencePlayer {
public:
    /// Setup all player references
    /// @param engine: Sound engine that will be used by this player
    /// @param library: Reference to library of ambient songs
    void init(vsound::Engine* engine, const AmbienceLibrary* library);
    /// Destroy all streams held by the player
    void dispose();

    /// Play a new mix of ambient music
    /// @param name: Name of the ambience (that must be found in the library) or "" to stop all music
    /// @param fadeTime: Amount of time until this stream becomes dominant
    void setToTrack(const nString& name, UNIT_SPACE(SECONDS) const f32& fadeTime);

    /// Update streams, loading in music tracks as necessary
    /// @param dt: Elapsed time since the last update
    /// TODO: return volume-change bool to allow intelligent sleeping
    void update(UNIT_SPACE(SECONDS) const f32& dt);

    const f32& getVolume() const { return m_volume; }
    void setVolume(f32 volume);
private:
    /// A stream with a controller and sound information
    struct SoundStream {
    public:
        AmbienceStream stream; ///< Stream controller
        vsound::Resource resource; ///< Sound's resource data
        vsound::Instance instance; ///< Playing sound handle
    };

    typedef std::unordered_map<nString, SoundStream> StreamMap;

    UNIT_SPACE(SECONDS) f32 m_timerDisposal = 0.0f; ///< Time until dead streams are destroyed
    vsound::Engine* m_engine = nullptr; ///< Reference to the sound engine
    const AmbienceLibrary* m_lib = nullptr; ///< Reference to library of ambient sounds
    StreamMap m_streams; ///< Currently playing ambience streams
    Random m_rand; ///< Random number generator

    f32 m_volume = 1.0f;
    nString currentAmbience = ""; ///< Current ambience type
    nString currentTrack = ""; ///< Currently playing track in the ambience stream
};

#endif // AmbiencePlayer_h__
