struct SimulationUniforms
{
    float4 grid;
    float4 timing;
    float4 splatPosition;
    float4 splatVelocity;
    float4 splatColor;
    float4 display;
};

[[vk::binding(0, 0)]]
ConstantBuffer<SimulationUniforms> uniforms;

[[vk::binding(1, 0)]]
Texture2D<float4> sourceTexture;

[[vk::binding(2, 0)]]
Texture2D<float4> auxiliaryTexture;

[[vk::binding(3, 0)]]
[[vk::image_format("rgba16f")]]
WTexture2D<float4> outputTexture;

static int2 gridSize()
{
    return int2(uniforms.grid.xy);
}

static bool isOutside(int2 coordinate)
{
    int2 size = gridSize();
    return any(coordinate < int2(0, 0)) || any(coordinate >= size);
}

static int2 clampCoordinate(int2 coordinate)
{
    return clamp(coordinate, int2(0, 0), gridSize() - int2(1, 1));
}

static float4 loadSource(int2 coordinate)
{
    return sourceTexture.Load(int3(clampCoordinate(coordinate), 0));
}

static float4 loadAuxiliary(int2 coordinate)
{
    return auxiliaryTexture.Load(int3(clampCoordinate(coordinate), 0));
}

static float4 sampleSourceBilinear(float2 position)
{
    float2 samplePosition = position - 0.5;
    int2 lower = int2(floor(samplePosition));
    float2 fraction = frac(samplePosition);
    float4 bottom = lerp(
        loadSource(lower),
        loadSource(lower + int2(1, 0)),
        fraction.x
    );
    float4 top = lerp(
        loadSource(lower + int2(0, 1)),
        loadSource(lower + int2(1, 1)),
        fraction.x
    );
    return lerp(bottom, top, fraction.y);
}

static float splatFalloff(uint2 coordinate)
{
    float2 normalizedPosition = (float2(coordinate) + 0.5) * uniforms.grid.zw;
    float2 delta = normalizedPosition - uniforms.splatPosition.xy;
    delta.x *= uniforms.grid.x / uniforms.grid.y;
    float radius = max(uniforms.splatPosition.z, 0.0001);
    return exp(-dot(delta, delta) / (radius * radius));
}

static bool isDispatchOutside(uint3 threadId)
{
    return any(threadId.xy >= uint2(uniforms.grid.xy));
}

[shader("compute")]
[numthreads(8, 8, 1)]
void clearMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    outputTexture.Store(int2(threadId.xy), float4(0.0, 0.0, 0.0, 0.0));
}

[shader("compute")]
[numthreads(8, 8, 1)]
void splatVelocityMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    float4 velocity = loadSource(int2(threadId.xy));
    velocity.xy += uniforms.splatVelocity.xy * splatFalloff(threadId.xy);
    outputTexture.Store(int2(threadId.xy), velocity);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void splatDyeMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    float4 dye = loadSource(int2(threadId.xy));
    dye.rgb += uniforms.splatColor.rgb * splatFalloff(threadId.xy);
    outputTexture.Store(
        int2(threadId.xy),
        float4(max(dye.rgb, float3(0.0)), 1.0)
    );
}

[shader("compute")]
[numthreads(8, 8, 1)]
void advectVelocityMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    int2 coordinate = int2(threadId.xy);
    float4 velocity = loadSource(coordinate);
    float2 previousPosition =
        float2(coordinate) + 0.5 - uniforms.timing.x * velocity.xy;
    float4 advected = sampleSourceBilinear(previousPosition);
    float4 neighborAverage = 0.25 * (
        loadSource(coordinate + int2(-1, 0)) +
        loadSource(coordinate + int2(1, 0)) +
        loadSource(coordinate + int2(0, -1)) +
        loadSource(coordinate + int2(0, 1))
    );
    float viscosityBlend = saturate(uniforms.timing.w * uniforms.timing.x);
    advected = lerp(advected, neighborAverage, viscosityBlend);
    advected.xy *= uniforms.timing.y;

    if (
        coordinate.x == 0 ||
        coordinate.y == 0 ||
        coordinate.x == gridSize().x - 1 ||
        coordinate.y == gridSize().y - 1
    )
    {
        advected.xy = float2(0.0);
    }

    outputTexture.Store(
        int2(threadId.xy),
        float4(advected.xy, 0.0, 0.0)
    );
}

static float2 loadVelocityBoundary(int2 coordinate)
{
    if (isOutside(coordinate))
        return float2(0.0);

    return loadSource(coordinate).xy;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void divergenceMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    int2 coordinate = int2(threadId.xy);
    float left = loadVelocityBoundary(coordinate + int2(-1, 0)).x;
    float right = loadVelocityBoundary(coordinate + int2(1, 0)).x;
    float bottom = loadVelocityBoundary(coordinate + int2(0, -1)).y;
    float top = loadVelocityBoundary(coordinate + int2(0, 1)).y;
    float divergence = 0.5 * (right - left + top - bottom);
    outputTexture.Store(
        int2(threadId.xy),
        float4(divergence, 0.0, 0.0, 0.0)
    );
}

[shader("compute")]
[numthreads(8, 8, 1)]
void pressureJacobiMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    int2 coordinate = int2(threadId.xy);
    float left = loadSource(coordinate + int2(-1, 0)).x;
    float right = loadSource(coordinate + int2(1, 0)).x;
    float bottom = loadSource(coordinate + int2(0, -1)).x;
    float top = loadSource(coordinate + int2(0, 1)).x;
    float divergence = loadAuxiliary(coordinate).x;
    float pressure = 0.25 * (left + right + bottom + top - divergence);
    outputTexture.Store(
        int2(threadId.xy),
        float4(pressure, 0.0, 0.0, 0.0)
    );
}

[shader("compute")]
[numthreads(8, 8, 1)]
void subtractGradientMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    int2 coordinate = int2(threadId.xy);
    float left = loadAuxiliary(coordinate + int2(-1, 0)).x;
    float right = loadAuxiliary(coordinate + int2(1, 0)).x;
    float bottom = loadAuxiliary(coordinate + int2(0, -1)).x;
    float top = loadAuxiliary(coordinate + int2(0, 1)).x;
    float2 velocity =
        loadSource(coordinate).xy - 0.5 * float2(right - left, top - bottom);

    if (
        coordinate.x == 0 ||
        coordinate.y == 0 ||
        coordinate.x == gridSize().x - 1 ||
        coordinate.y == gridSize().y - 1
    )
    {
        velocity = float2(0.0);
    }

    outputTexture.Store(int2(threadId.xy), float4(velocity, 0.0, 0.0));
}

static float4 sampleAuxiliaryBilinear(float2 position)
{
    float2 samplePosition = position - 0.5;
    int2 lower = int2(floor(samplePosition));
    float2 fraction = frac(samplePosition);
    float4 bottom = lerp(
        loadAuxiliary(lower),
        loadAuxiliary(lower + int2(1, 0)),
        fraction.x
    );
    float4 top = lerp(
        loadAuxiliary(lower + int2(0, 1)),
        loadAuxiliary(lower + int2(1, 1)),
        fraction.x
    );
    return lerp(bottom, top, fraction.y);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void advectDyeMain(uint3 threadId : SV_DispatchThreadID)
{
    if (isDispatchOutside(threadId))
        return;

    int2 coordinate = int2(threadId.xy);
    float2 velocity = loadAuxiliary(coordinate).xy;
    float2 previousPosition =
        float2(coordinate) + 0.5 - uniforms.timing.x * velocity;
    float4 dye = sampleSourceBilinear(previousPosition);
    dye.rgb *= uniforms.timing.z;
    outputTexture.Store(
        int2(threadId.xy),
        float4(max(dye.rgb, float3(0.0)), 1.0)
    );
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
    float2 texturePosition = input.uv * uniforms.grid.xy;
    float3 dye = sampleSourceBilinear(texturePosition).rgb;
    float exposure = max(uniforms.display.x, 0.01);
    float3 mapped = 1.0 - exp(-dye * exposure);
    float vignette = 1.0 - 0.25 * dot(input.uv - 0.5, input.uv - 0.5);
    float3 background = float3(0.004, 0.006, 0.015);
    return float4(background + mapped * vignette, 1.0);
}
