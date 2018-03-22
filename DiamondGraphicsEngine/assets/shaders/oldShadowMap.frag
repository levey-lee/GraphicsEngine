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

const float C = 100.0f;
float CalcShadowFactor(in vec4 worldPos, in vec2 uv)
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
  float z = map_01(distance(worldPos.xyz, Lights[0].position.xyz), 10, 15);
  float lit = step(z, depth);
  return lit;
  
#else//ESM
  mat4 biasMatrix = mat4(vec4(0.5f,0,0,0),
                         vec4(0,0.5f,0,0),
                         vec4(0,0,0.5f,0),
                         vec4(0.5f,0.5f,0.5f,1));

  vec4 shadowCoord = biasMatrix*LightViewProj*worldPos;
  vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;
  if (shadowCoord.w > 0){
    if (shadowIndex.x >= 0 &&  shadowIndex.x <= 1){
      if (shadowIndex.y >= 0 &&  shadowIndex.y <= 1){
        float lightDepth =  texture(ShadowMaps_Texture, shadowIndex).w;
        float pixelDepth = shadowCoord.w;
        if (pixelDepth > lightDepth){
          return 0;
        }

      }
    }
  }
  return 1;
  
  
  //float p = textureProj(ShadowMaps_Texture, shadowCoord.xyw).r;
  // float z = map_01(distance(worldPos.xyz, Lights[0].position.xyz), 10, 15);
  // float visibility = clamp(exp(-C*(z-p)),0,1);
  // return visibility;
  // return clamp(p*exp( -C * z),0,1);
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
  
  vec4 specFactor=texture(SpecColor_SpecPow_Texture, uv);
  
  vec4 specular = light.specular
                * vec4(specFactor.xyz, 1)
                * pow(max(dot(reflectVec.xyz,viewVec.xyz),0),specFactor.w);

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
  
  vec4 specFactor=texture(SpecColor_SpecPow_Texture, uv);
  
  vec4 specular = light.specular
                * vec4(specFactor.xyz, 1)
                * pow(max(dot(reflect(lightVec, worldNormal),viewVec),0),specFactor.w);


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
  
  // vec4 viewVec = worldPos - vec4(Camera.Position_world, 1);
  // float fogFactor = (Camera.FarPlaneDist - length(viewVec))/(Camera.FarPlaneDist - Camera.NearPlaneDist);
  // lightColor = fogFactor*lightColor + (1-fogFactor)*Camera.FogColor;
    
  
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
  // if (pixelNormal == vec3(0,0,0))
    // discard;
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

    
    float depthExp = (texture(ShadowMaps_Texture, uvPos).xyz).r;
    float depth = log(depthExp)/C;

    vFragColor.xyz = vec3(depth);

    
    float d= texture(ShadowMaps_Texture, uvPos).r;;
    vFragColor.xyz = vec3(d);
    //vFragColor.xyz = vec3(z);
    //vFragColor.xyz = vec3(CalcShadowFactor(worldPos));
  }
  

}












