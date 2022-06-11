#include "Graphics2.h"
#include "MeshNode.h"
#include "TerrainNode.h"

Graphics2 app;

void Graphics2::CreateSceneGraph()
{
	GetCamera()->SetCameraPosition(0.0f, 0.0f, -50.0f);
	SceneGraphPointer sceneGraph = GetSceneGraph();
	shared_ptr<TerrainNode> terrainNode = make_shared<TerrainNode>(L"Terrain", L"rollinghills.raw", L"rollinghills.bmp");
	terrainNode->SetWorldTransform(XMMatrixTranslation(0, -300.0f, 0));
	shared_ptr<MeshNode> node = make_shared<MeshNode>(L"Plane1", L"airplane.x");
	node->SetWorldTransform(XMMatrixTranslation(-10.0f, 0, 0));
	_rotationAngle = 0;
	sceneGraph->Add(terrainNode);
	sceneGraph->Add(node);
}

void Graphics2::UpdateSceneGraph()
{
	SceneGraphPointer sceneGraph = GetSceneGraph();

	if (GetAsyncKeyState(VK_UP) < 0)
	{
		GetCamera()->SetForwardBack(1);
	}
	if (GetAsyncKeyState(VK_DOWN) < 0)
	{
		GetCamera()->SetForwardBack(-1);
	}
	if (GetAsyncKeyState(VK_LEFT) < 0)
	{
		GetCamera()->SetYaw(-1);
	}
	if (GetAsyncKeyState(VK_RIGHT) < 0)
	{
		GetCamera()->SetYaw(1);
	}
	if (GetAsyncKeyState(VK_PRIOR) < 0)
	{
		GetCamera()->SetPitch(-1);
	}
	if (GetAsyncKeyState(VK_NEXT) < 0)
	{
		GetCamera()->SetPitch(1);
	}
}
