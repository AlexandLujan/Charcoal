
// Data Type           ShaderBinding                    Description /Source
//
// Global Scene
// Settings            ConstantBuffer<SceneData>        Camera projectionmatrix, view matrix,
//                                                      ambient lighting, and global constants.
// Instance 
// Properties          StructuredBuffer <InstanceData>  A dynamic buffer containing a list of 
//                                                      World Matrices, one for every copy of the
//                                                      mesh you want todraw.
// Mesh Asset
// Data                StructuredBuffer<Meshlet>        The raw asset data(vertex streams, 
//                                                      index streams, and meshletmetadata)
//                                                      generated offline during your build step

#include "MyShaderCommon.hlsli"


#define ROOT_SIG "CBV(b0), \
                  RootConstants(b1, num32bitconstants=2), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3), \
                  SRV(t4), \
                  SRV(t5), \
                  SRV(t6), \
                  SRV(t7)"

ConstantBuffer<Constants> Globals : register(b0);
ConstantBuffer<MeshInfo> meshInfo : register(b1);
StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
StructuredBuffer<uint> UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);
StructuredBuffer<MaterialBuffer> materialProperty : register(t4);
StructuredBuffer<CullingData> cullingData : register(t5);
StructuredBuffer<Light> Lights : register(t6);
StructuredBuffer<InstanceData> InstanceBuffer : register(t7);



// Declare the payload in groupshared memory
//groupshared ThePayload payload;

bool IsVisible(InstanceData instance, CullingData c)
{
    //if ((instance.flags & CULL_FLAG) == 0)
    //   return true;

    //// Do a cull test of the bounding sphere against the view frustum planes.
    //float4 center = mul(float4(c.BoundingSphereCenter.xyz, 1), instance.worldMatrix);
    //float radius = c.BoundingSphereRadius * instance.scale;

    //for (int i = 0; i < 6; ++i)
    //{
    //    if (dot(center, Globals.Planes[i]) < -radius)
    //    {
    //        return false;
    //    }
    //}
    return true;
}

[RootSignature(ROOT_SIG)] 
[numthreads(1, 1, 1)] // 32 threads processing 32 instances at a time
void main(uint gthreadID : SV_GroupThreadID, uint groupID : SV_GroupID)
{
    ThePayload payload;
    // 1. Fetch instance data for this thread
    uint instanceID = (groupID * 32) + gthreadID;
    instanceID = groupID;
    InstanceData instance = InstanceBuffer[instanceID];

    // 2. Perform Culling (e.g., Frustum or Occlusion)
 //   bool isVisible = IsVisible(instance, cullingData[instanceID]);

    // 3. Compact visible instances using wave intrinsics
    // WaveActiveCountBits counts how many threads in the wave found a visible instance
 //   uint visibleCount = WaveActiveCountBits(isVisible);
    
    // Find where this specific thread should write in the payload array
 //   uint payloadSlot = WavePrefixCountBits(isVisible);

 //   if (isVisible)
  //  {
   //     s_Payload.InstanceIndices[payloadSlot] = instanceID;
  //  }

    // Initialize global payload data on the first thread
    if (gthreadID == 0)
    {
    }

    // 4. Dispatch the exact number of Mesh Shader groups needed
    // This tells the GPU how many Mesh Shader threadgroups to wake up
    payload.BaseMeshletID = instance.meshletIndex;
    payload.InstanceID = instanceID;
    payload.world = instance.worldMatrix;
    DispatchMesh(instance.meshletCount, 1, 1, payload);
}



////----------------------------------------------
//[numthreads(64, 1, 1)] // 64 threads to process vertex/primitive data
//void MS_Main(
//    uint gthreadID : SV_GroupThreadID,
//    uint msGroupID : SV_GroupID, // Maps directly to the visibleCount from DispatchMesh
//    in groupshared AmplificationPayload payload,
//    out indices uint3 prs[126],
//    out vertices VertexOutput verts[64])
//{
//    // 1. Read from the payload using the Mesh Shader's Group ID
//    // msGroupID ranges from 0 to (visibleCount - 1)
//    uint instanceID = payload.InstanceIndices[msGroupID];
//    InstanceData instance = InstanceBuffer[instanceID];

//    // 2. Determine which meshlet to load for this instance
//    uint meshletID = payload.BaseMeshletID + msGroupID;

//    // 3. Set the total allocation for this meshlet
//    uint vertexCount = GetMeshletVertexCount(meshletID);
//    uint triangleCount = GetMeshletTriangleCount(meshletID);
//    SetMeshOutputs(vertexCount, triangleCount);

//    // 4. Transform vertices using the matrix from our payload-selected instance
//    if (gthreadID < vertexCount)
//    {
//        Vertex v = GetVertex(meshletID, gthreadID);
        
//        // Multiply by the specific world matrix found via the payload index!
//        verts[gthreadID].Position = mul(float4(v.Position, 1.0f), instance.WorldMatrix);
//        verts[gthreadID].Position = mul(verts[gthreadID].Position, SceneBuffer.ViewProjection);
//        verts[gthreadID].UV = v.UV;
//    }

//    // 5. Output topology
//    if (gthreadID < triangleCount)
//    {
//        prs[gthreadID] = GetPrimitiveIndices(meshletID, gthreadID);
//    }
//}
