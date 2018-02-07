#version 430 core

#define MaxLights 64 // maximum possible lights this shader supports
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

in vec4 WorldNormal;
in vec3 VertexPosition;
in vec4 WorldPosition;
in vec2 Uv0;//regular uv 
in vec2 Uv1;//fixed border line
in mat4 TBN;
layout(location = 0) out vec4 vFragColor;


// only support directional lights for now
struct Light
{
  bool isActive;
  vec4 position;
  vec4 direction; // direction the light is directed
  vec4 ambient; // ambient light cast onto objects
  vec4 diffuse; // diffuse light cast onto objects
  float innerAngle;
  float outerAngle;
  float spotFalloff;
  vec4 specular;
  int type;
  float intensity;
  vec3 distanceAttenuation;
};

uniform Light Lights[MaxLights]; // support UP TO 64 lights
uniform int LightCount; // number of lights enabled THIS ROUND

// represents material properties of the surface passed by the application
uniform struct
{
  bool ReceiveLight;
  vec4 AmbientColor; // ambient color of the surface/how much ambient light to absorb
  vec4 DiffuseColor; // diffuse color of the surface/how much diffuse light to absorb
  vec4 EmissiveColor;
  vec4 SpecularColor;
  float SpecularExponent;
  
  bool DiffuseTextureEnabled;
  bool SpecularTextureEnabled;
  bool NormalMapTextureEnabled;
  sampler2D DiffuseTexture;
  sampler2D SpecularTexture;
  sampler2D NormalMapTexture;
} Material;

uniform struct
{
  vec3 ViewDir_world;
  vec3 Position_world;
  float FarPlaneDist;
  float NearPlaneDist;
  vec4 FogColor;
}Camera;

vec4 DoPointLight(in Light light, in vec4 worldNormal)
{
  return vec4(0,0,0,1);
}
vec4 DoSpotLight(in Light light, in vec4 worldNormal)
{
  return vec4(0,0,0,1);
}
vec4 DoDirectionalLight(in Light light, in vec4 worldNormal)
{
  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec = normalize(-light.direction);
  
  vec4 viewVec = WorldPosition - vec4(Camera.Position_world, 1);

  // ambient contribution from the light is always constant
  vec4 ambient = light.ambient * Material.AmbientColor;
  
  // compute diffuse contribution on the surface
  vec4 diffuse = max(dot(worldNormal, lightVec), 0) * light.diffuse * Material.DiffuseColor;
  
  
  vec4 specular = light.specular
                * Material.SpecularColor
                * pow(max(dot(reflect(lightVec, worldNormal),viewVec),0),Material.SpecularExponent);
  return light.intensity*(ambient + diffuse + specular); // total contribution from this light
}
vec4 computeLightingTerm(in int lightIdx, in vec4 worldNormal)
{
  // grab light
  Light light = Lights[lightIdx];
  
  vec4 lightColor = vec4(0,0,0,1);
  if (light.isActive == false)
    return vec4(0,0,0,1);
  ////////////////////////////////////////////////////////
  //    Calculate Surface Color
  ////////////////////////////////////////////////////////
    
  if(light.type == LIGHT_TYPE_POINT)
    lightColor = DoPointLight(light, worldNormal);
    
        
  else if(light.type == LIGHT_TYPE_SPOT)
    lightColor = DoSpotLight(light, worldNormal);
    
        
  else if(light.type == LIGHT_TYPE_DIRECTIONAL)
    lightColor = DoDirectionalLight(light, worldNormal);
  
  ////////////////////////////////////////////////////////
  //    Calculate Fog Color
  ////////////////////////////////////////////////////////
  
  vec4 viewVec = WorldPosition - vec4(Camera.Position_world, 1);
  float fogFactor = (Camera.FarPlaneDist - length(viewVec))/(Camera.FarPlaneDist - Camera.NearPlaneDist);
  lightColor = fogFactor*lightColor + (1-fogFactor)*Camera.FogColor;
    
  
  ////////////////////////////////////////////////////////
  //    Return the final surface color
  ////////////////////////////////////////////////////////
  return lightColor;
}

vec4 computeLightColor(in vec4 worldNormal, vec2 uv)
{
  vec4 normal = worldNormal;
  if (Material.NormalMapTextureEnabled)
  {
    normal= normalize(inverse(TBN) * (vec4(texture(Material.NormalMapTexture, uv).rgb, 0) * 2 - 1));
  }
  // Phong: total contribution of light is sum of all individual light contribs.
  vec4 color = vec4(0, 0, 0, 0); // no light = black
  for (int i = 0; i < LightCount; ++i)
    color += computeLightingTerm(i, normal); // contribution of light i
  return color + Material.EmissiveColor; // contribution from all lights onto surface
}


void main()
{
  vec4 lightColor;
  vec4 textureColor;
  vec2 uv;
  uv.x = ( fwidth( Uv0.x ) < fwidth( Uv1.x )-0.001f )? Uv0.x : Uv1.x; 
  uv.y = ( fwidth( Uv0.y ) < fwidth( Uv1.y )-0.001f )? Uv0.y : Uv1.y;
  
  lightColor = Material.ReceiveLight ? computeLightColor(WorldNormal, uv) : vec4(1,1,1,1);
  textureColor = Material.DiffuseTextureEnabled ? texture(Material.DiffuseTexture, uv) : vec4(1,1,1,1);
  //for now I use specular texture as a diffuse texture so 
  //that you can see they blend.
  //multiplicative blending.
  // if (Material.SpecularTextureEnabled)
    // textureColor*=texture(Material.SpecularTexture, uv).x;
    
  vFragColor = vec4(lightColor*textureColor).rgba;
  
}