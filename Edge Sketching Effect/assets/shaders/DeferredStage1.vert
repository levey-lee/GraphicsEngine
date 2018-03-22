#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUv;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

uniform mat4 LightVP; 
uniform mat4 ModelMatrix; 
uniform vec3 LightPosition;

out float Depth;
out float VertLightDistance;

void main()
{
  vec4 vertWorldPos = ModelMatrix * vec4(vPosition, 1);  
  
  VertLightDistance = distance(LightPosition, vertWorldPos.xyz);
  
  gl_Position = LightVP * vertWorldPos;
  
  Depth = gl_Position.w;
  
}