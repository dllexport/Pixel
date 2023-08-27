#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inJoint;
layout (location = 4) in vec4 inWeight;
layout (location = 5) in vec4 inTangent;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} ubo;

layout (binding = 1) uniform GUBO
{
	mat4 model;
} gbufferUBO;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec4 outJoint;
layout (location = 3) out vec4 outWeight;
layout (location = 4) out vec4 outTangent;
layout (location = 5) out vec3 outWorldPos;

void main() 
{
	gl_Position = ubo.projection * ubo.view * gbufferUBO.model * vec4(inPos, 1.0);
	
	outUV = inUV;
	
	outJoint = inJoint;
	outWeight = inWeight;

	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(gbufferUBO.model)));
	outNormal = mNormal * normalize(inNormal.xyz);	
	outTangent = vec4(mNormal * normalize(vec3(inTangent)), 1.0f);

	// Vertex position in world space
	outWorldPos = vec3(gbufferUBO.model * vec4(inPos, 1.0));
}
