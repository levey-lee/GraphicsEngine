#version 430 core

#define MAX_WIDTH 50
#define PI 3.1415926535897932384626433832795


// Ouput data
layout(location = 0) out float FilteredShadowPixel;

uniform vec2 ScreenDimension;
uniform int BlurWidth;
uniform int ShadowFilterWidth;
uniform float GaussianWeights[MAX_WIDTH*2+1];
uniform float EdgeStrength;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D Depth_Texture;
uniform sampler2D ShadowMaps_Texture;
uniform sampler2D SSAO_Texture;
uniform bool HorizontalBlur;

float GenSSAOFactor(in vec3 pixelNormal, in float pixelDepth,
                    in vec2 uvPos, in vec2 pixelFrac)
{
  float SSAOWeights[MAX_WIDTH*2+1];
  float PixelStrip[MAX_WIDTH*2+1];
  /////////////////////////////////////////////////
  ///  SSAOWeights
  const int width = BlurWidth;
  const int width2 = 2*width;
  const int width2p1 = 2*width+1;
  SSAOWeights[width] = width;
  float totalWeight = SSAOWeights[width];
  const float variance = EdgeStrength;
  //horizontal pass
  if(HorizontalBlur){
    //establish left weights
    for (int i = 0; i < width; ++i){
      float x = uvPos.x -(width-i)*pixelFrac.x;
      float y = uvPos.y;
      if (x < 0 ){
        SSAOWeights[i] = 1;
        totalWeight += 1;
        continue;
      }
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      SSAOWeights[i] = weight;
      totalWeight += weight;
    }
    //establish right weights
    for (int i = 0; i < width+1; ++i){
      float x = uvPos.x+(width-i)*pixelFrac.x;
      float y = uvPos.y;
      if (x > 1){
        SSAOWeights[width2-i] = 1;
        totalWeight += 1;
        continue;
      }
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      SSAOWeights[width2-i] = weight;
      totalWeight += weight;
    }
  }
  //vertical pass
  else{
    //establish top weights
    for (int i = 0; i < width; ++i){
      float x = uvPos.x;
      float y = uvPos.y-(width-i)*pixelFrac.y;
      if (y > 1){
        SSAOWeights[i] = 1;
        totalWeight += 1;
        continue;
      }
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      SSAOWeights[i] = weight;
      totalWeight += weight;
    }
    //establish bot weights
    for (int i = 0; i < width+1; ++i){
      float x = uvPos.x;
      float y = uvPos.y+(width-i)*pixelFrac.y;
      if (y < 0){
        SSAOWeights[width2-i] = 1;
        totalWeight += 1;
        continue;
      }
      vec3 thisNormal = texture(WorldNormal_ReceiveLight_Texture, vec2(x, y)).xyz*2-1;
      float thisDepth = texture(Depth_Texture, vec2(x, y)).r;
      float weight = i*max(dot(thisNormal, pixelNormal),0)+ exp(-pow(thisDepth - pixelDepth, 2)/(2*variance))/ sqrt(2*PI*variance);
      SSAOWeights[width2-i] = weight;
      totalWeight += weight;
    }
  }
  //normalize weights
  for (int i = 0; i < width2p1; ++i){
    SSAOWeights[i] /= totalWeight;
  }  
  ////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////
  
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
  for (int i=0; i < width2p1; ++i){
    sum += PixelStrip [i] * SSAOWeights[i];
  }
  return clamp(sum,0,1);
}

float FilterShadowMap(in vec2 pixelFrac, in vec2 uvPos)
{
  float DepthStrip[MAX_WIDTH*2+1];
  //texture(ShadowMaps_Texture, uvPos).r;
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
  return sum;
}



void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  
  vec3 pixelNormal = texture(WorldNormal_ReceiveLight_Texture, uvPos).xyz*2-1;
  float pixelDepth = texture(Depth_Texture, uvPos).r;
  
  gl_FragDepth = GenSSAOFactor(pixelNormal, pixelDepth, uvPos, pixelFrac); 
  FilteredShadowPixel = FilterShadowMap(pixelFrac, uvPos);
}


