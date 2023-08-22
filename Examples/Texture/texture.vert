#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 viewMatrix;
} ubo;

layout (binding = 1) uniform TUBO
{
	mat4 model;
	float lodBias;
} configUBO;

layout (location = 0) out vec2 outUV;
layout (location = 1) out float outLodBias;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	outUV = inUV;
	outLodBias = configUBO.lodBias;

	mat4 modelView = ubo.viewMatrix * configUBO.model;

	gl_Position = ubo.projection * modelView * vec4(inPos.xyz, 1.0);

	// calculate in view space
    vec4 pos = modelView * vec4(inPos, 1.0);

	outNormal = mat3(inverse(transpose(modelView))) * inNormal;
	
	vec3 lightPos = vec3(0.0);
	vec3 lPos = mat3(modelView) * lightPos.xyz;
    outLightVec = lPos - pos.xyz;

    outViewVec = ubo.viewMatrix[3] - pos.xyz;		
}
