#version 430 core

#define MAX_WIDTH 50
#define PI 3.1415926535897932384626433832795


// Ouput data
layout(location = 0) out float FilteredPixel;

uniform vec2 ScreenDimension;
uniform int BlurWidth;
uniform float EdgeStrength;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D Depth_Texture;
uniform sampler2D SSAO_Texture;
uniform bool HorizontalBlur;
uniform int EnableSSAO;

float Weights[MAX_WIDTH*2+1];
float PixelStrip[MAX_WIDTH*2+1];
void main()
{
  if (EnableSSAO == 0)
    return;
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  
  vec3 pixelNormal = texture(WorldNormal_ReceiveLight_Texture, uvPos).xyz*2-1;
  float pixelDepth = texture(Depth_Texture, uvPos).r;
  
  /////////////////////////////////////////////////
  ///  Weights
  const int width = BlurWidth;
  const int width2 = 2*width;
  const int width2p1 = 2*width+1;
  Weights[width] = width;
  float totalWeight = Weights[width];
  const float variance = EdgeStrength;
  //horizontal pass
  if(HorizontalBlur){
    //establish left weights
    for (int i = 0; i < width; ++i){
      float x = uvPos.x -(width-i)*pixelFrac.x;
      float y = uvPos.y;
      if (x < 0 || x > 1)
        continue;
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      Weights[i] = weight;
      totalWeight += weight;
    }
    //establish right weights
    for (int i = 0; i < width+1; ++i){
      float x = uvPos.x+(width-i)*pixelFrac.x;
      float y = uvPos.y;
      if (x < 0 || x > 1)
        continue;
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      Weights[width2-i] = weight;
      totalWeight += weight;
    }
  }
  //vertical pass
  else{
    //establish top weights
    for (int i = 0; i < width; ++i){
      float x = uvPos.x;
      float y = uvPos.y-(width-i)*pixelFrac.y;
      if (y < 0 || y > 1)
        continue;
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      Weights[i] = weight;
      totalWeight += weight;
    }
    //establish bot weights
    for (int i = 0; i < width+1; ++i){
      float x = uvPos.x;
      float y = uvPos.y+(width-i)*pixelFrac.y;
      if (y < 0 || y > 1)
        continue;
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      Weights[width2-i] = weight;
      totalWeight += weight;
    }
  }
  //normalize weights
  for (int i = 0; i < width2p1; ++i){
    Weights[i] /= totalWeight;
  }  
  /////////////////////////////////////////////////
  /////////////////////////////////////////////////
  
  float sum = 0;  
  PixelStrip[width] = texture(SSAO_Texture, uvPos).r;
  //horizontal pass
  if(HorizontalBlur){
    for (int i = 0; i < width; ++i){
      float depthLeft  = texture(SSAO_Texture, uvPos-vec2((width-i)*pixelFrac.x,0)).r;
      float depthRight = texture(SSAO_Texture, uvPos+vec2((width-i)*pixelFrac.x,0)).r;
      PixelStrip[i] = depthLeft;
      PixelStrip[width2-i] = depthRight;
    }
  }
  //vertical pass
  else{
    for (int i = 0; i < width; ++i){
      float depthUp  = texture(SSAO_Texture, uvPos-vec2(0, (width-i)*pixelFrac.y)).r;
      float depthDown = texture(SSAO_Texture, uvPos+vec2(0, (width-i)*pixelFrac.y)).r;
      PixelStrip[i] = depthUp;
      PixelStrip[width2-i] = depthDown;
    }
  }
  for (int i=0; i < width2p1; ++i)
  {
    sum += PixelStrip [i] * Weights[i];
  }
  
  FilteredPixel = clamp(sum,0,1);
}


