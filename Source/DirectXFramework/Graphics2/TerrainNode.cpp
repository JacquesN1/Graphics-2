#include "TerrainNode.h"
#include "ResourceManager.h"

struct CBUFFER
{
	XMMATRIX    CompleteTransformation;
	XMMATRIX	WorldTransformation;
	XMFLOAT4	CameraPosition;
	XMVECTOR    LightVector;
	XMFLOAT4    LightColor;
	XMFLOAT4    AmbientColor;
	XMFLOAT4    DiffuseColor;
	XMFLOAT4	SpecularColor;
	float		Shininess;
	float       Padding[3];
};

void TerrainNode::SetWorldTransformation(FXMMATRIX worldTransformation)
{
	XMStoreFloat4x4(&_worldTransformation, worldTransformation); 
}

void TerrainNode::SetAmbientLight(XMFLOAT4 ambientLight)
{
	_ambientLight = ambientLight;
}

void TerrainNode::SetDirectionalLight(FXMVECTOR lightVector, XMFLOAT4 lightColour)
{
	_directionalLightColour = lightColour;
	XMStoreFloat4(&_directionalLightVector, lightVector);
}

void TerrainNode::SetCameraPosition(XMFLOAT4 cameraPosition)
{
	_cameraPosition = cameraPosition;
}

bool TerrainNode::Initialise()
{
	_resourceManager = DirectXFramework::GetDXFramework()->GetResourceManager();
	_device = DirectXFramework::GetDXFramework()->GetDevice();
	_deviceContext = DirectXFramework::GetDXFramework()->GetDeviceContext();
	BuildRendererStates();
	LoadHeightMap(_fileName);
	BuildTerrainBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffer();
	return true;
}

void TerrainNode::Render()
{
	SetWorldTransformation(XMLoadFloat4x4(&_combinedWorldTransformation));
	SetCameraPosition(XMFLOAT4(0.0f, 0.0f, -25.0f, 0.0f));
	SetAmbientLight(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	SetDirectionalLight(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	XMMATRIX projectionTransformation = DirectXFramework::GetDXFramework()->GetProjectionTransformation();
	XMMATRIX viewTransformation = DirectXFramework::GetDXFramework()->GetCamera()->GetViewMatrix();

	XMMATRIX completeTransformation = XMLoadFloat4x4(&_worldTransformation) * viewTransformation * projectionTransformation;

	// Draw the first cube
	CBUFFER _cBuffer;
	_cBuffer.CompleteTransformation = completeTransformation;
	_cBuffer.WorldTransformation = XMLoadFloat4x4(&_worldTransformation);
	_cBuffer.AmbientColor = _ambientLight;
	_cBuffer.LightVector = XMVector4Normalize(XMLoadFloat4(&_directionalLightVector));
	_cBuffer.LightColor = _directionalLightColour;
	_cBuffer.CameraPosition = _cameraPosition;
	_resourceManager->CreateMaterialFromTexture(_textureFile);
	shared_ptr<Material> material = _resourceManager->GetMaterial(L"rollinghills.bmp");
	_cBuffer.DiffuseColor = material->GetDiffuseColour();
	_cBuffer.SpecularColor = material->GetSpecularColour();
	_cBuffer.Shininess = material->GetShininess();

	// Update the constant buffer 
	_deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, 0, &_cBuffer, 0, 0);

	_deviceContext->VSSetShader(_vertexShader.Get(), 0, 0);
	_deviceContext->PSSetShader(_pixelShader.Get(), 0, 0);
	_deviceContext->IASetInputLayout(_layout.Get());

	// Now render the cube
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	_deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	_texture = material->GetTexture();
	_deviceContext->PSSetShaderResources(0, 1, _texture.GetAddressOf());
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//_deviceContext->RSSetState(_wireframeRasteriserState.Get());
	_deviceContext->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}

void TerrainNode::Shutdown(void){}

void TerrainNode::BuildTerrainBuffers()
{
	float squareWidth = 10.0;
	float _x = ((0 - ((cols - 1) / 2)) * squareWidth);
	float _z = ((0 - ((rows - 1) / 2)) * squareWidth);
	float textureX = 0;
	float textureZ = 0;
	float texX = 1.0 / cols;
	float texZ = 1.0 / rows;

	VERTEX vertex;
	vertex.Colour = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	vertex.Normal = XMFLOAT3(0, 0, 0);
	vertex.TexCoord = XMFLOAT2(0, 0);

	for (int i = 0; i < cols; i++)
	{
		for (int _i = 0; _i < rows; _i++)
		{
			vertex.Position = XMFLOAT3(_x, 0, _z);
			vertex.TexCoord = XMFLOAT2(textureX, textureZ);
			vertices.push_back(vertex);
			counter.push_back(0);
			_z = _z + squareWidth;
			textureZ = textureZ + texZ;

		}
		_x = _x + squareWidth;
		_z = ((0 - ((rows - 1) / 2)) * squareWidth);
		textureX = textureX + texX;
		textureZ = 0;
	}

	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].Position.y = (_heightValues[i] * 256);
	}

	for (int i = 0; i < vertices.size(); i++)
	{
		if ((i + rows < vertices.size()) && ((i + 1) % (int)rows != 0))
		{
			POLYGON polygon;
			polygon.v3 = i;
			polygon.v2 = i + rows;
			polygon.v1 = i + 1;
			counter[i]++;
			counter[i + rows]++;
			counter[i + 1]++;

			indices.push_back((UINT)(polygon.v1));
			indices.push_back((UINT)(polygon.v2));
			indices.push_back((UINT)(polygon.v3));

			XMFLOAT3 a(vertices[polygon.v1].Position.x - vertices[polygon.v2].Position.x, vertices[polygon.v1].Position.y - vertices[polygon.v2].Position.y, vertices[polygon.v1].Position.z - vertices[polygon.v2].Position.z);
			XMFLOAT3 b(vertices[polygon.v1].Position.x - vertices[polygon.v3].Position.x, vertices[polygon.v1].Position.y - vertices[polygon.v3].Position.y, vertices[polygon.v1].Position.z - vertices[polygon.v3].Position.z);
			XMFLOAT3 normal((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
			polygon.normal = normal;
			polygons.push_back(polygon);
			CalculateNormals(polygon);

			polygon.v3 = i + 1;
			polygon.v2 = i + rows;
			polygon.v1 = i + rows + 1;
			counter[i + 1]++;
			counter[i + rows]++;
			counter[i + rows + 1]++;

			indices.push_back((UINT)(polygon.v1));
			indices.push_back((UINT)(polygon.v2));
			indices.push_back((UINT)(polygon.v3));

			XMFLOAT3 _a(vertices[polygon.v1].Position.x - vertices[polygon.v2].Position.x, vertices[polygon.v1].Position.y - vertices[polygon.v2].Position.y, vertices[polygon.v1].Position.z - vertices[polygon.v2].Position.z);
			XMFLOAT3 _b(vertices[polygon.v1].Position.x - vertices[polygon.v3].Position.x, vertices[polygon.v1].Position.y - vertices[polygon.v3].Position.y, vertices[polygon.v1].Position.z - vertices[polygon.v3].Position.z);
			XMFLOAT3 _normal((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
			polygon.normal = _normal;
			polygons.push_back(polygon);
			CalculateNormals(polygon);
		}
	}
	// Setup the structure that specifies how big the vertex 
	// buffer should be
	D3D11_BUFFER_DESC vertexBufferDescriptor;
	vertexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescriptor.ByteWidth = sizeof(VERTEX) * (UINT)vertices.size();
	vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescriptor.CPUAccessFlags = 0;
	vertexBufferDescriptor.MiscFlags = 0;
	vertexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the vertices from
	D3D11_SUBRESOURCE_DATA vertexInitialisationData;
	vertexInitialisationData.pSysMem = &vertices[0];

	// and create the vertex buffer
	ThrowIfFailed(_device->CreateBuffer(&vertexBufferDescriptor, &vertexInitialisationData, _vertexBuffer.GetAddressOf()));

	// Setup the structure that specifies how big the index 
	// buffer should be
	D3D11_BUFFER_DESC indexBufferDescriptor;
	indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescriptor.ByteWidth = sizeof(UINT) * (UINT)indices.size();
	indexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescriptor.CPUAccessFlags = 0;
	indexBufferDescriptor.MiscFlags = 0;
	indexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the indices from
	D3D11_SUBRESOURCE_DATA indexInitialisationData;
	indexInitialisationData.pSysMem = &indices[0];

	// and create the index buffer
	ThrowIfFailed(_device->CreateBuffer(&indexBufferDescriptor, &indexInitialisationData, _indexBuffer.GetAddressOf()));
};

void TerrainNode::BuildShaders()
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compilationMessages = nullptr;

	//Compile vertex shader
	HRESULT hr = D3DCompileFromFile(L"TexturedShaders.hlsl",
									nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
									"VShader", "vs_5_0",
									shaderCompileFlags, 0,
									_vertexShaderByteCode.GetAddressOf(),
									compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	// Even if there are no compiler messages, check to make sure there were no other errors.
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreateVertexShader(_vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), NULL, _vertexShader.GetAddressOf()));

	// Compile pixel shader
	hr = D3DCompileFromFile(L"TexturedShaders.hlsl",
							nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
							"PShader", "ps_5_0",
							shaderCompileFlags, 0,
							_pixelShaderByteCode.GetAddressOf(),
							compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreatePixelShader(_pixelShaderByteCode->GetBufferPointer(), _pixelShaderByteCode->GetBufferSize(), NULL, _pixelShader.GetAddressOf()));
}


void TerrainNode::BuildVertexLayout()
{
	// Create the vertex input layout. This tells DirectX the format
	// of each of the vertices we are sending to it.

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ThrowIfFailed(_device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), _vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), _layout.GetAddressOf()));
}

void TerrainNode::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBUFFER);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThrowIfFailed(_device->CreateBuffer(&bufferDesc, NULL, _constantBuffer.GetAddressOf()));
}

void TerrainNode::BuildRendererStates()
{
	// Set default and wireframe rasteriser states
	D3D11_RASTERIZER_DESC rasteriserDesc;
	rasteriserDesc.FillMode = D3D11_FILL_SOLID;
	rasteriserDesc.CullMode = D3D11_CULL_BACK;
	rasteriserDesc.FrontCounterClockwise = false;
	rasteriserDesc.DepthBias = 0;
	rasteriserDesc.SlopeScaledDepthBias = 0.0f;
	rasteriserDesc.DepthBiasClamp = 0.0f;
	rasteriserDesc.DepthClipEnable = true;
	rasteriserDesc.ScissorEnable = false;
	rasteriserDesc.MultisampleEnable = false;
	rasteriserDesc.AntialiasedLineEnable = false;
	ThrowIfFailed(_device->CreateRasterizerState(&rasteriserDesc, _defaultRasteriserState.GetAddressOf()));
	rasteriserDesc.FillMode = D3D11_FILL_WIREFRAME;
	ThrowIfFailed(_device->CreateRasterizerState(&rasteriserDesc, _wireframeRasteriserState.GetAddressOf()));
}

bool TerrainNode::LoadHeightMap(wstring heightMapFilename)
{
	int fileSize = (cols) * (rows);
	BYTE * rawFileValues = new BYTE[fileSize];

	std::ifstream inputHeightMap;
	inputHeightMap.open(heightMapFilename.c_str(), std::ios_base::binary);
	if (!inputHeightMap)
	{
		return false;
	}

	inputHeightMap.read((char*)rawFileValues, fileSize);
	inputHeightMap.close();

	// Normalise BYTE values to the range 0.0f - 1.0f;
	for (unsigned int i = 0; i < fileSize; i++)
	{
		_heightValues.push_back((float)rawFileValues[i] / 255);
	}
	delete[] rawFileValues;
	return true;
}

void TerrainNode::CalculateNormals(POLYGON _polygon)
{
	XMFLOAT3 totalNormal(0, 0, 0);
	std::vector<int> _polygons{ _polygon.v1, _polygon.v2, _polygon.v3 };

	for (int _i = 0; _i < _polygons.size(); _i++)
	{
		totalNormal.x = (totalNormal.x + _polygon.normal.x) / counter[_polygons[_i]];
		totalNormal.y = (totalNormal.y + _polygon.normal.y) / counter[_polygons[_i]];
		totalNormal.z = (totalNormal.x + _polygon.normal.z) / counter[_polygons[_i]];

		double length = sqrt(pow(totalNormal.x, 2) + pow(totalNormal.y, 2) + pow(totalNormal.z, 2));

		totalNormal.x = totalNormal.x / (float)length;
		totalNormal.y = totalNormal.y / (float)length;
		totalNormal.z = totalNormal.z / (float)length;

		vertices[_polygons[_i]].Normal = totalNormal;
	}

	

}
