#pragma once
#include "baseShader.h"
class textureShader : public baseShader
{
public:
	void destroy();
	void initShader(const char* vname, const char* fname);
	GLuint loadShader(const char* fileName, GLenum type);
};
