#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"

#define SHADOWSIZE 2048

const int STARS_NUM = 3200;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();

	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(0.0f, 0.0f, Vector3(0.0f, 10000.0f, -500)); 

	sunPosition = Vector3(-30, 10000, -540);

	planetPosition = Vector3(0, 10000, -544);

	alphaFactor = 0.0f;

	light = new Light(sunPosition, Vector4(1, 1, 1, 1), 100000.0f);

	myPlanet = Mesh::LoadFromMeshFile("Sphere.msh");

	sun = Mesh::LoadFromMeshFile("Sphere.msh");

	cylinder = Mesh::LoadFromMeshFile("Cylinder.msh");
	cube = Mesh::LoadFromMeshFile("OffsetCube.msh");
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");

	spaceShip = Mesh::LoadFromMeshFile("SF_Free-Fighter.msh");

	character = Mesh::LoadFromMeshFile("Role_T.msh");
	characterAnim = new MeshAnimation("Role_T.anm");
	characterMat = new MeshMaterial("Role_T.mat");


	if (!myPlanet || !sun || !spaceShip) {
		std::cout << "mesh load wrong!!";
		return;
	}
	sceneShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
	sunShader = new Shader("sunVertex.glsl", "sunFragment.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	spaceShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	starShader = new Shader("sceneVertex.glsl", "sceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	heightMapShader = new Shader("BumpVertex.glsl", "BumpFragment.glsl");

	characterShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess()||!sunShader->LoadSuccess() || !spaceShader->LoadSuccess() || !starShader->LoadSuccess() || !reflectShader->LoadSuccess() || !heightMapShader->LoadSuccess()) {
		std::cout << "shader load wrong!!";
		return;
	}

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	myPlanetTex = SOIL_load_OGL_texture(TEXTUREDIR"2k_jupiter.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sunTex = SOIL_load_OGL_texture(TEXTUREDIR"2k_sun.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	satelliteTex = SOIL_load_OGL_texture(TEXTUREDIR"2k_ceres_fictional.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	spaceShipTex = SOIL_load_OGL_texture(TEXTUREDIR"SF_Fighter-Albedo.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	spaceBox = SOIL_load_OGL_cubemap(TEXTUREDIR"space.png", TEXTUREDIR"space.png", TEXTUREDIR"space.png", TEXTUREDIR"space.png", TEXTUREDIR"space.png", TEXTUREDIR"space.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	
	if (!sunTex || !myPlanetTex || !sceneBump || !satelliteTex || !waterTex || !earthTex || !earthBump || !spaceBox || !spaceShipTex) {
		std::cout << "textures load wrong !!!";
		return;
	}

	rootStar = new SceneNode();



	satellites.resize(3);
	satellites[0] = cylinder;
	satellites[1] = cube;
	satellites[2] = sphere;

	satelliteTransforms.resize(3);
	
	for (int i = 0; i < STARS_NUM + STARS_NUM / 100; i++) {
		SceneNode* s = new SceneNode();
		
		if (i % 2 == 0) {
			s->SetMesh(sphere);
		}
		else if (i % 3 == 0 || i % 7 == 0) {
			s->SetMesh(cylinder);
		}
		else {
			s->SetMesh(cube);
		}

		s->SetColour(Vector4(0.5f + (float)(rand() / (float)RAND_MAX), 0.5f + (float)(rand() / (float)RAND_MAX), 0.5f + (float)(rand() / (float)RAND_MAX), 1));

		if(i >= STARS_NUM){
			Vector3 starPosition = Vector3(rand() % 60 - 40, rand() % 33 + 9987, -(rand() % 20) - 560);
			s->SetTransform(Matrix4::Translation(starPosition));
			s->SetModelScale(Vector3(0.4, 0.4, 0.4));
		}
		else {
			Vector3 starPosition = Vector3(rand() % (int)heightmapSize.x - 50, rand() % (int)(heightmapSize.y * 2) + (int)heightmapSize.y * 2, rand() % (int)heightmapSize.z - 50);
			s->SetTransform(Matrix4::Translation(starPosition));
			s->SetModelScale(Vector3(3, 3, 3));
		}
		rootStar->AddChild(s);

	}


	for (int i = 0; i < character->GetSubMeshCount(); i++) {
		const MeshMaterialEntry* matEntry = characterMat->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}

	characterTransform = Matrix4::Translation(Vector3(heightmapSize * Vector3(0.46, 0.8, 0.4))) * Matrix4::Rotation(180, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(50, 50, 50));

	SetTextureRepeating(myPlanetTex, true);
	SetTextureRepeating(sceneBump, true);
	SetTextureRepeating(sunTex, true);
	SetTextureRepeating(satelliteTex, true);
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(spaceShipTex, true);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	rotationDegree = 20;
	selfRotation = Matrix4::Rotation(rotationDegree, Vector3(1, 0, 0));
	waterRotation = 0.0f;
	waterCycle = 0.0f;
	sceneTime = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	isOuterSpace = true;
	enableFreeCamera = true;
	init = true;
}


Renderer::~Renderer(void) {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	delete light;
	delete camera;
	delete sun;
	delete myPlanet;
	delete sceneShader;
	delete shadowShader;
	delete sunShader;
	delete spaceShader;
	delete starShader;
	delete quad;
	delete heightMap;
	delete reflectShader;
	delete heightMapShader;
	delete spaceShip;
	delete character;
	delete characterMat;
	delete characterAnim;
	delete characterShader;
	delete rootStar;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	sceneTime += dt;
	waterRotation += dt * 2.0f;
	waterCycle += dt * 0.25f;
	Vector3 spaceShipPosition = Vector3(heightmapSize.x * 0.5, heightmapSize.y + 30, heightmapSize.z * 0.3);

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % characterAnim->GetFrameCount();
		frameTime += 1.0f / characterAnim->GetFrameRate();
	}
	
	Matrix4 planetScale = Matrix4::Scale(Vector3(2, 2, 2));

	if (((planetPosition.x + 5 > camera->GetPosition().x) && (camera->GetPosition().x > planetPosition.x - 5)) && ((planetPosition.y + 5 > camera->GetPosition().y) && (camera->GetPosition().y > planetPosition.y - 5)) && ((planetPosition.z + 5 > camera->GetPosition().z) && (camera->GetPosition().z > planetPosition.z - 5))) {
		isOuterSpace = false;
		camera->SetPosition(heightmapSize * Vector3(0.5f, 1.5f, 0.5f));
		camera->SetPitch(0.0f);
		camera->SetYaw(0.0f);
	}
	if (((spaceShipPosition.x + 700 > camera->GetPosition().x) && (camera->GetPosition().x > spaceShipPosition.x - 700)) && ((spaceShipPosition.y +700 > camera->GetPosition().y) && (camera->GetPosition().y > spaceShipPosition.y - 700)) && ((spaceShipPosition.z + 700 > camera->GetPosition().z) && (camera->GetPosition().z > spaceShipPosition.z - 700))) {
		isOuterSpace = true;
		camera->SetPosition(Vector3(0.0f, 10000.0f, -500.0f));
		camera->SetPitch(0.0f);
		camera->SetYaw(0.0f);
	}
	if (isOuterSpace == true) {
		alphaFactor = 0.0f;
		sunPosition = Vector3(-30, 10000, -540);
		light->SetPosition(sunPosition);
		Matrix4 sunScale = Matrix4::Scale(Vector3(5, 5, 5));
		selfRotation = Matrix4::Rotation(rotationDegree, Vector3(1, 0, 0)) * Matrix4::Rotation(rotationDegree + sceneTime * 20, Vector3(0, 1, 0));
		planetTransform = Matrix4::Translation(planetPosition) * planetScale * selfRotation;
		sunTransform = Matrix4::Translation(sunPosition) * Matrix4::Scale(Vector3(5, 5, 5)) * selfRotation;

		satelliteTransforms[0] = Matrix4::Translation(planetPosition) * Matrix4::Rotation(sceneTime * 30, Vector3(0, 0, 1)) * Matrix4::Translation(Vector3(-8.0f, -5.0f, -5.0f)) * Matrix4::Scale(Vector3(0.9, 0.9, 0.9));
		satelliteTransforms[1] = Matrix4::Translation(planetPosition) * Matrix4::Rotation(sceneTime * 20, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(10.0f, -10.0f, -5.0f)) * Matrix4::Scale(Vector3(1.0, 1.0, 1.0));
		satelliteTransforms[2] = Matrix4::Translation(planetPosition) * Matrix4::Rotation(sceneTime * 40, Vector3(1, 0, 0)) * Matrix4::Translation(Vector3(8.0f, 5.0f, 5.0f)) * Matrix4::Scale(Vector3(1.2, 1.2, 1.2));

		spaceShipTransform = Matrix4::Translation(camera->GetPosition()) * Matrix4::Rotation(camera->GetYaw(), Vector3(0, 1, 0)) * Matrix4::Rotation(camera->GetPitch(), Vector3(1, 0, 0)) * Matrix4::Translation(Vector3(-5, -5, -20)) * Matrix4::Scale(Vector3(2, 2, 2));
	}
	if (isOuterSpace == false) {
		sunPosition = Vector3(heightmapSize.x + 1, heightmapSize.y * 5.0f, heightmapSize.z + 1);
		alphaFactor = 1.0f;
		light->SetRadius(100000.0f);
		sunTransform = Matrix4::Translation(Vector3(heightmapSize.x * 0.5, heightmapSize.y * 1.6, heightmapSize.z * 0.5)) * Matrix4::Rotation(30 * sceneTime, Vector3(1, 0, 0)) * Matrix4::Translation(sunPosition) * selfRotation * Matrix4::Scale(Vector3(500, 500, 500));
		Vector3 lightNewPos = Vector3((sunTransform * Vector4(1, 1, 1, 1)).x, (sunTransform * Vector4(1, 1, 1, 1)).y, (sunTransform * Vector4(1, 1, 1, 1)).z);
		light->SetPosition(lightNewPos);
		spaceShipTransform = Matrix4::Translation(spaceShipPosition) * Matrix4::Scale(Vector3(500, 500, 500));
	}

	rootStar->Update(dt);
}


void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSpace();
	DrawStars(rootStar);
	DrawSun();
	DrawShadowScene();
	DrawMainScene();
	DrawHeightmap();
	DrawWater();
	DrawCharacter();
}



void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);

	//viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	modelMatrix = planetTransform;
	UpdateShaderMatrices();
	myPlanet->Draw();

	for (int i = 0; i < 3; i++) {
		modelMatrix = satelliteTransforms[1];
		UpdateShaderMatrices();
		satellites[i]->Draw();
	}

	modelMatrix = spaceShipTransform;
	UpdateShaderMatrices();
	spaceShip->Draw();

	modelMatrix = characterTransform;
	UpdateShaderMatrices();
	character->Draw();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMainScene() {
	BindShader(sceneShader);
	SetShaderLight(*light);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, myPlanetTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowFBO);

	modelMatrix = planetTransform;
	UpdateShaderMatrices();
	myPlanet->Draw();


	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 3);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, satelliteTex);

	for (int i = 0; i < 3; i++) {
		modelMatrix = satelliteTransforms[i];
		UpdateShaderMatrices();
		satellites[i]->Draw();
	}

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 4);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, spaceShipTex);

	modelMatrix = spaceShipTransform;

	UpdateShaderMatrices();
	spaceShip->Draw();
}

void Renderer::DrawSun() {
	BindShader(sunShader);
	SetShaderLight(*light);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(sunShader->GetProgram(), "diffuseTex"), 3);

	glUniform3fv(glGetUniformLocation(sunShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sunTex);

	modelMatrix = sunTransform;

	UpdateShaderMatrices();
	sun->Draw();
}

void Renderer::DrawSpace() {
	glDepthMask(GL_FALSE);
	BindShader(spaceShader);
	SetShaderLight(*light);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 1500.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(spaceShader->GetProgram(), "cubeTex"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, spaceBox);

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(heightMapShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "bumpTex"), 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1f(glGetUniformLocation(heightMapShader->GetProgram(), "alphaFactor"), alphaFactor);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);


	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "alphaFactor"), alphaFactor);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, spaceBox);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotation, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}

void Renderer::DrawStars(SceneNode *n) {
	BindShader(starShader);
	UpdateShaderMatrices();

	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale() * (0.5 * (1+sin(sceneTime))));

		glUniformMatrix4fv(glGetUniformLocation(starShader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(starShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		n->Draw(*this);
	}
	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i) {
		DrawStars(*i);
	}
}

void Renderer::DrawCharacter() {
	BindShader(characterShader);
	glUniform1i(glGetUniformLocation(characterShader->GetProgram(), "diffuseTex"), 0);

	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = character->GetInverseBindPose();
	const Matrix4* frameData = characterAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < character->GetJointCount(); i++) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(characterShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	glUniform1f(glGetUniformLocation(characterShader->GetProgram(), "alphaFactor"), alphaFactor);

	modelMatrix = characterTransform;

	for (int i = 0; i < character->GetSubMeshCount(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		UpdateShaderMatrices();
		character->DrawSubMesh(i);
	}
}