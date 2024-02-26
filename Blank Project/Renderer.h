#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Frustum.h"

class Camera;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class SceneNode;

class Renderer :public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawShadowScene();
	void DrawMainScene();
	void DrawSun();
	void DrawSpace();
	void DrawStars(SceneNode *n);
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawCharacter();
	
	GLuint shadowTex;
	GLuint shadowFBO;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	GLuint myPlanetTex;
	GLuint sceneBump;
	GLuint sunTex;
	GLuint spaceBox;

	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint satelliteTex;
	GLuint spaceShipTex;
	vector<GLuint> matTextures;

	float sceneTime;

	Shader* sceneShader;
	Shader* shadowShader;
	Shader* sunShader;
	Shader* spaceShader;
	Shader* starShader;
	Shader* heightMapShader;
	Shader* reflectShader;
	Shader* characterShader;

	SceneNode* rootStar;

	HeightMap* heightMap;

	Mesh* sun;
	Mesh* myPlanet;
	Mesh* quad;
	Mesh* cylinder;
	Mesh* cube;
	Mesh* sphere;
	Mesh* spaceShip;
	Mesh* character;

	MeshMaterial* spaceShipMat;

	MeshAnimation* characterAnim;
	MeshMaterial* characterMat;
	vector<Mesh*> satellites;

	Matrix4 selfRotation;
	int currentFrame;
	float frameTime;
	float rotationDegree;
	float waterRotation;
	float waterCycle;

	float alphaFactor;

	Matrix4 sunTransform;
	Matrix4 planetTransform;
	Matrix4 spaceShipTransform;
	Matrix4 characterTransform;

	Matrix4 cameraMatrix;

	Vector3 heightmapSize;
	vector<Matrix4> satelliteTransforms;

	Camera* camera;
	Light* light;
	Vector3 sunPosition;
	Vector3 planetPosition;

	bool isOuterSpace;
	bool enableFreeCamera;
};