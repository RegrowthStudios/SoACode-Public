//
// DLLAPI.h
//

#pragma once

#ifndef DLLAPI_h__

namespace DLLAPI {
    struct DLLInformation {
        const cString name;
        const cString friendlyName;

        union {
            struct {
                i32 major : 8;
                i32 minor : 12;
                i32 revision : 12;
            };
            i32 id;
        } version;
    };

    typedef DLLInformation (*FuncRetrieveInformation)();
}

#endif // DLLAPI_h__
