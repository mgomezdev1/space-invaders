#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;

// Uniform variables
uniform mat4 u_MVPMatrix;
uniform vec3 u_Color;

out vec3 v_Color;

void main()
{
    v_Color = u_Color;

    vec4 localSpace = vec4(position.x, position.y, position.z, 1.0f);
    vec4 screenSpace = u_MVPMatrix * localSpace;

    gl_Position = vec4(screenSpace.x, screenSpace.y, screenSpace.z, screenSpace.w);
}
