#pragma once

typedef GLenum VGEnum;
namespace vorb {
    namespace core {
        namespace graphics {
            enum class BufferTarget {
                NONE = 0,
                ARRAY_BUFFER = GL_ARRAY_BUFFER,
                ATOMIC_COUNTER_BUFFER = GL_ATOMIC_COUNTER_BUFFER,
                COPY_READ_BUFFER = GL_COPY_READ_BUFFER,
                COPY_WRITE_BUFFER = GL_COPY_WRITE_BUFFER,
                DISPATCH_INDIRECT_BUFFER = GL_DISPATCH_INDIRECT_BUFFER,
                DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
                ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
                PIXEL_PACK_BUFFER = GL_PIXEL_PACK_BUFFER,
                PIXEL_UNPACK_BUFFER = GL_PIXEL_UNPACK_BUFFER,
                QUERY_BUFFER = GL_QUERY_BUFFER,
                SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
                TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
                TRANSFORM_FEEDBACK_BUFFER = GL_TRANSFORM_FEEDBACK_BUFFER,
                UNIFORM_BUFFER = GL_UNIFORM_BUFFER
            };
            enum class BufferUsageHint {
                NONE = 0,
                DYNAMIC_COPY = GL_DYNAMIC_COPY,
                DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
                DYNAMIC_READ = GL_DYNAMIC_READ,
                STATIC_COPY = GL_STATIC_COPY,
                STATIC_DRAW = GL_STATIC_DRAW,
                STATIC_READ = GL_STATIC_READ,
                STREAM_COPY = GL_STREAM_COPY,
                STREAM_DRAW = GL_STREAM_DRAW,
                STREAM_READ = GL_STREAM_READ
            };
            enum class VertexAttribPointerType {
                NONE = 0,
                BYTE = GL_BYTE,
                DOUBLE = GL_DOUBLE,
                FIXED = GL_FIXED,
                FLOAT = GL_FLOAT,
                HALF_FLOAT = GL_HALF_FLOAT,
                INT = GL_INT,
                INT_2_10_10_10_REV = GL_INT_2_10_10_10_REV,
                SHORT = GL_SHORT,
                UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
                UNSIGNED_INT = GL_UNSIGNED_INT,
                UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV,
                UNSIGNED_SHORT = GL_UNSIGNED_SHORT
            };

            enum class TextureInternalFormat {
                NONE = 0,
                ALPHA = GL_ALPHA,
                COMPRESSED_ALPHA = GL_COMPRESSED_ALPHA,
                COMPRESSED_INTENSITY = GL_COMPRESSED_INTENSITY,
                COMPRESSED_LUMINANCE = GL_COMPRESSED_LUMINANCE,
                COMPRESSED_LUMINANCE_ALPHA = GL_COMPRESSED_LUMINANCE_ALPHA,
                COMPRESSED_RED = GL_COMPRESSED_RED,
                COMPRESSED_RED_RGTC1 = GL_COMPRESSED_RED_RGTC1,
                COMPRESSED_RG = GL_COMPRESSED_RG,
                COMPRESSED_RGB = GL_COMPRESSED_RGB,
                COMPRESSED_RGBA = GL_COMPRESSED_RGBA,
                COMPRESSED_RGBA_BPTC_UNORM = GL_COMPRESSED_RGBA_BPTC_UNORM,
                COMPRESSED_RGB_BPTC_SIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
                COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
                COMPRESSED_RG_RGTC2 = GL_COMPRESSED_RG_RGTC2,
                COMPRESSED_SIGNED_RED_RGTC1 = GL_COMPRESSED_SIGNED_RED_RGTC1,
                COMPRESSED_SIGNED_RG_RGTC2 = GL_COMPRESSED_SIGNED_RG_RGTC2,
                COMPRESSED_SLUMINANCE = GL_COMPRESSED_SLUMINANCE,
                COMPRESSED_SLUMINANCE_ALPHA = GL_COMPRESSED_SLUMINANCE_ALPHA,
                COMPRESSED_SRGB = GL_COMPRESSED_SRGB,
                COMPRESSED_SRGB_ALPHA = GL_COMPRESSED_SRGB_ALPHA,
                DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
                DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
                DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
                DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
                DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
                DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,
                DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
                DEPTH_STENCIL = GL_DEPTH_STENCIL,
                FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                LUMINANCE = GL_LUMINANCE,
                LUMINANCE_ALPHA = GL_LUMINANCE_ALPHA,
                R11F_G11F_B10F = GL_R11F_G11F_B10F,
                R16 = GL_R16,
                R16F = GL_R16F,
                R16I = GL_R16I,
                R16_SNORM = GL_R16_SNORM,
                R16UI = GL_R16UI,
                R32F = GL_R32F,
                R32I = GL_R32I,
                R32UI = GL_R32UI,
                R3_G3_B2 = GL_R3_G3_B2,
                R8 = GL_R8,
                R8I = GL_R8I,
                R8_SNORM = GL_R8_SNORM,
                R8UI = GL_R8UI,
                RG16 = GL_RG16,
                RG16F = GL_RG16F,
                RG16I = GL_RG16I,
                RG16_SNORM = GL_RG16_SNORM,
                RG16UI = GL_RG16UI,
                RG32F = GL_RG32F,
                RG32I = GL_RG32I,
                RG32UI = GL_RG32UI,
                RG8 = GL_RG8,
                RG8I = GL_RG8I,
                RG8_SNORM = GL_RG8_SNORM,
                RG8UI = GL_RG8UI,
                RGB = GL_RGB,
                RGB10 = GL_RGB10,
                RGB10_A2 = GL_RGB10_A2,
                RGB10_A2UI = GL_RGB10_A2UI,
                RGB12 = GL_RGB12,
                RGB16 = GL_RGB16,
                RGB16F = GL_RGB16F,
                RGB16I = GL_RGB16I,
                RGB16_SNORM = GL_RGB16_SNORM,
                RGB16UI = GL_RGB16UI,
                RGB32F = GL_RGB32F,
                RGB32I = GL_RGB32I,
                RGB32UI = GL_RGB32UI,
                RGB4 = GL_RGB4,
                RGB5 = GL_RGB5,
                RGB5_A1 = GL_RGB5_A1,
                RGB8 = GL_RGB8,
                RGB8I = GL_RGB8I,
                RGB8_SNORM = GL_RGB8_SNORM,
                RGB8UI = GL_RGB8UI,
                RGB9_E5 = GL_RGB9_E5,
                RGBA = GL_RGBA,
                RGBA12 = GL_RGBA12,
                RGBA16 = GL_RGBA16,
                RGBA16F = GL_RGBA16F,
                RGBA16I = GL_RGBA16I,
                RGBA16_SNORM = GL_RGBA16_SNORM,
                RGBA16UI = GL_RGBA16UI,
                RGBA2 = GL_RGBA2,
                RGBA32F = GL_RGBA32F,
                RGBA32I = GL_RGBA32I,
                RGBA32UI = GL_RGBA32UI,
                RGBA4 = GL_RGBA4,
                RGBA8 = GL_RGBA8,
                RGBA8I = GL_RGBA8I,
                RGBA8_SNORM = GL_RGBA8_SNORM,
                RGBA8UI = GL_RGBA8UI,
                SLUMINANCE = GL_SLUMINANCE,
                SLUMINANCE8 = GL_SLUMINANCE8,
                SLUMINANCE8_ALPHA8 = GL_SLUMINANCE8_ALPHA8,
                SLUMINANCE_ALPHA = GL_SLUMINANCE_ALPHA,
                SRGB = GL_SRGB,
                SRGB8 = GL_SRGB8,
                SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
                SRGB_ALPHA = GL_SRGB_ALPHA,
                ONE = GL_ONE,
                TWO = 2,
                THREE = 3,
                FOUR = 4
            };
            enum class TextureFormat {
                NONE = 0,
                ALPHA = GL_ALPHA,
                ALPHA_INTEGER = GL_ALPHA_INTEGER,
                BGR = GL_BGR,
                BGRA = GL_BGRA,
                BGRA_INTEGER = GL_BGRA_INTEGER,
                BGR_INTEGER = GL_BGR_INTEGER,
                BLUE = GL_BLUE,
                BLUE_INTEGER = GL_BLUE_INTEGER,
                COLOR_INDEX = GL_COLOR_INDEX,
                DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
                DEPTH_STENCIL = GL_DEPTH_STENCIL,
                GREEN = GL_GREEN,
                GREEN_INTEGER = GL_GREEN_INTEGER,
                LUMINANCE = GL_LUMINANCE,
                LUMINANCE_ALPHA = GL_LUMINANCE_ALPHA,
                RED = GL_RED,
                RED_INTEGER = GL_RED_INTEGER,
                RG = GL_RG,
                RGB = GL_RGB,
                RGBA = GL_RGBA,
                RGBA_INTEGER = GL_RGBA_INTEGER,
                RGB_INTEGER = GL_RGB_INTEGER,
                RG_INTEGER = GL_RG_INTEGER,
                STENCIL_INDEX = GL_STENCIL_INDEX,
                UNSIGNED_INT = GL_UNSIGNED_INT,
                UNSIGNED_SHORT = GL_UNSIGNED_SHORT
            };
            enum class TexturePixelType {
                NONE = 0,
                BYTE = GL_BYTE,
                FLOAT = GL_FLOAT,
                FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                HALF_FLOAT = GL_HALF_FLOAT,
                INT = GL_INT,
                SHORT = GL_SHORT,
                UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
                UNSIGNED_BYTE_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
                UNSIGNED_INT = GL_UNSIGNED_INT,
                UNSIGNED_INT_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
                UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,
                UNSIGNED_INT_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
                UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
                UNSIGNED_SHORT_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
                UNSIGNED_SHORT_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
                UNSIGNED_SHORT_5_6_5 = GL_UNSIGNED_SHORT_5_6_5
            };
            enum class TextureTarget {
                NONE = 0,
                PROXY_TEXTURE_1D = GL_PROXY_TEXTURE_1D,
                PROXY_TEXTURE_1D_ARRAY = GL_PROXY_TEXTURE_1D_ARRAY,
                PROXY_TEXTURE_2D = GL_PROXY_TEXTURE_2D,
                PROXY_TEXTURE_2D_ARRAY = GL_PROXY_TEXTURE_2D_ARRAY,
                PROXY_TEXTURE_2D_MULTISAMPLE = GL_PROXY_TEXTURE_2D_MULTISAMPLE,
                PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY = GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY,
                PROXY_TEXTURE_3D = GL_PROXY_TEXTURE_3D,
                PROXY_TEXTURE_CUBE_MAP = GL_PROXY_TEXTURE_CUBE_MAP,
                PROXY_TEXTURE_CUBE_MAP_ARRAY = GL_PROXY_TEXTURE_CUBE_MAP_ARRAY,
                PROXY_TEXTURE_RECTANGLE = GL_PROXY_TEXTURE_RECTANGLE,
                TEXTURE_1D = GL_TEXTURE_1D,
                TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
                TEXTURE_2D = GL_TEXTURE_2D,
                TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
                TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
                TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
                TEXTURE_3D = GL_TEXTURE_3D,
                TEXTURE_BASE_LEVEL = GL_TEXTURE_BASE_LEVEL,
                TEXTURE_BINDING_CUBE_MAP = GL_TEXTURE_BINDING_CUBE_MAP,
                TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
                TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
                TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,
                TEXTURE_CUBE_MAP_NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                TEXTURE_CUBE_MAP_NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                TEXTURE_CUBE_MAP_NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                TEXTURE_CUBE_MAP_POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                TEXTURE_CUBE_MAP_POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                TEXTURE_CUBE_MAP_POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                TEXTURE_MAX_LEVEL = GL_TEXTURE_MAX_LEVEL,
                TEXTURE_MAX_LOD = GL_TEXTURE_MAX_LOD,
                TEXTURE_MIN_LOD = GL_TEXTURE_MIN_LOD,
                TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE
            };
            enum class TextureParameterName {
                NONE = 0,
                CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
                CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
                DEPTH_TEXTURE_MODE = GL_DEPTH_TEXTURE_MODE,
                GENERATE_MIPMAP = GL_GENERATE_MIPMAP,
                TEXTURE_BASE_LEVEL = GL_TEXTURE_BASE_LEVEL,
                TEXTURE_BORDER_COLOR = GL_TEXTURE_BORDER_COLOR,
                TEXTURE_COMPARE_FUNC = GL_TEXTURE_COMPARE_FUNC,
                TEXTURE_COMPARE_MODE = GL_TEXTURE_COMPARE_MODE,
                TEXTURE_DEPTH = GL_TEXTURE_DEPTH,
                TEXTURE_LOD_BIAS = GL_TEXTURE_LOD_BIAS,
                TEXTURE_MAG_FILTER = GL_TEXTURE_MAG_FILTER,
                TEXTURE_MAX_LEVEL = GL_TEXTURE_MAX_LEVEL,
                TEXTURE_MAX_LOD = GL_TEXTURE_MAX_LOD,
                TEXTURE_MIN_FILTER = GL_TEXTURE_MIN_FILTER,
                TEXTURE_MIN_LOD = GL_TEXTURE_MIN_LOD,
                TEXTURE_PRIORITY = GL_TEXTURE_PRIORITY,
                TEXTURE_SWIZZLE_A = GL_TEXTURE_SWIZZLE_A,
                TEXTURE_SWIZZLE_B = GL_TEXTURE_SWIZZLE_B,
                TEXTURE_SWIZZLE_G = GL_TEXTURE_SWIZZLE_G,
                TEXTURE_SWIZZLE_R = GL_TEXTURE_SWIZZLE_R,
                TEXTURE_SWIZZLE_RGBA = GL_TEXTURE_SWIZZLE_RGBA,
                TEXTURE_WRAP_R = GL_TEXTURE_WRAP_R,
                TEXTURE_WRAP_S = GL_TEXTURE_WRAP_S,
                TEXTURE_WRAP_T = GL_TEXTURE_WRAP_T
            };
            
            enum class ShaderType {
                NONE = 0,
                COMPUTE_SHADER = GL_COMPUTE_SHADER,
                FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
                GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
                TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER,
                TESS_EVALUATION_SHADER = GL_TESS_EVALUATION_SHADER,
                VERTEX_SHADER = GL_VERTEX_SHADER
            };
            enum class ShaderParameter {
                NONE = 0,
                COMPILE_STATUS = GL_COMPILE_STATUS,
                DELETE_STATUS = GL_DELETE_STATUS,
                INFO_LOG_LENGTH = GL_INFO_LOG_LENGTH,
                SHADER_SOURCE_LENGTH = GL_SHADER_SOURCE_LENGTH,
                SHADER_TYPE = GL_SHADER_TYPE
            };
            enum class GetProgramParameterName {
                NONE = 0,
                ACTIVE_ATOMIC_COUNTER_BUFFERS = GL_ACTIVE_ATOMIC_COUNTER_BUFFERS,
                ACTIVE_ATTRIBUTE_MAX_LENGTH = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
                ACTIVE_ATTRIBUTES = GL_ACTIVE_ATTRIBUTES,
                ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH,
                ACTIVE_UNIFORM_BLOCKS = GL_ACTIVE_UNIFORM_BLOCKS,
                ACTIVE_UNIFORM_MAX_LENGTH = GL_ACTIVE_UNIFORM_MAX_LENGTH,
                ACTIVE_UNIFORMS = GL_ACTIVE_UNIFORMS,
                ATTACHED_SHADERS = GL_ATTACHED_SHADERS,
                DELETE_STATUS = GL_DELETE_STATUS,
                GEOMETRY_INPUT_TYPE = GL_GEOMETRY_INPUT_TYPE,
                GEOMETRY_OUTPUT_TYPE = GL_GEOMETRY_OUTPUT_TYPE,
                GEOMETRY_SHADER_INVOCATIONS = GL_GEOMETRY_SHADER_INVOCATIONS,
                GEOMETRY_VERTICES_OUT = GL_GEOMETRY_VERTICES_OUT,
                INFO_LOG_LENGTH = GL_INFO_LOG_LENGTH,
                LINK_STATUS = GL_LINK_STATUS,
                MAX_COMPUTE_WORK_GROUP_SIZE = GL_MAX_COMPUTE_WORK_GROUP_SIZE,
                PROGRAM_BINARY_RETRIEVABLE_HINT = GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
                PROGRAM_SEPARABLE = GL_PROGRAM_SEPARABLE,
                TESS_CONTROL_OUTPUT_VERTICES = GL_TESS_CONTROL_OUTPUT_VERTICES,
                TESS_GEN_MODE = GL_TESS_GEN_MODE,
                TESS_GEN_POINT_MODE = GL_TESS_GEN_POINT_MODE,
                TESS_GEN_SPACING = GL_TESS_GEN_SPACING,
                TESS_GEN_VERTEX_ORDER = GL_TESS_GEN_VERTEX_ORDER,
                TRANSFORM_FEEDBACK_BUFFER_MODE = GL_TRANSFORM_FEEDBACK_BUFFER_MODE,
                TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH,
                TRANSFORM_FEEDBACK_VARYINGS = GL_TRANSFORM_FEEDBACK_VARYINGS,
                VALIDATE_STATUS = GL_VALIDATE_STATUS
            };
            
            enum class FramebufferTarget {
                NONE = 0,
                DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER,
                FRAMEBUFFER = GL_FRAMEBUFFER,
                READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER
            };
            enum class FramebufferAttachment {
                NONE = 0,
                AUX0 = GL_AUX0,
                AUX1 = GL_AUX1,
                AUX2 = GL_AUX2,
                AUX3 = GL_AUX3,
                BACK_LEFT = GL_BACK_LEFT,
                BACK_RIGHT = GL_BACK_RIGHT,
                COLOR = GL_COLOR,
                COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
                COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
                COLOR_ATTACHMENT10 = GL_COLOR_ATTACHMENT10,
                COLOR_ATTACHMENT11 = GL_COLOR_ATTACHMENT11,
                COLOR_ATTACHMENT12 = GL_COLOR_ATTACHMENT12,
                COLOR_ATTACHMENT13 = GL_COLOR_ATTACHMENT13,
                COLOR_ATTACHMENT14 = GL_COLOR_ATTACHMENT14,
                COLOR_ATTACHMENT15 = GL_COLOR_ATTACHMENT15,
                COLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
                COLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
                COLOR_ATTACHMENT4 = GL_COLOR_ATTACHMENT4,
                COLOR_ATTACHMENT5 = GL_COLOR_ATTACHMENT5,
                COLOR_ATTACHMENT6 = GL_COLOR_ATTACHMENT6,
                COLOR_ATTACHMENT7 = GL_COLOR_ATTACHMENT7,
                COLOR_ATTACHMENT8 = GL_COLOR_ATTACHMENT8,
                COLOR_ATTACHMENT9 = GL_COLOR_ATTACHMENT9,
                DEPTH = GL_DEPTH,
                DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
                DEPTH_STENCIL_ATTACHMENT = GL_DEPTH_STENCIL_ATTACHMENT,
                FRONT_LEFT = GL_FRONT_LEFT,
                FRONT_RIGHT = GL_FRONT_RIGHT,
                STENCIL = GL_STENCIL,
                STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT
            };
            enum class RenderbufferTarget {
                NONE = 0,
                RENDERBUFFER = GL_RENDERBUFFER,
            };
            enum class RenderbufferStorage {
                NONE = 0,
                DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
                DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
                DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
                DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
                DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
                DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,
                DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
                DEPTH_STENCIL = GL_DEPTH_STENCIL,
                R11F_G11F_B10F = GL_R11F_G11F_B10F,
                R16 = GL_R16,
                R16F = GL_R16F,
                R16I = GL_R16I,
                R16UI = GL_R16UI,
                R32F = GL_R32F,
                R32I = GL_R32I,
                R32UI = GL_R32UI,
                R3_G3_B2 = GL_R3_G3_B2,
                R8 = GL_R8,
                R8I = GL_R8I,
                R8UI = GL_R8UI,
                RG16 = GL_RG16,
                RG16F = GL_RG16F,
                RG16I = GL_RG16I,
                RG16UI = GL_RG16UI,
                RG32F = GL_RG32F,
                RG32I = GL_RG32I,
                RG32UI = GL_RG32UI,
                RG8 = GL_RG8,
                RG8I = GL_RG8I,
                RG8UI = GL_RG8UI,
                RGB10 = GL_RGB10,
                RGB10_A2 = GL_RGB10_A2,
                RGB10_A2UI = GL_RGB10_A2UI,
                RGB12 = GL_RGB12,
                RGB16 = GL_RGB16,
                RGB16F = GL_RGB16F,
                RGB16I = GL_RGB16I,
                RGB16UI = GL_RGB16UI,
                RGB32F = GL_RGB32F,
                RGB32I = GL_RGB32I,
                RGB32UI = GL_RGB32UI,
                RGB4 = GL_RGB4,
                RGB5 = GL_RGB5,
                RGB8 = GL_RGB8,
                RGB8I = GL_RGB8I,
                RGB8UI = GL_RGB8UI,
                RGB9_E5 = GL_RGB9_E5,
                RGBA12 = GL_RGBA12,
                RGBA16 = GL_RGBA16,
                RGBA16F = GL_RGBA16F,
                RGBA16I = GL_RGBA16I,
                RGBA16UI = GL_RGBA16UI,
                RGBA2 = GL_RGBA2,
                RGBA32F = GL_RGBA32F,
                RGBA32I = GL_RGBA32I,
                RGBA32UI = GL_RGBA32UI,
                RGBA4 = GL_RGBA4,
                RGBA8 = GL_RGBA8,
                RGBA8I = GL_RGBA8I,
                RGBA8UI = GL_RGBA8UI,
                SRGB8 = GL_SRGB8,
                SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
                STENCIL_INDEX1 = GL_STENCIL_INDEX1,
                STENCIL_INDEX16 = GL_STENCIL_INDEX16,
                STENCIL_INDEX4 = GL_STENCIL_INDEX4,
                STENCIL_INDEX8 = GL_STENCIL_INDEX8
            };
            enum class DrawBufferMode {
                NONE = 0,
                BACK = GL_BACK,
                BACK_LEFT = GL_BACK_LEFT,
                BACK_RIGHT = GL_BACK_RIGHT,
                COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
                COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
                COLOR_ATTACHMENT10 = GL_COLOR_ATTACHMENT10,
                COLOR_ATTACHMENT11 = GL_COLOR_ATTACHMENT11,
                COLOR_ATTACHMENT12 = GL_COLOR_ATTACHMENT12,
                COLOR_ATTACHMENT13 = GL_COLOR_ATTACHMENT13,
                COLOR_ATTACHMENT14 = GL_COLOR_ATTACHMENT14,
                COLOR_ATTACHMENT15 = GL_COLOR_ATTACHMENT15,
                COLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
                COLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
                COLOR_ATTACHMENT4 = GL_COLOR_ATTACHMENT4,
                COLOR_ATTACHMENT5 = GL_COLOR_ATTACHMENT5,
                COLOR_ATTACHMENT6 = GL_COLOR_ATTACHMENT6,
                COLOR_ATTACHMENT7 = GL_COLOR_ATTACHMENT7,
                COLOR_ATTACHMENT8 = GL_COLOR_ATTACHMENT8,
                COLOR_ATTACHMENT9 = GL_COLOR_ATTACHMENT9,
                FRONT = GL_FRONT,
                FRONT_AND_BACK = GL_FRONT_AND_BACK,
                FRONT_LEFT = GL_FRONT_LEFT,
                FRONT_RIGHT = GL_FRONT_RIGHT,
                LEFT = GL_LEFT,
                NONE = GL_NONE,
                RIGHT = GL_RIGHT
            };
            enum class FramebufferErrorCode {
                NONE = 0,
                FRAMEBUFFER_COMPLETE = GL_FRAMEBUFFER_COMPLETE,
                FRAMEBUFFER_INCOMPLETE_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
                FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
                FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                FRAMEBUFFER_INCOMPLETE_READ_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
                FRAMEBUFFER_UNDEFINED = GL_FRAMEBUFFER_UNDEFINED,
                FRAMEBUFFER_UNSUPPORTED = GL_FRAMEBUFFER_UNSUPPORTED
            };
            
            enum class ErrorCode {
                NONE = GL_NO_ERROR,
                INVALID_ENUM = GL_INVALID_ENUM,
                INVALID_FRAMEBUFFER_OPERATION = GL_INVALID_FRAMEBUFFER_OPERATION,
                INVALID_OPERATION = GL_INVALID_OPERATION,
                INVALID_VALUE = GL_INVALID_VALUE,
                OUT_OF_MEMORY = GL_OUT_OF_MEMORY
            };
            enum class EnableCap {
                NONE = 0,
                BLEND = GL_BLEND,
                CLIP_DISTANCE0 = GL_CLIP_DISTANCE0,
                CLIP_DISTANCE1 = GL_CLIP_DISTANCE1,
                CLIP_DISTANCE2 = GL_CLIP_DISTANCE2,
                CLIP_DISTANCE3 = GL_CLIP_DISTANCE3,
                CLIP_DISTANCE4 = GL_CLIP_DISTANCE4,
                CLIP_DISTANCE5 = GL_CLIP_DISTANCE5,
                CLIP_PLANE0 = GL_CLIP_PLANE0,
                CLIP_PLANE1 = GL_CLIP_PLANE1,
                CLIP_PLANE2 = GL_CLIP_PLANE2,
                CLIP_PLANE3 = GL_CLIP_PLANE3,
                CLIP_PLANE4 = GL_CLIP_PLANE4,
                CLIP_PLANE5 = GL_CLIP_PLANE5,
                COLOR_LOGIC_OP = GL_COLOR_LOGIC_OP,
                COLOR_SUM = GL_COLOR_SUM,
                CULL_FACE = GL_CULL_FACE,
                DEBUG_OUTPUT = GL_DEBUG_OUTPUT,
                DEBUG_OUTPUT_SYNCHRONOUS = GL_DEBUG_OUTPUT_SYNCHRONOUS,
                DEPTH_CLAMP = GL_DEPTH_CLAMP,
                DEPTH_TEST = GL_DEPTH_TEST,
                DITHER = GL_DITHER,
                FOG_COORD_ARRAY = GL_FOG_COORD_ARRAY,
                FRAMEBUFFER_SRGB = GL_FRAMEBUFFER_SRGB,
                LINE_SMOOTH = GL_LINE_SMOOTH,
                MULTISAMPLE = GL_MULTISAMPLE,
                POINT_SPRITE = GL_POINT_SPRITE,
                POLYGON_OFFSET_FILL = GL_POLYGON_OFFSET_FILL,
                POLYGON_OFFSET_LINE = GL_POLYGON_OFFSET_LINE,
                POLYGON_OFFSET_POINT = GL_POLYGON_OFFSET_POINT,
                POLYGON_SMOOTH = GL_POLYGON_SMOOTH,
                PRIMITIVE_RESTART = GL_PRIMITIVE_RESTART,
                PRIMITIVE_RESTART_FIXED_INDEX = GL_PRIMITIVE_RESTART_FIXED_INDEX,
                PROGRAM_POINT_SIZE = GL_PROGRAM_POINT_SIZE,
                RASTERIZER_DISCARD = GL_RASTERIZER_DISCARD,
                RESCALE_NORMAL = GL_RESCALE_NORMAL,
                SAMPLE_ALPHA_TO_COVERAGE = GL_SAMPLE_ALPHA_TO_COVERAGE,
                SAMPLE_ALPHA_TO_ONE = GL_SAMPLE_ALPHA_TO_ONE,
                SAMPLE_COVERAGE = GL_SAMPLE_COVERAGE,
                SAMPLE_MASK = GL_SAMPLE_MASK,
                SAMPLE_SHADING = GL_SAMPLE_SHADING,
                SCISSOR_TEST = GL_SCISSOR_TEST,
                SECONDARY_COLOR_ARRAY = GL_SECONDARY_COLOR_ARRAY,
                STENCIL_TEST = GL_STENCIL_TEST,
                TEXTURE_1D = GL_TEXTURE_1D,
                TEXTURE_2D = GL_TEXTURE_2D,
                TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
                TEXTURE_CUBE_MAP_SEAMLESS = GL_TEXTURE_CUBE_MAP_SEAMLESS,
                TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE,
                VERTEX_PROGRAM_POINT_SIZE = GL_VERTEX_PROGRAM_POINT_SIZE,
                VERTEX_PROGRAM_TWO_SIDE = GL_VERTEX_PROGRAM_TWO_SIDE
            };
            
            enum class TextureMinFilter {
                NONE = 0,
                LINEAR = GL_LINEAR,
                LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
                LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
                NEAREST = GL_NEAREST,
                NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
                NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST
            };
            enum class TextureMagFilter {
                NONE = 0,
                LINEAR = GL_LINEAR,
                NEAREST = GL_NEAREST
            };
            enum class TextureWrapMode {
                NONE = 0,
                CLAMP_BORDER = GL_CLAMP_TO_BORDER,
                CLAMP_EDGE = GL_CLAMP_TO_EDGE,
                REPEAT_MIRRORED = GL_MIRRORED_REPEAT,
                REPEAT = GL_REPEAT
            };

            enum class DepthFunction {
                NONE = 0,
                ALWAYS = GL_ALWAYS,
                EQUAL = GL_EQUAL,
                GEQUAL = GL_GEQUAL,
                GREATER = GL_GREATER,
                LEQUAL = GL_LEQUAL,
                LESS = GL_LESS,
                NEVER = GL_NEVER,
                NOTEQUAL = GL_NOTEQUAL
            };
            
            enum class BlendEquationMode {
                NONE = 0,
                FUNC_ADD = GL_FUNC_ADD,
                FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
                FUNC_SUBTRACT = GL_FUNC_SUBTRACT,
                MAX = GL_MAX,
                MIN = GL_MIN
            };
            enum class BlendingFactorSrc {
                NONE = 0,
                CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
                CONSTANT_COLOR = GL_CONSTANT_COLOR,
                DST_ALPHA = GL_DST_ALPHA,
                DST_COLOR = GL_DST_COLOR,
                ONE = GL_ONE,
                ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
                ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
                ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
                ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
                ONE_MINUS_SRC1_ALPHA = GL_ONE_MINUS_SRC1_ALPHA,
                ONE_MINUS_SRC1_COLOR = GL_ONE_MINUS_SRC1_COLOR,
                ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
                ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
                SRC1_ALPHA = GL_SRC1_ALPHA,
                SRC1_COLOR = GL_SRC1_COLOR,
                SRC_ALPHA = GL_SRC_ALPHA,
                SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE,
                SRC_COLOR = GL_SRC_COLOR,
                ZERO = GL_ZERO
            };
            enum class BlendingFactorDest {
                NONE = 0,
                CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
                CONSTANT_COLOR = GL_CONSTANT_COLOR,
                DST_ALPHA = GL_DST_ALPHA,
                DST_COLOR = GL_DST_COLOR,
                ONE = GL_ONE,
                ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
                ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
                ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
                ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
                ONE_MINUS_SRC1_ALPHA = GL_ONE_MINUS_SRC1_ALPHA,
                ONE_MINUS_SRC1_COLOR = GL_ONE_MINUS_SRC1_COLOR,
                ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
                ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
                SRC1_ALPHA = GL_SRC1_ALPHA,
                SRC1_COLOR = GL_SRC1_COLOR,
                SRC_ALPHA = GL_SRC_ALPHA,
                SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE,
                SRC_COLOR = GL_SRC_COLOR,
                ZERO = GL_ZERO
            };
            
            enum class PrimitiveType {
                NONE = 0,
                LINE_LOOP = GL_LINE_LOOP,
                LINES = GL_LINES,
                LINES_ADJACENCY = GL_LINES_ADJACENCY,
                LINE_STRIP = GL_LINE_STRIP,
                LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,
                PATCHES = GL_PATCHES,
                POINTS = GL_POINTS,
                QUADS = GL_QUADS,
                TRIANGLE_FAN = GL_TRIANGLE_FAN,
                TRIANGLES = GL_TRIANGLES,
                TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
                TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
                TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY
            };
        }
    }
}
namespace vg = vorb::core::graphics;
