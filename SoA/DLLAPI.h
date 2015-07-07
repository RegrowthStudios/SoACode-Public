//
// DLLAPI.h
//

#pragma once

#ifndef DLLAPI_h__

namespace DLLAPI {
    struct Information {
        const cString name; ///< The name of the DLL
        const cString friendlyName; ///< A human readable form of the DLL

        union {
            struct {
                i32 major : 8;
                i32 minor : 12;
                i32 revision : 12;
            };
            i32 id;
        } version; ///< Versioning information
    };

    typedef void (*FuncRetrieveInformation)(DLLAPI::Information* info);
    typedef void (*FuncFillFuntionTable)(void*** table, size_t* count);

}

#endif // DLLAPI_h__
