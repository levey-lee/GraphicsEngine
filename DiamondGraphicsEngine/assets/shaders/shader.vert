#version 430 core


// These two must perfectly match the structure defined in inc/graphics/Vertex.h
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUv;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

out vec4 WorldNormal;
out vec3 VertexPosition;
out vec4 WorldPosition;

out vec2 Uv0;//regular uv 
out vec2 Uv1;//fixed border line

out mat4 TBN;

uniform mat4 ModelMatrix; // local->world matrix
uniform mat4 ModelViewProjectionMatrix; // local->NDC matrix [no camera support]


void main()
{
  VertexPosition = vPosition;
  Uv0.x = fract( vUv.x );
  Uv0.y = fract( vUv.y );
  
  Uv1.x = fract( vUv.x + 0.5f ) - 0.5f; 
  Uv1.y = fract( vUv.y + 0.5f ) - 0.5f; 
  
  
  vec4 fragTan = ModelMatrix * vec4(vTangent, 0);
	vec4 fragBitan = ModelMatrix * vec4(vBitangent, 0);
	vec4 fragNormal = ModelMatrix * vec4(vNormal, 0);
  TBN = transpose(mat4(fragTan, fragBitan, fragNormal, vec4(0, 0, 0, 1)));  
  
  
  
  // deal with position and normal in world space  
  WorldPosition = ModelMatrix * vec4(vPosition, 1);

  // vec4(vNormal, 0) because we don't want to translate a normal;
  // NOTE: this code is wrong if we support non-uniform scaling
  WorldNormal = normalize(ModelMatrix * vec4(vNormal, 0));

  
  // compute the final result of passing this vertex through the transformation
  // pipeline and yielding a coordinate in NDC space  
  gl_Position = ModelViewProjectionMatrix * vec4(vPosition, 1);
  
}