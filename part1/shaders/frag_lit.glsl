#version 410 core

#define MAX_LIGHTS 50

in vec3 v_vertex;
in vec4 v_vertexColors;
in vec3 v_rawNormals;
in vec3 v_vertexNormals;
in vec2 v_vertexUv;
in float v_Time;

in mat4 v_ViewModelTBNMatrix;
in mat4 v_ModelMatrix;
in mat4 v_ViewMatrix;

// Material-specific values
uniform vec4 u_Color;
uniform vec4 u_Ambient;
uniform vec4 u_Diffuse;
uniform vec4 u_Specular;
uniform vec4 u_Emissive;
uniform float u_Glossiness;
uniform sampler2D u_AmbientTexture;
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_EmissiveTexture;
uniform sampler2D u_NormalMap;
uniform sampler2D u_GlossinessMap;

// Lights
struct LightData {
	vec3 position; //World-space position
	int on;
	vec3 color;
	float PADDING0;
	vec4 attenuation;
	float ambientPower;
	float diffusePower;
	float specularPower;
	float PADDING1;
};
layout(std140) uniform LightBlock {
	LightData lights[MAX_LIGHTS];
	int lightCount;
} u_Lights;
uniform LightData[MAX_LIGHTS] lights;
uniform int lightCount;

out vec4 color;

void main() {
	vec3 illum = vec3(0,0,0);
	vec3 tangentNormal = texture(u_NormalMap, v_vertexUv).rgb * 2 - vec3(1);
	vec3 normal = (v_ViewModelTBNMatrix * vec4(tangentNormal, 0)).xyz;
	int i;

	vec3 mat_ambt = u_Ambient.xyz  * texture(u_AmbientTexture , v_vertexUv).xyz;
	vec3 mat_diff = u_Diffuse.xyz  * texture(u_DiffuseTexture , v_vertexUv).xyz;
	vec3 mat_spec = u_Specular.xyz * texture(u_SpecularTexture, v_vertexUv).xyz;
	float glossiness = u_Glossiness * texture(u_GlossinessMap , v_vertexUv).x;
	//color = vec4(glossiness); return;

	for (i = 0; i < MAX_LIGHTS; ++i) {
		if (lights[i].on == 0) continue;
		if (i >= lightCount) break;
		vec3 delta = (v_ViewMatrix * vec4(lights[i].position,1)).xyz - v_vertex;
		float dist = length(delta);
		float d2 = dist * dist;
		float attenuation = lights[i].attenuation.x
			+ lights[i].attenuation.y * dist
			+ lights[i].attenuation.z * d2
			+ lights[i].attenuation.w * dist * d2;

		vec3 ambt = mat_ambt * lights[i].ambientPower;
		vec3 diff = mat_diff * lights[i].diffusePower  * max(0,dot(normalize(delta), normal));
		vec3 spec = mat_spec * lights[i].specularPower * pow(max(0,dot(normalize(reflect(-delta,normal)), normalize(-v_vertex))),glossiness);

		illum += lights[i].on * lights[i].color * ((ambt) + (diff + spec) / attenuation);
	}
	vec3 emissive = u_Emissive.xyz * texture(u_EmissiveTexture, v_vertexUv).xyz;
	color = u_Color * vec4(illum + emissive,1);
}
