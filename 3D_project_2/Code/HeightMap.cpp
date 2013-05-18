#include "HeightMap.h"

HeightMap::HeightMap()
{
	width = 0;
	height = 0;

	heightMapData = NULL;
}

HeightMap::HeightMap(int w, int h)
{
	width = w;
	height = h;

	heightMapData = new float*[width];
	for(int i = 0; i < width; i++)
		heightMapData[i] = new float[height];
}

HeightMap::~HeightMap()
{
	for(int i = 0; i < width; i++)
		delete[] heightMapData[i];
	delete[] heightMapData;
}

//reads rawdata and creates the heightmap
bool HeightMap::loadRaw(int w, int h, string filename, float heightScale, float heightOffset)
{
	vector<unsigned char> rawData(w*h);

	ifstream fin(filename.c_str(), ios_base::binary);
	if(!fin)
		return false;

	fin.read((char*)&rawData[0], (streamsize)rawData.size());
	fin.close();

	width = w;
	height = h;

	heightMapData = new float*[width];
	for(int i = 0; i < width; i++)
		heightMapData[i] = new float[height];

	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
			int k = i * h + j;
			heightMapData[i][j] = (float) rawData[k] * heightScale + heightOffset;
		}
	}

	filter3x3();

	return true;
}

void HeightMap::filter3x3()
{
	float **tempHeightMap = new float*[width];
	for(int i = 0; i < width; i++)
		tempHeightMap[i] = new float[height];

	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
			tempHeightMap[i][j] = sampleHeight3x3(i, j);
	}

	for(int i = 0; i < width; i++)
		delete[] heightMapData[i];
	delete[] heightMapData;

	heightMapData = tempHeightMap;
}

float HeightMap::sampleHeight3x3(int i, int j)
{
	float average = 0.0f;
	float sample = 0.0f;

	for(int m = i - 1; m <= i+1; m++)
	{
		for(int n = j - 1; n <= j+1; n++)
		{
			if(inBounds(m, n))
			{
				average += heightMapData[m][n];
				sample += 1.0f;
			}
		}
	}

	return average/sample;
}

 bool HeightMap::inBounds(int x, int y)
 {
	 if(x >= 0 && x < width && y >= 0 && y < height)
		 return true;

	 return false;
 }

float HeightMap::getData(int x, int z)
{
	return heightMapData[x][z];
}