#version 430

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

uniform vec4 modelViewProj;

float billboardSize=0.1;

void main()
{
	
	vec4 initialPos=gl_in[0].gl_Position;

	//Create a billboard from the position
	vec4 v0= initialPos + vec4(-billboardSize, -billboardSize,0,0);
	vec4 v1= initialPos + vec4(	billboardSize, -billboardSize,0,0);
	vec4 v2= initialPos + vec4(-billboardSize,	billboardSize,0,0);
	vec4 v3= initialPos + vec4(	billboardSize,	billboardSize,0,0);

	gl_Position= modelViewProj * v0;
	EmitVertex();
	gl_Position= modelViewProj * v1;
	EmitVertex();
	gl_Position= modelViewProj * v2;
	EmitVertex();
	gl_Position= modelViewProj * v3;
	EmitVertex();

}