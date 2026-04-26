///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  Edits by Faith Agepogu
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables and defines
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_TextureOverlayValueName = "overlayTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
	const char* g_UseTextureOverlayName = "bUseTextureOverlay";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	// create the shape meshes object
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// free the allocated objects
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}

	// free the allocated OpenGL textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
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

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
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

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
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

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
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

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
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

/***********************************************************
 *  SetShaderTextureOverlay()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTextureOverlay(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		if (textureTag.size() > 0)
		{
			m_pShaderManager->setIntValue(g_UseTextureOverlayName, true);

			int textureID = -1;
			textureID = FindTextureSlot(textureTag);
			m_pShaderManager->setSampler2DValue(g_TextureOverlayValueName, textureID);
		}
		else
		{
			m_pShaderManager->setIntValue(g_UseTextureOverlayName, false);
		}
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
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
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** The code in the methods BELOW is for preparing and     ***/
/*** rendering the 3D replicated scenes.                    ***/
/**************************************************************/

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/wood_cherry_seamless.jpg",
		"table");

	bReturn = CreateGLTexture(
		"textures/bricks_weathered_seamless.jpg",
		"backdrop");

	bReturn = CreateGLTexture(
		"textures/screen_wallpaper_3.jpg",
		"screen");

	bReturn = CreateGLTexture(
		"textures/laptop_2.jpg",
		"keyboard");

	bReturn = CreateGLTexture(
		"textures/laptopplastic.jpg",
		"laptopplastic");

	bReturn = CreateGLTexture(
		"textures/plastic_blue_seamless.jpg",
		"vase1");

	bReturn = CreateGLTexture(
		"textures/plastic_grey_seamless.jpg",
		"vase2");

	bReturn = CreateGLTexture(
		"textures/leather_brown_seamless.jpg",
		"leftbook1");

	bReturn = CreateGLTexture(
		"textures/paper_lined_seamless.jpg",
		"notepaper");

	// after the texture image data backdrop is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	woodMaterial.shininess = 2.0f;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL stoneMaterial;
	stoneMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	stoneMaterial.specularColor = glm::vec3(0.73f, 0.3f, 0.3f);
	stoneMaterial.shininess = 6.0;
	stoneMaterial.tag = "stone";
	m_objectMaterials.push_back(stoneMaterial);

	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.diffuseColor = glm::vec3(0.8f, 0.4f, 0.8f);
	plasticMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.shininess = 2.0;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.8f);
	metalMaterial.shininess = 8.0;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// subtle lighting for the room
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.2f, -1.0f, 0.3f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.06f, 0.06f, 0.07f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.35f, 0.35f, 0.4f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// ceiling lighting
	m_pShaderManager->setVec3Value("pointLights[0].position", 0.0f, 6.0f, 0.5f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.08f, 0.08f, 0.07f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 1.3f, 1.25f, 1.15f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 1.0f, 1.0f, 0.9f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	// screen lighting
	m_pShaderManager->setVec3Value("pointLights[1].position", 0.0f, 2.2f, -1.2f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.015f, 0.015f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.22f, 0.32f, 0.5f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.18f, 0.22f, 0.35f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

	// i added more lights to address the feedback about weaker lighting
	m_pShaderManager->setVec3Value("pointLights[2].position", -3.0f, 3.0f, 2.0f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.02f, 0.02f, 0.025f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.25f, 0.25f, 0.3f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.15f, 0.15f, 0.2f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the texture image files for the textures applied
	// to objects in the 3D scene
	LoadSceneTextures();
	// define the materials that will be used for the objects
	// in the 3D scene
	DefineObjectMaterials();
	// add and defile the light sources for the 3D scene
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh(0.15);
	m_basicMeshes->LoadExtraTorusMesh1(0.2);
	m_basicMeshes->LoadExtraTorusMesh2();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	RenderTable();
	RenderBackdrop();
	RenderLaptop();
	RenderLeftVase();
	RenderRightVase();
	RenderLeftBooks();
	RenderNotebook();
}

/***********************************************************
 *  RenderLaptop()
 *
 *  This method renders a laptop on the table
 ***********************************************************/
void SceneManager::RenderLaptop()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// laptop base
	scaleXYZ = glm::vec3(4.0f, 0.2f, 2.5f);
	positionXYZ = glm::vec3(0.0f, 0.7f, 0.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);

	// top keyboard texture
	SetShaderTexture("keyboard");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	// other sides of the laptop
	SetShaderTexture("laptopplastic");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);


	// top part of the laptop
	scaleXYZ = glm::vec3(4.0f, 2.5f, 0.15f);
	XrotationDegrees = -10.0f;
	positionXYZ = glm::vec3(0.0f, 2.0f, -1.4f);
	SetTransformations(scaleXYZ, XrotationDegrees, 0, 0, positionXYZ);

	// top laptop lid texture
	SetShaderTexture("laptopplastic");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("plastic");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// other sides of the laptop lid
	SetShaderColor(0.05f, 0.05f, 0.05f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);

	
	// laptop screen
	scaleXYZ = glm::vec3(1.8f, 1.0f, 1.1f);
	XrotationDegrees = 80.0f;
	positionXYZ = glm::vec3(0.0f, 2.02f, -1.32f);

	SetTransformations(scaleXYZ, XrotationDegrees, 0, 0, positionXYZ);
	SetShaderTexture("screen");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawPlaneMesh();
	

	// left hinge
	scaleXYZ = glm::vec3(0.07f, 0.04f, 0.09f);
	ZrotationDegrees = 90.0f;
	XrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-1.5f, 0.8f, -1.2f);

	SetTransformations(scaleXYZ, XrotationDegrees, 0, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.35f, 0.38f, 0.42f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();


	// right hinge
	scaleXYZ = glm::vec3(0.07f, 0.04f, 0.09f);
	ZrotationDegrees = 90.0f;
	XrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(1.5f, 0.8f, -1.2f);

	SetTransformations(scaleXYZ, XrotationDegrees, 0, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.35f, 0.38f, 0.42f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();
}

/***********************************************************
 *  RenderTable()
 *
 *  This method is called to render the shapes for the table
 *  object.
 ***********************************************************/
void SceneManager::RenderTable()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh ***/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, .6f, 8.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.2f, -0.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("table");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("wood");

	// draw the mesh with transformation values - this plane is used for the base
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderBackdrop()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderBackdrop()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh ***/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 15.0f, -8.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("backdrop");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("stone");

	// draw the mesh with transformation values - this plane is used for the backdrop
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  RenderLeftVase()
 *
 *  This method is called to render the left vase.
 ***********************************************************/
void SceneManager::RenderLeftVase()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// bottom taper
	scaleXYZ = glm::vec3(0.5f, 0.3f, 0.4f);
	positionXYZ = glm::vec3(-3.0f, 0.8f, -1.1f);
	SetTransformations(scaleXYZ, 180.0f, 0, 0, positionXYZ);

	SetShaderTexture("vase1");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("plastic");
	m_basicMeshes->DrawTaperedCylinderMesh();

	// main body
	scaleXYZ = glm::vec3(0.5f, 1.2f, 0.45f);
	positionXYZ = glm::vec3(-3.0f, 0.8f, -1.1f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// top taper
	scaleXYZ = glm::vec3(0.5f, 0.4f, 0.5f);
	positionXYZ = glm::vec3(-3.0f, 2.0f, -1.1f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawTaperedCylinderMesh();

	// top ridge
	scaleXYZ = glm::vec3(0.29f, 0.1f, 0.29f);
	positionXYZ = glm::vec3(-3.0f, 2.4f, -1.1f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);

	SetShaderTexture("vase1");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("plastic");
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 *  RenderRightVase()
 *
 *  This method is called to render the right vase.
 ***********************************************************/
void SceneManager::RenderRightVase()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// bottom taper
	scaleXYZ = glm::vec3(0.5f, 0.3f, 0.4f);
	positionXYZ = glm::vec3(3.0f, 0.8f, -1.1f);
	SetTransformations(scaleXYZ, 180.0f, 0, 0, positionXYZ);

	SetShaderTexture("vase2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("plastic");
	m_basicMeshes->DrawTaperedCylinderMesh();

	// main body
	scaleXYZ = glm::vec3(0.5f, 1.2f, 0.45f);
	positionXYZ = glm::vec3(3.0f, 0.8f, -1.1f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	// top ridge
	scaleXYZ = glm::vec3(0.49f, 0.2f, 0.49f);
	positionXYZ = glm::vec3(3.0f, 2.0f, -1.1f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);

	SetShaderTexture("vase2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("plastic");
	m_basicMeshes->DrawTorusMesh();

	// spout
	scaleXYZ = glm::vec3(0.18f, 0.1f, 0.6f);
	positionXYZ = glm::vec3(2.45f, 1.65f, -1.1f);
	SetTransformations(scaleXYZ, -90.0f, 0, 20.0f, positionXYZ);
	m_basicMeshes->DrawPrismMesh();

	// handle
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.35f);
	positionXYZ = glm::vec3(3.3f, 1.55f, -1.1f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawExtraTorusMesh1();
}

/***********************************************************
 *  RenderLeftBooks()
 *
 *  This method is called to render the left book stack.
 ***********************************************************/
void SceneManager::RenderLeftBooks()
{
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;

	// book 2 pages
	scaleXYZ = glm::vec3(1.0f, 0.24f, 0.51f);
	positionXYZ = glm::vec3(-3.2f, 0.60f, 0.2f);
	SetTransformations(scaleXYZ, 0, -2.0f, 0, positionXYZ);
	SetShaderColor(0.9f, 0.9f, 0.85f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// book 2 top cover
	scaleXYZ = glm::vec3(1.05f, 0.03f, 0.57f);
	positionXYZ = glm::vec3(-3.2f, 0.74f, 0.2f);
	SetTransformations(scaleXYZ, 0, -2.0f, 0, positionXYZ);
	SetShaderTexture("leftbook1");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	// book 2 bottom cover
	positionXYZ = glm::vec3(-3.2f, 0.49f, 0.2f);
	SetTransformations(scaleXYZ, 0, -1.9f, 0, positionXYZ);
	SetShaderTexture("leftbook1");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();

	// book 2 spine
	scaleXYZ = glm::vec3(0.1f, 0.24f, 0.55f);
	positionXYZ = glm::vec3(-3.75f, 0.60f, 0.2f);
	SetTransformations(scaleXYZ, 0, -2.0f, 0, positionXYZ);
	SetShaderTexture("leftbook1");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh();


	// book 1 pages
	scaleXYZ = glm::vec3(1.0f, 0.24f, 0.51f);
	positionXYZ = glm::vec3(-3.05f, 0.92f, 0.25f);
	SetTransformations(scaleXYZ, 0, 4.0f, 0, positionXYZ);
	SetShaderColor(0.88f, 0.88f, 0.82f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// book 1 top cover
	scaleXYZ = glm::vec3(1.05f, 0.03f, 0.57f);
	positionXYZ = glm::vec3(-3.05f, 1.05f, 0.25f);
	SetTransformations(scaleXYZ, 0, 4.0f, 0, positionXYZ);
	SetShaderColor(0.2f, 0.3f, 0.5f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// book 1 bottom cover
	positionXYZ = glm::vec3(-3.05f, 0.78f, 0.25f);
	SetTransformations(scaleXYZ, 0, 4.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// book 1 spine
	scaleXYZ = glm::vec3(0.1f, 0.23f, 0.51f);
	positionXYZ = glm::vec3(-3.55f, 0.93f, 0.25f);
	SetTransformations(scaleXYZ, 0, 4.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderNotebook()
 *
 *  This method is called to render the right notebook.
 ***********************************************************/
void SceneManager::RenderNotebook()
{
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;

	// pages
	scaleXYZ = glm::vec3(1.2f, 0.1f, 0.7f);
	positionXYZ = glm::vec3(3.2f, 0.75f, 0.0f);
	SetTransformations(scaleXYZ, 0, 6.0f, 0, positionXYZ);

	SetShaderTexture("notepaper");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	SetShaderColor(0.9f, 0.9f, 0.85f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// bottom cover
	scaleXYZ = glm::vec3(1.25f, 0.035f, 0.75f);
	positionXYZ = glm::vec3(3.2f, 0.75f - 0.12f / 2 - 0.035f / 2, 0.0f);
	SetTransformations(scaleXYZ, 0, 6.0f, 0, positionXYZ);
	SetShaderColor(0.3f, 0.3f, 0.35f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// spiral rings
	scaleXYZ = glm::vec3(0.06f, 0.06f, 0.02f);

	positionXYZ = glm::vec3(2.6f, 0.78f, -0.15f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawTorusMesh();

	positionXYZ = glm::vec3(2.6f, 0.78f, 0.0f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawTorusMesh();

	positionXYZ = glm::vec3(2.6f, 0.78f, 0.15f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawTorusMesh();

	positionXYZ = glm::vec3(2.6f, 0.78f, 0.3f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawTorusMesh();

	positionXYZ = glm::vec3(2.6f, 0.78f, 0.45f);
	SetTransformations(scaleXYZ, 90.0f, 0, 0, positionXYZ);
	m_basicMeshes->DrawTorusMesh();
}
