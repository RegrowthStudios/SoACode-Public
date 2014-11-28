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

#include "Events.hpp"
#include "gtypes.h"
#include "GLenums.h"
#include "Vorb.h"

#define GL_PROGRAM_DEFAULT_SHADER_VERSION_MAJOR 1
#define GL_PROGRAM_DEFAULT_SHADER_VERSION_MINOR 3
#define GL_PROGRAM_DEFAULT_SHADER_VERSION_REVISION 0

namespace vorb {
    namespace core {
        namespace graphics {
            enum class ShaderType;

            /// Holds information necessary for shader compilation
            struct ShaderSource {
                ShaderType stage = ShaderType::VERTEX_SHADER; ///< Shader's pipeline stage

                i32 versionMajor = GL_PROGRAM_DEFAULT_SHADER_VERSION_MAJOR; ///< GLSL major version
                i32 versionMinor = GL_PROGRAM_DEFAULT_SHADER_VERSION_MINOR; ///< GLSL minor version
                i32 versionRevision = GL_PROGRAM_DEFAULT_SHADER_VERSION_REVISION;  ///< GLSL revision version

                std::vector<const cString> sources; ///< Strings of shader source code awaiting concatenation
            };

            // Encapsulates a simple OpenGL program and its shaders
            class GLProgram {
            public:
                typedef std::pair<nString, VGAttribute> AttributeBinding; ///< Binds attribute names to locations
                typedef std::map<nString, VGAttribute> AttributeMap; ///< Dictionary of attribute locations by name
                typedef std::map<nString, VGUniform> UniformMap; ///< Dictionary of uniform locations by name

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
                const VGProgram& getID() const  {
                    return _id;
                }

                /// Attaches shader to the build information
                /// @param data: Shader's type, code, and version
                /// @return true on success, false on failure
                bool addShader(const ShaderSource& data);

                /// Sets an attribute
                /// @param name: the attribute name
                /// @param index: the attribute index
                void setAttribute(nString name, VGAttribute index);
                /// Sets a list of attribute
                /// @param attr: map of attributes
                void setAttributes(const std::map<nString, VGAttribute>& attr);
                /// Sets a vector of attributes
                /// @param attr: vector of attributes
                void setAttributes(const std::vector<AttributeBinding>& attr);
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
                const ui32& getAttribute(const nString& name) const {
                    return _attributes.at(name);
                }
                /// Gets a uniform index
                /// @param name: the uniform to get the index for
                /// returns the integer uniform index
                const ui32& getUniform(const nString& name) const {
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

                Event<nString> onShaderCompilationError;
                Event<nString> onProgramLinkError;
            private:
                // The Current Program In Use
                static GLProgram* _programInUse;

                // Program ID
                VGProgram _id = 0;

                // Shader IDs
                VGShader _idVS = 0, _idFS = 0;

                // True On A Successful Link
                bool _isLinked = false;

                // Attribute And Uniform Map
                AttributeMap _attributes;
                UniformMap _uniforms;
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // GLPROGRAM_H_