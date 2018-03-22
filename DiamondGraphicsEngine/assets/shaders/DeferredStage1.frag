#version 430 core

// Ouput data
layout(location = 0) out float depth32;

uniform float LightNearPlane;
uniform float LightFarPlane;
uniform float LightShadowExp;


in float Depth;
in float VertLightDistance;

highp float map_01(float x, float v0, float v1)
{
  return (x - v0) / (v1 - v0);
}

void main(){  
  //float d = map_01(VertLightDistance, 10, 15);
  //depth32 = exp(C * d);
  //depth32 = d;
  
  // float depthDivisor = (1.0 / gl_FragCoord.w);
  // float mappedDivisor = map_01(depthDivisor, 1, 100);
  // depth32 = exp(C * mappedDivisor);
  
  
  //depth32 = Depth;
  
  float mappedDepth = map_01(Depth, LightNearPlane, LightFarPlane);
  depth32 = exp(LightShadowExp * mappedDepth);
}


