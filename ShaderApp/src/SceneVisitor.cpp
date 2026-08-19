#include <pch.h>
#include <SceneVisitor.h>
#include <EffectPSO.h>
#include <Camera.h>
#include <CommandList.h>
#include <IndexBuffer.h>
#include <Material.h>
#include <Mesh.h>
#include <SceneNode.h>

#include <DirectXMath.h>

using namespace dx12lib;
using namespace DirectX;

SceneVisitor::SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, bool transparent)
    : m_CommandList(commandList)
    , m_Camera(camera)
    , m_LightingPSO(pso)
    , m_TransparentPass(transparent)
{
}

void SceneVisitor::Visit(dx12lib::Scene& scene)
{
    m_LightingPSO.SetViewMatrix(m_Camera.get_ViewMatrix());
    m_LightingPSO.SetProjectionMatrix(m_Camera.get_ProjectionMatrix());
}

void SceneVisitor::Visit(dx12lib::SceneNode& sceneNode)
{
    auto world = sceneNode.GetWorldTransform();
    m_LightingPSO.SetWorldMatrix(world);
}

void SceneVisitor::Visit(Mesh& mesh)
{
    auto material = mesh.GetMaterial();
    m_LightingPSO.SetMaterial(material);
    m_LightingPSO.Apply(m_CommandList);
    mesh.Draw(m_CommandList);
}