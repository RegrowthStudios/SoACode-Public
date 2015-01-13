// 
//  GLProgramManager.h
//  Vorb Engine
//
//  Created by Ben Arnold on 16 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a GLProgram Manager that serves as a
//  registry for opengl program objects.
//
#pragma once

#ifndef GLPROGRAMMANAGER_H_
#define GLPROGRAMMANAGER_H_

#include <unordered_map>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/io/IOManager.h>

namespace vorb {
    namespace core {
        namespace graphics {

            class GLProgramManager
            {
            public:
                GLProgramManager();
                ~GLProgramManager();

                /// Creates a shader and adds it to a cache of programs
                /// @param shaderName: The string handle for the shader. Use this string
                /// to get the shader in the future.
                /// @param vertexPath: The file path for the vertex shader
                /// @param fragmentPath: The file path for the fragment shader
                /// @param attr: The vector of attributes for the shader. If it is nullptr
                /// attributes will be set automatically
                void addProgram(nString shaderName, cString vertexPath, cString fragmentPath, const std::vector<nString>* attr = nullptr, cString defines = nullptr);
                /// Adds an existing shader to the cache of programs
                /// @param shaderName: The string handle for the shader. Use this string
                /// to get the shader in the future.
                /// @param program: The program to add
                void addProgram(nString shaderName, GLProgram* program);

                /// Gets a shader from the cache
                /// returns nullptr if the shader doesn't exist
                GLProgram* getProgram(nString shaderName) const;

                /// Frees all resources
                void destroy();

            private:

                // Cache of GLProgram objects
                std::unordered_map<nString, GLProgram*> _programs;
            };
        }
    }
}

namespace vg = vorb::core::graphics;

#endif //GLPROGRAMMANAGER_H_