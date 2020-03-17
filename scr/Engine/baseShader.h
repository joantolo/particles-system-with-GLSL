#pragma once
#include "engine.h"
class baseShader
{
public:

	virtual void destroy() {};
	virtual GLuint loadShader(const char* fileName, GLenum type) {};
	virtual void initShader(const char* vname, const char* fname) {};

	unsigned int VShader;
	unsigned int GShader;
	unsigned int FShader;
	unsigned int program;

	//Atributos
	int inPos;
	int inColor;
	int inNormal;

	//Matrices Uniform
	int uModelViewMat;
	int uModelViewProjMat;
	int uNormalMat;

	int inTexCoord;

	//Identificadores de texturas Forward-rendering
	unsigned int colorTexId;
	unsigned int emiTexId;

	//Texturas Uniform
	int uColorTex;
	int uEmiTex;
};
