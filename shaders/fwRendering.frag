#version 330 core
out vec4 outColor;

in vec3 vPos;
in vec3 vNorm;
in vec2 vTexCoord;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
float alpha = 500.0;
vec3 Ke;
vec3 N;
vec3 pos;

//Propiedades de la luz
vec3 Ia = vec3 (0.3);
vec3 Id = vec3 (1.0);
vec3 Is = vec3 (0.7);

vec3 shade();

uniform vec3 lpos;

uniform sampler2D colorTex;
uniform sampler2D emiTex;

void main()
{
	Ka = texture(colorTex, vTexCoord).rgb;
	Kd = texture(colorTex, vTexCoord).rgb;
	Ke = texture(emiTex, vTexCoord).rgb;
	Ks = vec3 (1.0);

	N = vNorm;
	pos = vPos;

	outColor = vec4(shade(), 1.0);
}

vec3 shade()
{
	vec3 c = vec3(0.0);
	c = Ia * Ka;

	vec3 L = normalize(lpos - pos);
	vec3 diffuse = Id * Kd * dot(L, N);
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize(-pos);
	vec3 R = normalize(reflect(-L, N));
	float factor = max(dot(R, V), 0.01);
	vec3 specular = Is * Ks * pow(factor, alpha);
	c += clamp(specular, 0.0, 1.0);

	c += Ke;
	
	return c;
}
