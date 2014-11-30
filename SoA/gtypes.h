///
/// gtypes.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 26 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Typenames for general graphics types
///

#pragma once

#ifndef gtypes_h__
#define gtypes_h__

/************************************************************************/
/* Objects                                                              */
/************************************************************************/
typedef GLuint VGObject;
// Mesh
typedef VGObject VGBuffer;
typedef VGBuffer VGVertexBuffer;
typedef VGBuffer VGInstanceBuffer;
typedef VGBuffer VGIndexBuffer;
typedef VGObject VGVertexArray;
// Texture
typedef VGObject VGTexture;
typedef VGObject VGFramebuffer;
typedef VGObject VGRenderbuffer;
// Shading
typedef VGObject VGProgram;
typedef VGObject VGShader;
// Other
typedef VGObject VGSampler;
typedef VGObject VGQuery;

/************************************************************************/
/* Special identifiers                                                  */
/************************************************************************/
typedef GLint VGAttribute;
typedef GLint VGUniform;
typedef GLenum VGEnum;

#endif // gtypes_h__