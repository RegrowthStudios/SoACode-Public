#include "stdafx.h"
#include "AmbienceStream.h"

const f32& AmbienceStream::getVolume() const {
    return m_ratio;
}

bool AmbienceStream::isAlive() const {
    return m_ratio > 0.0f || m_change > 0.0f;
}
bool AmbienceStream::isDying() const {
    return m_change < 0.0f;
}

void AmbienceStream::setDeathTime(const f32& seconds) {
    m_change = -m_ratio / seconds;
}
void AmbienceStream::setPeakTime(const f32& seconds) {
    m_change = (1.0f - m_ratio) / seconds;
}

bool AmbienceStream::update(const f32& dt) {
    if (m_change == 0.0f) return false;

    m_ratio += m_change * dt;
    if (m_change > 0.0f) {
        if (m_ratio >= 1.0f) {
            m_ratio = 1.0f;
            m_change = 0.0f;
        }
    } else {
        if (m_ratio <= 0.0f) {
            m_ratio = 0.0f;
            m_change = 0.0f;
        }
    }
    return true;
}
