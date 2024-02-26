#pragma once
#include  <string>
#include "mesh.h"

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& name);
	~HeightMap(void) {};

	Vector3 GetHeightmapSize() const { return heightmapSize; }
	int GetHerightmapYCoord(float x, float z) { return vertices[(int)((z / vertexScale.z) * iwidth) + (int)(x / vertexScale.x)].y; }

protected:
	Vector3 heightmapSize;
	Vector3 vertexScale;
	float iwidth;
};