#pragma once
#include "Errors.h"

struct SoaOptionFlags {
    bool needsWindowReload : 1;
    bool needsFBOReload : 1;
    bool needsShaderReload : 1;
    bool needsTextureReload : 1;
};

enum class OptionValueType {
    NONE,
    F64,
    F32,
    I64,
    I32,
    BOOL,
    CHAR
};

struct OptionValue {
    OptionValue() {}
    OptionValue(f64 lf) : lf(lf), type(OptionValueType::F64) {}
    OptionValue(f32 f) : f(f), type(OptionValueType::F32) {}
    OptionValue(i64 li) : li(li), type(OptionValueType::I64) {}
    OptionValue(i32 i) : i(i), type(OptionValueType::I32) {}
    OptionValue(bool b) : b(b), type(OptionValueType::BOOL) {}
    OptionValue(char c) : c(c), type(OptionValueType::CHAR) {}
    union {
        f64 lf;
        f32 f;
        i64 li;
        i32 i;
        bool b;
        char c;
    };
    OptionValueType type = OptionValueType::NONE;
};

struct SoaOption {
    int id = -1;
    nString name = "";
    OptionValue defaultValue;
    OptionValue value;
    SoaOptionFlags flags;
};
struct SoaStringOption {
    nString name;
    nString defaultValue;
    nString value;
};

// Integer IDs for faster default options lookups
enum DefaultOptions : int {
    OPT_PLANET_DETAIL = 0,
    OPT_VOXEL_RENDER_DISTANCE,
    OPT_HUD_MODE,
    OPT_TEXTURE_RES,
    OPT_MOTION_BLUR,
    OPT_DEPTH_OF_FIELD,
    OPT_MSAA,
    OPT_MAX_MSAA,
    OPT_SPECULAR_EXPONENT,
    OPT_SPECULAR_INTENSITY,
    OPT_HDR_EXPOSURE,
    OPT_GAMMA,
    OPT_SEC_COLOR_MULT,
    OPT_FOV,
    OPT_VSYNC,
    OPT_MAX_FPS,
    OPT_VOXEL_LOD_THRESHOLD,
    OPT_MUSIC_VOLUME,
    OPT_EFFECT_VOLUME,
    OPT_MOUSE_SENSITIVITY,
    OPT_INVERT_MOUSE,
    OPT_FULLSCREEN,
    OPT_BORDERLESS,
    OPT_SCREEN_WIDTH,
    OPT_SCREEN_HEIGHT,
    OPT_NUM_OPTIONS // This should be last
};

class SoaOptions {
public:
    SoaOptions();
    ~SoaOptions();

    void addOption(int id, const nString& name, OptionValue defaultValue, SoaOptionFlags flags = {});
    void addOption(const nString& name, OptionValue defaultValue, SoaOptionFlags flags = {});
    void addStringOption(const nString& name, const nString& defaultValue);
 
    SoaOption& get(int id);
    SoaOption& get(const nString& name);
    SoaStringOption& getStringOption(const nString& name);

    const std::vector<SoaOption>& getOptions() const { return m_options; }

    const nString& getFilePath() const { return m_filePath; }

    void dispose();
private:
    std::vector<SoaOption> m_options;
    // String lookups are slower than int lookups
    std::map<nString, int> m_optionsLookup;
    std::map<nString, SoaStringOption> m_stringOptionsLookup;
    nString m_filePath = "Data/options.ini";
};

// Global options struct
extern SoaOptions soaOptions;