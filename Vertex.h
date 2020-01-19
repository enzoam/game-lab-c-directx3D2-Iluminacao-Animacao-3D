#pragma once

#include <Windows.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <vector>

class Vertex
{
	private:
		static IDirect3DVertexDeclaration9* declaration;
	public:
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texture;

		static IDirect3DVertexDeclaration9* getDeclaration(IDirect3DDevice9* device);
		static void releaseDeclaration();
		static IDirect3DVertexBuffer9* createVertexBuffer(IDirect3DDevice9* device, 
			const std::vector<Vertex>& vertices, D3DPOOL pool = D3DPOOL_MANAGED, 
			DWORD usage=0, HANDLE* pSharedHandle=nullptr);		

		Vertex(const D3DXVECTOR3& _position, 
				const D3DXVECTOR3& _normal, 
				const D3DXVECTOR2& texCoord);

		Vertex(float x, float y, float z, 
				float nx, float ny, float nz,
				float u, float v);
};
