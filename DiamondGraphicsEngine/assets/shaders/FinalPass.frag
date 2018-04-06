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
#define DEBUG_OUTPUT_SSAO 7

layout(location = 0) out vec4 vFragColor;

uniform int EnableBlur;
uniform int BlurStrength;
uniform int DebugOutputIndex;
uniform int EnableSSAO;
uniform vec2 ScreenDimension;
uniform vec3 LightPosition;
uniform sampler2D DiffuseColor_Empty_Texture;
uniform sampler2D WorldPosition_SpecPow_Texture;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D SpecColor_Empty_Texture;
uniform sampler2D Depth_Texture;
uniform sampler2D ShadowMaps_Texture; 
uniform sampler2D SSAO_Texture; 
uniform mat4 LightViewProj;
uniform float LightNearPlane;
uniform float LightFarPlane;
uniform float LightShadowExp;
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
  float innerAngle;
  float outerAngle;
  float spotFalloff;
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

const mat4 BiasMatrix = mat4(vec4(0.5f,0,0,0),
                             vec4(0,0.5f,0,0),
                             vec4(0,0,0.5f,0),
                             vec4(0.5f,0.5f,0.5f,1));
float CalcShadowFactor(in vec4 worldPos, in vec2 uv)
{
  if (Lights[0].shadowType == SHADOW_TYPE_NO_SHADOW)
    return 1.0f;
  vec4 shadowCoord = BiasMatrix*LightViewProj*worldPos;
#if 0 // Basic Shadow
  vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;
  if (shadowCoord.w > 0){
    if (shadowIndex.x >= 0 &&  shadowIndex.x <= 1){
      if (shadowIndex.y >= 0 &&  shadowIndex.y <= 1){
        float lightDepth =  texture(ShadowMaps_Texture, shadowIndex).r;
        float pixelDepth = shadowCoord.w;
        if (pixelDepth > lightDepth){
          return 0.1f;
        }
      }
    }
  }
  return 1;
#else // ESM
  
  float c = LightShadowExp;
  vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;
  float lightMappedExpDepth =  texture(ShadowMaps_Texture, shadowIndex).r;
  float pixelDepth = shadowCoord.w;
  float pixelMappedDepth = map_01(pixelDepth, LightNearPlane, LightFarPlane);
  float shadowFactor = lightMappedExpDepth*exp(-c*pixelMappedDepth); 
  if (Lights[0].shadowType == SHADOW_TYPE_HARD_SHADOW){
    return floor(clamp(shadowFactor,0,1)+0.5f);
  }
  return clamp(shadowFactor,0,1);
#endif
}

vec4 DoPointLight(in Light light, in vec4 worldNormal, in vec4 worldPos, in vec2 uv)
{
  return vec4(0,0,0,1);
}
vec4 DoSpotLight(in Light light, in vec4 worldNormal, in vec4 worldPos,in vec2 uv)
{
  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec = normalize(-light.direction);
  
  vec4 viewVec = vec4(normalize(Camera.Position_world - worldPos.xyz), 0);

  // ambient contribution from the light is always constant
  vec4 ambient = light.ambient;
  
  // compute diffuse contribution on the surface
  vec4 diffuse = max(dot(worldNormal, lightVec), 0) * light.diffuse;
 
  vec3 reflectVec = reflect(-lightVec.xyz, worldNormal.xyz);
  
  vec4 specFactor=texture(SpecColor_Empty_Texture, uv);
  float specPow=texture(WorldPosition_SpecPow_Texture, uv).w;
  
  vec4 specular = light.specular
                * vec4(specFactor.xyz, 1)
                * pow(max(dot(reflectVec.xyz,viewVec.xyz),0), specPow);

  float cos_inner = cos(light.innerAngle);
  float cos_outer = cos(light.outerAngle);
  float cos_alpha = (dot(normalize(light.direction.xyz), normalize(worldPos.xyz - light.position.xyz)));
 
  vec3 lightAtt = light.distanceAttenuation;
  float dist = length(light.position.xyz-worldPos.xyz);
  float att = min(1.0f/(lightAtt.x + lightAtt.y*dist + lightAtt.z*dist*dist),1.0f);
  
  vec4 finalColor = vec4(0,0,0,0);
  float visibility = CalcShadowFactor(worldPos, uv);
  if (cos_alpha > cos_outer && cos_alpha < cos_inner)//between
  {
    vec3 L = normalize(vec3(light.direction.x, light.direction.y, light.direction.z));
    vec3 D = normalize(worldPos.xyz - light.position.xyz);
    float cos_alpha = (dot(L, D));
    float spotEffect = pow((abs(cos_alpha - cos_outer) / abs(cos_inner - cos_outer)),  light.spotFalloff);
    finalColor = att*(ambient + spotEffect * visibility*(diffuse + specular));
  }
  else if (cos_alpha > cos_inner)//inside
  {
    finalColor = att * (ambient + visibility*(diffuse + specular));
  }
  else//outside
  {
    finalColor = vec4(0,0,0,0);
  }
  
  return light.intensity*finalColor; // total contribution from this light
}
vec4 DoDirectionalLight(in Light light, in vec4 worldNormal, in vec4 worldPos,in vec2 uv)
{
  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec = normalize(-light.direction);
  
  vec4 viewVec = worldPos - vec4(Camera.Position_world, 1);

  vec4 ambient = light.ambient;
  // compute diffuse contribution on the surface
  vec4 diffuse = max(dot(worldNormal, lightVec), 0) * light.diffuse;
  
  vec4 specFactor=texture(SpecColor_Empty_Texture, uv);

  float specPow=texture(WorldPosition_SpecPow_Texture, uv).w;
  
  vec4 specular = light.specular
                * vec4(specFactor.xyz, 1)
                * pow(max(dot(reflect(lightVec, worldNormal),viewVec),0),specPow);


  float visibility = CalcShadowFactor(worldPos, uv);
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
  
  if(EnableSSAO != 0)
    lightColor *= vec4(texture(SSAO_Texture, uv).r);
  
  
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
vec3 BlurScene(in vec2 uvPos, in vec2 pixelFrac)
{
  const int width = 5;
  const int width2 = width*2;
  const int width2p1 = width2+1;
  const float s = width/2;
  float weights[width2p1];
  weights[width] = 1;
  float totalWeight = weights[width];
  //establish weights
  for (int i = 0; i < width; ++i)
  {
    float weight = (-((width-i)*(width-i)) / (2*s*s));
    weights[i] = weight;
    weights[width2-i] = weight;  
    totalWeight += (2.0f * weight);
  }
  //normalize weights
  for (int i = 0; i < width2p1; ++i)
  {
    weights[i] /= totalWeight;
  }  
  
  // ////////////////////////
  vec3 colorVals[width2p1];
  vec3 sum2 = vec3(0,0,0);  
  colorVals[width] = texture(DiffuseColor_Empty_Texture, uvPos).rgb;
  
  for (int i = 0; i < width; ++i)
  {    
    vec3 depthLeft  = texture(DiffuseColor_Empty_Texture, uvPos-vec2((width-i)*pixelFrac.x,0)*BlurStrength).rgb;
    vec3 depthRight = texture(DiffuseColor_Empty_Texture, uvPos+vec2((width-i)*pixelFrac.x,0)*BlurStrength).rgb;
    colorVals[i] = depthLeft;
    colorVals[width2-i] = depthRight;
  }  
  for (int i=0; i < width2p1; ++i)
  {
    sum2 += colorVals [i] * weights[i];
  }
  
  
  for (int i = 0; i < width; ++i)
  {    
    vec3 depthTop  = texture(DiffuseColor_Empty_Texture, uvPos-vec2(0, (width-i)*pixelFrac.y)*BlurStrength).rgb;
    vec3 depthDown = texture(DiffuseColor_Empty_Texture, uvPos+vec2(0, (width-i)*pixelFrac.y)*BlurStrength).rgb;
    colorVals[i] = depthTop;
    colorVals[width2-i] = depthDown;
  }  
  for (int i=0; i < width2p1; ++i)
  {
    sum2 += colorVals [i] * weights[i];
  }
  
  
  return sum2*0.5f;
}
void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  vec3 pixelPos = texture(WorldPosition_SpecPow_Texture, uvPos).xyz;
  vec4 worldPos = vec4(pixelPos,1);
  vec4 normalTextureData = texture(WorldNormal_ReceiveLight_Texture, uvPos);
  vec3 pixelNormal = normalTextureData.xyz*2-1;
  // if (pixelNormal == vec3(0,0,0))
    // discard;
  vec4 worldNormal = vec4(pixelNormal, 0);
  
  vFragColor.w = 1;
  if (DebugOutputIndex == DEBUG_OUTPUT_COMBINED)
  {
    vec3 pixelMatColor = vec3(0,0,0);
    if(EnableBlur != 0)//enable blur
      pixelMatColor = BlurScene(uvPos, pixelFrac);
    else
      pixelMatColor = texture(DiffuseColor_Empty_Texture, vec2((gl_FragCoord.xy) * pixelFrac)).xyz;
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
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
    vFragColor.xyz = texture(DiffuseColor_Empty_Texture, uvPos).xyz;
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
    vFragColor.xyz = texture(SpecColor_Empty_Texture, uvPos).xyz;
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_DEPTH)
  {
    vFragColor = texture(Depth_Texture, uvPos);
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_SHADOWMAP)
  {    
    vec4 shadowCoord = LightViewProj*worldPos;
    vec3 ProjCoords = shadowCoord.xyz / shadowCoord.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
    // float depth = texture(ShadowMaps_Texture, uvPos).x;

    
    float c = LightShadowExp;
    float depthExp = (texture(ShadowMaps_Texture, uvPos).xyz).r;
    float depth = log(depthExp)/c;

    vFragColor.xyz = vec3(depth);

    
    //float d= texture(ShadowMaps_Texture, uvPos).r;;
    //vFragColor.xyz = vec3(d);
  }
  else if (DebugOutputIndex == DEBUG_OUTPUT_SSAO)
  {
    vFragColor.xyz = vec3(texture(SSAO_Texture, uvPos).r);
  }
  

}












