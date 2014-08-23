#pragma once
#include "Constants.h"
#include "GLProgram.h"

class Shader
{
public:
	Shader() : isInitialized(0){}

	GLuint shaderID;
	GLuint texturesID;
	void Initialize(){};
	void DeleteShader();
	void Bind(){};
	void UnBind(){};
	bool IsInitialized(){return isInitialized;}
protected:
	bool isInitialized;
};

class BasicColorShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint mvpID;
	GLuint colorID;
private:
    GLProgram program;
};

class SimplexNoiseShader : public Shader
{
public:
	SimplexNoiseShader() : permTexture(0), gradTexture(0){}
	void Initialize();
	void Bind();
	void UnBind();

	GLuint permID, permTexture;
	GLuint gradID, gradTexture;
	GLuint dtID;
	
	GLuint heightModifierID;
	GLuint baseTemperatureID;
	GLuint baseRainfallID;
	
	GLuint typesID;
	GLuint persistencesID;
	GLuint frequenciesID;
	GLuint octavesID;
	GLuint lowsID;
	GLuint highsID;
	GLuint scalesID;

	GLuint tempPID;
	GLuint tempFID;
	GLuint tempOID;
	GLuint tempLID;
	GLuint tempHID;
	
	GLuint rainPID;
	GLuint rainFID;
	GLuint rainOID;
	GLuint rainLID;
	GLuint rainHID;

	GLuint numFunctionsID;
};

class HDRShader : public Shader
{
public :
	void Initialize(std::string dirPath = "Shaders/PostProcessing/");
	void Bind();
	void UnBind();

	GLuint texID;
	GLuint fExposureID;
	GLuint gammaID;
	GLuint averageLuminanceID;
};


class MotionBlurShader : public Shader
{
public:
	void Initialize(std::string dirPath = "Shaders/PostProcessing/");
	void Bind();
	void UnBind();

	GLuint texID;
	GLuint depthID;
	GLuint fExposureID;
	GLuint gammaID;
	GLuint averageLuminanceID;

	GLuint numSamplesID;
	GLuint inverseVPID;
	GLuint prevVPID;

	glm::mat4 oldVP;
	glm::mat4 newInverseVP;
};

class BlockShader : public Shader
{
public:
	void Initialize(std::string dirPath = "Shaders/BlockShading/");
	void Bind();
	void UnBind();
	GLuint fadeDistanceID;
	GLuint fogStartID, fogEndID, fogColorID, lightTypeID, ambientID, lightColorID;
	GLuint sunValID, blockDtID;
	GLuint mvpID, mID;
	GLuint eyeVecID;
	GLuint alphaMultID;
    GLuint lightID;
	GLuint specularIntensityID, specularExponentID;
};

class NonBlockShader : public Shader
{
public:
    void Initialize(std::string dirPath = "Shaders/BlockShading/");
    void Bind();
    void UnBind();
    GLuint fadeDistanceID;
    GLuint fogStartID, fogEndID, fogColorID, lightTypeID, ambientID, lightColorID;
    GLuint sunValID, blockDtID;
    GLuint mvpID, mID;
    GLuint eyeVecID;
    GLuint alphaMultID;
    GLuint lightID;
    GLuint specularIntensityID, specularExponentID;
};

class TextureShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();
	
	GLuint mvpID;
	GLuint texID;
};

class AtmosphereShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint mvpID;
	GLuint mID;
	GLuint cameraPosID, lightPosID, invWavelengthID;
	GLuint cameraHeightID, cameraHeight2ID;
	GLuint outerRadiusID, outerRadius2ID;
	GLuint innerRadiusID, innerRadius2ID;
	GLuint KrESunID, KmESunID, Kr4PIID, Km4PIID;
	GLuint scaleID, scaleDepthID, scaleOverScaleDepthID;
	GLuint gID, g2ID;
	GLuint fSamplesID, nSamplesID;
};

class AtmosphereToSkyShader : public AtmosphereShader
{
public:
	void Initialize();
};

class SpaceToSkyShader : public AtmosphereShader
{
public:
	void Initialize();
};

class AtmosphereToGroundShader : public AtmosphereShader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint worldOffsetID;
	GLuint specularIntensityID, specularExponentID;
	GLuint sunColorTextureID;
	GLuint waterColorTextureID;
	GLuint fadeDistanceID;
	GLuint colorTextureID;
	GLuint secColorMultID;
	GLuint dtID;
	GLuint freezeTempID;
};

class SpaceToGroundShader : public AtmosphereShader
{
public: 
	void Initialize();
	void Bind();
	void UnBind();

	GLuint worldOffsetID;
	GLuint specularIntensityID, specularExponentID;
	GLuint sunColorTextureID;
	GLuint colorTextureID;
	GLuint waterColorTextureID;
	GLuint drawModeID;
	GLuint secColorMultID;
	GLuint dtID;
	GLuint freezeTempID;
};

class WaterShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();
	GLuint lightID;
	GLuint fogStartID, fogEndID, fogColorID, ambientID, lightColorID, sunValID;
	GLuint normalMapID;
	GLuint mvpID;
	GLuint mID;
	GLuint dtID;
	GLuint fadeDistanceID;
};

//class ParticleShader : public Shader
//{
//public:
//	void Initialize();
//	void Bind();
//	void UnBind();
//
//	GLuint ambientID;
//	GLuint colorID, textureUnitID, gVPID, UVstartID, UVwidthID, sunValID, lightTypeID, eyeNormalID;
//};

class BillboardShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint mvpID, mID, sunValID, lightTypeID, eyeNormalID, ambientID;
	GLuint cameraUpID, alphaThresholdID, cameraRightID;
};

class FixedSizeBillboardShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint colorID, textureUnitID, mvpID, uVstartID, uvModID, uVwidthID, widthID, heightID;
};

class SonarShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint distanceID, waveID, dtID;
	GLuint fadeDistanceID;
	GLuint mvpID;
	GLuint mID;
};

class PhysicsBlockShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint fadeDistanceID;
	GLuint fogStartID, fogEndID, fogColorID, lightTypeID, ambientID, lightColorID;
	GLuint sunValID;
	GLuint mvpID, mID;
	GLuint eyeVecID;
	GLuint lightID;
	GLuint alphaMultID;
	GLuint specularIntensityID, specularExponentID;
};

class TreeShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

//	GLuint playerPosID;
	GLuint mvpID;
	GLuint mID;
	GLuint worldUpID;
	GLuint fadeDistanceID;
	GLuint sunValID;
};

class CloudShader : public Shader
{
public:
	void Initialize();
	void Bind();
	void UnBind();

	GLuint hasAlphaID;
	GLuint textureUnitID, gVPID, mID, fogStartID, fogEndID, ambientID, lightColorID;
	GLuint colorID, lightID;
	GLuint glowID;
};

class Texture2DShader : public Shader
{
	public:
	void Initialize(std::string dirPath = "Shaders/TextureShading/");
	void Bind(GLfloat xdim = 1024.0f, GLfloat ydim = 768.0f);
	void UnBind();
	GLuint Text2DUniformID;
	GLuint Text2DUseRoundMaskID;
	GLuint Text2DRoundMaskID;
	GLuint Text2DStartUVID;
	GLuint xDimID, yDimID, xModID, yModID;
	GLuint Text2DTextureID, Text2DColorBufferID, Text2DVertexBufferID, Text2DUVBufferID, Text2DElementBufferID;
};

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path, GLuint &VertexShaderID, GLuint &FragmentShaderID);
void LinkShaders(GLuint ProgramID, GLuint VertexShaderID, GLuint FragmentShaderID);

GLuint GetUniform(GLuint shader, const char * name);

GLuint GetUniformBlock(GLuint shader, const char * name);

extern BasicColorShader basicColorShader;
extern HDRShader hdrShader;
extern MotionBlurShader motionBlurShader;
extern BlockShader blockShader;
extern NonBlockShader nonBlockShader;
extern TextureShader textureShader;
extern AtmosphereToSkyShader atmosphereToSkyShader;
extern AtmosphereToGroundShader atmosphereToGroundShader;
extern SpaceToSkyShader spaceToSkyShader;
extern SpaceToGroundShader spaceToGroundShader;
extern WaterShader waterShader;
//extern ParticleShader particleShader;
extern BillboardShader billboardShader;
extern FixedSizeBillboardShader fixedSizeBillboardShader;
extern SonarShader sonarShader;
extern PhysicsBlockShader physicsBlockShader;
extern TreeShader treeShader;
extern CloudShader cloudShader;
extern Texture2DShader texture2Dshader;
extern SimplexNoiseShader simplexNoiseShader;