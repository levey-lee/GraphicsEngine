#version 430 core

// Ouput data
layout(location = 0) out float depthExp;

in float Depth;
in float VertLightDistance;

highp float map_01(float x, float v0, float v1)
{
  return (x - v0) / (v1 - v0);
}

void main(){  
  // float d = (VertLightDistance-1) / (20-1);
  // const float c = 50.0f; 
  // depthExp = exp(c * d);

  
  float depthDivisor = (1.0 / gl_FragCoord.w);
  // float mappedDivisor = map_01(depthDivisor, 1, 20);
  // const float c = 60.0f; 
  // depthExp = exp(c * mappedDivisor);
  depthExp = gl_FragCoord.z;
}
//fov   w    h   n    f
//pi/2, 160, 80, 1, 1000


