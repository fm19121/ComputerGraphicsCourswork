#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex{
   vec3 colour;
   vec2 texCoord;
   vec3 worldPos;
}IN;

out vec4 fragColour;

void main(void){
    vec4 diffuse = texture(diffuseTex, IN.texCoord);

    fragColour.rgb = diffuse.rgb;
    fragColour.a = 1.0f;
}