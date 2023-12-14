#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUv;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform float u_Time;

// Pass vertex colors into the fragment shader
out vec2 v_vertexUv;
out mat4 v_ModelMatrix;
out mat4 v_ViewMatrix;
out float v_Time;

void main()
{
    v_vertexUv      = vertexUv;
    v_Time          = u_Time;

    v_ModelMatrix = u_ModelMatrix;
    v_ViewMatrix  = u_ViewMatrix;

    vec4 localSpace = vec4(position.x, position.y, position.z, 1.0f);
    vec4 screenSpace = u_ViewProjectionMatrix * u_ModelMatrix * localSpace;

    gl_Position = vec4(screenSpace.x, screenSpace.y, screenSpace.z, screenSpace.w);
}
