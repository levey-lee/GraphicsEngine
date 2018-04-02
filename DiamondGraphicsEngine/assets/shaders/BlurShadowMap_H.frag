#version 430 core

// Ouput data
layout(location = 0) out float FilteredDepth;

uniform vec2 ScreenDimension;
uniform sampler2D ShadowMaps_Texture;
uniform int ShadowFilterWidth;

//w = 5, 2w+1 = 11
// const float TotalWeight = 6.09866;
// const float TotalWeight = 12.09866;
// const float weights[11] = 
// {
// 0.13534/TotalWeight,0.27804/TotalWeight,0.48675/TotalWeight,0.72615/TotalWeight,0.92312/TotalWeight,
// 1/TotalWeight,
// 0.92312/TotalWeight,0.72615/TotalWeight,0.48675/TotalWeight,0.27804/TotalWeight,0.13534/TotalWeight
// }

;
#define MAX_WIDTH 50

float GaussianWeights[MAX_WIDTH*2+1];
float DepthStrip[MAX_WIDTH*2+1];

void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  

  const int width = ShadowFilterWidth;
  const int width2 = 2*width;
  const int width2p1 = 2*width+1;
  GaussianWeights[width] = 1;
  const float s = width/2;
  float totalWeight = GaussianWeights[width];
  //establish weights
  for (int i = 0; i < width; ++i)
  {
    float weight = exp(-((width-i)*(width-i)) / (2*s*s));
    GaussianWeights[i] = weight;
    GaussianWeights[width2-i] = weight;  
    totalWeight += 2.0f * weight;
  }
  //normalize weights
  for (int i = 0; i < width2p1; ++i)
  {
    GaussianWeights[i] /= totalWeight;
  }  
  
  // ////////////////////////
  float sum = 0;  
  DepthStrip[width] = texture(ShadowMaps_Texture, uvPos).r;
  
  for (int i = 0; i < width; ++i)
  {
    float depthLeft  = texture(ShadowMaps_Texture, uvPos-vec2((width-i)*pixelFrac.x,0)).r;
    float depthRight = texture(ShadowMaps_Texture, uvPos+vec2((width-i)*pixelFrac.x,0)).r;
    DepthStrip[i] = depthLeft;
    DepthStrip[width2-i] = depthRight;
  }  
  for (int i=0; i < width2p1; ++i)
  {
    sum += DepthStrip [i] * GaussianWeights[i];
  }
  FilteredDepth = sum;
}


