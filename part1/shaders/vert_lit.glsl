#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUv;
layout(location=2) in vec3 vertexNormals;
layout(location=3) in vec3 vertexTangent;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform float u_Time;

// Pass vertex colors into the fragment shader
out vec3 v_vertex;
out vec4 v_vertexColors;
out vec3 v_vertexNormals;
out vec3 v_rawNormals;
out vec2 v_vertexUv;
out mat4 v_ModelMatrix;
out mat4 v_ViewMatrix;
out mat4 v_ViewModelTBNMatrix;
out float v_Time;

void main()
{
    v_vertex        = (u_ViewMatrix * u_ModelMatrix * vec4(position, 1.0f)).xyz;
    v_vertexNormals = (u_ViewMatrix * u_ModelMatrix * vec4(vertexNormals, 0.0f)).xyz;
    v_rawNormals     = vertexNormals;
    v_vertexUv      = vertexUv;
    v_Time          = u_Time;

    vec3 bitangent = cross(vertexNormals, normalize(vertexTangent));
    mat4 TBNMatrix = mat4(
        vec4(normalize(vertexTangent), 0),
        vec4(bitangent, 0),
        vec4(vertexNormals, 0),
        vec4(0,0,0,1)
    );
    v_ViewModelTBNMatrix = u_ViewMatrix * u_ModelMatrix * TBNMatrix;

    v_ModelMatrix = u_ModelMatrix;
    v_ViewMatrix  = u_ViewMatrix;

    vec4 localSpace = vec4(position.x, position.y, position.z, 1.0f);
    vec4 screenSpace;
    //screenSpace = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * localSpace;
    screenSpace = u_ViewProjectionMatrix * u_ModelMatrix * localSpace;

    gl_Position = vec4(screenSpace.x, screenSpace.y, screenSpace.z, screenSpace.w);
}
