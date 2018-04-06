#version 430 core

#define MAX_WIDTH 50

layout(location = 0) out float FilteredDepth;

uniform sampler2D ShadowMaps_Texture;

uniform vec2 ScreenDimension;
uniform int ShadowFilterWidth;
uniform float GaussianWeights[MAX_WIDTH*2+1];
uniform bool HorizontalBlur;

float DepthStrip[MAX_WIDTH*2+1];
void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac); 

  const int width = ShadowFilterWidth;
  const int width2 = 2*width;
  const int width2p1 = 2*width+1;
  float sum = 0;  
  DepthStrip[width] = texture(ShadowMaps_Texture, uvPos).r;
  
  if (HorizontalBlur){
      for (int i = 0; i < width; ++i){
      float depthLeft  = texture(ShadowMaps_Texture, uvPos-vec2((width-i)*pixelFrac.x,0)).r;
      float depthRight = texture(ShadowMaps_Texture, uvPos+vec2((width-i)*pixelFrac.x,0)).r;
      DepthStrip[i] = depthLeft;
      DepthStrip[width2-i] = depthRight;
    }
  }
  else {    
    for (int i = 0; i < width; ++i){
      float depthTop  = texture(ShadowMaps_Texture, uvPos-vec2(0,(width-i)*pixelFrac.y)).r;
      float depthDown = texture(ShadowMaps_Texture, uvPos+vec2(0,(width-i)*pixelFrac.y)).r;
      DepthStrip[i] = depthTop;
      DepthStrip[width2-i] = depthDown;
    }
  }
  for (int i=0; i < width2p1; ++i)
  {
    sum += DepthStrip [i] * GaussianWeights[i];
  }
  FilteredDepth = sum;
}


