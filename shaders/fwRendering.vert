#version 330 core

in vec4 inPos;	
in vec4 inColor;

uniform mat4 normal;
uniform mat4 modelViewProj;
uniform mat4 modelView;

out vec4 vPos;
out vec4 vColor;

void main()
{
	vColor = inColor;
	vPos = modelView * inPos;
	
	gl_Position = modelView * inPos;
}
