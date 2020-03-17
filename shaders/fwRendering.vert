#version 330 core

in vec3 inPos;	
in vec2 inTexCoord;
in vec3 inNormal;

uniform mat4 normal;
uniform mat4 modelViewProj;
uniform mat4 modelView;

out vec3 vPos;
out vec3 vNorm;
out vec2 vTexCoord;

void main()
{
	vTexCoord = inTexCoord;
	vNorm = (normal * vec4(inNormal, 0.0)).xyz;
	vPos = (modelView * vec4(inPos, 1)).xyz;
	
	gl_Position = modelViewProj * vec4 (inPos, 1.0);
}
