//
// Copyright (C) 2014
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "PostProcessor.h"
#include <cmath>
#include <string>
#include <algorithm>

using std::string;

#define MULT 1

string skyboxVertShader =

"attribute vec3 Vertex;"
"attribute vec3 Normal;"
"attribute vec3 Tangent;"
"attribute vec3 Color;"

"uniform mat4 TextureMatrix[4];"
"uniform mat4 ModelViewMatrix;"
"uniform mat4 ProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform mat4 ProjModelViewMatrix;"

"varying vec2 texCoord;"
"varying vec4 position, normal, tangent;"

"attribute vec2 TexCoord;"

"void main(void)"
"{"
	"normal = NormalMatrix * vec4(Normal, 1.0);"
	"position = ModelViewMatrix * vec4(Vertex, 1.0);"
	"gl_Position = ProjModelViewMatrix * vec4(Vertex, 1.0);"
	"texCoord = TexCoord;"

	"tangent = NormalMatrix * vec4(Tangent, 1.0);"
"}\n";

string skyboxFragShader =
"uniform sampler2D Texture[5];"

"varying vec2 texCoord;"
"varying vec4 position, normal, tangent;"

"void main(void)"
"{"
	"gl_FragColor = texture2D(Texture[0], texCoord);"
"}\n";

inline int Pow2(int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x+1;
}

void PostProcessor::EraseTextures()
{
	MRenderingContext * render = MEngine::getInstance()->getRenderingContext();
	MSystemContext * system = MEngine::getInstance()->getSystemContext();

	render->deleteTexture(&m_ColourTexID);
	render->deleteTexture(&m_DepthTexID);

	render->deleteFrameBuffer(&m_BufferID);
}

void PostProcessor::UpdateResolution()
{
	MRenderingContext * render = MEngine::getInstance()->getRenderingContext();
	MSystemContext * system = MEngine::getInstance()->getSystemContext();

	// screen size
	unsigned int screenWidth = 0;
	unsigned int screenHeight = 0;
	system->getScreenSize(&screenWidth, &screenHeight);

	m_Resolution = Pow2(max(screenWidth, screenHeight));

	m_Resolution *= MULT;

	// create frame buffer
	render->createFrameBuffer(&m_BufferID);

	// create render textures
	render->createTexture(&m_ColourTexID);
	render->bindTexture(m_ColourTexID);
	render->setTextureFilterMode(M_TEX_FILTER_LINEAR, M_TEX_FILTER_LINEAR);
	render->setTextureUWrapMode(M_WRAP_CLAMP);
	render->setTextureVWrapMode(M_WRAP_CLAMP);
	render->texImage(0, m_Resolution, m_Resolution, M_UBYTE, M_RGB, 0);

	render->createTexture(&m_DepthTexID);
	render->bindTexture(m_DepthTexID);
	render->setTextureFilterMode(M_TEX_FILTER_NEAREST, M_TEX_FILTER_NEAREST);
	render->texImage(0, m_Resolution, m_Resolution, M_UBYTE, M_DEPTH, 0);
}

PostProcessor::PostProcessor()
	: m_BufferID(0)
	, m_ColourTexID(0)
	, m_DepthTexID(0)
	, m_Shader(0)
	, m_SkyboxShader(0)
{
    for(int i = 0; i < 6; i++)
	{
		m_SkyboxTexture[i].m_Image = NULL;
		m_SkyboxTexture[i].m_TexID = 0;
	}

	UpdateResolution();
}

bool PostProcessor::Render()
{
	if(!m_Shader)
		return false;

	MEngine * engine = MEngine::getInstance(); // get the engine instance
	MRenderingContext * render = engine->getRenderingContext(); // get the rendering context
	MSystemContext * system = engine->getSystemContext();
	unsigned int currentFrameBuffer = 0;
   	render->getCurrentFrameBuffer(&currentFrameBuffer);

	if(strcmp(engine->getRenderer()->getName(), "FixedRenderer") == 0)
		return false;


	// get level
	MLevel * level = MEngine::getInstance()->getLevel();
	if(! level)
		return false;

	// get current scene
	MScene * scene = level->getCurrentScene();
	if(! scene)
		return false;

	// get camera
	MOCamera * camera = scene->getCurrentCamera();

	// update listener
	camera->updateListener();

	// enable camera with current screen ratio
	camera->enable();

	MVector3 clearColor = camera->getClearColor();
	render->setClearColor(clearColor);

	// screen size
	unsigned int screenWidth = 0;
	unsigned int screenHeight = 0;
	system->getScreenSize(&screenWidth, &screenHeight);

	// render to texture
	render->bindFrameBuffer(m_BufferID);
	render->attachFrameBufferTexture(M_ATTACH_COLOR0, m_ColourTexID);
	render->attachFrameBufferTexture(M_ATTACH_DEPTH, m_DepthTexID);
	render->setViewport(0, 0, m_Resolution, m_Resolution); // change viewport

	render->clear(M_BUFFER_COLOR | M_BUFFER_DEPTH);

  	render->disableDepthTest();

    if(m_Skybox)
        DrawSkybox(camera->getPosition(), camera->getRotation().getEulerAngles());

	render->clear(M_BUFFER_DEPTH);
	render->enableDepthTest();

	// draw the scene
	scene->draw(camera);

    for(int i = 0; i < m_CameraLayersFX.size(); i++)
    {
        render->clear(M_BUFFER_DEPTH);
        camera_layer_t layer = m_CameraLayersFX[i];

        MScene* layerScene = level->getSceneByIndex(layer.scene);

       	layer.camera->enable();
        layerScene->draw(layer.camera);
    }

	// finish render to texture
    render->bindFrameBuffer(currentFrameBuffer);

	// draw the rendered textured with a shader effect
	render->setViewport(0, 0, screenWidth, screenHeight);
	render->setClearColor(MVector3(1, 0, 0));
	render->clear(M_BUFFER_COLOR | M_BUFFER_DEPTH);

	Set2D(screenWidth, screenHeight);

	m_Shader->Apply();
	render->bindTexture(m_ColourTexID);
	render->bindTexture(m_DepthTexID, 1);

	DrawQuad(MVector2((float)screenWidth, (float)screenHeight));
	m_Shader->Clear();

	for(int i = 0; i < m_CameraLayersNoFX.size(); i++)
    {
        render->clear(M_BUFFER_DEPTH);
        camera_layer_t layer = m_CameraLayersNoFX[i];

        MScene* layerScene = level->getSceneByIndex(layer.scene);

       	layer.camera->enable();
        layerScene->draw(layer.camera);
    }

	return true;
}

void PostProcessor::SetShader(Shader* s)
{
	EraseTextures();
	UpdateResolution();

	SAFE_DELETE(m_Shader);

	m_Shader = s;
}

void PostProcessor::Set2D(unsigned int w, unsigned int h)
{
	MRenderingContext * render = MEngine::getInstance()->getRenderingContext();
	render->setViewport(0, 0, w, h);

	// set ortho projection
	render->setMatrixMode(M_MATRIX_PROJECTION);
	render->loadIdentity();

	render->setOrthoView(0.0f, (float)w, (float)h, 0.0f, 1.0f, -1.0f);

	render->setMatrixMode(M_MATRIX_MODELVIEW);
	render->loadIdentity();
}

void PostProcessor::DrawQuad(MVector2 scale)
{
	MRenderingContext * render = MEngine::getInstance()->getRenderingContext();

	int vertexAttrib;
	int texcoordAttrib;
	static MVector2 vertices[4];
	static MVector2 texCoords[4];

	vertices[0] = MVector2(0, 0);
	vertices[1] = MVector2(0, scale.y);
	vertices[3] = MVector2(scale.x, scale.y);
	vertices[2] = MVector2(scale.x, 0);

	texCoords[0] = MVector2(0, 1);
	texCoords[1] = MVector2(0, 0);
	texCoords[3] = MVector2(1, 0);
	texCoords[2] = MVector2(1, 1);

    // Send settings to shader
    SendUniforms();

	// projmodelview matrix
	static MMatrix4x4 ProjMatrix;
	static MMatrix4x4 ModelViewMatrix;
	static MMatrix4x4 ProjModelViewMatrix;

	render->getProjectionMatrix(&ProjMatrix);
	render->getModelViewMatrix(&ModelViewMatrix);
	ProjModelViewMatrix = ProjMatrix * ModelViewMatrix;
	render->sendUniformMatrix(m_Shader->ExposeShader(), "ProjModelViewMatrix", &ProjModelViewMatrix);

	// Texture
	int texIds[4] = { 0, 1, 2, 3 };
	render->sendUniformInt(m_Shader->ExposeShader(), "Textures", texIds, 4);

	// Vertex
	render->getAttribLocation(m_Shader->ExposeShader(), "Vertex", &vertexAttrib);
	render->setAttribPointer(vertexAttrib, M_FLOAT, 2, vertices);
	render->enableAttribArray(vertexAttrib);

	// TexCoord
	render->getAttribLocation(m_Shader->ExposeShader(), "TexCoord", &texcoordAttrib);
	render->setAttribPointer(texcoordAttrib, M_FLOAT, 2, texCoords);
	render->enableAttribArray(texcoordAttrib);

	// draw
	render->drawArray(M_PRIMITIVE_TRIANGLE_STRIP, 0, 4);

	render->disableAttribArray(vertexAttrib);
	render->disableAttribArray(texcoordAttrib);
}

void PostProcessor::DrawQuad(MVector3 v1, MVector3 v2, MVector3 v3, MVector3 v4, MVector3 position, MVector3 rotation, MVector2* texCoords)
{
    MRenderingContext * render = MEngine::getInstance()->getRenderingContext();

	int vertexAttrib;
	int texcoordAttrib;
	static MVector3 vertices[4];

	vertices[0] = v1;
	vertices[1] = v2;
	vertices[3] = v3;
	vertices[2] = v4;

	// projmodelview matrix
	static MMatrix4x4 ProjMatrix;
	static MMatrix4x4 ModelViewMatrix;
	static MMatrix4x4 ProjModelViewMatrix;

	render->getProjectionMatrix(&ProjMatrix);
	render->getModelViewMatrix(&ModelViewMatrix);

	ModelViewMatrix.loadIdentity();
	ModelViewMatrix.translate(position);

    // First, rotate X and Y so Z points up
	ModelViewMatrix.setRotationEuler(rotation.x,rotation.y, 0);

    // Rotate around the Z axis
	ModelViewMatrix.rotate(MVector3(0,0,1), rotation.z);

	ProjModelViewMatrix = ProjMatrix * ModelViewMatrix;
	render->sendUniformMatrix(m_SkyboxShader->ExposeShader(), "ProjModelViewMatrix", &ProjModelViewMatrix);

	// Texture
	int texIds[4] = { 0, 1, 2, 3 };
	render->sendUniformInt(m_SkyboxShader->ExposeShader(), "Textures", texIds, 4);

	// Vertex
	render->getAttribLocation(m_SkyboxShader->ExposeShader(), "Vertex", &vertexAttrib);
	render->setAttribPointer(vertexAttrib, M_FLOAT, 3, vertices);
	render->enableAttribArray(vertexAttrib);

	// TexCoord
	render->getAttribLocation(m_SkyboxShader->ExposeShader(), "TexCoord", &texcoordAttrib);
	render->setAttribPointer(texcoordAttrib, M_FLOAT, 2, texCoords);
	render->enableAttribArray(texcoordAttrib);

	// draw
	render->drawArray(M_PRIMITIVE_TRIANGLE_STRIP, 0, 4);

	render->disableAttribArray(vertexAttrib);
	render->disableAttribArray(texcoordAttrib);
}

void PostProcessor::AddFloatUniform(const char* name)
{
    float_uniform_t uniform;
    strcpy(uniform.name, name);

    m_FloatUniformList.push_back(uniform);
}

void PostProcessor::AddIntUniform(const char* name)
{
    int_uniform_t uniform;
    strcpy(uniform.name, name);

    m_IntUniformList.push_back(uniform);
}

void PostProcessor::SetIntUniformValue(const char* name, int value)
{
    for(unsigned int i = 0; i < m_IntUniformList.size(); i++)
    {
        if(!strcmp(m_IntUniformList[i].name, name))
        {
            m_IntUniformList[i].value = value;
            return;
        }
    }
}

void PostProcessor::SetFloatUniformValue(const char* name, float value)
{
    for(unsigned int i = 0; i < m_FloatUniformList.size(); i++)
    {
        if(!strcmp(m_FloatUniformList[i].name, name))
        {
            m_FloatUniformList[i].value = value;
            return;
        }
    }
}

void PostProcessor::SendUniforms()
{
    MRenderingContext * render = MEngine::getInstance()->getRenderingContext();

    // Set dynamic uniforms
    for(unsigned int i = 0; i < m_FloatUniformList.size(); i++)
        m_Shader->SetValue(m_FloatUniformList[i].name, m_FloatUniformList[i].value);

    for(unsigned int i = 0; i < m_IntUniformList.size(); i++)
        m_Shader->SetValue(m_IntUniformList[i].name, m_IntUniformList[i].value);
}

void PostProcessor::Clear()
{
    m_FloatUniformList.clear();
    m_IntUniformList.clear();

    m_CameraLayersFX.clear();
    m_CameraLayersNoFX.clear();

    m_Skybox = false;
}

void PostProcessor::AddCameraLayer(int scene, MOCamera* camera, bool pfxEnabled)
{
    camera_layer_t layer;
    layer.camera = camera;
    layer.scene = scene;

    if(pfxEnabled) m_CameraLayersFX.push_back(layer);
    else m_CameraLayersNoFX.push_back(layer);

    printf("--> AddCameraLayer(%d, 0x%x, %d)\n", scene, camera, pfxEnabled);

    printf("----> AddCameraSceneLayer: m_CameraLayersFX.size() == %d\n", m_CameraLayersFX.size());
    printf("----> AddCameraSceneLayer: m_CameraLayersNoFX.size() == %d\n", m_CameraLayersNoFX.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Skybox functionality
////////////////////////////////////////////////////////////////////////////////////////////////////////

PostProcessor::Texture PostProcessor::LoadTexture(const char* path)
{
	MRenderingContext* render = MEngine::getInstance()->getRenderingContext();
	MSystemContext* system = MEngine::getInstance()->getSystemContext();
	MEngine* engine = MEngine::getInstance();

	Texture tex;
    tex.m_Image = new MImage;

	if(engine->getImageLoader()->loadData(path, tex.m_Image))
	{
		render->createTexture(&tex.m_TexID);
		render->bindTexture(tex.m_TexID);

		render->setTextureUWrapMode(M_WRAP_CLAMP);
		render->setTextureVWrapMode(M_WRAP_CLAMP);

		render->setTextureFilterMode(M_TEX_FILTER_NEAREST, M_TEX_FILTER_NEAREST);
		render->texImage(0, tex.m_Image->getWidth(), tex.m_Image->getHeight(), M_UBYTE, M_RGB, tex.m_Image->getData());
	}
	else
	{
		printf("--> LoadTexture: Could not load image '%s'\n", path);
		SAFE_DELETE(tex.m_Image);

		tex.m_Image = NULL;
	}

	return tex;
}

void PostProcessor::LoadSkyboxTextures(const char* path)
{
	MRenderingContext* render = MEngine::getInstance()->getRenderingContext();
	MSystemContext* system = MEngine::getInstance()->getSystemContext();
	MEngine* engine = MEngine::getInstance();

    SAFE_DELETE(m_SkyboxShader);
    m_SkyboxShader = new Shader();
    m_SkyboxShader->SetVertSrc(skyboxVertShader.c_str());
    m_SkyboxShader->SetPixSrc(skyboxFragShader.c_str());

	for(int i = 0; i < 6; i++)
	{
		if(m_SkyboxTexture[i].m_Image)
			SAFE_DELETE(m_SkyboxTexture[i].m_Image);

		if(m_SkyboxTexture[i].m_TexID)
			render->deleteTexture(&m_SkyboxTexture[i].m_TexID);
	}

	string basePath = path;
	char c = '0';
    const char* names[] = {"negx", "negy", "negz", "posx", "posy", "posz"};

#ifndef WIN32
	for(int i = 0; i < 6; i++)
	{
		m_SkyboxTexture[i] = LoadTexture((basePath + "/" + names[i] + ".jpg").c_str());
	}
#else
	for(int i = 0; i < 6; i++)
	{
		m_SkyboxTexture[i] = LoadTexture((basePath + "\\" + skyboxPosfix[i]).c_str());
	}
#endif
}

void PostProcessor::DrawSkybox(MVector3 position, MVector3 rotation)
{
    MRenderingContext* render = MEngine::getInstance()->getRenderingContext();

    m_SkyboxShader->Apply();

    static MVector2 texCoords[4];

	texCoords[0] = MVector2(1, 0);
	texCoords[1] = MVector2(0, 0);
	texCoords[3] = MVector2(0, 1);
	texCoords[2] = MVector2(1, 1);

	// unten 1
	render->bindTexture(m_SkyboxTexture[1].m_TexID);
    DrawQuad(MVector3(-500.0f, 500.0f, -500.0f), MVector3(500.0f, 500.0f, -500.0f),
            MVector3(500.0f,-500.0f, -500.0f), MVector3(-500.0f,-500.0f, -500.0f), position, -rotation, (MVector2*) &texCoords);

	texCoords[0] = MVector2(1, 1);
	texCoords[1] = MVector2(0, 1);
	texCoords[3] = MVector2(0, 0);
	texCoords[2] = MVector2(1, 0);

    // oben 2
   	render->bindTexture(m_SkyboxTexture[4].m_TexID);
    DrawQuad(MVector3(-500.0f, 500.0f, 500.0f), MVector3(500.0f, 500.0f, 500.0f),
            MVector3(500.0f,-500.0f, 500.0f), MVector3(-500.0f,-500.0f, 500.0f), position, -rotation, (MVector2*) &texCoords);

    texCoords[0] = MVector2(0, 1);
	texCoords[1] = MVector2(1, 1);
	texCoords[3] = MVector2(1, 0);
	texCoords[2] = MVector2(0, 0);

    // vorne 3
	render->bindTexture(m_SkyboxTexture[5].m_TexID);
    DrawQuad(MVector3(500.0f, 500.0f, -500.0f), MVector3(-500.0f, 500.0f, -500.0f),
            MVector3(-500.0f,500.0f, 500.0f), MVector3(500.0f,500.0f, 500.0f), position, -rotation, (MVector2*) &texCoords);

    texCoords[0] = MVector2(1, 1);
	texCoords[1] = MVector2(0, 1);
	texCoords[3] = MVector2(0, 0);
	texCoords[2] = MVector2(1, 0);

    // hinten 4
	render->bindTexture(m_SkyboxTexture[2].m_TexID);
    DrawQuad(MVector3(500.0f, -500.0f, -500.0f), MVector3(-500.0f, -500.0f, -500.0f),
            MVector3(-500.0f,-500.0f, 500.0f), MVector3(500.0f,-500.0f, 500.0f), position, -rotation, (MVector2*) &texCoords);

    texCoords[0] = MVector2(0, 0);
	texCoords[1] = MVector2(0, 1);
	texCoords[3] = MVector2(1, 1);
	texCoords[2] = MVector2(1, 0);

	// links 5
	render->bindTexture(m_SkyboxTexture[3].m_TexID);
    DrawQuad(MVector3(-500.0f, 500.0f, 500.0f), MVector3(-500.0f, 500.0f, -500.0f),
            MVector3(-500.0f,-500.0f, -500.0f), MVector3(-500.0f,-500.0f, 500.0f), position, -rotation, (MVector2*) &texCoords);


    texCoords[3] = MVector2(0, 1);
	texCoords[2] = MVector2(0, 0);
	texCoords[1] = MVector2(1, 1);
	texCoords[0] = MVector2(1, 0);

    // rechts 6
	render->bindTexture(m_SkyboxTexture[0].m_TexID);
    DrawQuad(MVector3(500.0f, 500.0f, 500.0f), MVector3(500.0f, 500.0f, -500.0f),
            MVector3(500.0f,-500.0f, -500.0f), MVector3(500.0f,-500.0f, 500.0f), position, -rotation, (MVector2*) &texCoords);

    m_SkyboxShader->Clear();
}
