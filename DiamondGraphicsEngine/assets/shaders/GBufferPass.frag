#version 430 core

in vec4 WorldNormal;
in vec4 WorldPosition;
in vec2 Uv0;//regular uv 
in vec2 Uv1;//fixed border line
in mat4 TBN;

// layout(location = 0) out vec4 vFragColor;
layout(location = 0) out vec4 vDiffuseColor_Empty;
layout(location = 1) out vec4 vWorldPosition_SpecPow;
layout(location = 2) out vec4 vWorldNormal_ReceiveLight;
layout(location = 3) out vec4 vSpecColor_Empty;

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

void main()
{
  vec2 uv;
  uv.x = ( fwidth( Uv0.x ) < fwidth( Uv1.x )-0.001f )? Uv0.x : Uv1.x; 
  uv.y = ( fwidth( Uv0.y ) < fwidth( Uv1.y )-0.001f )? Uv0.y : Uv1.y;

  vec4 materialColor = Material.AmbientColor + Material.DiffuseColor + Material.EmissiveColor;  
  vec4 textureColor  = Material.DiffuseTextureEnabled ? texture(Material.DiffuseTexture, uv) : vec4(1,1,1,1);

  //layout 0
  vDiffuseColor_Empty.xyz = clamp(vec3(materialColor.xyz)*vec3(textureColor.xyz), 0, 1);
  //vDiffuseColor_TexU.w = uv.x;
   vDiffuseColor_Empty.w = 1;

  //layout 1
  vWorldPosition_SpecPow.xyz = WorldPosition.xyz;
  //vWorldPosition_TexV.w = uv.y;
   vWorldPosition_SpecPow.w = Material.SpecularExponent;


  //layout 2
  if (Material.NormalMapTextureEnabled)
  {
    vWorldNormal_ReceiveLight = normalize(inverse(TBN) * (vec4(texture(Material.NormalMapTexture, uv).rgb, 0) * 2 - 1));
  }
  else
  {
    vWorldNormal_ReceiveLight.xyz = normalize(WorldNormal.xyz);
  }
  vWorldNormal_ReceiveLight.xyz = (vWorldNormal_ReceiveLight.xyz+1)*0.5f;
  if (Material.ReceiveLight)
  {
    vWorldNormal_ReceiveLight.w = 1.0f;
  }
  else
  {
    vWorldNormal_ReceiveLight.w = 0.0f;
  }
  //layout 3
  if (Material.SpecularTextureEnabled)
  {
    vSpecColor_Empty.xyz = texture(Material.SpecularTexture, uv).xyz;
  }
  else
  {
    vSpecColor_Empty.xyz = Material.SpecularColor.xyz;
  }
}
