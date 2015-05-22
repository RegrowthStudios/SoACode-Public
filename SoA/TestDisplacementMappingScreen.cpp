#include "stdafx.h"
#include "TestDisplacementMappingScreen.h"

#include <glm/gtx/transform.hpp>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/ui/InputDispatcher.h>

#include "Errors.h"
#include "ShaderLoader.h"

const char* vertexShader = R"(

uniform mat4 unModelMatrix;
uniform mat4 unMVP;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vUV;

out vec3 fPosition;
out vec2 fUV;
out mat3 fTbnMatrix;

void main()
{
	fPosition = (unModelMatrix * vec4(vPosition, 1.0)).xyz;
	fUV = vUV;

	vec3 normal = normalize((unModelMatrix * vec4(vNormal, 0.0)).xyz);
	vec3 tangent = normalize((unModelMatrix * vec4(vTangent, 0.0)).xyz);
	tangent = normalize(tangent - dot(tangent, normal) * normal);	
	vec3 biTangent = cross(tangent, normal);

	fTbnMatrix = mat3(tangent, biTangent, normal);

	gl_Position = unMVP * vec4(vPosition, 1.0);
}
)";

const char* fragmentShader = R"(

uniform vec3 unEyePosition;

uniform sampler2D unDiffuseTexture;
uniform sampler2D unNormalTexture;
uniform sampler2D unDispTexture;

uniform float unDispScale;

in vec3 fPosition;
in vec2 fUV;
in mat3 fTbnMatrix;

out vec4 pColor;

vec2 calculateOffset(vec2 uv, sampler2D dispTexture, mat3 tbnMatrix, vec3 directionToEye, float scale, float bias)
{
	return (directionToEye * tbnMatrix).xy * (texture(dispTexture, uv).r * scale + bias);
}

void main()
{
	vec3 directionToEye = normalize(unEyePosition - fPosition);
	float bias = -unDispScale / 2.0;
	vec2 offsetUV = fUV + calculateOffset(fUV, unDispTexture, fTbnMatrix, directionToEye, unDispScale, bias);
	
	vec3 correctNormal = fTbnMatrix * normalize(texture2D(unNormalTexture, offsetUV).rgb * 2.0 - 1.0);
	vec3 directionToLight = vec3(-0.5, 0.25, -2.5) - fPosition;
	float light = dot(correctNormal, normalize(directionToLight)) / (length(directionToLight) + 1.0);

	pColor = vec4(texture(unDiffuseTexture, offsetUV).rgb * light, 1.0);
}

)";

i32 TestDisplacementMappingScreen::getNextScreen() const
{
	return SCREEN_INDEX_NO_SCREEN;
}

i32 TestDisplacementMappingScreen::getPreviousScreen() const
{
	return SCREEN_INDEX_NO_SCREEN;
}

void TestDisplacementMappingScreen::build()
{

}
void TestDisplacementMappingScreen::destroy(const vui::GameTime& gameTime)
{

}

void TestDisplacementMappingScreen::onEntry(const vui::GameTime& gameTime)
{
	m_displacementScale = 0.05f;
	m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
		m_displacementScale += e.dy * 0.01;
		if (m_displacementScale < 0.0f) m_displacementScale = 0.0f;
	});

	m_camera.setPosition(f64v3(0.0f, 0.0f, 0.0f));
	m_camera.setFieldOfView(90.0f);
	m_camera.setAspectRatio(m_game->getWindow().getAspectRatio());
	m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
	m_camera.setUp(f32v3(0.0f, 1.0f, 0.0f));
	m_camera.setRight(f32v3(1.0f, 0.0f, 0.0f));
	m_camera.setClippingPlane(0.1f, 16.0f);
	m_camera.update();
	
	m_program = ShaderLoader::createProgram("ParallaxDisplacementMapping", vertexShader, fragmentShader);

	vg::BitmapResource rs = vg::ImageIO().load("Textures/Test/stone.png");
	if (rs.data == nullptr) pError("Failed to load texture");
	m_diffuseTexture = vg::GpuMemory::uploadTexture(&rs, vg::TexturePixelType::UNSIGNED_BYTE, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
	vg::ImageIO().free(rs);

	rs = vg::ImageIO().load("Textures/Test/stone_NRM.png");
	if (rs.data == nullptr) pError("Failed to load texture");
	m_normalTexture = vg::GpuMemory::uploadTexture(&rs, vg::TexturePixelType::UNSIGNED_BYTE, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
	vg::ImageIO().free(rs);

	rs = vg::ImageIO().load("Textures/Test/stone_DISP.png");
	if (rs.data == nullptr) pError("Failed to load texture");
	m_displacementTexture = vg::GpuMemory::uploadTexture(&rs, vg::TexturePixelType::UNSIGNED_BYTE, vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_WRAP_MIPMAP);
	vg::ImageIO().free(rs);

	float anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_displacementTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}

void TestDisplacementMappingScreen::onExit(const vui::GameTime& gameTime)
{
	delete m_program;
	glDeleteTextures(1, &m_diffuseTexture);
	glDeleteTextures(1, &m_normalTexture);
	glDeleteTextures(1, &m_displacementTexture);
}

void TestDisplacementMappingScreen::update(const vui::GameTime& gameTime)
{

}

void TestDisplacementMappingScreen::draw(const vui::GameTime& gameTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program->use();

	f32m4 unModelMatrix = glm::translate(0.0f, 0.0f, -3.0f) * glm::rotate(sinf(gameTime.total * 1.0f) * 80.0f, f32v3(0.0f, 1.0f, 0.0f)) * glm::rotate((float)gameTime.total * 30.0f, f32v3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(m_program->getUniform("unModelMatrix"), 1, false, (f32*)&unModelMatrix[0][0]);
	f32m4 unMVP = m_camera.getViewProjectionMatrix() * unModelMatrix;
	glUniformMatrix4fv(m_program->getUniform("unMVP"), 1, false, (f32*)&unMVP[0][0]);

	glUniform3f(m_program->getUniform("unEyePosition"), m_camera.getPosition().x, m_camera.getPosition().y, m_camera.getPosition().z);
	glUniform1i(m_program->getUniform("unDiffuseTexture"), 0);
	glUniform1i(m_program->getUniform("unNormalTexture"), 1);
	glUniform1i(m_program->getUniform("unDispTexture"), 2);

	glUniform1f(m_program->getUniform("unDispScale"), m_displacementScale);

	glBegin(GL_QUADS);
	float tangentDir = 1.0f;
	glVertexAttrib3f(0, -1.0f, 1.0f, 0.0f);
	glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
	glVertexAttrib3f(2, 0.0f, tangentDir, 0.0f);
	glVertexAttrib2f(3, 0.0f, 0.0f);

	glVertexAttrib3f(0, -1.0f, -1.0f, 0.0f);
	glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
	glVertexAttrib3f(2, 0.0f, tangentDir, 0.0f);
	glVertexAttrib2f(3, 0.0f, 1.0f);

	glVertexAttrib3f(0, 1.0f, -1.0f, 0.0f);
	glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
	glVertexAttrib3f(2, 0.0f, tangentDir, 0.0f);
	glVertexAttrib2f(3, 1.0f, 1.0f);

	glVertexAttrib3f(0, 1.0f, 1.0f, 0.0f);
	glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
	glVertexAttrib3f(2, 0.0f, tangentDir, 0.0f);
	glVertexAttrib2f(3, 1.0f, 0.0f);

	glEnd();
	
	m_program->unuse();
	checkGlError("TestDisplacementMappingScreen::draw");
}
