//
//  Shaders.hlsl
//  Rayne
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

Texture2D texture0 : register(t0);
SamplerState linearRepeatSampler : register(s0);

cbuffer vertexUniforms : register(b0)
{
	float4 diffuseColor;
};

struct InputVertex
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoords : TEXCOORD0;
};

struct FragmentVertex
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

FragmentVertex pp_vertex(InputVertex vert)
{
	FragmentVertex result;
	result.position = float4(vert.position, 1.0f).xyww;
	result.texCoords = vert.texCoords * diffuseColor.zw + diffuseColor.xy;

	return result;
}

float4 pp_blit_fragment(FragmentVertex vert) : SV_TARGET
{
	float4 color = texture0.Sample(linearRepeatSampler, vert.texCoords).rgba;
	return color;
}
