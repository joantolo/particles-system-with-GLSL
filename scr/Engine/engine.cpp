#include "engine.h"

int engine::Initialize(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	proj = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	modelLight = glm::mat4(1.0f);

	theta = 0.0f;
	phi = 0.0f;

	initContext(argc, argv);
	initOGL();

	engResources.initialize(this);

	currentShader.initShader("../shaders/fwRendering.vert", "../shaders/fwRendering.frag");
	engResources.initObj(currentShader);
	engResources.initPlane();

	engResources.initFBO();

	glutMainLoop();

	engResources.destroy(currentShader);
	currentShader.destroy();

	return 0;
}

void engine::renderFunc()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	///////////
	//Forward-rendering
	///////////
	glUseProgram(currentShader.program);

	//Texturas del forward-rendering
	if (currentShader.uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentShader.colorTexId);
		glUniform1i(currentShader.uColorTex, 0);
	}

	if (currentShader.uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, currentShader.emiTexId);
		glUniform1i(currentShader.uEmiTex, 1);
	}

	//Dibujado de objeto
	renderObject();

	/*glUseProgram(geometryProgram);

	renderObject();*/

	glutSwapBuffers();
}

void engine::renderObject()
{
	glm::mat4 modelView = view * engResources.modelObject;
	glm::mat4 modelViewProj = proj * view * engResources.modelObject;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (currentShader.uModelViewMat != -1)
		glUniformMatrix4fv(currentShader.uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));

	if (currentShader.uModelViewProjMat != -1)
		glUniformMatrix4fv(currentShader.uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));

	if (currentShader.uNormalMat != -1)
		glUniformMatrix4fv(currentShader.uNormalMat, 1, GL_FALSE, &(normal[0][0]));
	/*
		if (uModelViewMatGeo != -1)
			glUniformMatrix4fv(uModelViewMatGeo, 1, GL_FALSE, &(modelView[0][0]));

		if (uModelViewProjMatGeo != -1)
			glUniformMatrix4fv(uModelViewProjMatGeo, 1, GL_FALSE, &(modelViewProj[0][0]));

		if (uNormalMatGeo != -1)
			glUniformMatrix4fv(uNormalMatGeo, 1, GL_FALSE, &(normal[0][0]));*/

	glBindVertexArray(engResources.vao);
	glDrawElements(GL_TRIANGLES, engResources.nVertexIndex, GL_UNSIGNED_INT, (void*)0);
}

void engine::initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
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

void engine::initOGL()
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
	view = glm::lookAt(glm::vec3(0, 0, 40), glm::vec3(0), glm::vec3(0, 1, 0));
	moveCam = false;
}

void engine::resizeFunc(int width, int height)
{
	//Mantenimiento del aspect ratio
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) / float(height), projNear, projFar);

	engResources.resizeFBO(width, height);

	glutPostRedisplay();
}

void engine::idleFunc()
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

void engine::keyboardFunc(unsigned char key, int x, int y)
{
	glm::vec3 dir;
	switch (key)
	{
	case(' '):
		moveCam = !moveCam;
		break;
	case('a'):
	case('A'):
		dir = -view[3];
		view[3].x += dir.x * 0.1;
		view[3].y += dir.y * 0.1;
		view[3].z += dir.z * 0.1;
		break;
	case('s'):
	case('S'):
		dir = view[3];
		view[3].x += dir.x * 0.1;
		view[3].y += dir.y * 0.1;
		view[3].z += dir.z * 0.1;
		break;
	}
}

void engine::mouseFunc(int button, int state, int x, int y)
{
}