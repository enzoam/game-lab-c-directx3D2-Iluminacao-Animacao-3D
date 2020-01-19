// PUC-PR 2014 - ENZO AUGUSTO MARCHIORATO - DIRECTX - JOGOS 3D II
// SHADER ambient.fx / blinnphong30.fx

#include <sstream>
#include <d3d9.h>
#include <d3dx9.h>

//----------------------------------------------------------------

const float CAMERA_DISTANCIA = D3DXToRadian(40.0f);

//------------------------------------------------------------------

#define RESTABELECER_POSICAO(x) if ((x) != 0) { (x)->Release(); (x) = 0; }
#define NOME_CENA "DX3D2"

//-----------------------------------------------------------------

const float TAMANHO_DA_CENA_X = 200.0f;
const float TAMANHO_DA_CENA_X_MEIO = TAMANHO_DA_CENA_X / 2.0f;
const float TAMANHO_DA_CENA_Y = 100.0f;
const float TAMANHO_DA_CENA_Y_MEIO = TAMANHO_DA_CENA_Y / 2.0f;
const float TAMANHO_DA_CENA_Z = 200.0f;
const float TAMANHO_DA_CENA_Z_MEIO = TAMANHO_DA_CENA_Z / 2.0f;

//-----------------------------------------------------------------

const float TILE_TEXTURA_ALTURA_PAREDE = 5.0f;
const float TILE_TEXTURA_LARGURA_PAREDE = 3.0f;

//-----------------------------------------------------------------

const float TILE_TEXTURA_ALTURA_CHAO = 5.0f;
const float TILE_TEXTURA_LARGURA_CHAO = 3.0f;

//-----------------------------------------------------------------

const float TILE_TEXTURA_ALTURA_TETO = 5.0f;
const float TILE_TEXTURA_LARGURA_TETO = 5.0f;

//-----------------------------------------------------------------

float varEmissorLuzTamanho = 8.0f;
float varEmissorLuzVelocidade = 10.0f;

//-----------------------------------------------------------------

const float EMISSAO_MAXIMA_DE_LUZ = max(max(TAMANHO_DA_CENA_X, TAMANHO_DA_CENA_Y), TAMANHO_DA_CENA_Z) * 5.25f;
const float EMISSAO_MINIMA_DE_LUZ = 0.0f;

//-----------------------------------------------------------------------------
// TIPOS
//-----------------------------------------------------------------------------


struct Camera
{
    float pitch;
    float offset;
    D3DXVECTOR3 xAxis;
    D3DXVECTOR3 yAxis;
    D3DXVECTOR3 zAxis;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 target;
    D3DXQUATERNION orientation;
    D3DXMATRIX viewProjectionMatrix;
};

//------------------------------------------------------------------------------

struct Vertex
{
    float texCoord[10];
	float pos[5];
    float normal[5];
};

//------------------------------------------------------------------------------

struct Material
{
	float shininess;
	float diffuse[5];
	float ambient[5];
	float emissive[5];
	float specular[5];

};

//------------------------------------------------------------------------------

struct PointLight
{
	float pos[3];
	float diffuse[4];
	float specular[4];
	float ambient[4];
	float radius;
    D3DXVECTOR3 velocidade;

    void init();
    void update(float elapsedTimeSec);
};

//------------------------------------------------------------------------------

void PointLight::init()
{
	float variacao = varEmissorLuzVelocidade + 5.0f * (varEmissorLuzVelocidade * 
            (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));

    velocidade.x = variacao * 1.0f;
    velocidade.y = variacao * 0.0f;
    velocidade.z = variacao * 2.0f;
}

//-----------------------------------------------------------------------------------

void PointLight::update(float elapsedTimeSec)
{
    // Movimenta a Luz no deltatime

    pos[0] += velocidade.x * elapsedTimeSec;
    pos[1] += velocidade.y * elapsedTimeSec;
    pos[2] += velocidade.z * elapsedTimeSec;

    // Refletir a luz

	// Limitações de colisão

    if (pos[0] > (TAMANHO_DA_CENA_X_MEIO - varEmissorLuzTamanho * 3.0f))  velocidade.x = -velocidade.x;
    if (pos[0] < -(TAMANHO_DA_CENA_X_MEIO - varEmissorLuzTamanho * 3.0f)) velocidade.x = -velocidade.x;
    if (pos[2] > (TAMANHO_DA_CENA_Z_MEIO - varEmissorLuzTamanho * 3.0f))  velocidade.z = -velocidade.z;
    if (pos[2] < -(TAMANHO_DA_CENA_Z_MEIO - varEmissorLuzTamanho * 3.0f)) velocidade.z = -velocidade.z;
}

//-----------------------------------------------------------------------------
// DECLARACAO
//-----------------------------------------------------------------------------

HWND                         varhWnd;
HINSTANCE                    varhInstance;
D3DPRESENT_PARAMETERS        varparams;
ID3DXEffect                 *varpBlinnPhongEffect;
ID3DXEffect                 *varpAmbientEffect;
IDirect3D9                  *varpDirect3D;
IDirect3DDevice9            *varpDevice;
ID3DXFont                   *varFont;
IDirect3DVertexDeclaration9 *varpRoomVertexDecl;
IDirect3DVertexBuffer9      *varpRoomVertexBuffer;
ID3DXMesh                   *varpLightMesh;
D3DCAPS9                     varcaps;
IDirect3DTexture9           *varpNullTexture;
IDirect3DTexture9           *varpWallColorTexture;
IDirect3DTexture9           *varpCeilingColorTexture;
IDirect3DTexture9           *varpFloorColorTexture;
ID3DXMesh                   *varpStaticMesh;
ID3DXBuffer                 *varpStaticBuffer;

//--------------------------------------------------------

DWORD                        g_msaaSamples;
DWORD                        g_maxAnisotrophy;
DWORD                       *pNumMaterials;

//---------------------------------------------------------

bool                         enableVerticalSync;
bool                         hasFocus;
bool						 lights_way = true;
bool                         supportsShaderModel;

//---------------------------------------------------------

int                          windowWidth;
int                          windowHeight;
int                          numLights;
int                          max_light_pulse = 100;

//---------------------------------------------------------

float                        sceneAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};

//HRESULT hr=D3DXLoadMeshFromX("skullocc.x", gD3dDevice, 
//                             varpDevice, NULL, 
//                             &materialBuffer,NULL, &numMaterials, 
//                             &mesh );

Camera varcamera =
{
    0.0f, TAMANHO_DA_CENA_Z,
    D3DXVECTOR3(1.0f, 0.0f, 0.0f),
    D3DXVECTOR3(0.0f, 1.0f, 0.0f),
    D3DXVECTOR3(0.0f, 0.0f, 1.0f),
    D3DXVECTOR3(0.0f, 0.0f, 0.0f),
    D3DXVECTOR3(0.0f, 0.0f, 0.0f),
    D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f),
    D3DXMATRIX()
};

PointLight varlights[] =
{
    // PONTO DE LUZ BRANCO
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        100.0f,
        D3DXVECTOR3(0.0f, 0.0f, 0.0f)
    },

    // PONTO DE LUZ VERMELHO
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        100.0f,
        D3DXVECTOR3(0.0f, 0.0f, 0.0f)
    },

    // PONTO DE LUZ VERDE
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        100.0f,
        D3DXVECTOR3(0.0f, 0.0f, 0.0f)
    },
};

Material vardullMaterial =
{
    0.2f, 0.2f, 0.2f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    0.0f
};

Material varshinyMaterial =
{
    0.2f, 0.2f, 0.2f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    32.0f
};

D3DVERTEXELEMENT9 varroomVertexElements[] =
{
    {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
    D3DDECL_END()
};

// ADAPTACAO DE CODIGO EXTERNO
Vertex varroom[36] =
{
    // Wall: -Z face
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  0.0f,  1.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, 0.0f,                    0.0f,  0.0f,  1.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        0.0f,  0.0f,  1.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        0.0f,  0.0f,  1.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_PAREDE,                    0.0f,  0.0f,  1.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  0.0f,  1.0f},

    // Wall: +Z face
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  0.0f, -1.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, 0.0f,                    0.0f,  0.0f, -1.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        0.0f,  0.0f, -1.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        0.0f,  0.0f, -1.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_PAREDE,                    0.0f,  0.0f, -1.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  0.0f, -1.0f},

    // Wall: -X face
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                1.0f,  0.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, 0.0f,                    1.0f,  0.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        1.0f,  0.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,        1.0f,  0.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_PAREDE,                    1.0f,  0.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                1.0f,  0.0f,  0.0f},

    // Wall: +X face
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                               -1.0f,  0.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, 0.0f,                   -1.0f,  0.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,       -1.0f,  0.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_PAREDE, TILE_TEXTURA_LARGURA_PAREDE,       -1.0f,  0.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_PAREDE,                   -1.0f,  0.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                               -1.0f,  0.0f,  0.0f},

    // Ceiling: +Y face
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f, -1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_TETO, 0.0f,                 0.0f, -1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_TETO, TILE_TEXTURA_LARGURA_TETO,  0.0f, -1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_TETO, TILE_TEXTURA_LARGURA_TETO,  0.0f, -1.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_TETO,                 0.0f, -1.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO,  TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f, -1.0f,  0.0f},

    // Floor: -Y face
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_CHAO, 0.0f,                   0.0f,  1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_CHAO, TILE_TEXTURA_LARGURA_CHAO,      0.0f,  1.0f,  0.0f},
    { TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    TILE_TEXTURA_ALTURA_CHAO, TILE_TEXTURA_LARGURA_CHAO,      0.0f,  1.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO, -TAMANHO_DA_CENA_Z_MEIO,    0.0f, TILE_TEXTURA_LARGURA_CHAO,                   0.0f,  1.0f,  0.0f},
    {-TAMANHO_DA_CENA_X_MEIO, -TAMANHO_DA_CENA_Y_MEIO,  TAMANHO_DA_CENA_Z_MEIO,    0.0f, 0.0f,                                0.0f,  1.0f,  0.0f}
};

//------------------------------------------

void    ChooseBestMSAAMode(D3DFORMAT backBufferFmt, D3DFORMAT depthStencilFmt,
                           BOOL windowed, D3DMULTISAMPLE_TYPE &type,
                           DWORD &qualityLevels, DWORD &samplesPerPixel);
void    Cleanup();
void    CleanupApp();
HWND    CreateAppWindow(const WNDCLASSEX &wcl, const char *pszTitle);
float   GetElapsedTimeInSeconds();
bool    Init();
void    InitApp();
bool    InitD3D();
bool    InitFont(const char *pszFont, int ptSize, LPD3DXFONT &pFont);
void    InitRoom();
bool    LoadShader(const char *pszFilename, LPD3DXEFFECT &pEffect);
bool    MSAAModeSupported(D3DMULTISAMPLE_TYPE type, D3DFORMAT backBufferFmt,
                          D3DFORMAT depthStencilFmt, BOOL windowed,
                          DWORD &qualityLevels);

void    RenderFrame();
void    RenderRoomUsingBlinnPhong();
void    RenderLight(int i);
void    RenderText();
bool    ResetDevice();
void    UpdateFrame(float elapsedTimeSec);
void    UpdateFrameRate(float elapsedTimeSec);
void    UpdateEffects();
void    UpdateLights(float elapsedTimeSec);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// FUNCOES
//-----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

    MSG msg = {0};
    WNDCLASSEX wcl = {0};

    wcl.cbSize = sizeof(wcl);
    wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = WindowProc;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hInstance = varhInstance = hInstance;
    wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
    wcl.hCursor = LoadCursor(0, IDC_ARROW);
    wcl.hbrBackground = 0;
    wcl.lpszMenuName = 0;
    wcl.lpszClassName = "DX3D2";
    wcl.hIconSm = 0;

    RegisterClassEx(&wcl);

    varhWnd = CreateAppWindow(wcl, NOME_CENA);

    if (varhWnd)
    {
        if (Init())
        {
            ShowWindow(varhWnd, nShowCmd);
            UpdateWindow(varhWnd);

            while (true)
            {
                while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                if (hasFocus)
                {
                    UpdateFrame(GetElapsedTimeInSeconds());
                    RenderFrame();
                }
            }
        }

        Cleanup();
        UnregisterClass(wcl.lpszClassName, hInstance);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
        switch (wParam)
        {
        default:
        break;
        case WA_ACTIVE:
        case WA_CLICKACTIVE:
            hasFocus = true;
            break;
        case WA_INACTIVE:
            hasFocus = false;
            break;
        }
        break;

    case WM_CHAR:
        switch (static_cast<int>(wParam))
        {
        case '-':
			max_light_pulse--;
            break;
		case '=':
		case '+':
			max_light_pulse++;
            break;
		case 'a':
		case 'A':
//---------------------		
            break;
		case 'w':
		case 'W':
//---------------------			
            break;
		case 's':
		case 'S':
//---------------------
            break;
		case 'd':
		case 'D':
//---------------------
            break;
		default:
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        windowWidth = static_cast<int>(LOWORD(lParam));
        windowHeight = static_cast<int>(HIWORD(lParam));
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ChooseBestMSAAMode(D3DFORMAT backBufferFmt, D3DFORMAT depthStencilFmt,
                        BOOL windowed, D3DMULTISAMPLE_TYPE &type,
                        DWORD &qualityLevels, DWORD &samplesPerPixel)
{
    bool supported = false;

    struct MSAAMode
    {
        D3DMULTISAMPLE_TYPE type;
        DWORD samples;
    }
    multsamplingTypes[15] =
    {
        { D3DMULTISAMPLE_16_SAMPLES,  16 },
        { D3DMULTISAMPLE_15_SAMPLES,  15 },
        { D3DMULTISAMPLE_14_SAMPLES,  14 },
        { D3DMULTISAMPLE_13_SAMPLES,  13 },
        { D3DMULTISAMPLE_12_SAMPLES,  12 },
        { D3DMULTISAMPLE_11_SAMPLES,  11 },
        { D3DMULTISAMPLE_10_SAMPLES,  10 },
        { D3DMULTISAMPLE_9_SAMPLES,   9  },
        { D3DMULTISAMPLE_8_SAMPLES,   8  },
        { D3DMULTISAMPLE_7_SAMPLES,   7  },
        { D3DMULTISAMPLE_6_SAMPLES,   6  },
        { D3DMULTISAMPLE_5_SAMPLES,   5  },
        { D3DMULTISAMPLE_4_SAMPLES,   4  },
        { D3DMULTISAMPLE_3_SAMPLES,   3  },
        { D3DMULTISAMPLE_2_SAMPLES,   2  }
    };

    for (int i = 0; i < 15; ++i)
    {
        type = multsamplingTypes[i].type;
        supported = MSAAModeSupported(type, backBufferFmt, depthStencilFmt,
                        windowed, qualityLevels);
        if (supported)
        {
            samplesPerPixel = multsamplingTypes[i].samples;
            return;
        }
    }

    type = D3DMULTISAMPLE_NONE;
    qualityLevels = 0;
    samplesPerPixel = 1;
}

void Cleanup()
{
    CleanupApp();
    RESTABELECER_POSICAO(varpNullTexture);
    RESTABELECER_POSICAO(varFont);
    RESTABELECER_POSICAO(varpDevice);
    RESTABELECER_POSICAO(varpDirect3D);
}

void CleanupApp()
{
    RESTABELECER_POSICAO(varpAmbientEffect);
    RESTABELECER_POSICAO(varpBlinnPhongEffect);
    RESTABELECER_POSICAO(varpWallColorTexture);
    RESTABELECER_POSICAO(varpCeilingColorTexture);
    RESTABELECER_POSICAO(varpFloorColorTexture);
    RESTABELECER_POSICAO(varpRoomVertexBuffer);
    RESTABELECER_POSICAO(varpRoomVertexDecl);
    RESTABELECER_POSICAO(varpLightMesh);
	RESTABELECER_POSICAO(varpStaticMesh);
}

HWND CreateAppWindow(const WNDCLASSEX &wcl, const char *pszTitle)
{
    // CRIA A JANELA

    DWORD wndExStyle = WS_EX_OVERLAPPEDWINDOW;
    DWORD wndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                     WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    HWND hWnd = CreateWindowEx(wndExStyle, wcl.lpszClassName, pszTitle,
                    wndStyle, 0, 0, 0, 0, 0, 0, wcl.hInstance, 0);

    if (hWnd)
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int halfScreenWidth = screenWidth / 2;
        int halfScreenHeight = screenHeight / 2;
        int left = (screenWidth - halfScreenWidth) / 2;
        int top = (screenHeight - halfScreenHeight) / 2;
        RECT rc = {0};

        SetRect(&rc, left, top, left + halfScreenWidth, top + halfScreenHeight);
        AdjustWindowRectEx(&rc, wndStyle, FALSE, wndExStyle);
        MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

        GetClientRect(hWnd, &rc);
        windowWidth = rc.right - rc.left;
        windowHeight = rc.bottom - rc.top;
    }

    return hWnd;
}

//--------------------------CONTROLE DE PERFORMANCE-------------------------------------

float GetElapsedTimeInSeconds()
{
    static const int MAX_SAMPLE_COUNT = 50;

    static float frameTimes[MAX_SAMPLE_COUNT];
    static float timeScale = 0.0f;
    static float actualElapsedTimeSec = 0.0f;
    static INT64 freq = 0;
    static INT64 lastTime = 0;
    static int sampleCount = 0;
    static bool initialized = false;

    INT64 time = 0;
    float elapsedTimeSec = 0.0f;

    if (!initialized)
    {
        initialized = true;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&lastTime));
        timeScale = 1.0f / freq;
    }

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
    elapsedTimeSec = (time - lastTime) * timeScale;
    lastTime = time;

    if (fabsf(elapsedTimeSec - actualElapsedTimeSec) < 1.0f)
    {
        memmove(&frameTimes[1], frameTimes, sizeof(frameTimes) - sizeof(frameTimes[0]));
        frameTimes[0] = elapsedTimeSec;

        if (sampleCount < MAX_SAMPLE_COUNT)
            ++sampleCount;
    }

    actualElapsedTimeSec = 0.0f;

    for (int i = 0; i < sampleCount; ++i)
        actualElapsedTimeSec += frameTimes[i];

    if (sampleCount > 0)
        actualElapsedTimeSec /= sampleCount;

    return actualElapsedTimeSec;
}

//--------------------------------------------------------------------------------------

bool Init()
{
    if (!InitD3D())
    {
        return false;
    }

     InitApp();
     return true;
}

void InitApp()
{
	// VERIFICAR SE O SHADER É SUPORTADO

    DWORD dwVSVersion = varcaps.VertexShaderVersion;
    DWORD dwPSVersion = varcaps.PixelShaderVersion;

    supportsShaderModel = true;

    // CONFIGURAÇÃO DAS FONTES.

    InitFont("Arial", 10, varFont);

    // CARREGAR SHADERS.

    LoadShader("Content/ambient.fx", varpAmbientEffect);
    LoadShader("Content/blinnphong30.fx", varpBlinnPhongEffect);

    numLights = 3; //NUMERO MAXIMO DE PONTOS DE LUZ
    
    // CARREGA AS TEXTURAS

    D3DXCreateTextureFromFile(varpDevice, "Content/parede.jpg", &varpWallColorTexture);
	D3DXCreateTextureFromFile(varpDevice,  "Content/teto.jpg", &varpCeilingColorTexture);
	D3DXCreateTextureFromFile(varpDevice,  "Content/chao.jpg", &varpFloorColorTexture);

    // CRIA E CARREGA A GEOMETRIA DA CENA

    InitRoom();

    // CRIA A GEOMETRIA PARA A LUZ

	D3DXCreateSphere(varpDevice, varEmissorLuzTamanho, 3.0f, 10.0f, &varpLightMesh, 0);
	
	//CARREGA OBJETOS MESHES

	//D3DXLoadMeshFromX("skullocc.x", *pNumMaterials, varpDevice, NULL,
    //        &varpStaticBuffer, NULL, &numMaterials, varpStaticMesh);

    // D3DXCreateTeapot(varpDevice, &varpStaticMesh, 0);
  		
	// GERADOR DE NUMERO RANDOMIZADO
		
	srand(GetTickCount());

    // INICIALIZA OS PONTOS DE LUZ NA CENA

    for (int i = 0; i < numLights; ++i)
        varlights[i].init();
}

bool InitD3D()
{
	// Adaptacao de codigo externo :
	
	HRESULT hr = 0;
    D3DDISPLAYMODE desktop = {0};

    varpDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

    // USAR O MODO DE DISPLAY CORRESPONDENTE
    hr = varpDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &desktop);

    // CONFIGURAR DIRECT3D PARA TELA EM FORMATO JANELA
    varparams.BackBufferWidth = 0;
    varparams.BackBufferHeight = 0;
    varparams.BackBufferFormat = desktop.Format;
	varparams.BackBufferCount = 1;
    varparams.hDeviceWindow = varhWnd;
    varparams.Windowed = TRUE;
    varparams.EnableAutoDepthStencil = TRUE;
    varparams.AutoDepthStencilFormat = D3DFMT_D24S8;
    varparams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

    if (enableVerticalSync)
        varparams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    else
        varparams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    // Efeito de swap deve ser D3DSWAPEFFECT_DISCARD para suporte multi-sampling.
    varparams.SwapEffect = D3DSWAPEFFECT_DISCARD;

    // Seleciona o modo multi-sample anti-aliasing de alta qualidade ( MSAA ) 
    ChooseBestMSAAMode(varparams.BackBufferFormat, varparams.AutoDepthStencilFormat,
        varparams.Windowed, varparams.MultiSampleType, varparams.MultiSampleQuality,
        g_msaaSamples);

	// Ao criar um dispositivo puro perdemos a capacidade de depurar vértice
    // E pixel shaders .
    hr = varpDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, varhWnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
            &varparams, &varpDevice);

    if (SUCCEEDED(varpDevice->GetDeviceCaps(&varcaps)))
    {
        g_maxAnisotrophy = varcaps.MaxAnisotropy;
    }

    return true;
}

bool InitFont(const char *pszFont, int ptSize, LPD3DXFONT &pFont)
{
    int logPixelsY = 0;
    if (HDC hDC = GetDC((0)))
    {
        logPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(0, hDC);
    }

    // Configuração da fonte do Texto

    HRESULT fonte = D3DXCreateFont(varpDevice,12,0,FW_BOLD,1,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,0,DEFAULT_PITCH | FF_DONTCARE,pszFont,&pFont);

    return SUCCEEDED(fonte) ? true : false;
}

void InitRoom()
{
    varpDevice->CreateVertexDeclaration(varroomVertexElements, &varpRoomVertexDecl);
    varpDevice->CreateVertexBuffer(sizeof(Vertex) * 36, 0, 0, D3DPOOL_MANAGED, &varpRoomVertexBuffer, 0);
    Vertex *pVertices = 0;
    varpRoomVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertices), 0);
    memcpy(pVertices, varroom, sizeof(varroom));
    varpRoomVertexBuffer->Unlock();
}

// ADAPTACAO CODIGO EXTERNO
bool LoadShader(const char *pszFilename, LPD3DXEFFECT &pEffect)
{
    ID3DXBuffer *pCompilationErrors = 0;
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER;
	
    HRESULT hr = D3DXCreateEffectFromFile(varpDevice, pszFilename, 0, 0,
                    dwShaderFlags, 0, &pEffect, &pCompilationErrors);

    if (pCompilationErrors)
        pCompilationErrors->Release();

    return pEffect != 0;
}

bool MSAAModeSupported(D3DMULTISAMPLE_TYPE type, D3DFORMAT backBufferFmt,
                       D3DFORMAT depthStencilFmt, BOOL windowed,
                       DWORD &qualityLevels)
{
    DWORD backBufferQualityLevels = 0;
    DWORD depthStencilQualityLevels = 0;

    HRESULT hr = varpDirect3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL, backBufferFmt, windowed, type,
                    &backBufferQualityLevels);

    if (SUCCEEDED(hr))
    {
        hr = varpDirect3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL, depthStencilFmt, windowed, type,
                &depthStencilQualityLevels);

        if (SUCCEEDED(hr))
        {
			qualityLevels = backBufferQualityLevels - 1;
	        return true;
        }
    }

    return false;
}

void RenderFrame()
{
    varpDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	varpDevice->BeginScene();
	RenderRoomUsingBlinnPhong();

    for (int i = 0; i < numLights; ++i)
        RenderLight(i);

	RenderText();

    varpDevice->EndScene();
    varpDevice->Present(0, 0, 0, 0);
}

void RenderLight(int i)
{
    static UINT totalPasses;
    static D3DXHANDLE hTechnique;
    static D3DXMATRIX world;
    static D3DXMATRIX worldViewProjection;

    hTechnique = varpAmbientEffect->GetTechniqueByName("AmbientLighting");
	varpAmbientEffect->SetTechnique(hTechnique);

    D3DXMatrixTranslation(&world, varlights[i].pos[0], varlights[i].pos[1], varlights[i].pos[2]);
    worldViewProjection = world * varcamera.viewProjectionMatrix;

    varpAmbientEffect->SetMatrix("worldViewProjectionMatrix", &worldViewProjection);
    varpAmbientEffect->SetFloat("ambientIntensity", 1.0f);
    varpAmbientEffect->SetValue("ambientColor", varlights[i].ambient, sizeof(varlights[i].ambient));

    // DESENHAR O OBJETOS LUMINOSOS.

    if (SUCCEEDED(varpAmbientEffect->Begin(&totalPasses, 0)))
    {
        for (UINT pass = 0; pass < totalPasses; ++pass)
        {
            if (SUCCEEDED(varpAmbientEffect->BeginPass(pass)))
            {
                varpLightMesh->DrawSubset(0);
				//varpStaticMesh->DrawSubset(0);
                varpAmbientEffect->EndPass();
            }
        }

        varpAmbientEffect->End();
    }
}

void RenderRoomUsingBlinnPhong()
{
    static UINT totalPasses;
    static D3DXHANDLE hTechnique;

    hTechnique = varpBlinnPhongEffect->GetTechniqueByName("PerPixelPointLighting");

	varpBlinnPhongEffect->SetTechnique(hTechnique);
    varpDevice->SetVertexDeclaration(varpRoomVertexDecl);
    varpDevice->SetStreamSource(0, varpRoomVertexBuffer, 0, sizeof(Vertex));

    varpBlinnPhongEffect->SetValue("material.ambient", vardullMaterial.ambient, sizeof(vardullMaterial.ambient));
    varpBlinnPhongEffect->SetValue("material.diffuse", vardullMaterial.diffuse, sizeof(vardullMaterial.diffuse));
    varpBlinnPhongEffect->SetValue("material.emissive", vardullMaterial.emissive, sizeof(vardullMaterial.emissive));
    varpBlinnPhongEffect->SetValue("material.specular", vardullMaterial.specular, sizeof(vardullMaterial.specular));
    varpBlinnPhongEffect->SetFloat("material.shininess", vardullMaterial.shininess);

    // DESENHAR PAREDES

        varpBlinnPhongEffect->SetTexture("colorMapTexture", varpWallColorTexture);

    if (SUCCEEDED(varpBlinnPhongEffect->Begin(&totalPasses, 0)))
    {
        for (UINT pass = 0; pass < totalPasses; ++pass)
        {
            if (SUCCEEDED(varpBlinnPhongEffect->BeginPass(pass)))
            {
                varpDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);
                varpBlinnPhongEffect->EndPass();
            }
        }

        varpBlinnPhongEffect->End();
    }

    varpBlinnPhongEffect->SetValue("material.ambient", varshinyMaterial.ambient, sizeof(varshinyMaterial.ambient));
    varpBlinnPhongEffect->SetValue("material.diffuse", varshinyMaterial.diffuse, sizeof(varshinyMaterial.diffuse));
    varpBlinnPhongEffect->SetValue("material.emissive", varshinyMaterial.emissive, sizeof(varshinyMaterial.emissive));
    varpBlinnPhongEffect->SetValue("material.specular", varshinyMaterial.specular, sizeof(varshinyMaterial.specular));
    varpBlinnPhongEffect->SetFloat("material.shininess", varshinyMaterial.shininess);

    // DESENHAR TETO

        varpBlinnPhongEffect->SetTexture("colorMapTexture", varpCeilingColorTexture);

    if (SUCCEEDED(varpBlinnPhongEffect->Begin(&totalPasses, 0)))
    {
        for (UINT pass = 0; pass < totalPasses; ++pass)
        {
            if (SUCCEEDED(varpBlinnPhongEffect->BeginPass(pass)))
            {
                varpDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 24, 2);
                varpBlinnPhongEffect->EndPass();
            }
        }

        varpBlinnPhongEffect->End();
    }

    // DESENHAR CHAO.

    varpBlinnPhongEffect->SetTexture("colorMapTexture", varpFloorColorTexture);

    if (SUCCEEDED(varpBlinnPhongEffect->Begin(&totalPasses, 0)))
    {
        for (UINT pass = 0; pass < totalPasses; ++pass)
        {
            if (SUCCEEDED(varpBlinnPhongEffect->BeginPass(pass)))
            {
                varpDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 30, 2);
                varpBlinnPhongEffect->EndPass();
            }
        }

        varpBlinnPhongEffect->End();
    }
}

void RenderText()
{
    static RECT rcClient;

    std::ostringstream output;
	    output
		    << "PUC-PR - 2014 - DISCIPLINA DE JOGOS 3D II - ENZO A. MARCHIORATO" << std::endl
            << "TECLE (+) OU (-) PARA ALTERAR A VARIAÇÃO DA ILUMINAÇÃO : "  << std::endl;

    GetClientRect(varhWnd, &rcClient);
    rcClient.left += 4;
    rcClient.top += 2;

    varFont->DrawText(0, output.str().c_str(), -1, &rcClient,
        DT_EXPANDTABS | DT_LEFT, D3DCOLOR_XRGB(255, 255, 0));
}

bool ResetDevice()
{
	varpBlinnPhongEffect->OnLostDevice();
	varpAmbientEffect->OnLostDevice();
	varFont->OnLostDevice();
	varpDevice->Reset(&varparams);
	varFont->OnResetDevice();
	varpAmbientEffect->OnResetDevice();
	varpBlinnPhongEffect->OnResetDevice();

    return true;
}

void UpdateFrame(float elapsedTimeSec)
{
    UpdateFrameRate(elapsedTimeSec);
    UpdateLights(elapsedTimeSec);

	// MOVIMENTO DE CAMERA

	static float Prevx = 0.0;
    static float Currentx = 1.0;
	static float dx = 0.0f;
	static D3DXVECTOR3 xAxis(1.0f, 0.0f, 0.0f);
    static D3DXVECTOR3 yAxis(0.0f, 1.0f, 0.0f);
    static D3DXQUATERNION temp;
	
	dx = static_cast<float>(Prevx - Currentx);
    dx *= 0.1f;

    dx = D3DXToRadian(dx);
            
    D3DXQuaternionRotationAxis(&temp, &yAxis, dx);
    D3DXQuaternionMultiply(&varcamera.orientation, &temp, &varcamera.orientation);

	//ATUALIZA EFEITOS

    UpdateEffects();
}

void UpdateFrameRate(float elapsedTimeSec)
{
    static float accumTimeSec = 0.0f;
    static int frames = 0;

    accumTimeSec += elapsedTimeSec;

}

void UpdateEffects()
{
    static const D3DXMATRIX identity(1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f);
    static D3DXMATRIX view, proj;
    static D3DXMATRIX rot, xRot, yRot;

    // CONSTROI A PERSPECTIVA DA MATRIZ DE PROJECAO.

    D3DXMatrixPerspectiveFovLH(&proj, CAMERA_DISTANCIA,
        static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
        0.01f, 1000.0f);

    // CONSTROI A MATRIZ DE VISUALIZACAO.

    D3DXQuaternionNormalize(&varcamera.orientation, &varcamera.orientation);
    D3DXMatrixRotationQuaternion(&view, &varcamera.orientation);
    
    varcamera.xAxis = D3DXVECTOR3(view(0,0), view(1,0), view(2,0));
    varcamera.yAxis = D3DXVECTOR3(view(0,1), view(1,1), view(2,1));
    varcamera.zAxis = D3DXVECTOR3(view(0,2), view(1,2), view(2,2));
   
    varcamera.pos = varcamera.target + varcamera.zAxis * -varcamera.offset;

    view(3,0) = -D3DXVec3Dot(&varcamera.xAxis, &varcamera.pos);
    view(3,1) = -D3DXVec3Dot(&varcamera.yAxis, &varcamera.pos);
    view(3,2) = -D3DXVec3Dot(&varcamera.zAxis, &varcamera.pos);
    
    varcamera.viewProjectionMatrix = view * proj;

    ID3DXEffect *pEffect = varpBlinnPhongEffect;

    // MATRIZES DO SHADER.

    pEffect->SetMatrix("worldMatrix", &identity);
    pEffect->SetMatrix("worldInverseTransposeMatrix", &identity);
    pEffect->SetMatrix("worldViewProjectionMatrix", &varcamera.viewProjectionMatrix);

    // POSICAO DA CAMERA.
    
    pEffect->SetValue("cameraPos", &varcamera.pos, sizeof(varcamera.pos));

    // AMBIENTE / CENA.
    
    pEffect->SetValue("globalAmbient", &sceneAmbient, sizeof(sceneAmbient));

    // NUMERO DE LUZES ATIVAS
    
    if (pEffect == varpBlinnPhongEffect)
        pEffect->SetValue("numLights", &numLights, sizeof(numLights));

    // PARAMETROS DE LUZ DO SHADER.
    
    const PointLight *pLight = 0;
    D3DXHANDLE hLight;
    D3DXHANDLE hLightPos;
    D3DXHANDLE hLightAmbient;
    D3DXHANDLE hLightDiffuse;
    D3DXHANDLE hLightSpecular;
    D3DXHANDLE hLightRadius;

    for (int i = 0; i < numLights; ++i)
    {
        pLight = &varlights[i];
        hLight = pEffect->GetParameterElement("lights", i);
        
        hLightPos = pEffect->GetParameterByName(hLight, "pos");
        hLightAmbient = pEffect->GetParameterByName(hLight, "ambient");
        hLightDiffuse = pEffect->GetParameterByName(hLight, "diffuse");
        hLightSpecular = pEffect->GetParameterByName(hLight, "specular");
        hLightRadius = pEffect->GetParameterByName(hLight, "radius");

        pEffect->SetValue(hLightPos, pLight->pos, sizeof(pLight->pos));
        pEffect->SetValue(hLightAmbient, pLight->ambient, sizeof(pLight->ambient));
        pEffect->SetValue(hLightDiffuse, pLight->diffuse, sizeof(pLight->diffuse));
        pEffect->SetValue(hLightSpecular, pLight->specular, sizeof(pLight->specular));
        pEffect->SetFloat(hLightRadius, pLight->radius);
    }
}

void UpdateLights(float elapsedTimeSec)
{
    for (int i = 0; i < sizeof(varlights) / sizeof(varlights[0]); ++i){
        varlights[i].update(elapsedTimeSec);   
		if (varlights[i].radius > max_light_pulse){
			lights_way = true;
		}

		if (varlights[i].radius < 50){
			lights_way = false;
		}
		
		if (max_light_pulse <= 50){
			max_light_pulse = 51;
		}
		if  (lights_way == 0) {
			varlights[i].radius = varlights[i].radius+0.01;
		}else{
			varlights[i].radius = varlights[i].radius-0.01;
		}
	}
}