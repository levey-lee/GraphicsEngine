#version 430 core

// Ouput data
layout(location = 0) out float depth32;

uniform float LightNearPlane;
uniform float LightFarPlane;
uniform float LightShadowExp;


in float Depth;

highp float map_01(float x, float v0, float v1)
{
  return (x - v0) / (v1 - v0);
}

void main(){  
  
  float mappedDepth = map_01(Depth, LightNearPlane, LightFarPlane);
  depth32 = exp(LightShadowExp * mappedDepth);
}


