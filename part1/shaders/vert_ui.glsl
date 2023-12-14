#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUv;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform float u_Time;
uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

// Pass vertex colors into the fragment shader
out vec2 v_vertexUv;
out float v_Time;

void main() {
    v_vertexUv = vertexUv;
    v_Time = u_Time;

    //vec4 clipSpace = vec4(position.xyz / 2, 1);
    vec4 localSpace = vec4(position.x, position.y, -0.9999, 1);
    vec4 clipSpace = localSpace;

    gl_Position = clipSpace;
}