#version 410 core

in vec2 v_vertexUv;
in float v_Time;

in mat4 v_ViewModelTBNMatrix;
in mat4 v_ModelMatrix;
in mat4 v_ViewMatrix;

// Material-specific values
uniform vec4 u_Color;
uniform vec4 u_Emissive;
uniform sampler2D u_EmissiveTexture;

out vec4 color;

void main() {
	vec3 emissive = u_Emissive.xyz * texture(u_EmissiveTexture, v_vertexUv).xyz;
	color = u_Color * vec4(emissive,1);
}
