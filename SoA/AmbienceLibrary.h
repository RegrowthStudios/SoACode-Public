///
/// AmbienceLibrary.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Library of all ambience tracks
///

#pragma once

#ifndef AmbienceLibrary_h__
#define AmbienceLibrary_h__

#include <Vorb/IO.h>
#include <Vorb/Random.h>

typedef std::unordered_map<nString, vpath> AmbienceList;

class AmbienceLibrary {
public:
    typedef std::pair<nString, vpath> Track;

    void addTrack(const nString& list, const nString& name, const vpath& file);

    const AmbienceList& getTracks(const nString& name) const;
    Track getTrack(const nString& list, Random& rGen) const;
private:
    std::unordered_map<nString, AmbienceList> m_lists; ///< Lists of ambiences that may play
};

#endif // AmbienceLibrary_h__
