#pragma once
#include "EditorVariables.h"

class ZipFile;
struct TreeType;
struct Biome;

struct IniValue {
    IniValue() : isFloat(false) {}
    IniValue(const nString& k, const nString& v);
    IniValue(const nString& k, const f64 f);

    bool getBool();
    i32 getInt();
    f32 getFloat();
    f64 getDouble();
    nString getStr();

    nString key;

    bool isFloat;
    nString val;
    f64 fval;
};

static enum INI_KEYS {
    INI_NONE, INI_DENSITY, INI_X, INI_Y, INI_Z, INI_RADIUS, INI_GRAVITYCONSTANT, //planet properties
    INI_M_KR, INI_M_KM, INI_M_ESUN, INI_M_G, INI_M_EXP, INI_WAVELENGTHR, INI_WAVELENGTHG, INI_WAVELENGTHB, INI_NSAMPLES, //atmosphere properties
    INI_ATMOSPHERE_SEC_COLOR_EXPOSURE, INI_EFFECT_VOLUME, INI_ENABLE_PARTICLES, INI_FOV,
    INI_GAMMA, INI_MOUSESENSITIVITY, INI_MUSIC_VOLUME, INI_RENDER_DISTANCE,
    INI_SCREEN_WIDTH, INI_SCREEN_HEIGHT, INI_TERRAIN_QUALITY, INI_R, INI_G, INI_B, INI_FULLSCREEN, INI_ISBORDERLESS,
    INI_TEXTUREPACK, INI_INVERTMOUSE, INI_MAXFPS, INI_VSYNC, INI_BASETEMPERATURE, INI_BASEHUMIDITY, INI_AXIALTILT,
    INI_MINCELSIUS, INI_MAXCELSIUS, INI_MINHUMIDITY, INI_MAXHUMIDITY, INI_MINSIZE, INI_MAXSIZE, INI_SPAWNTIME,
    INI_TYPE, INI_PARTICLE, INI_RANDPOSOFFSET, INI_POSOFFSET, INI_DURATION, INI_REPETITIONS, INI_TEXTURE,
    INI_XFRAMES, INI_YFRAMES, INI_COLOR, INI_ANIMATION, INI_MINDURATION, INI_MAXDURATION, INI_INITIALVEL,
    INI_FADEOUTBEGIN, INI_MOTIONBLUR, INI_MSAA, INI_METHOD, INI_INNERSEAMS, INI_WEIGHTS, INI_SYMMETRY,
    INI_WIDTH, INI_HEIGHT, INI_PATH_OVERLAY, INI_BLEND_MODE, INI_PATH_BASE_TEXTURE, INI_USEMAPCOLOR,

    TREE_INI_BRANCHCHANCEBOTTOM, TREE_INI_BRANCHDIRECTIONBOTTOM, TREE_INI_BRANCHDIRTOP, TREE_INI_BRANCHLENGTHBOTTOM,
    TREE_INI_BRANCHWIDTHBOTTOM, TREE_INI_BRANCHLEAFSHAPE, TREE_INI_BRANCHLEAFSIZEMOD, TREE_INI_IDCORE,
    TREE_INI_DROOPYLEAVESLENGTH, TREE_INI_DROOPYLEAVESSLOPE, TREE_INI_DROOPYLEAVESDSLOPE, TREE_INI_DROOPYLEAVESACTIVE,
    TREE_INI_HASTHICKCAPBRANCHES, TREE_INI_MUSHROOMCAPINVERTED, TREE_INI_ISSLOPERANDOM, TREE_INI_LEAFCAPSHAPE,
    TREE_INI_LEAFCAPSIZE, TREE_INI_IDLEAVES, TREE_INI_MUSHROOMCAPCURLLENGTH, TREE_INI_MUSHROOMCAPGILLTHICKNESS,
    TREE_INI_MUSHROOMCAPSTRETCHMOD, TREE_INI_MUSHROOMCAPTHICKNESS, TREE_INI_IDBARK, TREE_INI_IDROOT,
    TREE_INI_IDSPECIALBLOCK, TREE_INI_BRANCHDIRECTIONTOP, TREE_INI_BRANCHLENGTHTOP, TREE_INI_BRANCHWIDTHTOP,
    TREE_INI_TRUNKHEIGHTBASE, TREE_INI_TRUNKWIDTHBASE, TREE_INI_TRUNKCHANGEDIRCHANCE, TREE_INI_TRUNKCOREWIDTH,
    TREE_INI_TRUNKSLOPEEND, TREE_INI_TRUNKHEIGHT, TREE_INI_TRUNKWIDTHMID, TREE_INI_TRUNKWIDTHTOP, TREE_INI_TRUNKSLOPESTART,
    TREE_INI_BRANCHSTART, TREE_INI_BRANCHCHANCETOP, TREE_INI_BRANCHCHANCECAPMOD, TREE_INI_BRANCHLEAFYMOD, TREE_INI_ROOTDEPTH,

    BLOCK_INI_NONE, BLOCK_INI_ID, BLOCK_INI_ALLOWSLIGHT, BLOCK_INI_BLOCKSSUNRAYS, BLOCK_INI_BREAKSBYWATER,
    BLOCK_INI_COLLISION,
    BLOCK_INI_CRUSHABLE, BLOCK_INI_EXPLOSIONPOWER, BLOCK_INI_EXPLOSIONPOWERLOSS,
    BLOCK_INI_EXPLOSIONRAYS, BLOCK_INI_EXPLOSIVERESISTANCE, BLOCK_INI_FLOATINGACTION, BLOCK_INI_HEALTH, BLOCK_INI_LIGHTACTIVE,
    BLOCK_INI_LIGHTCOLOR, BLOCK_INI_MATERIAL, BLOCK_INI_MESHTYPE, BLOCK_INI_MOVEMENTMOD, BLOCK_INI_MOVESPOWDER,
    BLOCK_INI_PHYSICSPROPERTY, BLOCK_INI_SUPPORTIVE, BLOCK_INI_COLOR, BLOCK_INI_COLORFILTER, BLOCK_INI_OVERLAYCOLOR,
    BLOCK_INI_USEABLE, BLOCK_INI_VALUE, BLOCK_INI_WATERMESHLEVEL, BLOCK_INI_WAVEEFFECT, BLOCK_INI_WEIGHT, BLOCK_INI_OCCLUDE,
    BLOCK_INI_SOURCE, BLOCK_INI_SINK, BLOCK_INI_TEXTURE, BLOCK_INI_TEXTURETOP, BLOCK_INI_TEXTURESIDE, BLOCK_INI_TEXTUREBOTTOM,
    BLOCK_INI_TEXTURE_PARTICLE, BLOCK_INI_FLAMMABILITY, BLOCK_INI_BURNTRANSFORMID,
    BLOCK_INI_EMITTER, BLOCK_INI_EMITTERONBREAK, BLOCK_INI_EMITTERRANDOM,

    BIOME_INI_NONE, BIOME_INI_NAME, BIOME_INI_ALTCOLORACTIVE, BIOME_INI_ALTCOLOR, BIOME_INI_MAXHEIGHT,
    BIOME_INI_MAXHEIGHTTRANSITIONLENGTH, BIOME_INI_BEACHBLOCK, BIOME_INI_SURFACEBLOCK, BIOME_INI_UNDERWATERBLOCK,
    BIOME_INI_TREECHANCEBASE, BIOME_INI_DISTRIBUTIONNOISE
};

class FileManager {
public:
    FileManager();
    void initialize();
    i32 readZipFile(nString fileName);

    i32 deleteDirectory(const nString& refcstrRootDirectory);

    i32 loadCloudNoiseFunctions(const cString filename, class Planet* planet);
    i32 loadNoiseFunctions(const cString filename, bool mandatory, Planet* planet);
    i32 loadFloraNoiseFunctions(const cString filename, Planet* planet);
    i32 loadFloraData(Planet* planet, nString worldFilePath);
    i32 loadAllTreeData(Planet* planet, nString worldFilePath);
    i32 loadTreeType(nString filePath, TreeType* tree);
    i32 loadBiomeData(Planet* planet, nString worldFilePath);
    i32 readBiome(Biome* biome, nString fileName, Planet* planet, nString worldFilePath);

    nString loadTexturePackDescription(nString fileName);

    i32 loadBlocks(nString filePath);
    i32 saveBlocks(nString filePath);

    i32 saveBiomeData(Planet* planet, nString worldFilePath);
    i32 saveBiome(Biome* biome);
    void saveAllBiomes(Planet* planet);

    i32 saveTreeData(TreeType* tt);
    void saveAllTreeFiles(Planet* planet);

    nString getFileNameDialog(const nString& prompt, const cString initialDir);
    nString getSaveFileNameDialog(const nString& prompt, const cString initialDir);

    void loadNoiseDescriptions(const cString filename);

    INI_KEYS getIniVal(nString& s);
    INI_KEYS getBlockIniVal(nString& s);

    i32 makeSaveDirectories(nString filePath);
    i32 createSaveFile(nString filePath);
    i32 createWorldFile(nString filePath);
    nString getWorldString(nString filePath);
    i32 setSaveFile(nString filePath);
    i32 getSaveListSize();
    i32 loadMarkers(class Player* player);
    i32 loadPlayerFile(Player* player);
    i32 saveMarkers(Player* player);
    i32 savePlayerFile(Player* player);

    i32 loadIniFile(nString filePath, std::vector< std::vector<IniValue> >& iniValues, std::vector<nString>& iniSections, ZipFile* zipFile = nullptr);
    i32 saveIniFile(nString filePath, std::vector< std::vector<IniValue> >& iniValues, std::vector<nString>& iniSections);

    class ParticleEmitter* loadEmitter(nString fileName);
    struct Animation* loadAnimation(nString fileName, ZipFile* zipFile = nullptr);
    i32 getParticleType(nString fileName);

    std::map<nString, BiomeVariable> biomeVariableMap;
    std::map<nString, NoiseVariable> noiseVariableMap;
private:
    void initializeINIMaps();
    void makeBiomeVariableMap();
    void makeNoiseVariableMap();
    void loadGameData();

    std::map<nString, i32> _particleTypeMap;
    std::map<nString, ParticleEmitter*> _emitterMap;
    std::map<nString, Animation*> _animationMap;

    bool isInitialized;
};

extern FileManager fileManager;
