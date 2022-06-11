#pragma once
#include "DirectXFramework.h"

class Graphics2 : public DirectXFramework
{
public:
	void CreateSceneGraph();
	void UpdateSceneGraph();

	float _rotationAngle;
};

