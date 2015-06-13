//
// MetaSection.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 25 May 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Provides a method of
//

#pragma once

#ifndef MetaSection_h__
#define MetaSection_h__

typedef ui32 MetaID;
typedef void* MetaData;

/*! @brief A description of a field in the metadata
 */
struct MetaFieldInformation {
    MetaID id; ///< The unique ID of the field
    
    UNIT_SPACE(BYTES) size_t offset; ///< The offset of the value in the section
    UNIT_SPACE(BYTES) size_t size; ///< The total size of the section

    nString name; ///< The section's string description
};

/*! @brief Stores and keeps track of all metadata fields a metadata section will have.
 */
class MetaSection {
public:
    template<typename T = void>
    static T* retrieve(const MetaData& data, const MetaFieldInformation& information) {
        return (T*)((ui8*)data + information.offset);
    }

    MetaID add(size_t s, const nString& name, size_t alignment = 1);
    
    MetaFieldInformation& operator[] (MetaID id);
    const MetaFieldInformation& operator[] (MetaID id) const;

    /*! @brief Retrieve the total size of the metadata section to be able to accommodate all fields.
     * 
     * @return The total size of the section.
     */
    UNIT_SPACE(BYTES) const size_t& getTotalSize() const {
        return m_size;
    }
private:
    std::vector<MetaFieldInformation> m_fields; ///< All the fields that this metadata section contains
    size_t m_size = 0; ///< The total size of a meta-section
};

#endif // MetaSection_h__
