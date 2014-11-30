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
namespace vvox = vorb::voxel;
namespace vg = vorb::core::graphics;
namespace vui = vorb::ui;

#endif // VORB_H_