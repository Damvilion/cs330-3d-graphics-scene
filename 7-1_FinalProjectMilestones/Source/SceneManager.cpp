///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <string>

namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	DestroyGLTextures();
}

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;
	return false;
}

void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	scale = glm::scale(scaleXYZ);
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

void SceneManager::PrepareScene()
{
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();

	if (!CreateGLTexture("../../Utilities/textures/pavers.jpg", "ground"))
	{
		CreateGLTexture("../../../Utilities/textures/pavers.jpg", "ground");
	}
	if (!CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "wood"))
	{
		CreateGLTexture("../../../Utilities/textures/rusticwood.jpg", "wood");
	}
	if (!CreateGLTexture("../../Utilities/textures/stainless.jpg", "metal"))
	{
		CreateGLTexture("../../../Utilities/textures/stainless.jpg", "metal");
	}
	if (!CreateGLTexture("../../Utilities/textures/tilesf2.jpg", "keyboard"))
	{
		CreateGLTexture("../../../Utilities/textures/tilesf2.jpg", "keyboard");
	}
	if (!CreateGLTexture("textures/laptop.png", "screen"))
	{
		CreateGLTexture("../textures/laptop.png", "screen");
	}
	if (!CreateGLTexture("../../Utilities/textures/abstract.jpg", "book"))
	{
		CreateGLTexture("../../../Utilities/textures/abstract.jpg", "book");
	}
	BindGLTextures();

	m_objectMaterials.clear();
	m_objectMaterials.push_back({ 0.38f, glm::vec3(0.84f, 0.84f, 0.84f), glm::vec3(0.95f, 0.95f, 0.95f), glm::vec3(0.20f, 0.20f, 0.20f), 0.12f, "ground" });
	m_objectMaterials.push_back({ 0.52f, glm::vec3(0.86f, 0.72f, 0.48f), glm::vec3(0.88f, 0.74f, 0.50f), glm::vec3(0.28f, 0.22f, 0.14f), 0.25f, "wood" });
	m_objectMaterials.push_back({ 0.28f, glm::vec3(0.58f, 0.58f, 0.62f), glm::vec3(0.86f, 0.86f, 0.90f), glm::vec3(0.95f, 0.95f, 0.98f), 0.72f, "metal" });
	m_objectMaterials.push_back({ 0.30f, glm::vec3(0.72f, 0.72f, 0.76f), glm::vec3(0.80f, 0.80f, 0.84f), glm::vec3(0.50f, 0.50f, 0.55f), 0.30f, "keyboard" });
	m_objectMaterials.push_back({ 0.30f, glm::vec3(0.84f, 0.84f, 0.88f), glm::vec3(0.95f, 0.95f, 1.00f), glm::vec3(0.80f, 0.80f, 0.85f), 0.75f, "screen" });
	m_objectMaterials.push_back({ 0.34f, glm::vec3(0.86f, 0.76f, 0.60f), glm::vec3(0.90f, 0.80f, 0.64f), glm::vec3(0.18f, 0.18f, 0.18f), 0.18f, "book" });
	m_objectMaterials.push_back({ 0.42f, glm::vec3(0.94f, 0.94f, 0.94f), glm::vec3(0.98f, 0.98f, 0.98f), glm::vec3(0.78f, 0.78f, 0.78f), 0.55f, "ceramic" });
}

void SceneManager::SetLighting()
{
	if (m_pShaderManager == NULL)
	{
		return;
	}

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	m_pShaderManager->setVec3Value("lightSources[0].position", 3.5f, 10.0f, 7.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.55f, 0.55f, 0.58f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 1.20f, 1.20f, 1.18f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.40f);

	m_pShaderManager->setVec3Value("lightSources[1].position", -6.5f, 5.0f, 4.5f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.18f, 0.10f, 0.08f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 1.05f, 0.65f, 0.38f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 1.00f, 0.70f, 0.45f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 24.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.35f);

	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 6.5f, -8.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.12f, 0.14f, 0.18f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.55f, 0.65f, 0.85f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.45f, 0.55f, 0.75f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 18.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.20f);

	const std::string lightPrefix = "lightSources[3]";
	m_pShaderManager->setVec3Value(lightPrefix + ".position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value(lightPrefix + ".ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value(lightPrefix + ".diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value(lightPrefix + ".specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue(lightPrefix + ".focalStrength", 1.0f);
	m_pShaderManager->setFloatValue(lightPrefix + ".specularIntensity", 0.0f);
}

void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	SetLighting();

	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, -0.5f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("ground");
	SetTextureUVScale(4.0f, 4.0f);
	SetShaderMaterial("ground");
	m_basicMeshes->DrawPlaneMesh();

	scaleXYZ = glm::vec3(8.0f, 0.35f, 4.0f);
	positionXYZ = glm::vec3(0.0f, 3.0f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("wood");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(0.30f, 3.0f, 0.30f);
	positionXYZ = glm::vec3(-3.5f, 0.0f, 1.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	positionXYZ = glm::vec3(3.5f, 0.0f, 1.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	positionXYZ = glm::vec3(-3.5f, 0.0f, -1.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	positionXYZ = glm::vec3(3.5f, 0.0f, -1.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	scaleXYZ = glm::vec3(3.0f, 0.15f, 2.0f);
	positionXYZ = glm::vec3(0.0f, 3.3f, 0.0f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("keyboard");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("keyboard");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(3.0f, 2.0f, 0.10f);
	positionXYZ = glm::vec3(0.0f, 4.3f, -0.65f);
	SetTransformations(scaleXYZ, -20.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("screen");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("screen");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(1.6f, 0.25f, 2.2f);
	positionXYZ = glm::vec3(2.8f, 3.275f, 0.5f);
	SetTransformations(scaleXYZ, 0.0f, 15.0f, 0.0f, positionXYZ);
	SetShaderTexture("book");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("book");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(0.45f, 0.90f, 0.45f);
	positionXYZ = glm::vec3(-2.8f, 3.15f, 0.6f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderColor(0.97f, 0.97f, 0.97f, 1.0f);
	SetShaderMaterial("ceramic");
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	scaleXYZ = glm::vec3(0.28f, 0.28f, 0.28f);
	positionXYZ = glm::vec3(-2.35f, 3.6f, 0.6f);
	SetTransformations(scaleXYZ, 0.0f, 90.0f, 0.0f, positionXYZ);
	SetShaderColor(0.97f, 0.97f, 0.97f, 1.0f);
	SetShaderMaterial("ceramic");
	m_basicMeshes->DrawTorusMesh();
}
