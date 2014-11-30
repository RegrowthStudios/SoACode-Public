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

            /// Store shader language version
            struct ShaderLanguageVersion {
                ShaderLanguageVersion(
                    i32 maj = GL_PROGRAM_DEFAULT_SHADER_VERSION_MAJOR,
                    i32 min = GL_PROGRAM_DEFAULT_SHADER_VERSION_MINOR,
                    i32 rev = GL_PROGRAM_DEFAULT_SHADER_VERSION_REVISION) :
                    major(maj), minor(min), revision(rev) {
                    // Empty
                }

                i32 major; ///< GLSL major version
                i32 minor; ///< GLSL minor version
                i32 revision;  ///< GLSL revision version
            };
            extern const ShaderLanguageVersion DEFAULT_SHADING_LANGUAGE_VERSION; ///< Default language version

            /// Holds information necessary for shader compilation
            struct ShaderSource {
                ShaderType stage = ShaderType::VERTEX_SHADER; ///< Shader's pipeline stage
                ShaderLanguageVersion version; ///< Language version
                std::vector<const cString> sources; ///< Strings of shader source code awaiting concatenation
            };

            // Encapsulates a simple OpenGL program and its shaders
            class GLProgram {
            public:
                typedef std::pair<nString, VGAttribute> AttributeBinding; ///< Binds attribute names to locations
                typedef std::map<nString, VGAttribute> AttributeMap; ///< Dictionary of attribute locations by name
                typedef std::map<nString, VGUniform> UniformMap; ///< Dictionary of uniform locations by name

                /// Create and possibly initialize program
                /// @param init: True to call init()
                GLProgram(bool init = false);
                ~GLProgram();

                /// Create GPU resources
                void init();
                /// Free all resources
                void destroy();
                /// @return The program ID
                const VGProgram& getID() const {
                    return _id;
                }
                /// @return True if the program was created
                bool getIsCreated() const {
                    return _id != 0;
                }
                /// @return True if the program is linked
                bool getIsLinked() const {
                    return _isLinked;
                }
                /// @return True if the program is in use
                bool getIsInUse() const {
                    return _programInUse == this;
                }

                /// Attaches shader to the build information
                /// @param data: Shader's type, code, and version
                /// @return True on success
                bool addShader(const ShaderSource& data);
                /// Attaches shader to the build information
                /// @param type: Shader's stage in the pipeline
                /// @param code: Shader's source code
                /// @param version: Language version
                /// @return True on success
                bool addShader(const ShaderType& type, const cString code, const ShaderLanguageVersion& version = DEFAULT_SHADING_LANGUAGE_VERSION);

                /// Sets an attribute before the link step
                /// @param name: Attribute name
                /// @param index: Attribute index
                void setAttribute(nString name, VGAttribute index);
                /// Sets a list of attributes before the link step
                /// @param attr: Map of attributes
                void setAttributes(const std::map<nString, VGAttribute>& attr);
                /// Sets a list of attributes before the link step
                /// @param attr: List of attribute bindings
                void setAttributes(const std::vector<AttributeBinding>& attr);
                /// Sets a list of attributes before the link step
                /// @param attr: List of attributes (array index as attribute index)
                void setAttributes(const std::vector<nString>& attr);
                
                /// Links the shader program using its currently added shaders
                /// @return True on success
                bool link();

                /// Creates mappings for attributes
                void initAttributes();
                /// Creates mappings for uniforms
                void initUniforms();

                /// Gets an attribute index
                /// @param name: The attribute's name
                /// @return Attribute location
                const VGAttribute& getAttribute(const nString& name) const {
                    return _attributes.at(name);
                }
                /// Gets a uniform index
                /// @param name: The uniform's name
                /// @return Uniform location
                const VGUniform& getUniform(const nString& name) const {
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

                /// @return The current program that is in use
                static GLProgram* getCurrentProgram() {
                    return _programInUse;
                }

                Event<nString> onShaderCompilationError; ///< Event signaled during addShader when an error occurs
                Event<nString> onProgramLinkError; ///< Event signaled during link when an error occurs
            private:
                VGProgram _id = 0; ///< Program
                VGShader _idVS = 0; ///< Vertex shader
                VGShader _idFS = 0; ///< Fragment shader

                bool _isLinked = false; ///< Keeps track of link status

                AttributeMap _attributes; ///< Dictionary of attributes
                UniformMap _uniforms; ///< Dictionary of uniforms

                static GLProgram* _programInUse; ///< The current program in use
            };
        }
    }
}
namespace vg = vorb::core::graphics;

#endif // GLPROGRAM_H_