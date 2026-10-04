RWStructuredBuffer<float4> clearOutput;

[shader("compute")]
[numthreads(8, 8, 1)]
void clearMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.y * 8 + threadId.x;
    clearOutput[index] = float4(0.0, 0.0, 0.0, 1.0);
}

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VertexOutput fullScreenVertex(uint vertexId : SV_VertexID)
{
    float2 position = float2(
        float((vertexId << 1) & 2),
        float(vertexId & 2)
    );

    VertexOutput output;
    output.position = float4(position * 2.0 - 1.0, 0.0, 1.0);
    output.uv = float2(position.x, 1.0 - position.y);
    return output;
}

[shader("fragment")]
float4 displayFragment(VertexOutput input) : SV_Target
{
    return float4(input.uv, 0.5, 1.0);
}
