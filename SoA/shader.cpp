#include "stdafx.h"
#include "shader.h"

#include <algorithm>

#include "Errors.h"
#include "GameManager.h"
#include "Planet.h"
#include "SimplexNoise.h"
#include "TerrainGenerator.h"

// TODO: Remove This
using namespace std;


BasicColorShader basicColorShader;
HDRShader hdrShader;
MotionBlurShader motionBlurShader;
BlockShader blockShader;
CutoutShading cutoutShader;
TransparentShading transparencyShader;
TextureShader textureShader;
AtmosphereToSkyShader atmosphereToSkyShader;
AtmosphereToGroundShader atmosphereToGroundShader;
SpaceToSkyShader spaceToSkyShader;
SpaceToGroundShader spaceToGroundShader;
WaterShader waterShader;
//ParticleShader particleShader;
BillboardShader billboardShader;
FixedSizeBillboardShader fixedSizeBillboardShader;
SonarShader sonarShader;
PhysicsBlockShader physicsBlockShader;
TreeShader treeShader;
CloudShader cloudShader;
Texture2DShader texture2Dshader;
SimplexNoiseShader simplexNoiseShader;

void Shader::DeleteShader() {
    if (shaderID != 0) {
        glUseProgram(0);
        glDeleteProgram(shaderID);
        shaderID = 0;
    }
    isInitialized = 0;
}

void BasicColorShader::Initialize() {
    if (isInitialized) return;
    program.init();
    printf("Loading basicColorShader\n");
    program.addShaderFile(GL_VERTEX_SHADER, "Shaders/BasicShading/BasicColorShading.vert");
    program.addShaderFile(GL_FRAGMENT_SHADER, "Shaders/BasicShading/BasicColorShading.frag");
    glBindAttribLocation(program.getID(), 0, "vertexPosition_modelspace");
    program.link();
    program.initAttributes();
    program.initUniforms();
    colorID = program.getUniform("Color");
    mvpID = program.getUniform("MVP");
    isInitialized = program.getIsLinked();
    if (isInitialized) printf("basicColorShader Loaded\n");
    else printf("basicColorShader Error\n");
}
void BasicColorShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BOUND BEFORE INITIALIZATION");
        puts(getchar() == 0xDEADF00D ? "You Hacker" : "");
    }

    program.use();
    glEnableVertexAttribArray(0);
}
void BasicColorShader::UnBind() {
    GLProgram::unuse();
    glDisableVertexAttribArray(0);
}

//it is grad3 + 1
static const GLubyte bgrad[12][3] = {
    { 2, 2, 1 }, { 0, 2, 1 }, { 2, 0, 1 }, { 0, 0, 1 },
    { 2, 1, 2 }, { 0, 1, 2 }, { 2, 1, 0 }, { 0, 1, 0 },
    { 1, 2, 2 }, { 1, 0, 2 }, { 1, 2, 0 }, { 1, 0, 0 }
};
// Permutation table.  The same list is repeated twice.
static const GLubyte bperm[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
    35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
    18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
    250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
    172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
    228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
    107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
    35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
    18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
    250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
    172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
    228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
    107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

void SimplexNoiseShader::Initialize() {
    if (isInitialized) return;
    cout << "Loading simplexNoiseShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/NoiseGeneration/simplexNoise.vert", "Shaders/NoiseGeneration/simplexNoise.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition");
    LinkShaders(shaderID, vID, fID);

    //GLuint permSampler2dID, permSampler2dTexture;
    //GLuint permGradSamplerID, permGradSamplerTexture;
    permID = GetUniform(shaderID, "perm");
    gradID = GetUniform(shaderID, "grad");

    dtID = GetUniform(shaderID, "dt");

    heightModifierID = GetUniform(shaderID, "heightModifier");
    baseTemperatureID = GetUniform(shaderID, "baseTemperature");
    baseRainfallID = GetUniform(shaderID, "baseRainfall");

    typesID = GetUniform(shaderID, "types");
    persistencesID = GetUniform(shaderID, "persistences");
    frequenciesID = GetUniform(shaderID, "frequencies");
    octavesID = GetUniform(shaderID, "octaves");
    lowsID = GetUniform(shaderID, "lows");
    highsID = GetUniform(shaderID, "highs");
    scalesID = GetUniform(shaderID, "scales");

    tempPID = GetUniform(shaderID, "tempP");
    tempFID = GetUniform(shaderID, "tempF");
    tempOID = GetUniform(shaderID, "tempO");
    tempLID = GetUniform(shaderID, "tempL");
    tempHID = GetUniform(shaderID, "tempH");

    rainPID = GetUniform(shaderID, "rainP");
    rainFID = GetUniform(shaderID, "rainF");
    rainOID = GetUniform(shaderID, "rainO");
    rainLID = GetUniform(shaderID, "rainL");
    rainHID = GetUniform(shaderID, "rainH");

    numFunctionsID = GetUniform(shaderID, "numFunctions");

    //Generate perm texture
    GLubyte data[512][3];
    for (int i = 0; i < 512; i++) {
        data[i][0] = bperm[i];
        data[i][1] = 255;
        data[i][2] = 255;
    }
    if (permTexture == 0) glGenTextures(1, &permTexture);
    glBindTexture(GL_TEXTURE_1D, permTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Generate grad texture
    if (gradTexture == 0) glGenTextures(1, &gradTexture);
    glBindTexture(GL_TEXTURE_1D, gradTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 12, 0, GL_RGB, GL_UNSIGNED_BYTE, bgrad);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glUseProgram(shaderID);
    glUniform1i(permID, 0);
    glUniform1i(gradID, 1);
    glUseProgram(0);
    isInitialized = 1;
}
//O = 30F 60S
//P - 30 F 30 S
void SimplexNoiseShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BOUND BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);


    static float dtt = 0.0f;
    dtt += 0.01f;
    glUniform1f(dtID, dtt);

    glUniform1i(numFunctionsID, (GLint)GameManager::planet->generator->noiseFunctions.size());

    glUniform1f(heightModifierID, (GLfloat)0.0f);
    glUniform1f(baseTemperatureID, (GLfloat)GameManager::planet->baseTemperature);
    glUniform1f(baseRainfallID, (GLfloat)GameManager::planet->baseRainfall);

    int size = GameManager::planet->generator->noiseFunctions.size();
    GLint types[32];
    GLfloat pers[32];
    GLfloat freq[32];
    GLint oct[32];
    GLfloat low[32];
    GLfloat high[32];
    GLfloat scales[32];
    for (size_t i = 0; i < GameManager::planet->generator->noiseFunctions.size(); i++) {
        types[i] = (GLint)GameManager::planet->generator->noiseFunctions[i].type;
        pers[i] = (GLfloat)GameManager::planet->generator->noiseFunctions[i].persistence;
        freq[i] = (GLfloat)GameManager::planet->generator->noiseFunctions[i].frequency;
        oct[i] = (GLint)GameManager::planet->generator->noiseFunctions[i].octaves;
        low[i] = (GLfloat)GameManager::planet->generator->noiseFunctions[i].lowBound;
        high[i] = (GLfloat)GameManager::planet->generator->noiseFunctions[i].upBound;
        scales[i] = (GLfloat)GameManager::planet->generator->noiseFunctions[i].scale;
    }

    glUniform1iv(typesID, size, types);
    glUniform1fv(persistencesID, size, pers);
    glUniform1fv(frequenciesID, size, freq);
    glUniform1iv(octavesID, size, oct);
    glUniform1fv(lowsID, size, low);
    glUniform1fv(highsID, size, high);
    glUniform1fv(scalesID, size, scales);

    glUniform1f(tempPID, (GLfloat)GameManager::planet->generator->temperatureNoiseFunction->persistence);
    glUniform1f(tempFID, (GLfloat)GameManager::planet->generator->temperatureNoiseFunction->frequency);
    glUniform1i(tempOID, (GLint)GameManager::planet->generator->temperatureNoiseFunction->octaves);
    glUniform1f(tempLID, (GLfloat)GameManager::planet->generator->temperatureNoiseFunction->lowBound);
    glUniform1f(tempHID, (GLfloat)GameManager::planet->generator->temperatureNoiseFunction->upBound);

    glUniform1f(rainPID, (GLfloat)GameManager::planet->generator->rainfallNoiseFunction->persistence);
    glUniform1f(rainFID, (GLfloat)GameManager::planet->generator->rainfallNoiseFunction->frequency);
    glUniform1i(rainOID, (GLint)GameManager::planet->generator->rainfallNoiseFunction->octaves);
    glUniform1f(rainLID, (GLfloat)GameManager::planet->generator->rainfallNoiseFunction->lowBound);
    glUniform1f(rainHID, (GLfloat)GameManager::planet->generator->rainfallNoiseFunction->upBound);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, permTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, gradTexture);
}
void SimplexNoiseShader::UnBind() {
    glDisableVertexAttribArray(0);
}

void HDRShader::Initialize(std::string dirPath) {
    if (isInitialized) return;
    cout << "Loading HDRShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "PassThrough.vert").c_str(), (dirPath + "HDRShader.frag").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_screenspace");
    LinkShaders(shaderID, vID, fID);


    gammaID = GetUniform(shaderID, "gamma");
    //averageLuminanceID = GetUniform(shaderID, "averageLuminance");
    texID = GetUniform(shaderID, "renderedTexture");
    fExposureID = GetUniform(shaderID, "fExposure");

    isInitialized = 1;

}
void HDRShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BOUND BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
}
void HDRShader::UnBind() {
    glDisableVertexAttribArray(0);
}

void MotionBlurShader::Initialize(std::string dirPath) {
    if (isInitialized) return;
    cout << "Loading MotionBlurShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "PassThrough.vert").c_str(), (dirPath + "MotionBlur.frag").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_screenspace");
    LinkShaders(shaderID, vID, fID);


    gammaID = GetUniform(shaderID, "gamma");
    //averageLuminanceID = GetUniform(shaderID, "averageLuminance");
    texID = GetUniform(shaderID, "renderedTexture");
    depthID = GetUniform(shaderID, "depthTexture");
    fExposureID = GetUniform(shaderID, "fExposure");


    numSamplesID = GetUniform(shaderID, "numSamples");
    inverseVPID = GetUniform(shaderID, "inverseVP");
    prevVPID = GetUniform(shaderID, "prevVP");

    isInitialized = 1;

}
void MotionBlurShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BOUND BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
}
void MotionBlurShader::UnBind() {
    glDisableVertexAttribArray(0);
}

void BlockShader::Initialize(string dirPath) {
    if (isInitialized) return;
    cout << "Loading blockShader\n";

    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "standardShading.vert").c_str(), (dirPath + "standardShading.frag").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "position_TextureType");
    glBindAttribLocation(shaderID, 1, "uvs_animation_blendMode");
    glBindAttribLocation(shaderID, 2, "textureAtlas_textureIndex");
    glBindAttribLocation(shaderID, 3, "textureDimensions");
    glBindAttribLocation(shaderID, 4, "color");
    glBindAttribLocation(shaderID, 5, "overlayColor");
    glBindAttribLocation(shaderID, 6, "light_sunlight");
    glBindAttribLocation(shaderID, 7, "normal");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");

    fogStartID = GetUniform(shaderID, "fogStart");
    fogEndID = GetUniform(shaderID, "fogEnd");
    fogColorID = GetUniform(shaderID, "fogColor");

    lightTypeID = GetUniform(shaderID, "lightType");

    ambientID = GetUniform(shaderID, "ambientLight");
    lightColorID = GetUniform(shaderID, "lightColor");
    sunValID = GetUniform(shaderID, "sunVal");

    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");

    alphaMultID = GetUniform(shaderID, "alphaMult");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    lightID = GetUniform(shaderID, "lightPosition_worldspace");

    blockDtID = GetUniform(shaderID, "dt");

    glUseProgram(shaderID);
    glUniform1i(texturesID, 0);
    isInitialized = 1;

    checkGlError("BlockShader::Initialize()");
}
void BlockShader::Bind() {
    glUseProgram(shaderID);
}
void BlockShader::UnBind() {}

void CutoutShading::Initialize(string dirPath) {
    if (isInitialized) return;
    cout << "Loading CutoutShading\n";

    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "standardShading.vert").c_str(), (dirPath + "cutoutShading.frag").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "position_TextureType");
    glBindAttribLocation(shaderID, 1, "uvs_animation_blendMode");
    glBindAttribLocation(shaderID, 2, "textureAtlas_textureIndex");
    glBindAttribLocation(shaderID, 3, "textureDimensions");
    glBindAttribLocation(shaderID, 4, "color");
    glBindAttribLocation(shaderID, 5, "overlayColor");
    glBindAttribLocation(shaderID, 6, "light_sunlight");
    glBindAttribLocation(shaderID, 7, "normal");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");

    fogStartID = GetUniform(shaderID, "fogStart");
    fogEndID = GetUniform(shaderID, "fogEnd");
    fogColorID = GetUniform(shaderID, "fogColor");

    lightTypeID = GetUniform(shaderID, "lightType");

    ambientID = GetUniform(shaderID, "ambientLight");
    lightColorID = GetUniform(shaderID, "lightColor");
    sunValID = GetUniform(shaderID, "sunVal");

    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");

    alphaMultID = GetUniform(shaderID, "alphaMult");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    lightID = GetUniform(shaderID, "lightPosition_worldspace");

    blockDtID = GetUniform(shaderID, "dt");

    eyeVecID = GetUniform(shaderID, "eyeNormalWorldspace");

    glUseProgram(shaderID);
    glUniform1i(texturesID, 0);
    isInitialized = 1;
}
void CutoutShading::Bind() {
    glUseProgram(shaderID);
}
void CutoutShading::UnBind() {}

void TransparentShading::Initialize(string dirPath) {
    if (isInitialized) return;
    cout << "Loading TransparentShading\n";

    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "standardShading.vert").c_str(), (dirPath + "transparentShading.frag").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "position_TextureType");
    glBindAttribLocation(shaderID, 1, "uvs_animation_blendMode");
    glBindAttribLocation(shaderID, 2, "textureAtlas_textureIndex");
    glBindAttribLocation(shaderID, 3, "textureDimensions");
    glBindAttribLocation(shaderID, 4, "color");
    glBindAttribLocation(shaderID, 5, "overlayColor");
    glBindAttribLocation(shaderID, 6, "light_sunlight");
    glBindAttribLocation(shaderID, 7, "normal");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");

    fogStartID = GetUniform(shaderID, "fogStart");
    fogEndID = GetUniform(shaderID, "fogEnd");
    fogColorID = GetUniform(shaderID, "fogColor");

    lightTypeID = GetUniform(shaderID, "lightType");

    ambientID = GetUniform(shaderID, "ambientLight");
    lightColorID = GetUniform(shaderID, "lightColor");
    sunValID = GetUniform(shaderID, "sunVal");

    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");

    alphaMultID = GetUniform(shaderID, "alphaMult");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    lightID = GetUniform(shaderID, "lightPosition_worldspace");

    blockDtID = GetUniform(shaderID, "dt");

    eyeVecID = GetUniform(shaderID, "eyeNormalWorldspace");

    glUseProgram(shaderID);
    glUniform1i(texturesID, 0);
    isInitialized = 1;
}
void TransparentShading::Bind() {
    glUseProgram(shaderID);
}
void TransparentShading::UnBind() {}

void TextureShader::Initialize() {
    if (isInitialized) return;
    cout << "Loading textureShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/TextureShading/TextureShading.vertexshader", "Shaders/TextureShading/TextureShading.fragmentshader", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    LinkShaders(shaderID, vID, fID);


    mvpID = GetUniform(shaderID, "MVP");
    texID = GetUniform(shaderID, "myTextureSampler");
    isInitialized = 1;
}
void TextureShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}
void TextureShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void AtmosphereShader::Initialize() {
    cout << "ERROR: LOADING AtmosphereShader, which is only meant to be base class!\n";
    int a;
    cin >> a;
}
void AtmosphereShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
}
void AtmosphereShader::UnBind() {
    glDisableVertexAttribArray(0);
}

void AtmosphereToSkyShader::Initialize() {
    cout << "Loading SkyFromAtmosphere shader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/AtmosphereShading/SkyFromAtmosphere.vert", "Shaders/AtmosphereShading/SkyFromAtmosphere.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    LinkShaders(shaderID, vID, fID);


    mvpID = GetUniform(shaderID, "MVP");
    cameraPosID = GetUniform(shaderID, "v3CameraPos");
    lightPosID = GetUniform(shaderID, "v3LightPos");
    invWavelengthID = GetUniform(shaderID, "v3InvWavelength");
    cameraHeightID = GetUniform(shaderID, "fCameraHeight");
    //    cameraHeight2ID = GetUniform(shaderID, "fCameraHeight2");
    outerRadiusID = GetUniform(shaderID, "fOuterRadius");
    //    outerRadius2ID = GetUniform(shaderID, "fOuterRadius2");
    innerRadiusID = GetUniform(shaderID, "fInnerRadius");
    //    innerRadius2ID = GetUniform(shaderID, "fInnerRadius2");
    KrESunID = GetUniform(shaderID, "fKrESun");
    KmESunID = GetUniform(shaderID, "fKmESun");
    Kr4PIID = GetUniform(shaderID, "fKr4PI");
    Km4PIID = GetUniform(shaderID, "fKm4PI");
    scaleID = GetUniform(shaderID, "fScale");
    scaleDepthID = GetUniform(shaderID, "fScaleDepth");
    scaleOverScaleDepthID = GetUniform(shaderID, "fScaleOverScaleDepth");
    gID = GetUniform(shaderID, "g");
    g2ID = GetUniform(shaderID, "g2");

    fSamplesID = GetUniform(shaderID, "fSamples");
    nSamplesID = GetUniform(shaderID, "nSamples");

    isInitialized = 1;
}
void AtmosphereToGroundShader::Initialize() {
    cout << "Loading GroundFromAtmosphere shader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/TerrainShading/GroundFromAtmosphere.vert", "Shaders/TerrainShading/GroundFromAtmosphere.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    glBindAttribLocation(shaderID, 2, "vertexNormal_modelspace");
    glBindAttribLocation(shaderID, 3, "vertexColor");
    glBindAttribLocation(shaderID, 4, "vertexSlopeColor");
    glBindAttribLocation(shaderID, 5, "vertexBeachColor");
    glBindAttribLocation(shaderID, 6, "texTempRainSpec");
    LinkShaders(shaderID, vID, fID);


    mvpID = GetUniform(shaderID, "MVP");
    //    mID = GetUniform(shaderID, "M");
    cameraPosID = GetUniform(shaderID, "cameraPos");
    lightPosID = GetUniform(shaderID, "lightPos");
    invWavelengthID = GetUniform(shaderID, "invWavelength");
    cameraHeightID = GetUniform(shaderID, "cameraHeight");
    //    cameraHeight2ID = GetUniform(shaderID, "fCameraHeight2");
    //    outerRadiusID = GetUniform(shaderID, "fOuterRadius");
    //    outerRadius2ID = GetUniform(shaderID, "fOuterRadius2");
    innerRadiusID = GetUniform(shaderID, "innerRadius");
    //    innerRadius2ID = GetUniform(shaderID, "fInnerRadius2");
    KrESunID = GetUniform(shaderID, "krESun");
    KmESunID = GetUniform(shaderID, "kmESun");
    Kr4PIID = GetUniform(shaderID, "kr4PI");
    Km4PIID = GetUniform(shaderID, "km4PI");
    scaleID = GetUniform(shaderID, "fScale");
    scaleDepthID = GetUniform(shaderID, "scaleDepth");
    scaleOverScaleDepthID = GetUniform(shaderID, "fScaleOverScaleDepth");

    dtID = GetUniform(shaderID, "dt");

    secColorMultID = GetUniform(shaderID, "secColorMult");
    //    gID = GetUniform(shaderID, "g");
    //    g2ID = GetUniform(shaderID, "g2");

    fSamplesID = GetUniform(shaderID, "fSamples");
    nSamplesID = GetUniform(shaderID, "nSamples");

    worldOffsetID = GetUniform(shaderID, "worldOffset");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    sunColorTextureID = GetUniform(shaderID, "sunColorTexture");
    texturesID = GetUniform(shaderID, "textures");

    fadeDistanceID = GetUniform(shaderID, "FadeDistance");

    colorTextureID = GetUniform(shaderID, "colorTexture");
    waterColorTextureID = GetUniform(shaderID, "waterColorTexture");

    freezeTempID = GetUniform(shaderID, "freezeTemp");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 6, txv);
    glUniform1i(sunColorTextureID, txv[6]);
    glUniform1i(colorTextureID, txv[7]);
    glUniform1i(waterColorTextureID, txv[3]);

    isInitialized = 1;
}
void AtmosphereToGroundShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
}
void AtmosphereToGroundShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
}

void SpaceToSkyShader::Initialize() {
    cout << "Loading SkyFromSpace shader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/AtmosphereShading/SkyFromSpace.vert", "Shaders/AtmosphereShading/SkyFromSpace.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    LinkShaders(shaderID, vID, fID);


    mvpID = GetUniform(shaderID, "MVP");
    cameraPosID = GetUniform(shaderID, "v3CameraPos");
    lightPosID = GetUniform(shaderID, "v3LightPos");
    invWavelengthID = GetUniform(shaderID, "v3InvWavelength");
    //    cameraHeightID = GetUniform(shaderID, "fCameraHeight");
    cameraHeight2ID = GetUniform(shaderID, "fCameraHeight2");
    outerRadiusID = GetUniform(shaderID, "fOuterRadius");
    //    outerRadius2ID = GetUniform(shaderID, "fOuterRadius2");
    innerRadiusID = GetUniform(shaderID, "fInnerRadius");
    //    innerRadius2ID = GetUniform(shaderID, "fInnerRadius2");
    KrESunID = GetUniform(shaderID, "fKrESun");
    KmESunID = GetUniform(shaderID, "fKmESun");
    Kr4PIID = GetUniform(shaderID, "fKr4PI");
    Km4PIID = GetUniform(shaderID, "fKm4PI");
    scaleID = GetUniform(shaderID, "fScale");
    scaleDepthID = GetUniform(shaderID, "fScaleDepth");
    scaleOverScaleDepthID = GetUniform(shaderID, "fScaleOverScaleDepth");
    gID = GetUniform(shaderID, "g");
    g2ID = GetUniform(shaderID, "g2");

    fSamplesID = GetUniform(shaderID, "fSamples");
    nSamplesID = GetUniform(shaderID, "nSamples");

    isInitialized = 1;
}
void SpaceToGroundShader::Initialize() {
    cout << "Loading GroundFromSpace shader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/TerrainShading/GroundFromSpace.vert", "Shaders/TerrainShading/GroundFromSpace.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    glBindAttribLocation(shaderID, 2, "vertexNormal_modelspace");
    glBindAttribLocation(shaderID, 3, "vertexColor");
    glBindAttribLocation(shaderID, 4, "vertexSlopeColor");
    glBindAttribLocation(shaderID, 5, "vertexBeachColor");
    glBindAttribLocation(shaderID, 6, "texTempRainSpec");
    LinkShaders(shaderID, vID, fID);


    mvpID = GetUniform(shaderID, "MVP");
    //    mID = GetUniform(shaderID, "M");
    cameraPosID = GetUniform(shaderID, "cameraPos");
    lightPosID = GetUniform(shaderID, "lightPos");
    invWavelengthID = GetUniform(shaderID, "invWavelength");
    //    cameraHeightID = GetUniform(shaderID, "cameraHeight");
    cameraHeight2ID = GetUniform(shaderID, "cameraHeight2");
    outerRadiusID = GetUniform(shaderID, "outerRadius");
    outerRadius2ID = GetUniform(shaderID, "outerRadius2");
    innerRadiusID = GetUniform(shaderID, "innerRadius");
    //    innerRadius2ID = GetUniform(shaderID, "fInnerRadius2");
    KrESunID = GetUniform(shaderID, "krESun");
    KmESunID = GetUniform(shaderID, "kmESun");
    Kr4PIID = GetUniform(shaderID, "kr4PI");
    Km4PIID = GetUniform(shaderID, "km4PI");
    scaleID = GetUniform(shaderID, "fScale");
    scaleDepthID = GetUniform(shaderID, "scaleDepth");
    scaleOverScaleDepthID = GetUniform(shaderID, "fScaleOverScaleDepth");

    dtID = GetUniform(shaderID, "dt");

    secColorMultID = GetUniform(shaderID, "secColorMult");
    //    gID = GetUniform(shaderID, "g");
    //    g2ID = GetUniform(shaderID, "g2");

    worldOffsetID = GetUniform(shaderID, "worldOffset");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    fSamplesID = GetUniform(shaderID, "fSamples");
    nSamplesID = GetUniform(shaderID, "nSamples");

    sunColorTextureID = GetUniform(shaderID, "sunColorTexture");
    texturesID = GetUniform(shaderID, "textures");

    colorTextureID = GetUniform(shaderID, "colorTexture");
    waterColorTextureID = GetUniform(shaderID, "waterColorTexture");

    drawModeID = GetUniform(shaderID, "drawMode");

    freezeTempID = GetUniform(shaderID, "freezeTemp");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 6, txv);
    glUniform1i(sunColorTextureID, txv[6]);
    glUniform1i(colorTextureID, txv[7]);
    glUniform1i(waterColorTextureID, txv[3]);

    isInitialized = 1;
}
void SpaceToGroundShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);

}
void SpaceToGroundShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
}

void WaterShader::Initialize() {
    cout << "Loading waterShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/WaterShading/WaterShading.vert", "Shaders/WaterShading/WaterShading.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_modelspace");
    glBindAttribLocation(shaderID, 1, "vertexUv_texUnit_texIndex");
    glBindAttribLocation(shaderID, 2, "vertexColor");
    glBindAttribLocation(shaderID, 3, "light_sunlight");
    LinkShaders(shaderID, vID, fID);

    lightID = GetUniform(shaderID, "LightPosition_worldspace");

    fogStartID = GetUniform(shaderID, "FogStart");
    fogEndID = GetUniform(shaderID, "FogEnd");
    fogColorID = GetUniform(shaderID, "FogColor");

    ambientID = GetUniform(shaderID, "AmbientLight");

    lightColorID = GetUniform(shaderID, "LightColor");
    normalMapID = GetUniform(shaderID, "normalMap");

    sunValID = GetUniform(shaderID, "sunVal");

    mvpID = GetUniform(shaderID, "MVP");

    mID = GetUniform(shaderID, "M");
    fadeDistanceID = GetUniform(shaderID, "FadeDistance");

    dtID = GetUniform(shaderID, "dt");
    isInitialized = 1;
}
void WaterShader::Bind()
{
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}
void WaterShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

//void ParticleShader::Initialize()
//{
///*    cout << "Loading particleShader\n";
//    GLuint vID, fID;
//    shaderID = LoadShaders( "Shaders/ParticleShading/ParticleShading.vertexshader", "Shaders/ParticleShading/ParticleShading.geometryshader","Shaders/ParticleShading/ParticleShading.fragmentshader", vID, fID );
//    LinkShaders(shaderID, vID, fID);
//    
//
//    texturesID = GetUniform(shaderID, "textures");
//
//    ambientID = GetUniform(shaderID, "AmbientLight");
//
//    colorID = GetUniform(shaderID, "particleColor");
//    textureUnitID = GetUniform(shaderID, "textureUnitID");
//    gVPID = GetUniform(shaderID, "gVP");
//    UVstartID = GetUniform(shaderID, "UVstart");
//    UVwidthID = GetUniform(shaderID, "UVwidth");
//    sunValID = GetUniform(shaderID, "sunVal");
//    lightTypeID = GetUniform(shaderID, "lightType");
//    eyeNormalID = GetUniform(shaderID, "eyeNormalWorldspace");
//
//    int txv[8] = {0,1,2,3,4,5,6,7};
//    glUseProgram(shaderID);
//    glUniform1iv(texturesID, 8, txv);
//    isInitialized = 1;*/
//}
//
//void ParticleShader::Bind()
//{
///*    if (!isInitialized){
//        printf("SHADER BINDED BEFORE INITIALIZATION");
//        int a;
//        cin >> a;
//    }
//    glUseProgram(shaderID);
//    glEnableVertexAttribArray(0);
//    glEnableVertexAttribArray(1);
//    glEnableVertexAttribArray(2);
//    glEnableVertexAttribArray(3);
//    glEnableVertexAttribArray(4);*/
//}
//
//void ParticleShader::UnBind()
//{
//    /*glDisableVertexAttribArray(0);
//    glDisableVertexAttribArray(1);
//    glDisableVertexAttribArray(2);
//    glDisableVertexAttribArray(3);
//    glDisableVertexAttribArray(4);*/
//}

void BillboardShader::Initialize() {
    cout << "Loading billboardShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/BillboardShading/BillboardShading.vertexshader", "Shaders/BillboardShading/BillboardShading.fragmentshader", vID, fID);
    glBindAttribLocation(shaderID, 0, "position");
    glBindAttribLocation(shaderID, 1, "uv");
    glBindAttribLocation(shaderID, 2, "uvMult");
    glBindAttribLocation(shaderID, 3, "texUtexIDlight");
    glBindAttribLocation(shaderID, 4, "color");
    glBindAttribLocation(shaderID, 5, "sizeXmod");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");

    alphaThresholdID = GetUniform(shaderID, "alphaThreshold");
    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");
    sunValID = GetUniform(shaderID, "sunVal");
    lightTypeID = GetUniform(shaderID, "lightType");
    ambientID = GetUniform(shaderID, "AmbientLight");
    cameraUpID = GetUniform(shaderID, "cameraUp_worldspace");
    cameraRightID = GetUniform(shaderID, "cameraRight_worldspace");
    eyeNormalID = GetUniform(shaderID, "eyeNormalWorldspace");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 8, txv);
    isInitialized = 1;
}
void BillboardShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
}
void BillboardShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
}

void FixedSizeBillboardShader::Initialize() {
    cout << "Loading fixedSizeBillboardShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/BillboardShading/FixedSizeBillboardShading.vert", "Shaders/BillboardShading/FixedSizeBillboardShading.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "position");
    glBindAttribLocation(shaderID, 1, "uv_light");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");

    colorID = GetUniform(shaderID, "particleColor");
    textureUnitID = GetUniform(shaderID, "textureUnitID");
    mvpID = GetUniform(shaderID, "MVP");
    uvModID = GetUniform(shaderID, "UVmod");
    uVstartID = GetUniform(shaderID, "UVstart");
    uVwidthID = GetUniform(shaderID, "UVwidth");
    widthID = GetUniform(shaderID, "width");
    heightID = GetUniform(shaderID, "height");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 8, txv);
    isInitialized = 1;
}
void FixedSizeBillboardShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}
void FixedSizeBillboardShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void SonarShader::Initialize() {
    cout << "Loading sonarShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/BlockShading/standardShading.vert", "Shaders/BlockShading/sonarShading.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "position_TextureType");
    glBindAttribLocation(shaderID, 1, "uvs_animation_blendMode");
    glBindAttribLocation(shaderID, 2, "textureAtlas_textureIndex");
    glBindAttribLocation(shaderID, 3, "textureDimensions");
    glBindAttribLocation(shaderID, 4, "color");
    glBindAttribLocation(shaderID, 5, "overlayColor");
    glBindAttribLocation(shaderID, 6, "light_sunlight");
    glBindAttribLocation(shaderID, 7, "normal");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");
    distanceID = GetUniform(shaderID, "sonarDistance");
    waveID = GetUniform(shaderID, "waveWidth");
    dtID = GetUniform(shaderID, "dt");

    mvpID = GetUniform(shaderID, "MVP");

    mID = GetUniform(shaderID, "M");

    glUseProgram(shaderID);
    glUniform1i(texturesID, 0);
    isInitialized = 1;

    checkGlError("SonarShader::Initialize()");
}
void SonarShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
}
void SonarShader::UnBind() {}

void PhysicsBlockShader::Initialize() {
    cout << "Loading physicsBlockShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/PhysicsBlockShading/PhysicsBlockShading.vertexshader", "Shaders/BlockShading/standardShading.frag", vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_blendMode");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    glBindAttribLocation(shaderID, 2, "textureAtlas_textureIndex");
    glBindAttribLocation(shaderID, 3, "textureDimensions");
    glBindAttribLocation(shaderID, 4, "normal");
    glBindAttribLocation(shaderID, 5, "centerPosition");
    glBindAttribLocation(shaderID, 6, "vertexColor");
    glBindAttribLocation(shaderID, 7, "overlayColor");
    glBindAttribLocation(shaderID, 8, "vertexLight");
    LinkShaders(shaderID, vID, fID);

    texturesID = GetUniform(shaderID, "textures");
    fadeDistanceID = GetUniform(shaderID, "fadeDistance");

    fogStartID = GetUniform(shaderID, "fogStart");
    fogEndID = GetUniform(shaderID, "fogEnd");
    fogColorID = GetUniform(shaderID, "fogColor");
    alphaMultID = GetUniform(shaderID, "alphaMult");

    lightTypeID = GetUniform(shaderID, "lightType");

    ambientID = GetUniform(shaderID, "ambientLight");
    lightColorID = GetUniform(shaderID, "lightColor");
    sunValID = GetUniform(shaderID, "sunVal");

    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");

    specularExponentID = GetUniform(shaderID, "specularExponent");
    specularIntensityID = GetUniform(shaderID, "specularIntensity");

    lightID = GetUniform(shaderID, "lightPosition_worldspace");

    eyeVecID = GetUniform(shaderID, "eyeNormalWorldspace");


    glUseProgram(shaderID);
    glUniform1i(texturesID, 0);
    isInitialized = 1;

    checkGlError("BlockShader::Initialize()");
}
void PhysicsBlockShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
}
void PhysicsBlockShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(7);
    glDisableVertexAttribArray(8);
}

void TreeShader::Initialize() {
    cout << "Loading treeShader\n";
    GLuint vID, fID;
    shaderID = LoadShaders("Shaders/TreeBillboardShading/TreeBillboardShading.vertexShader", "Shaders/TreeBillboardShading/TreeBillboardShading.fragmentShader", vID, fID);
    glBindAttribLocation(shaderID, 0, "position");
    glBindAttribLocation(shaderID, 1, "particleCenter_worldspace");
    glBindAttribLocation(shaderID, 2, "leafColor_Size");
    glBindAttribLocation(shaderID, 3, "trunkColor_ltex");
    LinkShaders(shaderID, vID, fID);


    texturesID = GetUniform(shaderID, "textures");

    mvpID = GetUniform(shaderID, "MVP");
    mID = GetUniform(shaderID, "M");
    //playerPosID = GetUniform(shaderID, "playerPos");
    worldUpID = GetUniform(shaderID, "worldUp");

    fadeDistanceID = GetUniform(shaderID, "FadeDistance");
    sunValID = GetUniform(shaderID, "sunVal");
    //mvpID = GetUniform(shaderID, "MVP");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 8, txv);
    isInitialized = 1;
}
void TreeShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}
void TreeShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void CloudShader::Initialize() {
    cout << "Loading CloudShader\n";
    //shaderID = LoadShaders( "Shaders/CloudShading/CloudShading.vertexshader", "Shaders/CloudShading/CloudShading.geometryshader","Shaders/CloudShading/CloudShading.fragmentshader" );


    fogStartID = GetUniform(shaderID, "FogStart");
    fogEndID = GetUniform(shaderID, "FogEnd");
    texturesID = GetUniform(shaderID, "textures");

    hasAlphaID = GetUniform(shaderID, "hasAlpha");
    textureUnitID = GetUniform(shaderID, "textureUnitID");
    gVPID = GetUniform(shaderID, "gVP");
    mID = GetUniform(shaderID, "M");
    ambientID = GetUniform(shaderID, "AmbientLight");

    //lightColorID = GetUniform(shaderID, "LightColor");

    lightID = GetUniform(shaderID, "LightPosition_worldspace");

    glowID = GetUniform(shaderID, "Glow");
    colorID = GetUniform(shaderID, "Color");

    int txv[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUseProgram(shaderID);
    glUniform1iv(texturesID, 8, txv);
    isInitialized = 1;
}
void CloudShader::Bind() {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}
void CloudShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Texture2DShader::Initialize(string dirPath) {
    if (isInitialized) return;
    cout << "Loading Texture2DShader\n";
    // Initialize VBO
    glGenBuffers(1, &(Text2DVertexBufferID));
    glGenBuffers(1, &(Text2DUVBufferID));
    glGenBuffers(1, &(Text2DColorBufferID));
    glGenBuffers(1, &(Text2DElementBufferID));

    // Initialize Shader
    GLuint vID, fID;
    shaderID = LoadShaders((dirPath + "Texture2dShader.vertexshader").c_str(), (dirPath + "Texture2dShader.fragmentshader").c_str(), vID, fID);
    glBindAttribLocation(shaderID, 0, "vertexPosition_screenspace");
    glBindAttribLocation(shaderID, 1, "vertexUV");
    glBindAttribLocation(shaderID, 2, "vertexColor");
    LinkShaders(shaderID, vID, fID);

    // Initialize uniforms' IDs
    Text2DUniformID = GetUniform(shaderID, "myTextureSampler");
    Text2DUseRoundMaskID = GetUniform(shaderID, "isRound");
    Text2DRoundMaskID = GetUniform(shaderID, "roundMaskTexture");
    xDimID = GetUniform(shaderID, "xdim");
    yDimID = GetUniform(shaderID, "ydim");
    Text2DStartUVID = GetUniform(shaderID, "startUV");
    xModID = GetUniform(shaderID, "xmod");
    yModID = GetUniform(shaderID, "ymod");

    glUseProgram(shaderID);
    glUniform1i(Text2DUniformID, 0);
    glUniform1i(Text2DRoundMaskID, 1);

    isInitialized = 1;
}
void Texture2DShader::Bind(GLfloat xdim, GLfloat ydim) {
    if (!isInitialized) {
        printf("SHADER BINDED BEFORE INITIALIZATION");
        int a;
        cin >> a;
    }
    glUseProgram(shaderID);
    glUniform1f(xDimID, xdim);
    glUniform1f(yDimID, ydim);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}
void Texture2DShader::UnBind() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path, GLuint &VertexShaderID, GLuint &FragmentShaderID) {
    int a;
    // Create the shaders
    VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    } else {
        printf("Vertex Shader did not open!\n");
        printf("Enter any key to exit...\n");
        cin >> a;
        exit(5553);
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    } else {
        printf("Fragment Shader did not open!\n");
        printf("Enter any key to exit...\n");
        cin >> a;
        exit(5553);
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    if (Result != 1) {
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    }


    // Compile Fragment Shader
    //printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    if (Result != 1) {
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    //fprintf(stdout, "\nLinking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);

    return ProgramID;
}

void LinkShaders(GLuint ProgramID, GLuint VertexShaderID, GLuint FragmentShaderID) {
    GLint Result = GL_FALSE;
    int InfoLogLength;

    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
    if (Result != 1) {
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    }
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
}

GLuint GetUniform(GLuint shader, const char * name) {
    GLuint rval = glGetUniformLocation(shader, name);
    if (rval == GL_INVALID_INDEX) {
        int a;
        printf("Uniform %s was not found in shader # %d.\n", name, shader);
        printf("Enter any key to continue...\n");
        cin >> a;
        return 0;
    }
    return rval;
}

GLuint GetUniformBlock(GLuint shader, const char * name) {
    GLuint rval = glGetUniformBlockIndex(shader, name);
    if (rval == GL_INVALID_INDEX) {
        int a;
        printf("Uniform Block %s was not found in shader # %d.\n", name, shader);
        printf("Enter any key to exit...\n");
        cin >> a;
        exit(5233);
    }
    return rval;
}