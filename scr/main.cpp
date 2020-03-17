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

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int colorVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

///////////
//Compute
///////////
unsigned int computeShader;
unsigned int computeProgram;

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
int inColor;
int inTexCoord;

//Matrices Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Identificadores de texturas Forward-rendering
unsigned int colorTexId;
unsigned int emiTexId;

//Texturas Uniform
int uColorTex;
int uEmiTex;
int uLightPos;

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 modelObject = glm::mat4(1.0f);
glm::mat4 modelLight = glm::mat4(1.0f);

//Variable del modelo
unsigned int nVertexIndex;

//Control de camara
float theta = 0.0f;
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
void renderObject();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderCompute(const char* cname);
void initShaderFw(const char* vname, const char* gname, const char* fname);
void initParticles(const char* filename);
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

	initShaderCompute("../shaders/particleIntegrator.comp");
	initShaderFw("../shaders/fwRendering.vert", "../shaders/computeRendering.geom", "../shaders/fwRendering.frag");

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

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);

	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
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
	forwardGShader = loadShader(vname, GL_GEOMETRY_SHADER);
	forwardFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	forwardProgram = glCreateProgram();
	glAttachShader(forwardProgram, forwardVShader);
	glAttachShader(forwardProgram, forwardGShader);
	glAttachShader(forwardProgram, forwardFShader);

	/*
	glBindAttribLocation(forwardProgram, 0, "inPos");
	glBindAttribLocation(forwardProgram, 1, "inColor");
	glBindAttribLocation(forwardProgram, 2, "inNormal");
	glBindAttribLocation(forwardProgram, 3, "inTexCoord");
	*/

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

	uNormalMat = glGetUniformLocation(forwardProgram, "normal");
	uModelViewMat = glGetUniformLocation(forwardProgram, "modelView");
	uModelViewProjMat = glGetUniformLocation(forwardProgram, "modelViewProj");
	uLightPos = glGetUniformLocation(forwardProgram, "lpos");

	uColorTex = glGetUniformLocation(forwardProgram, "colorTex");
	uEmiTex = glGetUniformLocation(forwardProgram, "emiTex");

	inPos = glGetAttribLocation(forwardProgram, "inPos");
	inColor = glGetAttribLocation(forwardProgram, "inNormal");
	inTexCoord = glGetAttribLocation(forwardProgram, "inTexCoord");
}

void initParticles(const char* filename)
{
	unsigned int nParticles = 20;

	std::vector< glm::vec4 > positions(nParticles, glm::vec4(0));
	std::vector< glm::vec4 > velocities(nParticles, glm::vec4(0));

	for (int i = 0; i < nParticles; i++)
	{
		positions[i] = glm::vec4(rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX, 1);
		velocities[i] = glm::vec4(rand() / RAND_MAX, rand() / RAND_MAX, rand() / RAND_MAX, 1);
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

	//////////////////////
	// Forward
	/////////////////////
	// Read our .obj file
	std::vector< glm::vec3 > vertexes;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector <unsigned int> indexes;

	nVertexIndex = indexes.size();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		//Usamos las posiciones calculadas en el shader de computo
		glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
		glVertexAttribPointer(inPos, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec3),
			&normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec2),
			&uvs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		nVertexIndex * sizeof(unsigned int), &indexes[0],
		GL_STATIC_DRAW);

	modelObject = glm::mat4(1.0f);

	colorTexId = loadTex("../img/map_tex.png");
	emiTexId = loadTex("../img/emissive.png");
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

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	if (uLightPos != -1)
	{
		glm::vec3 lpos = (view * modelLight) * lightPos;
		glUniform3fv(uLightPos, 1, &lpos[0]);
	}

	//Dibujado de objeto
	renderObject();

	glutSwapBuffers();
}

void renderObject()
{
	glm::mat4 modelView = view * modelObject;
	glm::mat4 modelViewProj = proj * view * modelObject;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));

	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));

	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, nVertexIndex, GL_UNSIGNED_INT, (void*)0);
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
		else if (theta < M_2PI)
		{
			theta += 0.02f;
			view = glm::rotate(view, 0.02f, glm::vec3(1, 0, 0));
		}
		else
		{
			theta = 0.0f;
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

void mouseFunc(int button, int state, int x, int y)
{
}