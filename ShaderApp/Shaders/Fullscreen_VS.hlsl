struct VSOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput OUT;

    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f)
    };

    float2 uvs[3] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, -1.0f),
        float2(2.0f, 1.0f)
    };

    OUT.Position = float4(
        positions[vertexID],
        0.0f,
        1.0f
    );

    OUT.TexCoord = uvs[vertexID];

    return OUT;
}