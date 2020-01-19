#pragma once

#include "mage/AbstractGameLoop.h"
#include "mage/Effect.h"

#include <d3dx9.h>
#include <vector>

//Passo A1 - Definição do formato do vértice (ver .h)

class MeshProject : public mage::AbstractGameLoop
{			
	private:		
		mage::Effect* shader;
		ID3DXMesh* mshModelo;
		std::vector<D3DMATERIAL9> matsModelo;
		std::vector<IDirect3DTexture9*> texsModelo;

		IDirect3DTexture9* texWhite;
		float angle;

	public:
		virtual void setup(IDirect3DDevice9* device);
		virtual bool process(float time);
		virtual void paint(IDirect3DDevice9* device);
		virtual void shutDown(IDirect3DDevice9* device);
		virtual void processEvent(const mage::WindowsEvent& evt);
		void MeshProject::loadXFile(IDirect3DDevice9* device, 
			const mage::TString& filename, ID3DXMesh** meshOut, std::vector<D3DMATERIAL9>& mtrls, std::vector<IDirect3DTexture9*>& texs);
};