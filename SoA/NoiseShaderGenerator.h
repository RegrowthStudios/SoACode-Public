///
/// NoiseShaderGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates shaders for procedural terrain generation
///

#pragma once

#ifndef NoiseShaderGenerator_h__
#define NoiseShaderGenerator_h__

#include <Vorb/graphics/Texture.h>
#include <Vorb/VorbPreDecl.inl>

#include "Biome.h"

DECL_VG(class GLProgram)
DECL_VCORE(class RPCManager);
struct PlanetGenData;
struct TerrainFuncKegProperties;

class NoiseShaderGenerator {
public:
    /// Generates a shader program for generation
    /// @param genData; The planet generation data
    /// @param glrpc: Optional RPC for loading in non-render thread
    /// @return glProgram. Caller should handle resource destruction
    CALLER_DELETE vg::GLProgram* generateProgram(PlanetGenData* genData,
                                   vcore::RPCManager* glrpc = nullptr);
    /// Generates a default shader program for generation
    /// @param glrpc: Optional RPC for loading in non-render thread
    /// @return glProgram. Caller should handle resource destruction
    CALLER_DELETE vg::GLProgram* getDefaultProgram(vcore::RPCManager* glrpc = nullptr);
private:
    /// Adds noise functions to the shader code
    /// @param fSource: Source to be modified
    /// @param variable: The variable that the function should modify
    /// @param funcs: The terrain functions to add
    /// @param modifier: Modifier string
    void addNoiseFunctions(OUT nString& fSource, const nString& variable,
                           const Array<TerrainFuncKegProperties>& funcs, const nString& modifier);
    /// Adds biome code to the shader code
    /// @param fsource: The source to be modified
    /// @param genData: The planet generation data
    void addBiomes(OUT nString& fSource, PlanetGenData* genData);
    /// Dumps the shader code to an output stream. Also formats the code.
    /// @param stream: Stream to write to
    /// @param source: Shader code source to write
    /// @param addLineNumbers: Determines if line numbers should be added to the code
    void dumpShaderCode(std::ostream& stream, nString source, bool addLineNumbers);
    int modCounter = 0;
};

#endif // NoiseShaderGenerator_h__
