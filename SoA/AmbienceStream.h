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

class AmbienceStream {
public:
    const f32& getVolume() const;
    bool isAlive() const;
    bool isDying() const;

    void setDeathTime(const f32& seconds);
    void setPeakTime(const f32& seconds);

    bool update(const f32& dt);
private:
    f32 m_ratio = 0.0f; ///< The current liveliness level of the stream [0,1]
    f32 m_change = 0.0f; ///< The rate of change of the ratio per second
};

#endif // AmbienceStream_h__
