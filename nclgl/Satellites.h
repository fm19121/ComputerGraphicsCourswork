#pragma once
#include "../nclgl/SceneNode.h"
#include "HeightMap.h"

class Satellites : public SceneNode {
public:
	//Satellites(Mesh* cylinder, Mesh* cube, Mesh* sphere, GLuint texture, Vector3 surfaceSize);
	Satellites(Mesh* cube);
	~Satellites(void) {}
	void Update(float dt) override;

protected:
	SceneNode* SatelliteQ;
	SceneNode* SatelliteW;
	SceneNode* SatelliteE;

	HeightMap* surface;

	GLuint texture;
};
