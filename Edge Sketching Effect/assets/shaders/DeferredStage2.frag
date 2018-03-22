#version 430 core

#define MaxLights 64 // maximum possible lights this shader supports
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define SHADOW_TYPE_NO_SHADOW 0
#define SHADOW_TYPE_HARD_SHADOW 1
#define SHADOW_TYPE_SOFT_SHADOW 2

#define DEBUG_OUTPUT_COMBINED 0
#define DEBUG_OUTPUT_DIFFUSE 1
#define DEBUG_OUTPUT_WORLD_POSITION 2
#define DEBUG_OUTPUT_WORLD_NORMAL 3
#define DEBUG_OUTPUT_SPECULAR_COLOR 4
#define DEBUG_OUTPUT_DEPTH 5
#define DEBUG_OUTPUT_SHADOWMAP 6

layout(location = 0) out vec4 vFragColor;

uniform int EnableBlur;
uniform int BlurStrength;
uniform int DebugOutputIndex;
uniform vec2 ScreenDimension;
uniform vec3 LightPosition;
uniform sampler2D DiffuseColor_TexU_Texture;
uniform sampler2D WorldPosition_TexV_Texture;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D SpecColor_SpecPow_Texture;
uniform sampler2D Depth_Texture;
uniform sampler2D ShadowMaps_Texture; 
uniform mat4 LightViewProj;
struct Light
{
  bool isActive;
  vec4 position;
  vec4 direction; // direction the light is directed
  vec4 ambient;   // ambient light cast onto objects
  vec4 diffuse;   // diffuse light cast onto objects
  vec4 specular;
  int lightType;
  int shadowType;
  float shadowStrength;
  float intensity;
  vec3 distanceAttenuation;
};

uniform struct
{
  vec3 Position_world;
  float FarPlaneDist;
  float NearPlaneDist;
  vec4 FogColor;
}Camera;

uniform Light Lights[MaxLights]; // support UP TO 64 lights
uniform int LightCount; // number of lights enabled THIS ROUND

highp float map_01(float x, float v0, float v1)
{
  return (x - v0) / (v1 - v0);
}

const float C = 60.0f;
float CalcShadowFactor(in vec4 worldPos)
{
#if 0//basic shadow test

  // vec4 shadowCoord = LightViewProj*worldPos;
  // vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
  // vec3 uv3 = projCoords * 0.5f + 0.5f;
  // float depth = texture(ShadowMaps_Texture, uv3.xy).x;
  // return step(uv3.z,depth);
  
  vec4 shadowCoord = LightViewProj*worldPos;
  vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
  vec3 uv3 = projCoords * 0.5f + 0.5f;
  float depth = texture(ShadowMaps_Texture, uv3.xy).x;
  float z = distance(worldPos.xyz , Lights[0].position.xyz);
  z = (z - 1) / (20-1);
  return step(z, depth);
  
#else//ESM

  // const float c = 50.0f;
  // vec4 shadowCoord = LightViewProj*worldPos;
  // vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
  // vec2 uv = projCoords.xy * 0.5f + 0.5f;  
  // float p = texture(ShadowMaps_Texture, uv).r;
  // //float z = (shadowCoord.w-1) / (20-1);  
  // float z = distance(worldPos.xyz, Lights[0].position.xyz);
    // z = (z - 1) / (20-1);
  // //return step(z,p);
  // return clamp(p*exp( -c * z),0,1);


  mat4 biasMatrix = mat4(vec4(0.5f,0,0,0),
                         vec4(0,0.5f,0,0),
                         vec4(0,0,0.5f,0),
                         vec4(0.5f,0.5f,0.5f,1));

  vec4 shadowCoord = biasMatrix*LightViewProj*worldPos;
  float p = textureProj(ShadowMaps_Texture, shadowCoord.xyw).r;
  float z = map_01(shadowCoord.w, 1, 20);
  return clamp((p * exp(-C * z)),0,1);
#endif
}

vec4 DoPointLight(in Light light, in vec4 worldNormal, in vec4 worldPos, in vec2 uv)
{
  return vec4(0,0,0,1);
}
vec4 DoSpotLight(in Light light, in vec4 worldNormal, in vec4 worldPos,in vec2 uv)
{
  return vec4(0,0,0,1);
}
vec4 DoDirectionalLight(in Light light, in vec4 worldNormal, in vec4 worldPos,in vec2 uv)
{
  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec = normalize(-light.direction);
  
  vec4 viewVec = worldPos - vec4(Camera.Position_world, 1);

  vec4 ambient = light.ambient;
  // compute diffuse contribution on the surface
  vec4 diffuse = max(dot(worldNormal, lightVec), 0) * light.diffuse;
  
  vec4 specFactor=texture(SpecColor_SpecPow_Texture, uv);
  
  vec4 specular = light.specular
                * vec4(specFactor.xyz, 1)
                * pow(max(dot(reflect(lightVec, worldNormal),viewVec),0),specFactor.w);


  float visibility = CalcShadowFactor(worldPos);
  return visibility*light.intensity*(ambient + (diffuse + specular)); // total contribution from this light
}
vec4 computeLightingTerm(in int lightIdx, in vec4 worldNormal, in vec4 worldPos,in vec2 uv)
{
  // grab light
  Light light = Lights[lightIdx];
  
  vec4 lightColor = vec4(0,0,0,1);
  if (light.isActive == false)
    return vec4(0,0,0,1);
  ////////////////////////////////////////////////////////
  //    Calculate Surface Color
  ////////////////////////////////////////////////////////
    
  if(light.lightType == LIGHT_TYPE_POINT)
    lightColor = DoPointLight(light, worldNormal, worldPos, uv);
    
        
  else if(light.lightType == LIGHT_TYPE_SPOT)
    lightColor = DoSpotLight(light, worldNormal, worldPos, uv);
    
        
  else if(light.lightType == LIGHT_TYPE_DIRECTIONAL)
    lightColor = DoDirectionalLight(light, worldNormal, worldPos, uv);
  
  ////////////////////////////////////////////////////////
  //    Calculate Fog Color
  ////////////////////////////////////////////////////////
  
  vec4 viewVec = worldPos - vec4(Camera.Position_world, 1);
  float fogFactor = (Camera.FarPlaneDist - length(viewVec))/(Camera.FarPlaneDist - Camera.NearPlaneDist);
  lightColor = fogFactor*lightColor + (1-fogFactor)*Camera.FogColor;
    
  
  ////////////////////////////////////////////////////////
  //    Return the final surface color
  ////////////////////////////////////////////////////////
  return lightColor;
}

vec4 computeSurfaceColor(in vec4 worldNormal,in vec4 worldPos,in vec2 uv)
{
  vec4 color = vec4(0, 0, 0, 0); // no light = black
  for (int i = 0; i < LightCount; ++i)
    color += computeLightingTerm(i, worldNormal, worldPos, uv); // contribution of light i
  return color; // contribution from all lights onto surface
}

void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  vec3 pixelPos = texture(WorldPosition_TexV_Texture, uvPos).xyz;
  vec4 worldPos = vec4(pixelPos,1);
  vec4 normalTextureData = texture(WorldNormal_ReceiveLight_Texture, uvPos);
  vec3 pixelNormal = normalTextureData.xyz*2-1;
  if (pixelNormal == vec3(0,0,0))
    discard;
  vec4 worldNormal = vec4(pixelNormal, 0);
  
  vFragColor.w = 1;
  if (DebugOutputIndex == DEBUG_OUTPUT_COMBINED)
  {
    vec3 pixelMatColor = vec3(0,0,0);
    vec3 nearColors[9];
    ////////////////////////////////////////////////////
    float strength = BlurStrength;
    if(EnableBlur != 0)
    {
      nearColors[0] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(-strength,-strength)) * pixelFrac)).xyz;
      nearColors[1] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( 0,-strength)) * pixelFrac)).xyz;
      nearColors[2] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( strength,-strength)) * pixelFrac)).xyz;    
      nearColors[3] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(-strength, 0)) * pixelFrac)).xyz;
      nearColors[4] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( 0, 0)) * pixelFrac)).xyz;
      nearColors[5] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( strength, 0)) * pixelFrac)).xyz;
      nearColors[6] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(-strength, strength)) * pixelFrac)).xyz;
      nearColors[7] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( 0, strength)) * pixelFrac)).xyz;
      nearColors[8] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2( strength, strength)) * pixelFrac)).xyz;
    }
    else
    {
      nearColors[0] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[1] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[2] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[3] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[4] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[5] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[6] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[7] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
      nearColors[8] = texture(DiffuseColor_TexU_Texture, vec2((gl_FragCoord.xy+vec2(0,0)) * pixelFrac)).xyz;
    }
    
    for (int i = 0; i < 9; ++i)
    {
      pixelMatColor += nearColors[i];
    }
    pixelMatColor /= 9;
    
    ///////////////////////////////////////////////////
    vec3 outputColor;     
    if (normalTextureData.w > 0.5)
    {
      vec3 lightColor = computeSurfaceColor(worldNormal, worldPos,uvPos).xyz;
      outputColor = pixelMatColor*lightColor;
    }
    else
    {
      outputColor = pixelMatColor;
    }
    vFragColor.xyz = outputColor;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_DIFFUSE)
  {
    vFragColor.xyz = texture(DiffuseColor_TexU_Texture, uvPos).xyz;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_WORLD_POSITION)
  {    
    vFragColor.xyz = pixelPos;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_WORLD_NORMAL)
  {
    vFragColor.xyz = texture(WorldNormal_ReceiveLight_Texture, uvPos).xyz;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_SPECULAR_COLOR)
  {
    vFragColor.xyz = texture(SpecColor_SpecPow_Texture, uvPos).xyz;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_DEPTH)
  {
    vFragColor = texture(Depth_Texture, uvPos);
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_SHADOWMAP)
  {
    // vFragColor = texture(ShadowMaps_Texture, uvPos);
    mat4 biasMatrix = mat4(vec4(0.5f,0,0,0),
                          vec4(0,0.5f,0,0),
                          vec4(0,0,0.5f,0),
                          vec4(0.5f,0.5f,0.5f,1));
    
    vec4 shadowCoord = LightViewProj*worldPos;
    vec3 ProjCoords = shadowCoord.xyz / shadowCoord.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
    // float depth = texture(ShadowMaps_Texture, uvPos).x;

    
    float depth = (texture(ShadowMaps_Texture, UVCoords).xyz).r-0.001;

    vFragColor.xyz = vec3((step(z,depth)));

    //vFragColor.xyz = vec3(z);
    //vFragColor.xyz = vec3(CalcShadowFactor(worldPos));
  }
  

}












