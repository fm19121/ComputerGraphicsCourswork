#version 330 core
uniform sampler2D diffuseTex;
uniform float alphaFactor;

in Vertex{
  vec2 texCoord;
}IN;

out vec4 fragColour;
void main(void){
  fragColour.xyz = texture(diffuseTex, IN.texCoord).xyz;
  fragColour.a = texture(diffuseTex, IN.texCoord).a * alphaFactor;
}