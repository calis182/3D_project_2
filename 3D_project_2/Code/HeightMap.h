#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <string>
#include <vector>
#include <fstream>
using namespace std;

class HeightMap
{
public:
	HeightMap();
	HeightMap(int w, int h);
	~HeightMap();

	bool loadRaw(int w, int h, string filename, float heightScale, float heightOffset);
	void filter3x3();
	float sampleHeight3x3(int i, int j);
	bool inBounds(int x, int y);

	int getWidth() { return width; }
	int getHeight() { return height; }

	float getData(int x, int z);

	float** getHeightMapData() { return heightMapData; }

private:
	float **heightMapData;
	int width, height;

};

#endif