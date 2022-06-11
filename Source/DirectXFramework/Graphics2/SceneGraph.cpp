#include "SceneGraph.h"

void SceneGraph::Add(SceneNodePointer node) 
{
	_children.push_back(node);
};

void SceneGraph::Remove(SceneNodePointer node) 
{
	bool isPresent = false;
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator)
	{
		if ((*listIterator) == node)
		{
			isPresent = true;
		}
		while (listIterator != _children.end() && isPresent == true)
		{
			Remove(*listIterator);
		}
	};
};

SceneNodePointer  SceneGraph::Find(wstring name) 
{
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator)
	{
		if (_name == name)
		{
			return shared_from_this();
		}
	};
};

bool  SceneGraph::Initialise(void) 
{
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator) 
	{
		(*listIterator)->Initialise();
	};
	return true;
};

void  SceneGraph::Update(FXMMATRIX& currentWorldTransformation) 
{
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator) 
	{
		XMMATRIX transformation = XMLoadFloat4x4(&_worldTransformation) * currentWorldTransformation;
		(*listIterator)->Update(transformation);
	};
}; 

void  SceneGraph::Render(void) 
{
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator)
	{
		(*listIterator)->Render();
	};
};

void  SceneGraph::Shutdown(void) 
{
	for (list<SceneNodePointer>::iterator listIterator = _children.begin(); listIterator != _children.end(); ++listIterator)
	{
		(*listIterator)->Shutdown();
	};
};