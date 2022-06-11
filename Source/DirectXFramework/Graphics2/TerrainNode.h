#pragma once
#include "Renderer.h"
#include "DirectXFramework.h"
#include "SceneNode.h"
#include "WICTextureLoader.h"
#include <iostream>
#include <fstream>  
#include <list>
#include <vector>

struct POLYGON
{
	int v1;
	int v2;
	int v3;
	XMFLOAT3 normal;
};

class TerrainNode : public SceneNode
{
public:
	TerrainNode(wstring terrainNode, wstring fileName, wstring texture) : SceneNode(terrainNode) { _fileName = fileName; _textureFile = texture; };
	void SetWorldTransformation(FXMMATRIX worldTransformation);
	void SetAmbientLight(XMFLOAT4 ambientLight);
	void SetDirectionalLight(FXMVECTOR lightVector, XMFLOAT4 lightColour);
	void SetCameraPosition(XMFLOAT4 cameraPosition);
	bool Initialise();
	void Render();
	void Shutdown(void);

private:
	std::vector<VERTEX>				vertices;
	std::vector<UINT>				indices;
	std::vector<int>				counter;
	std::vector<float>				_heightValues;
	std::vector<POLYGON>			polygons;
	XMFLOAT4X4						_worldTransformation;
	XMFLOAT4						_ambientLight;
	XMFLOAT4						_directionalLightVector;
	XMFLOAT4						_directionalLightColour;
	XMFLOAT4						_cameraPosition;

	ComPtr<ID3D11Device>			_device;
	ComPtr<ID3D11DeviceContext>		_deviceContext;

	ComPtr<ID3D11Buffer>			_vertexBuffer;
	ComPtr<ID3D11Buffer>			_indexBuffer;

	ComPtr<ID3DBlob>				_vertexShaderByteCode = nullptr;
	ComPtr<ID3DBlob>				_pixelShaderByteCode = nullptr;
	ComPtr<ID3D11VertexShader>		_vertexShader;
	ComPtr<ID3D11PixelShader>		_pixelShader;
	ComPtr<ID3D11InputLayout>		_layout;
	ComPtr<ID3D11Buffer>			_constantBuffer;

	ComPtr<ID3D11RasterizerState>	_defaultRasteriserState;
	ComPtr<ID3D11RasterizerState>	_wireframeRasteriserState;

	wstring							_textureFile;
	wstring							_fileName;
	shared_ptr<ResourceManager>		_resourceManager;

	ComPtr<ID3D11ShaderResourceView> _texture;
	shared_ptr<Material>			 _material;

	float rows = 257;
	float cols = 257;

	void BuildRendererStates();
	void BuildTerrainBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void CalculateNormals(POLYGON _polygon);
	bool LoadHeightMap(wstring heightMapFilename);
};