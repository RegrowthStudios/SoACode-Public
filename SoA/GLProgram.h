// 
//  GLProgram.h
//  Vorb Engine
//
//  Created by Cristian Zaloj in 2014
//  Modified by Ben Arnold on 16 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a GLProgram class that 
//  provides a wrapper around an openGL shader program
//

#pragma once

#ifndef GLPROGRAM_H_
#define GLPROGRAM_H_

#include "Vorb.h"

namespace vorb {
    namespace core {
        namespace graphics {
            enum class ShaderType;

            // Encapsulates A Simple OpenGL Program And Its Shaders
            class GLProgram {
            public:
                /// Create And Possibly Initialize Program
                /// @param init: set to true to also call init()
                GLProgram(bool init = false);
                ~GLProgram();

                /// Create GPU Resources
                void init();
                /// Free all resources
                void destroy();
                /// Returns true if the program was created
                bool getIsCreated() const {
                    return _id != 0;
                }
                /// Returns the program ID
                int getID() const  {
                    return _id;
                }

                /// Attatches shader to the build information
                /// @param type: the type of shader
                /// @param src: the shader source
                /// @return true on success, false on failure
                bool addShader(ShaderType type, const cString src);
                /// Attatches shader to the build information
                /// @param type: the type of shader
                /// @param src: the shader file name
                /// @return true on success, false on failure
                bool addShaderFile(ShaderType type, const cString file);

                /// Sets an attribute
                /// @param name: the attribute name
                /// @param index: the attribute index
                void setAttribute(nString name, ui32 index);
                /// Sets a list of attribute
                /// @param attr: map of attributes
                void setAttributes(const std::map<nString, ui32>& attr);
                /// Sets a vector of attributes
                /// @param attr: vector of attributes
                void setAttributes(const std::vector<std::pair<nString, ui32> >& attr);
                /// Sets a vector of attributes
                /// @param attr: vector of attributes. Uses array index as element
                void setAttributes(const std::vector<nString>& attr);

                /// Links the shader program. Should be called
                /// after shaders are added
                bool link();
                /// Returns true if the program is linked
                bool getIsLinked() const {
                    return _isLinked;
                }

                /// Creates mappings for attributes
                void initAttributes();
                /// Creates mappings for uniforms
                void initUniforms();

                /// Gets an attribute index
                /// @param name: the attribute to get the index for
                /// returns the integer attribute index
                inline const ui32& getAttribute(const nString& name) const {
                    return _attributes.at(name);
                }
                /// Gets a uniform index
                /// @param name: the uniform to get the index for
                /// returns the integer uniform index
                inline const ui32& getUniform(const nString& name) const {
                    return _uniforms.at(name);
                }

                /// Enables all vertex attrib arrays used by the program
                void enableVertexAttribArrays() const;

                /// Disables all vertex attrib arrays used by the program
                void disableVertexAttribArrays() const;

                /// Tell the GPU to use the program
                void use();

                /// Will unuse whatever program is currently in use
                static void unuse();

                /// Returns true if the program is in use
                bool getIsInUse() const {
                    return _programInUse == this;
                }

                /// Returns the current program that is in use
                static GLProgram* getCurrentProgram() {
                    return _programInUse;
                }
            private:
                // The Current Program In Use
                static GLProgram* _programInUse;

                // Program ID
                ui32 _id;

                // Shader IDs
                ui32 _idVS, _idFS;

                // True On A Successful Link
                bool _isLinked;

                // Attribute And Uniform Map
                std::map<nString, ui32> _attributes;
                std::map<nString, ui32> _uniforms;
            };

        }
    }
}

namespace vg = vorb::core::graphics;

#endif // GLPROGRAM_H_