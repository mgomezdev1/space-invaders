#version 410 core

in vec3 v_Color;

out vec4 color;

void main() {
	color = vec4(v_Color,1);
}
