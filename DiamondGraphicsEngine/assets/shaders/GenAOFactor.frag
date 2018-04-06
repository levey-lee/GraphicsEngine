#version 430 core
#define MAX_SAMPLE_POINTS_NUM 64
#define c_PI 3.1415926535897932384626433832795
#define c_2PI 2*c_PI

const vec3 sample_sphere[MAX_SAMPLE_POINTS_NUM] = {
    vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
    vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
    vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
    vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
    vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
    vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
    vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
    vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271),
    vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
    vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
    vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
    vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
    vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
    vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
    vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
    vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271),
    vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
    vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
    vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
    vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
    vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
    vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
    vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
    vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271),
    vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
    vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
    vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
    vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
    vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
    vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
    vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
    vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
};

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

vec3 NormalFromDepth(float depth, in vec2 uvPos) {
  
  const vec2 offset1 = vec2(0.0,0.001);
  const vec2 offset2 = vec2(0.001,0.0);
  
  float depth1 = texture(Depth_Texture, uvPos + offset1).r;
  float depth2 = texture(Depth_Texture, uvPos + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}
float DepthBasedSSAO(in vec2 uvPos)
{
  const float total_strength = ControlVariable.x;
  const float base = ControlVariable.y;
  const float area = 0.075f;
  const float falloff = 0.01f*RangeOfInfluence;
  const float radius = RangeOfInfluence;
  const int samples = SamplePointNum;
  vec3 random = normalize( texture(WorldPosition_TexV_Texture, uvPos * 4.0).rgb );
  float depth = texture(Depth_Texture, uvPos).r;
  vec3 position = vec3(uvPos, depth);
  vec3 normal = NormalFromDepth(depth, uvPos);
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i=0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(sample_sphere[i], random);
    vec3 hemi_ray = position + sign(dot(ray,normal)) * ray;
    
    float occ_depth = texture(Depth_Texture, clamp(hemi_ray.xy,0,1)).r;
    float difference = depth - occ_depth;
    
    occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
  }
  return clamp(1.0 - total_strength * occlusion * (1.0 / samples) + base,0,1);
}
float RandomBasedSSAO(in vec3 worldPosition, in vec3 worldNormal,
 in float depth)
{
    int xp = int(gl_FragCoord.x);
    int yp = int(gl_FragCoord.y);
    
    float width = ScreenDimension.x;
    float height = ScreenDimension.y;
    
    float x = xp / width;
    float y = yp / height;
      
    int n = SamplePointNum;
    float R = RangeOfInfluence;
    float d = depth;
    
    for (int i = 0; i < n; i++){ 
      vec2 uvPos = vec2(x,y) ;  
      
      SamplePoints[i] = worldPosition + 
      normalize(worldPosition+sample_sphere[i])*R;  
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
  else
    factor = RandomBasedSSAO(pixelPos, pixelNormal,depth);
  gl_FragDepth = factor;
  
}












