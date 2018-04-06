#version 430 core
#define MAX_SAMPLE_POINTS_NUM 64
#define PI 3.1415926535897932384626433832795

layout(location = 0) out float AOFactor;

uniform vec2 ScreenDimension;
uniform sampler2D WorldPosition_TexV_Texture;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D Depth_Texture;

uniform vec2 ControlVariable; 
uniform int SamplePointNum;
uniform float RangeOfInfluence;

vec3 SamplePoints[MAX_SAMPLE_POINTS_NUM];
float Depths[MAX_SAMPLE_POINTS_NUM];
void SelectSamplePoints(in vec3 worldPosition, in vec3 worldNormal,
 in float depth)
{
  int xp = int(gl_FragCoord.x);
  int yp = int(gl_FragCoord.y);
  
  float width = ScreenDimension.x;
  float height = ScreenDimension.y;
  
  float x = xp / width;
  float y = yp / height;

  int phi = (30*xp^yp) + 10*xp*yp;
  
  int n = SamplePointNum;
  float R = RangeOfInfluence;
  float d = depth;
  for (int i = 0; i < n; i++){
    float alpha = (i+0.5)/n;
    float h = alpha*R/d;
    float theta = 2*PI*alpha*(7*n/9)+phi;    
    vec2 uvPos = vec2(x,y) + h*vec2(cos(theta), sin(theta));    
    SamplePoints[i] = texture(WorldPosition_TexV_Texture, uvPos).xyz;  
    Depths[i] = texture(WorldPosition_TexV_Texture, uvPos).r;  
  }
}
void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  vec3 pixelPos = texture(WorldPosition_TexV_Texture, uvPos).xyz;
  vec3 pixelNormal = texture(WorldNormal_ReceiveLight_Texture, uvPos).xyz*2-1;
  
  vec4 P = vec4(pixelPos, 1);
  vec4 N = vec4(pixelNormal, 0);
  float d = texture(Depth_Texture, uvPos).r;
  
  SelectSamplePoints(P.xyz, N.xyz, d);
  
  int n = SamplePointNum;
  float R = RangeOfInfluence;  
  const float C = 0.1*R;
  const float delta = 0.001;
  
  float S = 2*PI*C/n;
  float sum = 0;  
  
  for (int i = 0; i < n; i++){
    vec3 omega = SamplePoints[i] - P.xyz;
    sum += (max(0, dot(N.xyz, omega)-delta*Depths[i])
    *step(length(omega), R))/max(C*C, dot(omega, omega));
  }
  S *= sum; 
  
  float s = ControlVariable.x;
  float k = ControlVariable.y;
  
  AOFactor = clamp(max(pow((1-s*S), k),0),0,1);
  gl_FragDepth = AOFactor;
  
}












