#include "Satellites.h"

Satellites::Satellites(Mesh* cube) {

	cube = Mesh::LoadFromMeshFile("OffsetCube.msh");

	surface = new HeightMap(TEXTUREDIR"noise.png");

	texture = SOIL_load_OGL_texture(TEXTUREDIR"2k_ceres_fictional.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	Vector3 surfaceSize = surface->GetHeightmapSize();

	SatelliteQ = new SceneNode(cube);
	SatelliteQ->SetTransform(Matrix4::Translation(surfaceSize * Vector3(-0.34f, 6.0f, -0.01f)));
	SatelliteQ->SetModelScale(Vector3(50.0f, 50.0f, 50.0f));
	SatelliteQ->SetTexture(texture);
	SatelliteQ->AddChild(SatelliteQ);

	SatelliteW = new SceneNode(cube);
	SatelliteW->SetTransform(Matrix4::Translation(surfaceSize * Vector3(-0.5f, 6.0f, -0.02f)));
	SatelliteW->SetModelScale(Vector3(50.0f, 50.0f, 50.0f));
	SatelliteW->SetTexture(texture);
	SatelliteW->AddChild(SatelliteW);

	SatelliteE = new SceneNode(cube);
	SatelliteE->SetTransform(Matrix4::Translation(surfaceSize * Vector3(-0.02f, 7.0f, 0.0006f)));
	SatelliteE->SetModelScale(Vector3(60.0f, 60.0f, 60.0f));
	SatelliteE->SetTexture(texture);
	SatelliteE->AddChild(SatelliteE);

}

void Satellites::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0.8f, 0.6f, 0.0f));

	SatelliteQ->SetTransform(GetTransform() * Matrix4::Rotation(dt * 20, Vector3(1, 1, 0)));
	SatelliteW->SetTransform(GetTransform() * Matrix4::Rotation(dt * 20, Vector3(0, 1, 0)));
	SatelliteE->SetTransform(GetTransform() * Matrix4::Rotation(dt * 20, Vector3(1, 0, 1)));


	SceneNode::Update(dt);
}