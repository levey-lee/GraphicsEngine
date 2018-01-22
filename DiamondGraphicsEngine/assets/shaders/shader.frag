#version 430 core

#define MaxLights 64 // maximum possible lights this shader supports
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

in vec4 worldNormal;
in vec3 VertexPosition;
in vec4 WorldPosition;
in vec2 Uv0;//regular uv 
in vec2 Uv1;//fixed border line

layout(location = 0) out vec4 vFragColor;


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

// represents material properties of the surface passed by the application
uniform struct
{
  bool ReceiveLight;
  vec4 AmbientColor; // ambient color of the surface/how much ambient light to absorb
  vec4 DiffuseColor; // diffuse color of the surface/how much diffuse light to absorb
  vec4 EmissiveColor;
  vec4 SpecularColor;
  vec4 SpecularExponent;
  
  bool DiffuseTextureEnabled;
  bool SpecularTextureEnabled;
  bool NormalMapTextureEnabled;
  sampler2D DiffuseTexture;
  sampler2D SpecularTexture;
  sampler2D NormalMapTexture;
} Material;

vec2 computeBoxUV()
{
  // pick UV based on maximal extents (this is not used in CS300 assignment 2,
  // it's merely another way to generate UVs)...only allow comps in [-1, 1]
  vec3 position = clamp(VertexPosition.xyz, vec3(-1), vec3(1));
  
  // find largest standard basis bias
  vec3 mag = abs(position);
  vec3 biasUVs = vec3(0.5) + 0.5 * position;
  if (mag.x > mag.y && mag.x > mag.z)
  {
    // facing pos or neg x axis; use corrected y/z for UV
    return biasUVs.yz;
  }
  else if (mag.y > mag.z)
  {
    // facing pos or neg y axis; use corrected x/z for UV
    return biasUVs.xz;
  }
  else // z is the largest
  {
    // facing pos or neg z axis; use corrected x/y for UV
    return biasUVs.xy;
  }
}
vec4 computeLightingTerm(in int lightIdx, in vec4 worldNormal)
{
  // grab light
  Light light = Lights[lightIdx];
    
    if (light.isActive == false)
      return vec4(0,0,0,1);
  // light vector points from the surface toward the light (opposite light dir.)
  vec4 lightVec;
  if  (light.type == LIGHT_TYPE_POINT)
  {
    lightVec = normalize(light.position - WorldPosition);
  }
  else
  {
    lightVec = normalize(-light.direction);
  }
  

  // ambient contribution from the light is always constant
  vec4 ambient = light.ambient * Material.AmbientColor;
  
  // initially, diffuse contribution is black
  vec4 diffuse = vec4(0); // same as vec4(0, 0, 0, 0), or black

  // compute the Lambertian term
  float diffuseFactor = dot(worldNormal, lightVec);

  if (diffuseFactor > 0) // is there a diffuse contribution?
  {
    // compute diffuse contribution on the surface
    diffuse = diffuseFactor * light.diffuse * Material.DiffuseColor;
  }
  return ambient + diffuse + Material.EmissiveColor; // total contribution from this light
}

vec4 computeSurfaceColor(in vec4 worldNormal)
{
  // Phong: total contribution of light is sum of all individual light contribs.
  vec4 color = vec4(0, 0, 0, 0); // no light = black
  for (int i = 0; i < LightCount; ++i)
    color += computeLightingTerm(i, worldNormal); // contribution of light i
  return color; // contribution from all lights onto surface
}


void main()
{
  vec4 lightColor;
  vec4 textureColor;
  vec4 materialColor = Material.AmbientColor+Material.DiffuseColor+Material.EmissiveColor;
  vec2 uv;
  uv.x = ( fwidth( Uv0.x ) < fwidth( Uv1.x )-0.001f )? Uv0.x : Uv1.x; 
  uv.y = ( fwidth( Uv0.y ) < fwidth( Uv1.y )-0.001f )? Uv0.y : Uv1.y;
  
  lightColor = Material.ReceiveLight ? computeSurfaceColor(worldNormal) : materialColor;
  textureColor = Material.DiffuseTextureEnabled ? texture(Material.DiffuseTexture, uv) : materialColor;
  //for now I use specular texture as a diffuse texture so 
  //that you can see they blend.
  //multiplicative blending.
  if (Material.SpecularTextureEnabled)
    textureColor*=texture(Material.SpecularTexture, uv).x;
    
  vFragColor = lightColor*textureColor;
}