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

typedef std::unordered_map<nString, vpath> AmbienceList; ///< List of tracks (name, file) for an ambience type

/// Library of music tracks organized by ambience
class AmbienceLibrary {
public:
    typedef std::pair<nString, vpath> Track; ///< A named music file

    /// Add a track to the library
    /// @param list: Ambience type
    /// @param name: Name of the track
    /// @param file: File for the track
    void addTrack(const nString& list, const nString& name, const vpath& file);

    /// Obtain a list of tracks for a certain type of ambience
    /// @param name: Ambience type
    /// @return The list of tracks 
    const AmbienceList& getTracks(const nString& name) const;
    /// Obtain a random track in an ambient list
    /// @param list: Ambience type
    /// @param rGen: Random generator
    /// @return A random track that can be played
    Track getTrack(const nString& list, Random& rGen) const;
private:
    std::unordered_map<nString, AmbienceList> m_lists; ///< Lists of ambiences that may play
};

#endif // AmbienceLibrary_h__
