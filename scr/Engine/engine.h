#pragma once
#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include "resources.h"

#define RAND_SEED 31415926
#define M_2PI 6.28318530718
#define M_PI 3.14159265359
#define SCREEN_SIZE 800,800

class engine
{
public:
	int Initialize(int argc, char** argv);
	void initContext(int argc, char** argv);
	void initOGL();
	static void renderObject();
	static void renderFunc();
	static void resizeFunc(int width, int height);
	static void idleFunc();
	static void keyboardFunc(unsigned char key, int x, int y);
	static void mouseFunc(int button, int state, int x, int y);

	static resources engResources;
	static baseShader currentShader;

	//Matrices
	static  glm::mat4 proj;
	static  glm::mat4 view;
	static  glm::mat4 modelLight;

	//Variable del modelo
	unsigned int nVertexIndex;

	//Control de camara
	static float theta;
	static float phi;
	static bool moveCam;

	//Variables near y far
	static float projNear, projFar;

	//Posicion de la luz
	glm::vec4 lightPos;

private:
	engine() {}
};
