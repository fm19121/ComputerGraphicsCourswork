#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 lightPos;

in vec3 position;
in vec3 normal;
in vec3 colour;
in vec4 tangent;
in vec2 texCoord;

out Vertex{
   vec3 colour;
   vec2 texCoord;
   vec3 worldPos;
}OUT;

void main(void){
     OUT.colour = colour;
     OUT.texCoord = texCoord;


     vec4 worldPos = (modelMatrix * vec4(position, 1));

     OUT.worldPos = worldPos.xyz;

     gl_Position = (projMatrix * viewMatrix) * worldPos;
}