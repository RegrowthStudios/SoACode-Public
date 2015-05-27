#include "stdafx.h"
#include "AmbiencePlayer.h"

#include <Vorb/sound/SoundEngine.h>

#include "AmbienceLibrary.h"

#define AMBIENCE_PLAYER_DISPOSAL_RATE 2.0f

void AmbiencePlayer::init(vsound::Engine* engine, const AmbienceLibrary* library) {
    m_engine = engine;
    m_lib = library;
}

void AmbiencePlayer::dispose() {
    m_engine = nullptr;
    m_lib = nullptr;
}

void AmbiencePlayer::setToTrack(const nString& name, const f32& fadeTime) {
    // Set current ambience
    currentAmbience = name;

    if (!currentAmbience.empty()) {
        // Make one track come alive
        auto& track = m_streams.find(currentAmbience);
        if (track != m_streams.end()) {
            // Reset track to be alive
            track->second.stream.setPeakTime(fadeTime);
        } else {
            // Add a new stream
            SoundStream stream;
            stream.stream.setPeakTime(fadeTime);
            m_streams[name] = stream;
        }
    }

    // Make all other tracks fade away
    for (auto& kvp : m_streams) {
        if (kvp.first != currentAmbience) {
            kvp.second.stream.setDeathTime(fadeTime);
        }
    }
}

void AmbiencePlayer::update(const f32& dt) {
    if (!m_engine || !m_lib) return;

    m_timerDisposal += dt;

    // Update volumes if necessary
    for (auto& kvp : m_streams) {
        SoundStream& stream = kvp.second;
        bool soundChanged = stream.stream.update(dt);

        if (stream.stream.isAlive()) {
            if (!stream.resource.isNull() && !stream.instance.isPlaying()) {
                // Delete a finished track
                m_engine->freeSound(stream.resource);
            }

            if (!stream.stream.isDying() && stream.resource.isNull()) {
                // Get a new track
                AmbienceLibrary::Track track = m_lib->getTrack(currentAmbience, m_rand);
                currentTrack = track.first;

                // Load and play the track
                stream.resource = m_engine->loadSound(track.second, false, true);
                stream.instance = m_engine->createInstance(stream.resource);
                stream.instance.setLooped(false);
                stream.instance.setVolume(stream.stream.getVolume() * m_volume);
                stream.instance.play();
            }

            // Update volume
            if (soundChanged) {
                stream.instance.setVolume(stream.stream.getVolume() * m_volume);
            }
        }
    }

    // Dispose of old streams
    if (m_timerDisposal > AMBIENCE_PLAYER_DISPOSAL_RATE) {
        m_timerDisposal = 0.0f;

        StreamMap updated;
        for (auto& kvp : m_streams) {
            if (kvp.second.stream.isAlive()) {
                updated[kvp.first] = kvp.second;
            } else {
                if (!kvp.second.resource.isNull()) {
                    m_engine->freeSound(kvp.second.resource);
                }
            }
        }
        m_streams.swap(updated);
    }
}

void AmbiencePlayer::setVolume(f32 volume) {
    m_volume = volume;
    // Update volumes if necessary
    for (auto& kvp : m_streams) {
        SoundStream& stream = kvp.second;

        if (stream.stream.isAlive()) {
            stream.instance.setVolume(stream.stream.getVolume() * m_volume);
        }
    }
}