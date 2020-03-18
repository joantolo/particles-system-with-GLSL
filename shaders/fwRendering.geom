#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 vPos[];
in vec4 vColor[];

out vec4 gPos;
out vec2 gTexCoord;
out vec4 gColor;

uniform mat4 proj;

float billboardSize = 2.0;

void main()
{
	vec4 initialPos = gl_in[0].gl_Position;

	//Create a billboard from the position
	vec4 v0 = initialPos + vec4(-billboardSize, -billboardSize, 0, 0);
	vec4 v1 = initialPos + vec4( billboardSize, -billboardSize, 0, 0);
	vec4 v2 = initialPos + vec4(-billboardSize,	 billboardSize, 0, 0);
	vec4 v3 = initialPos + vec4( billboardSize,	 billboardSize, 0, 0);

	gPos = vPos[0];
	gColor = vColor[0];

	gTexCoord = vec2(0.0, 0.0);
	gl_Position = proj * v0;
	EmitVertex();

	gTexCoord = vec2(1.0, 0.0);
	gl_Position = proj * v1;
	EmitVertex();

	gTexCoord = vec2(0.0, 1.0);
	gl_Position = proj * v2;
	EmitVertex();

	gTexCoord = vec2(1.0, 1.0);
	gl_Position = proj * v3;
	EmitVertex();

}