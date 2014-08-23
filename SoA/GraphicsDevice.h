#pragma once

struct GraphicsDeviceProperties {
public:
    i32 maxColorSamples;
    i32 maxDepthSamples;
    
    i32 maxVertexAttributes;

    i32 maxTextureUnits;
    i32 maxTextureSize;
    i32 max3DTextureSize;
    i32 maxArrayTextureLayers;
    
    i32 nativeScreenWidth;
    i32 nativeScreenHeight;
    i32 nativeRefreshRate;

    const cString glVendor;
    const cString glVersion;
    i32 glVersionMajor;
    i32 glVersionMinor;
    const cString glslVersion;
};

class GraphicsDevice {
public:
    GraphicsDevice();

    static GraphicsDevice* getCurrent() {
        return _current;
    }

    void refreshInformation();

    const GraphicsDeviceProperties& getProperties() const {
        return _props;
    }
private:
    GraphicsDeviceProperties _props;

    static GraphicsDevice* _current;
};
