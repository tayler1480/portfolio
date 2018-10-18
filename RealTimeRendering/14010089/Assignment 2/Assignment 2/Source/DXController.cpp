//
// DXController.cpp
//

#pragma region Libaries etc
#include <stdafx.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DXController.h>
#include <DirectXMath.h>
#include <DXSystem.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>
#include <GUClock.h>
#include <DXModel.h>
#include <LookAtCamera.h>
#define	NUM_TREES 25

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
#pragma endregion

#pragma region System Settings

// Load the Compiled Shader Object (CSO) file 'filename' and return the bytecode in the blob object **bytecode.  This is used to create shader interfaces that require class linkage interfaces.
void DXLoadCSO(const char *filename, DXBlob **bytecode)
{

	ifstream	*fp = nullptr;
	DXBlob		*memBlock = nullptr;

	try
	{
		// Validate parameters
		if (!filename || !bytecode)
			throw exception("loadCSO: Invalid parameters");

		// Open file
		fp = new ifstream(filename, ios::in | ios::binary);

		if (!fp->is_open())
			throw exception("loadCSO: Cannot open file");

		// Get file size
		fp->seekg(0, ios::end);
		uint32_t size = (uint32_t)fp->tellg();

		// Create blob object to store bytecode (exceptions propagate up if any occur)
		memBlock = new DXBlob(size);

		// Read binary data into blob object
		fp->seekg(0, ios::beg);
		fp->read((char*)(memBlock->getBufferPointer()), memBlock->getBufferSize());


		// Close file and release local resources
		fp->close();
		delete fp;

		// Return DXBlob - ownership implicity passed to caller
		*bytecode = memBlock;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;

		// Cleanup local resources
		if (fp) {

			if (fp->is_open())
				fp->close();

			delete fp;
		}

		if (memBlock)
			delete memBlock;

		// Re-throw exception
		throw;
	}
}

// Helper Generates random number between -1.0 and +1.0
float randM1P1()
{	// use srand((unsigned int)time(NULL)); to seed rand()
	float r = (float)((double)rand() / (double)(RAND_MAX)) * 2.0f - 1.0f;
	return r;
}

//
// Private interface implementation
//

// Private constructor
DXController::DXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {

	try
	{
		// 1. Register window class for main DirectX window
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
		wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = wndClassName;
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex))
			throw exception("Cannot register window class for DXController HWND");

		
		// 2. Store instance handle in our global variable
		hInst = hInstance;


		// 3. Setup window rect and resize according to set styles
		RECT		windowRect;

		windowRect.left = 0;
		windowRect.right = _width;
		windowRect.top = 0;
		windowRect.bottom = _height;

		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		// 4. Create and validate the main window handle
		wndHandle = CreateWindowEx(dwExStyle, wndClassName, wndTitle, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 500, 500, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hInst, this);

		if (!wndHandle)
			throw exception("Cannot create main window handle");

		ShowWindow(wndHandle, nCmdShow);
		UpdateWindow(wndHandle);
		SetFocus(wndHandle);


		// 5. Initialise render pipeline model (simply sets up an internal std::vector of pipeline objects)
	

		// 6. Create DirectX host environment (associated with main application wnd)
		dx = DXSystem::CreateDirectXSystem(wndHandle);

		if (!dx)
			throw exception("Cannot create Direct3D device and context model");

		// 7. Setup application-specific objects
		HRESULT hr = initialiseSceneResources();

		if (!SUCCEEDED(hr))
			throw exception("Cannot initalise scene resources");


		// 8. Create main clock / FPS timer (do this last with deferred start of 3 seconds so min FPS / SPF are not skewed by start-up events firing and taking CPU cycles).
		mainClock = GUClock::CreateClock(string("mainClock"), 3.0f);

		if (!mainClock)
			throw exception("Cannot create main clock / timer");

	}
	catch (exception &e)
	{
		cout << e.what() << endl;

		// Re-throw exception
		throw;
	}
	
}

// Return TRUE if the window is in a minimised state, FALSE otherwise
BOOL DXController::isMinimised() {

	WINDOWPLACEMENT				wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	return (GetWindowPlacement(wndHandle, &wp) != 0 && wp.showCmd == SW_SHOWMINIMIZED);
}

//
// Public interface implementation
//

// Factory method to create the main DXController instance (singleton)
DXController* DXController::CreateDXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {

	static bool _controller_created = false;

	DXController *dxController = nullptr;

	if (!_controller_created) {

		dxController = new DXController(_width, _height, wndClassName, wndTitle, nCmdShow, hInstance, WndProc);

		if (dxController)
			_controller_created = true;
	}

	return dxController;
}

// Decouple the encapsulated HWND and call DestoryWindow on the HWND
void DXController::destoryWindow() {

	if (wndHandle != NULL) {

		HWND hWnd = wndHandle;

		wndHandle = NULL;
		DestroyWindow(hWnd);
	}
}

// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
HRESULT DXController::resizeResources() {

	if (dx) {

		// Only process resize if the DXSystem *dx exists (on initial resize window creation this will not be the case so this branch is ignored)
		HRESULT hr = dx->resizeSwapChainBuffers(wndHandle);
		rebuildViewport();
		RECT clientRect;
		GetClientRect(wndHandle, &clientRect);


		if (!isMinimised())
			renderScene();
	}

	return S_OK;
}

// Helper function to call updateScene followed by renderScene
HRESULT DXController::updateAndRenderScene() {

	HRESULT hr = updateScene();

	if (SUCCEEDED(hr))
		hr = renderScene();

	return hr;
}

// Clock handling methods ( Shows times on exit)
void DXController::startClock() {

	mainClock->start();
}
void DXController::stopClock() {

	mainClock->stop();
}
void DXController::reportTimingData() {

	cout << "Actual time elapsed = " << mainClock->actualTimeElapsed() << endl;
	cout << "Game time elapsed = " << mainClock->gameTimeElapsed() << endl << endl;
	mainClock->reportTimingData();
}

//
// Event handling methods
//
// Process mouse move with the left button held down
void DXController::handleMouseLDrag(const POINT &disp) {

	mainCamera->rotateElevation((float)-disp.y * 0.01f);
	mainCamera->rotateOnYAxis((float)-disp.x * 0.01f);
}

// Process mouse wheel movement
void DXController::handleMouseWheel(const short zDelta) {

	if (zDelta<0)
		mainCamera->zoomCamera(1.2f);
	else if (zDelta>0)
		mainCamera->zoomCamera(0.9f);
}


// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
void DXController::handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags) {

	// Add key down handler here...
}


// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
void DXController::handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags) {

	// Add key up handler here...
}

//
// Methods to handle initialisation, update and rendering of the scene
//
HRESULT DXController::rebuildViewport(){
	// Binds the render target view and depth/stencil view to the pipeline.
	// Sets up viewport for the main window (wndHandle) 
	// Called at initialisation or in response to window resize


	ID3D11DeviceContext *context = dx->getDeviceContext();

	if ( !context)
		return E_FAIL;

	// Bind the render target view and depth/stencil view to the pipeline.
	ID3D11RenderTargetView* renderTargetView = dx->getBackBufferRTV();
	context->OMSetRenderTargets(1, &renderTargetView, dx->getDepthStencil());
	// Setup viewport for the main window (wndHandle)
	RECT clientRect;
	GetClientRect(wndHandle, &clientRect);
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(clientRect.right - clientRect.left);
	viewport.Height = static_cast<FLOAT>(clientRect.bottom - clientRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//Set Viewport
	context->RSSetViewports(1, &viewport);
	
	// Compute the projection matrix.
	projMatrix->projMatrix = XMMatrixPerspectiveFovLH(0.25f*3.14, viewport.Width / viewport.Height, 1.0f, 1000.0f);
	return S_OK;
}
#pragma endregion

#pragma region Load_Shaders
HRESULT DXController::LoadShader(ID3D11Device *device, const char *filename, DXBlob **PSBytecode, ID3D11PixelShader **pixelShader){

	//Load the compiled shader byte code.
	DXLoadCSO(filename, PSBytecode);
	// Create shader objects
	HRESULT hr = device->CreatePixelShader((*PSBytecode)->getBufferPointer(), (*PSBytecode)->getBufferSize(), NULL, pixelShader);

	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create PixelShader interface");
	return hr;
}
HRESULT DXController::LoadShader(ID3D11Device *device, const char *filename, DXBlob **VSBytecode, ID3D11VertexShader **vertexShader){

	//Load the compiled shader byte code.
	DXLoadCSO(filename, VSBytecode);
	HRESULT hr = device->CreateVertexShader((*VSBytecode)->getBufferPointer(), (*VSBytecode)->getBufferSize(), NULL, vertexShader);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create VertexShader interface");
	return hr;
}
#pragma endregion

#pragma region Destructor
// Destructor
DXController::~DXController() {

	//Clean Up- release local interfaces

	// Release RSstate
	defaultRSstate->Release();
	// Release RSstate
	skyRSState->Release();
	// Release dsState
	defaultDSstate->Release();
	// Release blendState
	defaultBlendState->Release();
	// Release VertexShader interface
	skyBoxVS->Release();
	// Release PixelShader interface
	skyBoxPS->Release();

	// Grass/Trees
	grassVS->Release();
	// Release PixelShader interface
	grassPS->Release();
	// Release VertexShader interface
	treeVS->Release();
	// Release PixelShader interface
	treePS->Release();
	// Release cBuffer
	cBufferTree->Release();

	// Fire/Logs
	fireVS->Release();
	// Release PixelShader interface
	firePS->Release();
	// Release VertexShader interface
	perPixelLightingVS->Release();
	// Release PixelShader interface
	perPixelLightingPS->Release();

	// Release cBuffer
	cBufferLogs->Release();

	// Release VertexShader interface
	reflectionMapVS->Release();
	// Release PixelShader interface
	reflectionMapPS->Release();
	// Release VertexShader interface
	oceanVS->Release();
	// Release PixelShader interface
	oceanPS->Release();
	// Release cBuffer
	cBufferShark->Release();
	// Release cBuffer
	cBufferShark2->Release();


	// Release cBuffer
	cBufferSky->Release();

	if (cBufferExtSrc)
		_aligned_free(cBufferExtSrc);
	if (projMatrix)
		_aligned_free(projMatrix);
	if (treeInstance)
		_aligned_free(treeInstance);
	if (mainCamera)
		mainCamera->release();
	if (mainClock)
		mainClock->release();
	// Release shark
	if (shark)
		shark->release();
	// Release tree
	if (tree)
		tree->release();
	// Release skyBox
	if (skyBox)
		skyBox->release();
	// Release Box
	if (water)
		water->release();
	if (floor)
		floor->release();
	// Release skyBox
	if (logs)
		logs->release();
	//	Release Box
	if (fire)
		fire->release();


	if (dx) {

		dx->release();
		dx = nullptr;
	}

	if (wndHandle)
		DestroyWindow(wndHandle);
}
#pragma endregion

HRESULT  DXController::bindDefaultPipeline(){

	ID3D11DeviceContext *context = dx->getDeviceContext();
	if (!context)
		return E_FAIL;
	// Apply RSState
	context->RSSetState(defaultRSstate);
	// Apply dsState
	context->OMSetDepthStencilState(defaultDSstate, 0);
	//Apply blendState
	FLOAT			blendFactor[4]; blendFactor[0] = blendFactor[1] = blendFactor[2] = blendFactor[3] = 1.0f;
	UINT			sampleMask = 0xFFFFFFFF; // Bitwise flags to determine which samples to process in an MSAA context
	context->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);
	return S_OK;
}
HRESULT DXController::initDefaultPipeline(){
	
	ID3D11Device *device = dx->getDevice();
	if (!device)
		return E_FAIL;
	// Initialise default Rasteriser state object
	D3D11_RASTERIZER_DESC			RSdesc;

	ZeroMemory(&RSdesc, sizeof(D3D11_RASTERIZER_DESC));
	// Setup default rasteriser state 
	RSdesc.FillMode = D3D11_FILL_SOLID;
	RSdesc.CullMode = D3D11_CULL_NONE;
	RSdesc.FrontCounterClockwise = TRUE;
	RSdesc.DepthBias = 0;
	RSdesc.SlopeScaledDepthBias = 0.0f;
	RSdesc.DepthBiasClamp = 0.0f;
	RSdesc.DepthClipEnable = TRUE;
	RSdesc.ScissorEnable = FALSE;
	RSdesc.MultisampleEnable = TRUE;
	RSdesc.AntialiasedLineEnable = FALSE;
	HRESULT hr = device->CreateRasterizerState(&RSdesc, &defaultRSstate);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Rasterise state interface");
	
	
	// Sky Box RSState
	RSdesc.CullMode = D3D11_CULL_NONE;
	hr = device->CreateRasterizerState(&RSdesc, &skyRSState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Rasterise state interface");
	

	// Output - Merger Stage

	// Initialise default depth-stencil state object
	D3D11_DEPTH_STENCIL_DESC	dsDesc;

	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Setup default depth-stencil descriptor
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// Initialise depth-stencil state object based on the given descriptor
	hr = device->CreateDepthStencilState(&dsDesc, &defaultDSstate);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create DepthStencil state interface");

	//// Add Code Here (Disable Depth Writing for Fire)
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	hr = device->CreateDepthStencilState(&dsDesc, &fireDSstate);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create DepthStencil state interface");

	// Initialise default blend state object (Alpha Blending On)
	D3D11_BLEND_DESC	blendDesc;

	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	// Setup default blend state descriptor 
	// Add Code Here (Set Alpha Blending On)
	blendDesc.AlphaToCoverageEnable = FALSE; // Use pixel coverage info from rasteriser (default)
	blendDesc.IndependentBlendEnable = FALSE; // The following array of render target blend properties uses the blend properties from RenderTarget[0] for ALL render targets
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	// Create blendState
	hr = device->CreateBlendState(&blendDesc, &defaultBlendState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Blend state interface");

	// Modify Code Here (Enable Alpha to Coverage)
	// Use pixel coverage info from rasteriser for tree leaves
	// Check Rasteriser state MultisampleEnable = TRUE (see above)
	//Lecture 18 - Slide 22
	RSdesc.MultisampleEnable = TRUE;
	blendDesc.AlphaToCoverageEnable = TRUE;
	hr = device->CreateBlendState(&blendDesc, &treesBlendState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Blend state interface");

	RSdesc.MultisampleEnable = TRUE;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	//Add Code Here (Enable Alpha Blending for Fire)
	hr = device->CreateBlendState(&blendDesc, &fireBlendState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Blend state interface");

	return hr;
}

// Main resource setup for the application.  These are setup around a given Direct3D device.
HRESULT DXController::initialiseSceneResources() {
	ID3D11DeviceContext *context = dx->getDeviceContext();
	ID3D11Device *device = dx->getDevice();
	if (!device)
		return E_FAIL;

	//
	// Setup main pipeline objects
	//

	// Setup objects for fixed function pipeline stages
	// Rasterizer Stage
	// Bind the render target view and depth/stencil view to the pipeline
	// and sets up viewport for the main window (wndHandle) 


	// Allocate the projection matrix (it is setup in rebuildViewport).
	projMatrix = (projMatrixStruct*)_aligned_malloc(sizeof(projMatrixStruct), 16);

	// Allocate the world matrices for the  tree instances
	treeInstance = (worldMatrixStruct*)_aligned_malloc(sizeof(worldMatrixStruct)*NUM_TREES, 16);
	// Setup tree instance positions

	
	for (int i = 0; i < NUM_TREES; i++)
	{
		float x = randM1P1()*forestSize, y=0, z = randM1P1()*forestSize;

		// Translate and Rotate trees randomly
		// Modify code here (randomly rotate trees)
		treeInstance[i].worldMatrix = XMMatrixScaling(2.0, 2.0, 2.0)*XMMatrixTranslation(randM1P1()*forestSize, 0, randM1P1()*forestSize)*XMMatrixRotationY(0);
		//treeInstance[i].worldMatrix = XMMatrixScaling(2.0, 2.0, 2.0)*XMMatrixTranslation(x, y, z)*XMMatrixRotationY(0);
	}


	rebuildViewport();
	initDefaultPipeline();
	bindDefaultPipeline();

	// Setup objects for the programmable (shader) stages of the pipeline
	// Skybox
	DXBlob *skyBoxVSBytecode = nullptr;
	DXBlob *skyBoxPSBytecode = nullptr;
	// Grass
	DXBlob *grassVSBytecode = nullptr;
	DXBlob *grassPSBytecode = nullptr;
	// Trees
	DXBlob *treeVSBytecode = nullptr;
	DXBlob *treePSBytecode = nullptr;
	// Reflections
	DXBlob *reflectionMapVSBytecode = nullptr;
	DXBlob *reflectionMapPSBytecode = nullptr;
	// Ocean
	DXBlob *oceanVSBytecode = nullptr;
	DXBlob *oceanPSBytecode = nullptr;
	// Fire
	DXBlob *fireVSBytecode = nullptr;
	DXBlob *firePSBytecode = nullptr;
	// perPixelLighting
	DXBlob *perPixelLightingVSBytecode = nullptr;
	DXBlob *perPixelLightingPSBytecode = nullptr;

	// Skybox
	LoadShader(device, "Shaders\\cso\\sky_box_vs.cso", &skyBoxVSBytecode, &skyBoxVS);
	LoadShader(device, "Shaders\\cso\\sky_box_ps.cso", &skyBoxPSBytecode, &skyBoxPS);
	// Grass
	LoadShader(device, "Shaders\\cso\\grass_vs.cso", &grassVSBytecode, &grassVS);
	LoadShader(device, "Shaders\\cso\\grass_ps.cso", &grassPSBytecode, &grassPS);
	// Trees
	LoadShader(device, "Shaders\\cso\\tree_vs.cso", &treeVSBytecode, &treeVS);
	LoadShader(device, "Shaders\\cso\\tree_ps.cso", &treePSBytecode, &treePS);
	// Reflections
	LoadShader(device, "Shaders\\cso\\reflection_map_vs.cso", &reflectionMapVSBytecode, &reflectionMapVS);
	LoadShader(device, "Shaders\\cso\\reflection_map_ps.cso", &reflectionMapPSBytecode, &reflectionMapPS);
	// Ocean
	LoadShader(device, "Shaders\\cso\\ocean_vs.cso", &oceanVSBytecode, &oceanVS);
	LoadShader(device, "Shaders\\cso\\ocean_ps.cso", &oceanPSBytecode, &oceanPS);
	// Fire
	LoadShader(device, "Shaders\\cso\\fire_vs.cso", &fireVSBytecode, &fireVS);
	LoadShader(device, "Shaders\\cso\\fire_ps.cso", &firePSBytecode, &firePS);
	// perPixelLighting
	LoadShader(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", &perPixelLightingVSBytecode, &perPixelLightingVS);
	LoadShader(device, "Shaders\\cso\\per_pixel_lighting_ps.cso", &perPixelLightingPSBytecode, &perPixelLightingPS);

	// Create main camera
	mainCamera = new LookAtCamera();
	mainCamera->setPos(XMVectorSet(25, 10, -14.5, 1));

	cBufferExtSrc = (CBufferExt*)_aligned_malloc(sizeof(CBufferExt), 16);
	cBufferExtSrc->worldMatrix = XMMatrixIdentity();
	cBufferExtSrc->worldITMatrix = XMMatrixIdentity();
	cBufferExtSrc->WVPMatrix = mainCamera->dxViewTransform()*projMatrix->projMatrix;
	cBufferExtSrc->lightVec = XMFLOAT4(-250.0, 130.0, 145.0, 1.0); // Positional light
	cBufferExtSrc->lightAmbient = XMFLOAT4(0.3, 0.3, 0.3, 1.0);  // Ambient light
	cBufferExtSrc->lightDiffuse = XMFLOAT4(0.8, 0.8, 0.8, 1.0);  // light Diffuse
	cBufferExtSrc->lightSpecular = XMFLOAT4(1.0, 1.0, 1.0, 1.0); // light Specular 
	XMStoreFloat4(&cBufferExtSrc->eyePos, mainCamera->getCameraPos());// camera->pos;

	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbufferDesc.ByteWidth = sizeof(CBufferExt);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferExtSrc;

#pragma region Create_ConstantBuffers
	// Create CBuffers

	// SkyBox CBuffer
	HRESULT
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferSky);
	// Floor(Grass?) CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferGrass);
	// Tree CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferTree);
	// Water(Ocean?) CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferWater);
	// Logs CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferLogs);
	// Fire CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferFire);
	// Shark CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferShark);
	// Shark2 CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferShark2);
	//Castle cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle);
	//Castle2 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle2);
	//Castle3 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle3);
	//Castle4 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle4);
	//Castle5 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle5);
	//Castle6 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle6);
	//Castle7 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle7);
	//Castle8 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle8);
	//Castle9 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle9);
	//Castle10 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle10);
	//Castle11 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle11);
	//Castle12 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle12);
	//Castle13 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle13);
	//Castle14 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle14);
	//Castle15 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle15);
	//Castle16 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle16);
	//Castle17 cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle17);





#pragma endregion Create_ConstantBuffers

#pragma region LoadTextures
	//
	// Setup example objects
	//

	// Load textures

	// SkyBox
	ID3D11Resource *cubeMapResource = static_cast<ID3D11Resource*>(cubeMapTexture);
	hr = CreateDDSTextureFromFile(device, L"Resources\\Textures\\grassenvmap1024.dds", &cubeMapResource, &cubeMapTextureSRV);
	// Terrain(Grass?) Textures
	ID3D11Resource *grassNormalResource = static_cast<ID3D11Resource*>(grassNormalMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\normalmap.bmp", &grassNormalResource, &grassNormalMapSRV);
	ID3D11Resource *grassHeightResource = static_cast<ID3D11Resource*>(grassAlphaMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\heightmap.bmp", &grassHeightResource, &grassHeightMapSRV);
	ID3D11Resource *grassResource = static_cast<ID3D11Resource*>(grassDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\grass.png", &grassResource, &grassDiffuseMapSRV);
	ID3D11Resource *grassAlphaResource = static_cast<ID3D11Resource*>(grassAlphaMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\grassAlpha.tif", &grassAlphaResource, &grassAlphaMapSRV);
	// Tree textures
	ID3D11Resource *treeResource = static_cast<ID3D11Resource*>(treeTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\tree.tif", &treeResource, &treeTextureSRV);
	// Water Textures
	ID3D11Resource *waterResource = static_cast<ID3D11Resource*>(waterNormalMap);
	hr = CreateDDSTextureFromFile(device, L"Resources\\Textures\\Waves.dds", &waterResource, &waterNormalMapSRV);
	// Logs Textures
	ID3D11Resource *logsResource = static_cast<ID3D11Resource*>(logsTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\logs.jpg", &logsResource, &logsTextureSRV);
	// Fire Textures
	ID3D11Resource *fireResource = static_cast<ID3D11Resource*>(fireDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\fire.tif", &fireResource, &fireDiffuseMapSRV);
	// Smoke Texyures
	ID3D11Resource *smokeResource = static_cast<ID3D11Resource*>(smokeDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\smoke.tif", &smokeResource, &smokeDiffuseMapSRV);
	// Shark Texture
	ID3D11Resource *sharkResource = static_cast<ID3D11Resource*>(sharkTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\greatwhiteshark.png", &sharkResource, &sharkTextureSRV);
	// Castle Texture
	ID3D11Resource *castleResource = static_cast<ID3D11Resource*>(castleTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\castle.jpg", &castleResource, &castleTextureSRV);
	
#pragma region CastleWalls

	// Castle Texture2
	ID3D11Resource *castleResource2 = static_cast<ID3D11Resource*>(castleTexture2);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource2, &castleTexture2SRV);
	// Castle Texture3
	ID3D11Resource *castleResource3 = static_cast<ID3D11Resource*>(castleTexture3);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource3, &castleTexture3SRV);
	// Castle Texture4
	ID3D11Resource *castleResource4 = static_cast<ID3D11Resource*>(castleTexture4);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource4, &castleTexture4SRV);
	// Castle Texture5
	ID3D11Resource *castleResource5 = static_cast<ID3D11Resource*>(castleTexture5);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\castlewall_3.png", &castleResource5, &castleTexture5SRV);
	// Castle Texture6
	ID3D11Resource *castleResource6 = static_cast<ID3D11Resource*>(castleTexture6);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource6, &castleTexture6SRV);
	// Castle Texture7
	ID3D11Resource *castleResource7 = static_cast<ID3D11Resource*>(castleTexture7);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource7, &castleTexture7SRV);
	// Castle Texture8
	ID3D11Resource *castleResource8 = static_cast<ID3D11Resource*>(castleTexture8);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource8, &castleTexture8SRV);
	// Castle Texture9
	ID3D11Resource *castleResource9 = static_cast<ID3D11Resource*>(castleTexture9);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource9, &castleTexture9SRV);
	// Castle Texture10
	ID3D11Resource *castleResource10 = static_cast<ID3D11Resource*>(castleTexture10);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource10, &castleTexture10SRV);
	// Castle Texture11
	ID3D11Resource *castleResource11 = static_cast<ID3D11Resource*>(castleTexture11);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource11, &castleTexture11SRV);
	// Castle Texture12
	ID3D11Resource *castleResource12 = static_cast<ID3D11Resource*>(castleTexture12);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource12, &castleTexture12SRV);
	// Castle Texture13
	ID3D11Resource *castleResource13 = static_cast<ID3D11Resource*>(castleTexture13);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource13, &castleTexture13SRV);
	// Castle Texture14
	ID3D11Resource *castleResource14 = static_cast<ID3D11Resource*>(castleTexture14);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource14, &castleTexture14SRV);
	// Castle Texture15
	ID3D11Resource *castleResource15 = static_cast<ID3D11Resource*>(castleTexture15);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource15, &castleTexture15SRV);
	// Castle Texture16
	ID3D11Resource *castleResource16 = static_cast<ID3D11Resource*>(castleTexture16);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource16, &castleTexture16SRV);
	// Castle Texture17
	ID3D11Resource *castleResource17 = static_cast<ID3D11Resource*>(castleTexture17);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\Castlewalls_1.png", &castleResource17, &castleTexture17SRV);

#pragma endregion

	skyBox = new Box(device, skyBoxVSBytecode, cubeMapTextureSRV);
	floor = new Grid(device, grassVSBytecode, grassDiffuseMapSRV);
	tree = new DXModel(device, treeVSBytecode, wstring(L"Resources\\Models\\tree.3ds"), treeTextureSRV, XMCOLOR(1.0, 1.0, 1.0, 1.0), XMCOLOR(0, 0, 0.0, 0.0));
	water = new Ocean(device, oceanVSBytecode, waterNormalMapSRV);
	logs = new DXModel(device, perPixelLightingVSBytecode, wstring(L"Resources\\Models\\logs.obj"), logsTextureSRV, XMCOLOR(1.0, 1.0, 1.0, 1.0), XMCOLOR(0, 0, 0.0, 0.0));
	fire = new Particles(device, fireVSBytecode, fireDiffuseMapSRV);
	shark = new DXModel(device, reflectionMapVSBytecode, wstring(L"Resources\\Models\\shark.obj"), sharkTextureSRV, XMCOLOR(1, 1, 1, 1), XMCOLOR(1, 1, 1, 0.5));
	castle = new DXModel(device, reflectionMapVSBytecode, wstring(L"Resources\\Models\\castle.obj"), castleTextureSRV, XMCOLOR(1, 1, 1, 1), XMCOLOR(1, 1, 1, 0.5));
	castle2 = new DXModel(device, reflectionMapVSBytecode, wstring(L"Resources\\Models\\WS.obj"), castleTexture2SRV, XMCOLOR(1, 1, 1, 1), XMCOLOR(1, 1, 1, 0.5));

#pragma endregion LoadTextures

#pragma region Release_Pixel/Vertex_Shaders

	// Release PixelShader DXBlob
		// Skybox
		skyBoxPSBytecode->release();
		// Release vertexShader DXBlob
		skyBoxVSBytecode->release();
		// Grass
		grassVSBytecode->release();
		// Release PixelShader DXBlob
		grassPSBytecode->release();
		// Trees
		// Release vertexShader DXBlob
		treeVSBytecode->release();
		// Release PixelShader DXBlob
		treePSBytecode->release();
		// Reflections
		// Release vertexShader DXBlob
		reflectionMapVSBytecode->release();
		// Release PixelShader DXBlob
		reflectionMapPSBytecode->release();
		// Ocena
		// Release vertexShader DXBlob
		oceanVSBytecode->release();
		// Release PixelShader DXBlob
		oceanPSBytecode->release();
		// Fire
		// Release vertexShader DXBlob
		fireVSBytecode->release();
		//Release PixelShader DXBlob
		firePSBytecode->release();
		// perPixelLighting
		// Release vertexShader DXBlob
		perPixelLightingVSBytecode->release();
		// Release PixelShader DXBlob
		perPixelLightingPSBytecode->release();

#pragma endregion Release_Pixel/Vertex_Shaders 

		return S_OK;
}

#pragma region Update_Scene
// Update scene state (perform animations etc)
HRESULT DXController::updateScene() {

	ID3D11DeviceContext *context = dx->getDeviceContext();

	mainClock->tick();
	gu_seconds tDelta = mainClock->gameTimeElapsed();

	cBufferExtSrc->Timer = (FLOAT)tDelta;
	XMStoreFloat4(&cBufferExtSrc->eyePos, mainCamera->getCameraPos());

	// Update  skyBox/water/shark 1/shark 2/castle/logs cBuffer
	// Scale (x, y, z) and translate water world matrix
	cBufferExtSrc->WVPMatrix = XMMatrixScaling(100, 100, 100)*mainCamera->dxViewTransform()*projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferSky);

#pragma region Update_Water
	/// Water
	///I've moved water instead of everything else so everything else is in the corner of the water area to make it look like the land can be used as a shore
	/// Changed XMMatrixScaling from (10, 5, 10) > (20, 5, 20) too make it easier to add more sharks
	/// Changed MatrixTranslation from (0, -1, 0) > (25, -1, -25)
	cBufferExtSrc->worldMatrix = XMMatrixScaling(20, 5, 20)*XMMatrixTranslation(0, -1, 0);
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferWater);

#pragma endregion

#pragma region Update_Shark1
	/// Shark 1
	//cBufferExtSrc->worldMatrix = XMMatrixScaling(0.5, 0.5, 0.5)*XMMatrixRotationX(1.4)*XMMatrixRotationY(-1.4)*XMMatrixTranslation(-30, 0, 0)*XMMatrixRotationZ(1 * +tDelta);
	cBufferExtSrc->worldMatrix = XMMatrixScaling(1, 1, 1)*XMMatrixTranslation(50, -3.5, 0)*XMMatrixRotationY(1 * -tDelta);
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferShark);

#pragma endregion

#pragma region Update_Shark2
	/// Shark 2
	cBufferExtSrc->worldMatrix = XMMatrixScaling(-1, 1, -1)*XMMatrixTranslation(60, -3.5, 0)*XMMatrixRotationY(1 * tDelta);
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferShark2);

#pragma endregion

#pragma region Update_Castle
	// Castle
	cBufferExtSrc->worldMatrix = XMMatrixScaling(1, 1, 1)*XMMatrixTranslation(4, 0, -23.25)*XMMatrixRotationY(XMConvertToRadians (90));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle);
#pragma endregion

#pragma region CastleWalls

#pragma region Update_Castle2
	// Castle2
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(20, 0, 20)*XMMatrixRotationX(XMConvertToRadians(0));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle2);
#pragma endregion

#pragma region Update_Castle3
	// Castle3
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(5, 0, 20)*XMMatrixRotationX(XMConvertToRadians(0));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle3);
#pragma endregion

#pragma region Update_Castle4
	// Castle4
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(-10, 0, 20)*XMMatrixRotationX(XMConvertToRadians(0));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle4);
#pragma endregion

#pragma region Update_Castle5
	// Castle5
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(18, 0, 25)*XMMatrixRotationY(XMConvertToRadians(-90));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle5);
#pragma endregion

#pragma region Update_Castle6
	// Castle6
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(3, 0, 25)*XMMatrixRotationY(XMConvertToRadians(-90));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle6);
#pragma endregion

#pragma region Update_Castle7
	// Castle7
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(-12, 0, 25)*XMMatrixRotationY(XMConvertToRadians(-90));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle7);
#pragma endregion

#pragma region Update_Castle8
	// Castle8
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(23, 0, 27)*XMMatrixRotationY(XMConvertToRadians(-180));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle8);
#pragma endregion

#pragma region Update_Castle9
	// Castle9
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(8, 0, 27)*XMMatrixRotationY(XMConvertToRadians(-180));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle9);
#pragma endregion

#pragma region Update_Castle10
	// Castle10
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(-7, 0, 27)*XMMatrixRotationY(XMConvertToRadians(-180));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle10);
#pragma endregion

#pragma region Update_Castle11
	// Castle11
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(25, 0, 22)*XMMatrixRotationY(XMConvertToRadians(-270));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle11);
#pragma endregion

#pragma region Update_Castle12
	// Castle12
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(10, 0, 22)*XMMatrixRotationY(XMConvertToRadians(-270));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle12);
#pragma endregion

#pragma region Update_Castle13
	// Castle13
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.045, 0.045, 0.045)*XMMatrixTranslation(-5, 0, 22)*XMMatrixRotationY(XMConvertToRadians(-270));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle13);
#pragma endregion

#pragma endregion

#pragma region Update_Logs
	// Logs
	cBufferExtSrc->worldMatrix = XMMatrixScaling(0.001, 0.001, 0.001)*XMMatrixTranslation(-20, 20, 0)*XMMatrixRotationX(XMConvertToRadians(-90));
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferLogs);
#pragma endregion

	return S_OK;
}
#pragma endregion

// Helper function to copy cbuffer data from cpu to gpu
HRESULT DXController::mapCbuffer(void *cBufferExtSrcL, ID3D11Buffer *cBufferExtL)
{
	ID3D11DeviceContext *context = dx->getDeviceContext();
	// Map cBuffer
	D3D11_MAPPED_SUBRESOURCE res;
	HRESULT hr = context->Map(cBufferExtL, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	if (SUCCEEDED(hr)) {
		memcpy(res.pData, cBufferExtSrcL, sizeof(CBufferExt));
		context->Unmap(cBufferExtL, 0);
	}
	return hr;
}

#pragma region RenderScene
// Render scene
HRESULT DXController::renderScene() {

	ID3D11DeviceContext *context = dx->getDeviceContext();

#pragma region Validate_Window_and_D3Dcontext
	// Validate window and D3D context
	if (isMinimised() || !context)
		return E_FAIL;
#pragma endregion 

#pragma region Clear_Screen
	// Clear the screen
	static const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(dx->getBackBufferRTV(), clearColor);
	context->ClearDepthStencilView(dx->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#pragma endregion

	// Apply  skyBox RSState
	context->RSSetState(skyRSState);

#pragma region Draw_SkyBox
	// Draw Skybox
	if (skyBox) 
	{
		// set the  skyBox shader objects
		context->VSSetShader(skyBoxVS, 0, 0);
		context->PSSetShader(skyBoxPS, 0, 0);
		// Apply the skyBox cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferSky);
		context->PSSetConstantBuffers(0, 1, &cBufferSky);
		// Render Skybox
		skyBox->render(context);
	}
#pragma endregion Draw_SkyBox

	// Return to default RSState
	//context->RSSetState(defaultRSstate);

#pragma region Draw_Floor
	// Draw the Grass
	if (floor)
	{
		// Set ground vertex and pixel shaders
		context->VSSetShader(grassVS, 0, 0);
		context->PSSetShader(grassPS, 0, 0);

		dx->getDeviceContext()->PSSetShaderResources(1, 1, &grassAlphaMapSRV);
		// Terrain
		dx->getDeviceContext()->VSSetShaderResources(0, 1, &grassHeightMapSRV);
		dx->getDeviceContext()->VSSetShaderResources(1, 1, &grassNormalMapSRV);


		// Update floor cBuffer
		// Scale and translate floor world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(7, 7, 7)*XMMatrixTranslation(0, 0, 0);
		cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;


		for (int i = 0; i < numGrassPasses; i++)
		{
			cBufferExtSrc->grassHeight = (grassLength / numGrassPasses)*i;
			mapCbuffer(cBufferExtSrc, cBufferGrass);
			//// Apply the cBuffer.
			context->VSSetConstantBuffers(0, 1, &cBufferGrass);
			context->PSSetConstantBuffers(0, 1, &cBufferGrass);
			// Render
			floor->render(context);
		}
	}
#pragma endregion 

#pragma region Draw_Trees

	// Draw tree
	if (tree) {
		// Render trees
		for (int i = 0; i < NUM_TREES; i++)
		{
			// set  shaders for tree
			context->VSSetShader(treeVS, 0, 0);
			context->PSSetShader(treePS, 0, 0);
			// Update tree cBuffer for each tree instance
			cBufferExtSrc->worldMatrix = treeInstance[i].worldMatrix;
			cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
			cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
			mapCbuffer(cBufferExtSrc, cBufferTree);
			// Apply the tree cBuffer.
			context->VSSetConstantBuffers(0, 1, &cBufferTree);
			context->PSSetConstantBuffers(0, 1, &cBufferTree);

			tree->render(context);
		}
	}
#pragma endregion 

#pragma region Draw_Shark
	// Draw shark
	if (shark) {
		// set shader reflection map shaders for shark
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the shark cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferShark);
		context->PSSetConstantBuffers(0, 1, &cBufferShark);
		// Render
		shark->render(context);
	}
#pragma endregion 

#pragma region Draw_Shark2
	// Draw shark
	if (shark) {
		// set shader reflection map shaders for shark
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the shark cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferShark2);
		context->PSSetConstantBuffers(0, 1, &cBufferShark2);
		// Render
		shark->render(context);
	}
#pragma endregion 

#pragma region Draw_Castle
	// Draw shark
	if (castle) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle);
		// Render
		castle->render(context);
	}
#pragma endregion 

#pragma region CastleWalls

#pragma region Draw_Castle2
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle2);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle2);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle3
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle3);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle3);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle4
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle4);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle4);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle5
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle5);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle5);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle6
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle6);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle6);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle7
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle7);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle7);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle8
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle8);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle8);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle9
	// Draw shark
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle9);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle9);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle10
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle10);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle10);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle11
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle11);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle11);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle12
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle12);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle12);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma region Draw_Castle13
	if (castle2) {
		// set shader reflection map shaders for castle
		context->VSSetShader(reflectionMapVS, 0, 0);
		context->PSSetShader(reflectionMapPS, 0, 0);
		// Apply the castle cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferCastle13);
		context->PSSetConstantBuffers(0, 1, &cBufferCastle13);
		// Render
		castle2->render(context);
	}
#pragma endregion

#pragma endregion 

#pragma region Draw_Water
	// Draw the Water
	if (water) {
		// Set ocean vertex and pixel shaders
		context->VSSetShader(oceanVS, 0, 0);
		context->PSSetShader(oceanPS, 0, 0);

		dx->getDeviceContext()->PSSetShaderResources(1, 1, &cubeMapTextureSRV);

		// Apply the water cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferWater);
		context->PSSetConstantBuffers(0, 1, &cBufferWater);
		// Render
		water->render(context);
	}
#pragma endregion Draw_Water

#pragma region Draw_Logs
	// Draw the Logs
	if (logs) {
		// Set logs vertex and pixel shaders
		context->VSSetShader(perPixelLightingVS, 0, 0);
		context->PSSetShader(perPixelLightingPS, 0, 0);
		// Apply the logs cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferLogs);
		context->PSSetConstantBuffers(0, 1, &cBufferLogs);
		// Render
		logs->render(context);
	}
#pragma endregion Draw_Logs

#pragma region Draw_Fire/Smoke
	// Draw the Fire
	if (fire) {
		// Set fire vertex and pixel shaders
		context->VSSetShader(fireVS, 0, 0);
		context->PSSetShader(firePS, 0, 0);
		//Apply blendState
		FLOAT			blendFactor[] = { 1, 1, 1, 1 };
		context->OMSetBlendState(fireBlendState, blendFactor, 0xFFFFFFFF);
		context->OMSetDepthStencilState(fireDSstate, 0);

		// Apply the cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferFire);
		context->PSSetConstantBuffers(0, 1, &cBufferFire);

		// Update fire and smoke cBuffer
		cBufferExtSrc->Timer = cBufferExtSrc->Timer / 4;// smoke changes more slowly than flames
		// Scale and translate smoke world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(1, 1, 1)*XMMatrixTranslation(-20, 0.5, -20);
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferFire);
		fire->setTexture(smokeDiffuseMapSRV);
		// Render Smoke
		fire->render(context);

		cBufferExtSrc->Timer = cBufferExtSrc->Timer * 6;// fire changes more quickly than flame	
		// Scale and translate fire world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(0.5, 0.5, 0.5)*XMMatrixTranslation(-20, 0.5, -20);
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferFire);
		fire->setTexture(fireDiffuseMapSRV);

		// Render Fire
		fire->render(context);

		context->OMSetBlendState(defaultBlendState, blendFactor, 0xFFFFFFFF);
		context->OMSetDepthStencilState(defaultDSstate, 0);
	}
#pragma endregion Draw_Fire/Smoke

	// Present current frame to the screen
	HRESULT hr = dx->presentBackBuffer();

	return S_OK;

}
#pragma endregion

