#version 330 core
layout (location = 0) in vec3 inVertexPosition;
layout (location = 1) in vec3 inVertexNormal;
layout (location = 2) in vec2 inTextureCoordinate;

out vec3 fragmentPosition;
out vec3 fragmentVertexNormal;
out vec2 fragmentTextureCoordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   vec4 worldPosition = model * vec4(inVertexPosition, 1.0);
   mat3 normalMatrix = transpose(inverse(mat3(model)));

   fragmentPosition = worldPosition.xyz;
   fragmentVertexNormal = normalize(normalMatrix * inVertexNormal);
   fragmentTextureCoordinate = inTextureCoordinate;

   gl_Position = projection * view * worldPosition;
}
