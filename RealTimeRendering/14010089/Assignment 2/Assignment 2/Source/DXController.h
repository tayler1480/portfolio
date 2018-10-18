
//
// DXController.h
//

// DXController represents the controller tier of the MVC architecture pattern and references the main window (view), DirectX interfaces and scene variables (model)

#pragma region HeaderFiles

#include <GUObject.h>
#include <Windows.h>
#include <buffers.h>
#include <Triangle.h>
#include <Box.h>
#include <Ocean.h>
#include <Grid.h>
#include <Particles.h>

#pragma endregion HeaderFiles

class DXSystem;
class GUClock;
class DXModel;
class LookAtCamera;

#pragma region CBuffer struct
// CBuffer struct
// Use 16byte aligned so can use optimised XMMathFunctions instead of setting _XM_NO_INNTRINSICS_ define when compiling for x86
__declspec(align(16)) struct CBufferExt  {  
	DirectX::XMMATRIX						WVPMatrix;
	DirectX::XMMATRIX						worldITMatrix; // Correctly transform normals to world space
	DirectX::XMMATRIX						worldMatrix;
	DirectX::XMFLOAT4						eyePos;
	DirectX::XMFLOAT4						windDir;
	// Simple single light source properties
	DirectX::XMFLOAT4						lightVec; // w=1: Vec represents position, w=0: Vec  represents direction.
	DirectX::XMFLOAT4						lightAmbient;
	DirectX::XMFLOAT4						lightDiffuse;
	DirectX::XMFLOAT4						lightSpecular; 
	FLOAT									Timer;
	FLOAT									grassHeight;
};
#pragma endregion CBuffer struct

__declspec(align(16)) struct projMatrixStruct  {
	DirectX::XMMATRIX						projMatrix;
};

__declspec(align(16)) struct worldMatrixStruct  {
	DirectX::XMMATRIX						worldMatrix;
};

class DXController : public GUObject {

	HINSTANCE								hInst = NULL;
	HWND									wndHandle = NULL;

	// Strong reference to associated Direct3D device and rendering context.
	DXSystem								*dx = nullptr;

	// Default pipeline stage states
	ID3D11RasterizerState					*defaultRSstate = nullptr;
	ID3D11RasterizerState					*skyRSState = nullptr;
	ID3D11DepthStencilState					*defaultDSstate = nullptr;
	ID3D11BlendState						*defaultBlendState = nullptr;

	ID3D11VertexShader						*skyBoxVS = nullptr;
	ID3D11PixelShader						*skyBoxPS = nullptr;
	ID3D11Buffer							*cBufferSky = nullptr;

	ID3D11VertexShader						*grassVS = nullptr;
	ID3D11PixelShader						*grassPS = nullptr;
	ID3D11Buffer							*cBufferGrass = nullptr;

	ID3D11VertexShader						*treeVS = nullptr;
	ID3D11PixelShader						*treePS = nullptr;
	ID3D11BlendState						*treesBlendState = nullptr;
	ID3D11Buffer							*cBufferTree = nullptr;

	ID3D11VertexShader						*reflectionMapVS = nullptr;
	ID3D11PixelShader						*reflectionMapPS = nullptr;

	ID3D11VertexShader						*oceanVS = nullptr;
	ID3D11PixelShader						*oceanPS = nullptr;
	ID3D11Buffer							*cBufferWater = nullptr;

	ID3D11VertexShader						*perPixelLightingVS = nullptr;
	ID3D11PixelShader						*perPixelLightingPS = nullptr;

	ID3D11VertexShader						*fireVS = nullptr;
	ID3D11PixelShader						*firePS = nullptr;
	ID3D11DepthStencilState					*fireDSstate = nullptr;
	ID3D11BlendState						*fireBlendState = nullptr;
	ID3D11Buffer							*cBufferFire = nullptr;

	ID3D11Buffer							*cBufferLogs = nullptr;
	ID3D11Buffer							*cBufferShark = nullptr;
	ID3D11Buffer							*cBufferShark2 = nullptr;
	ID3D11Buffer							*cBufferCastle = nullptr;
	ID3D11Buffer							*cBufferCastle2 = nullptr;
	ID3D11Buffer							*cBufferCastle3 = nullptr;
	ID3D11Buffer							*cBufferCastle4 = nullptr;
	ID3D11Buffer							*cBufferCastle5 = nullptr;
	ID3D11Buffer							*cBufferCastle6 = nullptr;
	ID3D11Buffer							*cBufferCastle7 = nullptr;
	ID3D11Buffer							*cBufferCastle8 = nullptr;
	ID3D11Buffer							*cBufferCastle9 = nullptr;
	ID3D11Buffer							*cBufferCastle10 = nullptr;
	ID3D11Buffer							*cBufferCastle11 = nullptr;
	ID3D11Buffer							*cBufferCastle12 = nullptr;
	ID3D11Buffer							*cBufferCastle13 = nullptr;
	ID3D11Buffer							*cBufferCastle14 = nullptr;
	ID3D11Buffer							*cBufferCastle15 = nullptr;
	ID3D11Buffer							*cBufferCastle16 = nullptr;
	ID3D11Buffer							*cBufferCastle17 = nullptr;

	CBufferExt								*cBufferExtSrc = nullptr;

	// Main FPS clock
	GUClock									*mainClock = nullptr;

	LookAtCamera							*mainCamera = nullptr;
	projMatrixStruct 						*projMatrix = nullptr;
	worldMatrixStruct 						*treeInstance = nullptr;

	float									forestSize = 10.0f;
	float									grassLength = 0.005f;
	int										numGrassPasses = 40;

	// Direct3D scene objects
	Box										*skyBox = nullptr;
	Grid									*floor = nullptr;
	DXModel									*tree = nullptr;
	Ocean									*water = nullptr;
	DXModel									*logs = nullptr;
	Particles								*fire = nullptr;
	DXModel									*shark = nullptr;
	DXModel									*castle = nullptr;
	DXModel									*castle2 = nullptr;
	DXModel									*castle3 = nullptr;
	DXModel									*castle4 = nullptr;
	DXModel									*castle5 = nullptr;
	DXModel									*castle6 = nullptr;
	DXModel									*castle7 = nullptr;
	DXModel									*castle8 = nullptr;
	DXModel									*castle9 = nullptr;
	DXModel									*castle10 = nullptr;
	DXModel									*castle11 = nullptr;
	DXModel									*castle12 = nullptr;
	DXModel									*castle13 = nullptr;
	DXModel									*castle14 = nullptr;
	DXModel									*castle15 = nullptr;
	DXModel									*castle16 = nullptr;
	DXModel									*castle17 = nullptr;

	ID3D11SamplerState						*linearSampler = nullptr;

	// Instances of models that appear in the scene
//Skybox
	ID3D11Texture2D							*cubeMapTexture = nullptr;
	ID3D11ShaderResourceView				*cubeMapTextureSRV = nullptr;
//Grass
	ID3D11Texture2D							*grassDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*grassDiffuseMapSRV = nullptr;
	ID3D11Texture2D							*grassAlphaMap = nullptr;
	ID3D11ShaderResourceView				*grassAlphaMapSRV = nullptr;
	ID3D11Texture2D							*grassHeightMap = nullptr;
	ID3D11ShaderResourceView				*grassHeightMapSRV = nullptr;
	ID3D11Texture2D							*grassNormalMap = nullptr;
	ID3D11ShaderResourceView				*grassNormalMapSRV = nullptr;
//Tree
	ID3D11Texture2D							*treeTexture = nullptr;
	ID3D11ShaderResourceView				*treeTextureSRV = nullptr;
//Water
	ID3D11Texture2D							*waterNormalMap = nullptr;
	ID3D11ShaderResourceView				*waterNormalMapSRV = nullptr;
//Logs
	ID3D11Texture2D							*logsTexture = nullptr;
	ID3D11ShaderResourceView				*logsTextureSRV = nullptr;
// Fire
	ID3D11Texture2D							*fireDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*fireDiffuseMapSRV = nullptr;
// Smoke
	ID3D11Texture2D							*smokeDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*smokeDiffuseMapSRV = nullptr;
//Shark
	ID3D11Texture2D							*sharkTexture = nullptr;
	ID3D11ShaderResourceView				*sharkTextureSRV = nullptr;
//Castle
	ID3D11Texture2D							*castleTexture = nullptr;
	ID3D11ShaderResourceView				*castleTextureSRV = nullptr;

	ID3D11Texture2D							*castleTexture2 = nullptr;
	ID3D11ShaderResourceView				*castleTexture2SRV = nullptr;

	ID3D11Texture2D							*castleTexture3 = nullptr;
	ID3D11ShaderResourceView				*castleTexture3SRV = nullptr;

	ID3D11Texture2D							*castleTexture4 = nullptr;
	ID3D11ShaderResourceView				*castleTexture4SRV = nullptr;

	ID3D11Texture2D							*castleTexture5 = nullptr;
	ID3D11ShaderResourceView				*castleTexture5SRV = nullptr;

	ID3D11Texture2D							*castleTexture6 = nullptr;
	ID3D11ShaderResourceView				*castleTexture6SRV = nullptr;

	ID3D11Texture2D							*castleTexture7 = nullptr;
	ID3D11ShaderResourceView				*castleTexture7SRV = nullptr;

	ID3D11Texture2D							*castleTexture8 = nullptr;
	ID3D11ShaderResourceView				*castleTexture8SRV = nullptr;

	ID3D11Texture2D							*castleTexture9 = nullptr;
	ID3D11ShaderResourceView				*castleTexture9SRV = nullptr;

	ID3D11Texture2D							*castleTexture10 = nullptr;
	ID3D11ShaderResourceView				*castleTexture10SRV = nullptr;

	ID3D11Texture2D							*castleTexture11 = nullptr;
	ID3D11ShaderResourceView				*castleTexture11SRV = nullptr;

	ID3D11Texture2D							*castleTexture12 = nullptr;
	ID3D11ShaderResourceView				*castleTexture12SRV = nullptr;

	ID3D11Texture2D							*castleTexture13 = nullptr;
	ID3D11ShaderResourceView				*castleTexture13SRV = nullptr;

	ID3D11Texture2D							*castleTexture14 = nullptr;
	ID3D11ShaderResourceView				*castleTexture14SRV = nullptr;

	ID3D11Texture2D							*castleTexture15 = nullptr;
	ID3D11ShaderResourceView				*castleTexture15SRV = nullptr;

	ID3D11Texture2D							*castleTexture16 = nullptr;
	ID3D11ShaderResourceView				*castleTexture16SRV = nullptr;

	ID3D11Texture2D							*castleTexture17 = nullptr;
	ID3D11ShaderResourceView				*castleTexture17SRV = nullptr;


	//
	// Private interface
	//

	// Private constructor
	DXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);

	// Return TRUE if the window is in a minimised state, FALSE otherwise
	BOOL isMinimised();


public:

	//
	// Public interface
	//

	// Factory method to create the main DXController instance (singleton)
	static DXController* CreateDXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);

	// Destructor
	~DXController();

	// Decouple the encapsulated HWND and call DestoryWindow on the HWND
	void destoryWindow();

	// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
	HRESULT resizeResources();

	// Helper function to call updateScene followed by renderScene
	HRESULT updateAndRenderScene();
	HRESULT mapCbuffer(void *cBufferExtSrcL, ID3D11Buffer *cBufferExtL);
	// Clock handling methods
	void startClock();
	void stopClock();
	void reportTimingData();


#pragma region EventHandlingMethods
	//
	// Event handling methods
	//

	// Process mouse move with the left button held down
	void handleMouseLDrag(const POINT &disp);

	// Process mouse wheel movement
	void handleMouseWheel(const short zDelta);

	// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
	void handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags);
	
	// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
	void handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags);
#pragma endregion EventHandlingMethods
	
	//
	// Methods to handle initialisation, update and rendering of the scene
	//
	HRESULT rebuildViewport();
	HRESULT initDefaultPipeline();
	HRESULT bindDefaultPipeline();
	HRESULT LoadShader(ID3D11Device *device, const char *filename, DXBlob **PSBytecode, ID3D11PixelShader **pixelShader);
	HRESULT LoadShader(ID3D11Device *device, const char *filename, DXBlob **VSBytecode, ID3D11VertexShader **vertexShader);
	HRESULT initialiseSceneResources();
	HRESULT updateScene();
	HRESULT renderScene();

};

// normal mapping to generate ripples
// The evaluate rav calculate normal & tangent & those calculate "something"
// Displacement creates the wave
// effient limited fps impact (good)

// shadow mapping