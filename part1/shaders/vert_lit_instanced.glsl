#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUv;
layout(location=2) in vec3 vertexNormals;
layout(location=3) in vec3 vertexTangent;

layout(location=5) in mat4 modelMatrix;
layout(location=9) in vec4 colorModifier;

// Uniform variables
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

// Pass instance data to the fragment shader
out vec4 i_color;

void main()
{
    // Temporarily until instancing works
    mat4 model = v_ModelMatrix;

    v_vertex        = (u_ViewMatrix * model * vec4(position, 1.0f)).xyz;
    v_vertexNormals = (u_ViewMatrix * model * vec4(vertexNormals, 0.0f)).xyz;
    v_rawNormals    = vertexNormals;
    v_vertexUv      = vertexUv;
    v_Time          = u_Time;
    i_color         = colorModifier;

    vec3 bitangent = cross(vertexNormals, normalize(vertexTangent));
    mat4 TBNMatrix = mat4(
        vec4(normalize(vertexTangent), 0),
        vec4(bitangent, 0),
        vec4(vertexNormals, 0),
        vec4(0,0,0,1)
    );
    v_ViewModelTBNMatrix = u_ViewMatrix * modelMatrix * TBNMatrix;

    v_ModelMatrix = modelMatrix;
    v_ViewMatrix  = u_ViewMatrix;

    vec4 localSpace = vec4(position.x, position.y, position.z, 1.0f);
    vec4 screenSpace;
    //screenSpace = u_ProjectionMatrix * u_ViewMatrix * modelMatrix * localSpace;
    screenSpace = u_ViewProjectionMatrix * modelMatrix * localSpace;

    gl_Position = vec4(screenSpace.x, screenSpace.y, screenSpace.z, screenSpace.w);
}
