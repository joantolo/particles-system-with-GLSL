#include "BOX.h"

#include <windows.h>

#include <fstream>

//Carga de texturas
#include <FreeImage.h>
#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <string>
#include <algorithm>

//////////////////////////////////////////
// Funciones auxiliares ya implementadas

//Funciones para la carga de los shaders
char *loadStringFromFile(const char *fileName, unsigned int &fileLen)
{
	//Se carga el fichero
	std::ifstream file;
	file.open(fileName, std::ios::in);
	if (!file) return 0;

	//Se calcula la longitud del fichero
	file.seekg(0, std::ios::end);
	fileLen = unsigned int(file.tellg());
	file.seekg(std::ios::beg);

	//Se lee el fichero
	char *source = new char[fileLen + 1];

	int i = 0;
	while (file.good())
	{
		source[i] = char(file.get());
		if (!file.eof()) i++;
		else fileLen = i;
	}
	source[fileLen] = '\0';
	file.close();

	return source;
}

unsigned char *loadTexture(const char* fileName, unsigned int &w, unsigned int &h)
{
	FreeImage_Initialise(TRUE);

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);
	if (format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(fileName);
	if ((format == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(format))
		return NULL;

	FIBITMAP* img = FreeImage_Load(format, fileName);
	if (img == NULL)
		return NULL;

	FIBITMAP* tempImg = img;
	img = FreeImage_ConvertTo32Bits(img);
	FreeImage_Unload(tempImg);

	w = FreeImage_GetWidth(img);
	h = FreeImage_GetHeight(img);

	//BGRA a RGBA
	unsigned char * map = new unsigned char[4 * w*h];
	char *buff = (char*)FreeImage_GetBits(img);

	for (unsigned int j = 0; j<w*h; j++){
		map[j * 4 + 0] = buff[j * 4 + 2];
		map[j * 4 + 1] = buff[j * 4 + 1];
		map[j * 4 + 2] = buff[j * 4 + 0];
		map[j * 4 + 3] = buff[j * 4 + 3];
	}

	FreeImage_Unload(img);
	FreeImage_DeInitialise();

	return map;
}


glm::vec3 getVertex3FromString(FILE* file)
{
	char strX[32];
	char strY[32];
	char strZ[32];
	std::string aux;
	float x, y, z;

	fscanf(file, "%s %s %s\n", strX, strY, strZ);

	aux = strX;
	std::replace(aux.begin(), aux.end(), '.', ',');
	x = std::stof(aux);

	aux = strY;
	std::replace(aux.begin(), aux.end(), '.', ',');
	y = std::stof(aux);

	aux = strZ;	
	std::replace(aux.begin(), aux.end(), '.', ',');
	z = std::stof(aux);

	return glm::vec3(x, y, z);
}

glm::vec2 getVertex2FromString(FILE* file)
{
	char strS[16];
	char strT[16];
	std::string aux;
	double t, s;

	fscanf(file, "%s %s\n", strS, strT);
	
	aux = strS;
	std::replace(aux.begin(), aux.end(), '.', ',');
	s = std::stod(aux);

	aux = strT;
	std::replace(aux.begin(), aux.end(), '.', ',');
	t = std::stod(aux);

	return glm::vec2(s, t);
}

//
// Function obtained with the help from:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
//
bool loadOBJ(const char* path, std::vector <glm::vec3 >& out_vertexes, std::vector <glm::vec2>& out_uvs, std::vector <glm::vec3>& out_normals, std::vector<unsigned int>& out_index)
{
	std::vector< unsigned int > vertexIndex, uvIndex, normalIndex;
	std::vector< glm::vec3 > temp_vertexes;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	FILE* file = fopen(path, "r");
	if (file == NULL) 
	{
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// Lee la primera palabra de la línea
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File, es decir, el final del archivo. Se finaliza el ciclo.

		// else : analizar el lineHeader
		if (strcmp(lineHeader, "v") == 0)
		{
			temp_vertexes.push_back(getVertex3FromString(file));
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			temp_uvs.push_back(getVertex2FromString(file));
		}
		else if (strcmp(lineHeader, "vn") == 0) 
		{
			temp_normals.push_back(getVertex3FromString(file));
		}
		else if (strcmp(lineHeader, "f") == 0) 
		{
			unsigned int temp_vertexIndex[3], temp_uvIndex[3], temp_normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &temp_vertexIndex[0], &temp_uvIndex[0], &temp_normalIndex[0], &temp_vertexIndex[1], &temp_uvIndex[1], &temp_normalIndex[1], &temp_vertexIndex[2], &temp_uvIndex[2], &temp_normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}

			vertexIndex.push_back(temp_vertexIndex[0]);
			vertexIndex.push_back(temp_vertexIndex[1]);
			vertexIndex.push_back(temp_vertexIndex[2]);
			uvIndex.push_back(temp_uvIndex[0]);
			uvIndex.push_back(temp_uvIndex[1]);
			uvIndex.push_back(temp_uvIndex[2]);
			normalIndex.push_back(temp_normalIndex[0]);
			normalIndex.push_back(temp_normalIndex[1]);
			normalIndex.push_back(temp_normalIndex[2]);
		}
	}

	// Para cada vértice de cada triángulo
	for (unsigned int i = 0; i < vertexIndex.size(); i++) 
	{
		unsigned int index = vertexIndex[i];
		glm::vec3 vertex = temp_vertexes[index - 1];
		out_vertexes.push_back(vertex);
	}

	for (unsigned int i = 0; i < uvIndex.size(); i++)
	{
		unsigned int index = uvIndex[i];
		glm::vec2 uv = temp_uvs[index - 1];
		out_uvs.push_back(uv);
	}

	for (unsigned int i = 0; i < normalIndex.size(); i++)
	{
		unsigned int index = normalIndex[i];
		glm::vec3 normal = temp_normals[index - 1];
		out_normals.push_back(normal);
	}

	for (unsigned int i = 0; i < vertexIndex.size(); i++)
	{
		out_index.push_back(i);
	}
	
	return true;
}
