#pragma once
#include "SceneNode.h"
#include <iostream>
#include <list>

class SceneGraph : public SceneNode
{
public:
	typedef list<SceneNodePointer> sceneNodeList;

	SceneGraph() : SceneNode(L"Root") {};
	SceneGraph(wstring name) : SceneNode(name) {};
	~SceneGraph(void) {};

	virtual bool Initialise(void);
	virtual void Update(FXMMATRIX& currentWorldTransformation);
	virtual void Render(void);
	virtual void Shutdown(void);

	void Add(SceneNodePointer node); 
	void Remove(SceneNodePointer node);
	SceneNodePointer Find(wstring name);

private:

	sceneNodeList		_children;
};

typedef shared_ptr<SceneGraph>			 SceneGraphPointer;
