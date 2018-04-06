#version 430 core
#define MAX_SAMPLE_POINTS_NUM 64
#define c_PI 3.1415926535897932384626433832795
#define c_2PI 2*c_PI

uniform vec2 ScreenDimension;
uniform sampler2D WorldPosition_TexV_Texture;
uniform sampler2D WorldNormal_ReceiveLight_Texture;
uniform sampler2D Depth_Texture;

uniform vec2 ControlVariable; 
uniform int SamplePointNum;
uniform float RangeOfInfluence;
uniform int UseSpiralAlgorithm;

vec3 SamplePoints[MAX_SAMPLE_POINTS_NUM];
float Depths[MAX_SAMPLE_POINTS_NUM];
float SSAO_Spiral(in vec3 worldPosition, in vec3 worldNormal,
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
      float alpha = (i+0.5f)/n;
      float h = alpha*R/d;
      float theta = c_2PI*alpha*(7*n/9)+phi;    
      vec2 uvPos = vec2(x,y) + h*vec2(cos(theta), sin(theta));  
      //vec2 uvPos = vec2(x,y) ;  
      SamplePoints[i] = texture(WorldPosition_TexV_Texture, uvPos).xyz;  
      Depths[i] = texture(Depth_Texture, uvPos).r;  
    }
    
    const float C = 0.1f*R;
    const float Csqr = C*C;
    const float delta = 0.001f;
    
    float S = c_2PI*C/n;
    float sum = 0;  
    
    for (int i = 0; i < n; i++){
      vec3 omega = SamplePoints[i] - worldPosition;
      sum += (max(0, dot(worldNormal, omega)-delta*Depths[i])
      *step(length(omega), R))/max(Csqr, dot(omega, omega));
    }
    S *= sum; 
    
    float s = ControlVariable.x;
    float k = ControlVariable.y;
  return clamp(max(pow((1-s*S), k),0),0,1);
}
void main()
{
  vec2 pixelFrac = vec2(1.0f/ScreenDimension.x,1.0f/ScreenDimension.y );
  vec2 uvPos = vec2(gl_FragCoord.xy * pixelFrac);
  vec3 pixelPos = texture(WorldPosition_TexV_Texture, uvPos).xyz;
  vec3 pixelNormal = texture(WorldNormal_ReceiveLight_Texture, uvPos).xyz*2-1;
  
  float depth = texture(Depth_Texture, uvPos).r;
  
  float factor = 1;
  
  if (UseSpiralAlgorithm != 0)
    factor = SSAO_Spiral(pixelPos, pixelNormal,depth);
  

  gl_FragDepth = factor;
  
}












