#pragma once
#include "engine.h"
#include "textureShader.h"
#include "baseShader.h"
class resources
{
public:
	void initialize(engine* engine);
	void initPlane();
	void initFBO();
	void resizeFBO(unsigned int w, unsigned int h);
	unsigned int loadTex(const char* fileName);

	void initObj(baseShader shader);
	void destroy(baseShader shader);

	engine* OGLengine;

	//////////////////////////////////////////////////////////////
	// Variables que nos dan acceso a Objetos OpenGL
	//////////////////////////////////////////////////////////////

	//Variable del modelo
	unsigned int nVertexIndex;

	unsigned int colorTexId;
	unsigned int emiTexId;

	//VAO
	unsigned int vao;

	//VBO
	unsigned int fbo;

	//VBOs que forman parte del objeto
	unsigned int posVBO;
	unsigned int colorVBO;
	unsigned int normalVBO;
	unsigned int texCoordVBO;
	unsigned int triangleIndexVBO;
	glm::mat4 modelObject = glm::mat4(1.0f);

	unsigned int planeVAO;
	unsigned int planeVertexVBO;

	unsigned int colorBuffTexId;
	unsigned int emiBuffTexId;
	unsigned int depthBuffTexId;
	unsigned int vertexBuffTexId;
	unsigned int normalBuffTexId;
};
