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

#define RAND_SEED 31415926
#define M_2PI 6.28318530718
#define M_PI 3.14159265359
#define SCREEN_SIZE 800,800

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//VAO
unsigned int vao;
unsigned int posSSbo;
unsigned int velSSbo;
unsigned int colorSSbo;

///////////
//Compute
///////////
unsigned int computeShader;
unsigned int sortingShader;
unsigned int computeProgram;
unsigned int sortingComputeProgram;

//Uniforms
int locStage;
int locSubStage;
int uSortingModelViewMat;

const unsigned int nParticles = 1048576; //2^20
const unsigned int workGroupSize = 256;

///////////
//Forward-rendering
///////////
unsigned int forwardVShader;
unsigned int forwardGShader;
unsigned int forwardFShader;
unsigned int forwardProgram;

//Atributos
int inPos;
int inColor;

//Matrices Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uProjMat;

//Identificadores de texturas Forward-rendering
unsigned int colorTexId;

//Texturas Uniform
int uColorTex;

//Posicion de la luz
int uLightPos;

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 modelObject = glm::mat4(1.0f);
glm::mat4 modelLight = glm::mat4(1.0f);

//Control de camara
float phi = 0.0f;
bool moveCam;
int cameraStartingDistance = 5.0f;
float cameraStep = 3.0f;

//Variables near y far
float projNear, projFar;

//Posicion de la luz
glm::vec4 lightPos;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void renderPraticles();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void intShaderSortingCompute(const char* cname);
void initShaderCompute(const char* cname);
void initShaderFw(const char* vname, const char* gname, const char* fname);
void initParticles(const char* filename);
float ranf(float, float);
void destroy();

//Carga el shader indicado, devuele el ID del shader
GLuint loadShader(const char* fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL,
//y devuelve el identificador de la textura
unsigned int loadTex(const char* fileName);

int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();

	intShaderSortingCompute("../shaders/sortingShader.comp");
	initShaderCompute("../shaders/particleIntegrator.comp");
	initShaderFw("../shaders/fwRendering.vert", "../shaders/fwRendering.geom", "../shaders/fwRendering.frag");

	initParticles("../models/box.obj");

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Prácticas PGATR");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	//Inicializamos matriz de proyeccion
	projNear = 1.0f;
	projFar = 400.0f;
	proj = glm::perspective(glm::radians(60.0f), 1.0f, projNear, projFar);

	//Establecemos la posición de la luz
	lightPos = glm::vec4(0, 30, 0, 1);
	modelLight = glm::mat4(1);

	//Inicializamos la cámara
	view = glm::lookAt(glm::vec3(0, 0, cameraStartingDistance), glm::vec3(0), glm::vec3(0, 1, 0));
	moveCam = false;
}

void destroy()
{
	glDetachShader(forwardProgram, forwardVShader);
	glDetachShader(forwardProgram, forwardFShader);
	glDetachShader(forwardProgram, forwardGShader);
	glDeleteShader(forwardVShader);
	glDeleteShader(forwardFShader);
	glDeleteShader(forwardGShader);
	glDeleteProgram(forwardProgram);

	glDetachShader(computeProgram, computeShader);
	glDeleteShader(computeShader);

	glDetachShader(sortingComputeProgram, sortingShader);
	glDeleteShader(sortingShader);

	if (inPos != -1) glDeleteBuffers(1, &posSSbo);
	if (inColor != -1) glDeleteBuffers(1, &colorSSbo);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
}

void intShaderSortingCompute(const char* cname)
{
	sortingShader = loadShader(cname, GL_COMPUTE_SHADER);

	sortingComputeProgram = glCreateProgram();
	glAttachShader(sortingComputeProgram, sortingShader);

	glLinkProgram(sortingComputeProgram);

	int linked;
	glGetProgramiv(sortingComputeProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(sortingComputeProgram, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetProgramInfoLog(sortingComputeProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(sortingComputeProgram);
		sortingComputeProgram = 0;
		exit(-1);
	}

	locStage = glGetUniformLocation(sortingComputeProgram, "stage");
	locSubStage = glGetUniformLocation(sortingComputeProgram, "subStage");
	uSortingModelViewMat = glGetUniformLocation(sortingComputeProgram, "modelView");
}

void initShaderCompute(const char* cname)
{
	computeShader = loadShader(cname, GL_COMPUTE_SHADER);

	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);

	glLinkProgram(computeProgram);

	int linked;
	glGetProgramiv(computeProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetProgramInfoLog(computeProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(computeProgram);
		computeProgram = 0;
		exit(-1);
	}
}

void initShaderFw(const char* vname, const char* gname, const char* fname)
{
	forwardVShader = loadShader(vname, GL_VERTEX_SHADER);
	forwardGShader = loadShader(gname, GL_GEOMETRY_SHADER);
	forwardFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	forwardProgram = glCreateProgram();
	glAttachShader(forwardProgram, forwardVShader);
	glAttachShader(forwardProgram, forwardGShader);
	glAttachShader(forwardProgram, forwardFShader);

	glLinkProgram(forwardProgram);

	int linked;
	glGetProgramiv(forwardProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(forwardProgram, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetProgramInfoLog(forwardProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(forwardProgram);
		forwardProgram = 0;
		exit(-1);
	}

	uProjMat = glGetUniformLocation(forwardProgram, "proj");
	uNormalMat = glGetUniformLocation(forwardProgram, "normal");
	uModelViewMat = glGetUniformLocation(forwardProgram, "modelView");
	uModelViewProjMat = glGetUniformLocation(forwardProgram, "modelViewProj");
	uLightPos = glGetUniformLocation(forwardProgram, "lpos");

	uColorTex = glGetUniformLocation(forwardProgram, "colorTex");

	inPos = glGetAttribLocation(forwardProgram, "inPos");
	inColor = glGetAttribLocation(forwardProgram, "inColor");
}

void initParticles(const char* filename)
{
	std::vector< glm::vec4 > positions(nParticles, glm::vec4(0));
	std::vector< glm::vec4 > velocities(nParticles, glm::vec4(0));
	std::vector< glm::vec4 > colors(nParticles, glm::vec4(0));

	for (int i = 0; i < nParticles; i++)
	{
		positions[i] = glm::vec4(ranf(-2, 2), ranf(0, 2), ranf(-2, 2), 1);//positions[i] = glm::vec4(ranf(-2, 2), ranf(0, 2), ranf(-2, 2), 1);
		//positions[i] = glm::vec4(-3 + ((float)i / nParticles) * 4.0f, 1.0f, -3 + ((float)i / nParticles) * 4.0f, 1);//positions[i] = glm::vec4(ranf(-2, 2), ranf(0, 2), ranf(-2, 2), 1);
		velocities[i] = glm::vec4(ranf(-2, 2), ranf(-2, 2), ranf(-2, 2), 0);
		colors[i] = glm::vec4(ranf(0, 1), ranf(0, 1), ranf(0, 1), 0.6);
	}

	/////////////////////
	//Compute
	/////////////////////
	glGenBuffers(1, &posSSbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles *
		sizeof(glm::vec4), &positions[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSbo);

	glGenBuffers(1, &velSSbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles *
		sizeof(glm::vec4), &velocities[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSbo);

	glGenBuffers(1, &colorSSbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, nParticles *
		sizeof(glm::vec4), &colors[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorSSbo);

	//////////////////////
	// Forward
	/////////////////////
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
		glVertexAttribPointer(inPos, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, colorSSbo);
		glVertexAttribPointer(inColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	glBindVertexArray(0);

	modelObject = glm::mat4(1.0f);

	colorTexId = loadTex("../img/map_tex.png");
}

GLuint loadShader(const char* fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char* fileName)
{
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	///////////
	//Compute-rendering
	///////////
	glUseProgram(sortingComputeProgram);

	if (uSortingModelViewMat != -1) glUniformMatrix4fv(uSortingModelViewMat, 1, GL_FALSE, &((view * modelObject)[0][0]));

	int stages = log2(nParticles);
	for (int stage = 0; stage < stages; ++stage)
	{
		if (locStage != -1)
			glUniform1i(locStage, stage);

		for (int subStage = 0; subStage < stage + 1; ++subStage)
		{
			if (locSubStage != -1)
				glUniform1i(locSubStage, subStage);

			glDispatchCompute(nParticles / workGroupSize, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}

	glUseProgram(computeProgram);
	glDispatchCompute(nParticles / workGroupSize, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	///////////
	//Forward-rendering
	///////////
	glUseProgram(forwardProgram);

	//Texturas del forward-rendering
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uLightPos != -1)
	{
		glm::vec3 lpos = (view * modelLight) * lightPos;
		glUniform3fv(uLightPos, 1, &lpos[0]);
	}

	//Dibujado de objeto
	renderPraticles();

	glutSwapBuffers();
}

void renderPraticles()
{
	glm::mat4 modelView = view * modelObject;
	glm::mat4 modelViewProj = proj * view * modelObject;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uProjMat != -1)
		glUniformMatrix4fv(uProjMat, 1, GL_FALSE, &(proj[0][0]));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));

	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));

	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, nParticles);
	glBindVertexArray(0);
}

void resizeFunc(int width, int height)
{
	//Mantenimiento del aspect ratio
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), projNear, projFar);

	glutPostRedisplay();
}

void idleFunc()
{
	if (moveCam)
	{
		if (phi < M_2PI)
		{
			phi += 0.02f;
			view = glm::rotate(view, 0.02f, glm::vec3(0, 1, 0));
		}
		else
		{
			phi = 0.0f;
		}
	}

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	glm::vec3 dir;
	switch (key)
	{
	case(' '):
		moveCam = !moveCam;
		break;
	case('a'):
	case('A'):
		dir = -glm::normalize(view[3]) * cameraStep;
		view[3].x += dir.x;
		view[3].y += dir.y;
		view[3].z += dir.z;
		break;
	case('s'):
	case('S'):
		dir = glm::normalize(view[3]) * cameraStep;
		view[3].x += dir.x;
		view[3].y += dir.y;
		view[3].z += dir.z;
		break;
	case('q'):
	case('Q'):
		break;
	}
}

float ranf(float min, float max)
{
	return (min + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (max - min));
}

void mouseFunc(int button, int state, int x, int y)
{
}