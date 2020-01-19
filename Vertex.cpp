#include "Vertex.h"
#include "mage/HandleError.h"

IDirect3DVertexDeclaration9* Vertex::declaration = nullptr;

using namespace std;

Vertex::Vertex(const D3DXVECTOR3& _position, 
	const D3DXVECTOR3& _normal, 
	const D3DXVECTOR2& texCoord)
	: position(_position), normal(_normal), texture(texCoord)
{
}

Vertex::Vertex(float x, float y, float z, 
	float nx, float ny, float nz, 
	float u, float v)
	: position(x, y, z), normal(nx, ny, nz), texture(u, v)
{
}

IDirect3DVertexDeclaration9* Vertex::getDeclaration(IDirect3DDevice9* device)
{	
	if (declaration)
		return declaration;

	D3DVERTEXELEMENT9 decl[] = 
	{
	   {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	   {0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	   {0,  24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	   D3DDECL_END()
	};
	
	HR(device->CreateVertexDeclaration(decl, &declaration));
	return declaration;
}

void Vertex::releaseDeclaration()
{
	declaration->Release();
	declaration = nullptr;
}

IDirect3DVertexBuffer9* Vertex::createVertexBuffer(IDirect3DDevice9* device, const std::vector<Vertex>& vertices, D3DPOOL pool, DWORD usage, HANDLE* pSharedHandle)
{
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;

	HR(device->CreateVertexBuffer(
		sizeof(Vertex) * vertices.size(), usage, 0, pool,
		&vertexBuffer, pSharedHandle));

	Vertex* v = 0;
	HR(vertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&v), 0));
	for (unsigned i = 0; i < vertices.size(); ++i)
		v[i] = vertices[i];
	HR(vertexBuffer->Unlock());

	return vertexBuffer;
}
