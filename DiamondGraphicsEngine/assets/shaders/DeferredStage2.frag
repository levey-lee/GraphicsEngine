#version 430 core

#define MaxLights 64 // maximum possible lights this shader supports
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define DEBUG_OUTPUT_COMBINED 0
#define DEBUG_OUTPUT_DIFFUSE 1
#define DEBUG_OUTPUT_WORLD_POSITION 2
#define DEBUG_OUTPUT_WORLD_NORMAL 3
#define DEBUG_OUTPUT_SPECULAR_COLOR 4
#define DEBUG_OUTPUT_DEPTH 5

layout(location = 0) out vec4 vFragColor;

uniform int DebugOutputIndex;
uniform vec2 ScreenDimension;
uniform vec3 LightPosition;
uniform sampler2D DiffuseColor_TexU_Texture;
uniform sampler2D WorldPosition_TexV_Texture;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D SpecColor_SpecPow_Texture;
uniform sampler2D Depth_Texture;
// only support directional lights for now
struct Light
{
  bool isActive;
  vec4 position;
  vec4 direction; // direction the light is directed
  vec4 ambient; // ambient light cast onto objects
  vec4 diffuse; // diffuse light cast onto objects
  vec4 specular;
  int type;
  float intensity;
  vec3 distanceAttenuation;
};

uniform Light Lights[MaxLights]; // support UP TO 64 lights
uniform int LightCount; // number of lights enabled THIS ROUND
vec4 computeLightingTerm(in int lightIdx, in vec4 worldNormal, in vec4 worldPos)
{
  vec4 normal = worldNormal;
  // grab light
  Light light = Lights[lightIdx];
  
  if (light.isActive == false)
    return vec4(0,0,0,1);

  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec;
  if  (light.type == LIGHT_TYPE_POINT)
  {
    lightVec = normalize(light.position - worldPos);
  }
  else
  {
    lightVec = normalize(-light.direction);
  }
  

  // ambient contribution from the light is always constant
  vec4 ambient = light.ambient;
  
  // initially, diffuse contribution is black
  vec4 diffuse = vec4(0); // same as vec4(0, 0, 0, 0), or black

  // compute the Lambertian term
  float diffuseFactor = dot(normal, lightVec);

  if (diffuseFactor > 0) // is there a diffuse contribution?
  {
    // compute diffuse contribution on the surface
    diffuse = diffuseFactor * light.diffuse;
  }
  return ambient + diffuse; // total contribution from this light
}

vec4 computeSurfaceColor(in vec4 worldNormal, vec4 worldPos)
{
  // Phong: total contribution of light is sum of all individual light contribs.
  vec4 color = vec4(0, 0, 0, 0); // no light = black
  for (int i = 0; i < LightCount; ++i)
    color += computeLightingTerm(i, worldNormal, worldPos); // contribution of light i
  return color; // contribution from all lights onto surface
}

void main()
{
  vec2 pixelSize = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelSize);
  vec3 pixelPos = texture(WorldPosition_TexV_Texture, uvPos).xyz;
  vec4 worldPos = vec4(pixelPos,1);
  vec4 normalTextureDatanormalize = texture(WorldNormal_ReceiveLight_Texture, uvPos);
  vec3 pixelNormal = normalTextureDatanormalize.xyz*2-1;
  if (pixelNormal == vec3(0,0,0))
    discard;
  vec4 worldNormal = vec4(pixelNormal, 0);
  
  vec3 LightPosition = vec3(0,0,1);
  vec3 LookAtLightSource = 
  normalize(LightPosition - pixelPos);
  vFragColor.w = 1;
  if (DebugOutputIndex == DEBUG_OUTPUT_COMBINED)
  {
    vec3 pixelMatColor = texture(DiffuseColor_TexU_Texture, uvPos).xyz;    
    if (normalTextureDatanormalize.w > 0.5)
    {
      vec3 lightColor = computeSurfaceColor(worldNormal, worldPos).xyz;
      vFragColor.xyz = pixelMatColor*lightColor;
    }
    else
      vFragColor.xyz = pixelMatColor;    
  }
  if (DebugOutputIndex == DEBUG_OUTPUT_DIFFUSE)
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
  
  
  
}
