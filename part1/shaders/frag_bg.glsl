#version 410 core

in vec2 v_vertexUv;
in float v_Time;

uniform vec4 u_Emissive;
uniform sampler2D u_EmissiveTexture;

out vec4 color;

void main() {
    vec2 tiledUv = v_vertexUv * 2;
    tiledUv += vec2(0, v_Time / 10);
    color = u_Emissive * texture2D(u_EmissiveTexture, tiledUv);
}