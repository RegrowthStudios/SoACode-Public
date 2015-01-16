///
/// AmbienceStream.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A stream of music created from a list of ambient music
///

#pragma once

#ifndef AmbienceStream_h__
#define AmbienceStream_h__

/// Encapsulates a stream of ambient music
class AmbienceStream {
public:
    /// @return The relative volume of the stream [0,1]
    const f32& getVolume() const;
    /// @return True if this stream is in a playing state
    bool isAlive() const;
    /// @return True if this stream is fading away
    bool isDying() const;

    /// Set this stream to fade away to nothingness
    /// @param seconds: Time until this stream is dead
    void setDeathTime(UNIT_SPACE(SECONDS) const f32& seconds);
    /// Set this stream to come alive from its current state
    /// @param seconds: Time until this stream is fully audible
    void setPeakTime(UNIT_SPACE(SECONDS) const f32& seconds);

    /// Update a stream of music
    /// @param dt: Elapsed time
    /// @return True if the volume changed
    bool update(UNIT_SPACE(SECONDS) const f32& dt);
private:
    f32 m_ratio = 0.0f; ///< The current liveliness level of the stream [0,1]
    f32 m_change = 0.0f; ///< The rate of change of the ratio per second
};

#endif // AmbienceStream_h__
