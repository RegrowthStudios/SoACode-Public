// 
//  Vorb.h
//  Vorb Engine
//
//  Created by Ben Arnold on 2/10/2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This is the main header for the vorb engine. It 
//  contains the namespace declarations.

#pragma once

#ifndef VORB_H_
#define VORB_H_

// Create empty namespaces for aliasing purposes
namespace vorb{
    namespace core{
        namespace graphics { }
    }
    namespace voxel { }
    namespace ui { }
}

//Namespace aliases
namespace vcore = vorb::core;
#define DECL_VCORE(CONTAINER, TYPE) namespace vorb { namespace core { CONTAINER TYPE; }  }
namespace vvox = vorb::voxel;
#define DECL_VVOX(CONTAINER, TYPE) namespace vorb { namespace voxel { CONTAINER TYPE; }  }
namespace vg = vorb::core::graphics;
#define DECL_VG(CONTAINER, TYPE) namespace vorb { namespace core { namespace graphics { CONTAINER TYPE; } }  }
namespace vui = vorb::ui;
#define DECL_VUI(CONTAINER, TYPE) namespace vorb { namespace ui { CONTAINER TYPE; }  }

#endif // VORB_H_