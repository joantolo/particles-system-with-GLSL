#ifndef __IG_AUXILIAR__
#define __IG_AUXILIAR__

#include <vector>
#include <glm/glm.hpp>
#include <stdio.h>

//Caraga una textura y devuelve un puntero a su ubicacion en mememoria principal
//también devuelve el tamaño de la textura (w,h)
//!!Ya implementada
unsigned char *loadTexture(const char* fileName, unsigned int &w, unsigned int &h);

//Carga un fichero en una cadena de caracteres
char *loadStringFromFile(const char *fileName, unsigned int &fileLen);

//Carga un objeto .obj
bool loadOBJ(const char* path, std::vector <glm::vec3 >& out_vertices, std::vector <glm::vec2>& out_uvs, std::vector <glm::vec3>& out_normals, std::vector<unsigned int>& out_index);

glm::vec3 getVertex3FromString(FILE* file);

glm::vec2 getVertex2FromString(FILE* file);

#endif