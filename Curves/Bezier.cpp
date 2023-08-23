/**********************************************************************************
// Bezier (Código Fonte)
//
// Criação:     20 Ago 2023
// Atualização: 23 Ago 2023
// Compilador:  Visual C++ 2022
//
// Descrição:   Base para gerar curvas usando curvas de Bezier
//
**********************************************************************************/

#include "DXUT.h"

// ------------------------------------------------------------------------------

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

// ------------------------------------------------------------------------------

class Bezier : public App
{
private:
    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineState;
    Mesh* bezier;
    Mesh* points[4];
    Mesh* supLines[2];

    static const uint MaxCurveVertex = 20;
    Vertex curveVertex[20][MaxCurveVertex];
    uint countCurveVertex = 0;
    uint indexCurve = 0;
    uint countCurve = 0;

    uint countCurveSaved = 0;
    Vertex curveVertexSaved[20][MaxCurveVertex];
    uint indexPoint = 0;
    uint indexPointSaved;

    float pointX[4];
    float pointY[4];
    float curveX[20];
    float curveY[20];
    float t = 0;

    float pointXSaved[4];
    float pointYSaved[4];

    static const uint MaxPointsVertex = 5;
    Vertex pointsVertex[4][MaxPointsVertex];

    static const uint MaxSupLineVertex = 2;
    Vertex supLineVertex[2][MaxSupLineVertex];

    float cx = float(window->CenterX());
    float cy = float(window->CenterY());
    float mx = float(input->MouseX());
    float my = float(input->MouseY());

    float x = (mx - cx) / cx;
    float y = (cy - my) / cy;

public:
    void Init();
    void Update();
    void Display();
    void Finalize();

    void BuildRootSignature();
    void BuildPipelineState();
};

// ------------------------------------------------------------------------------

void Bezier::Init()
{
    graphics->ResetCommands();

    // ---------[ Build Geometry ]------------

    // tamanho do buffer de vértices em bytes
    const uint vbSize = (MaxCurveVertex * 20) * sizeof(Vertex);

    // cria malha 3D
    bezier = new Mesh(vbSize, sizeof(Vertex));

    for (int i = 0; i < 4; i++)
    {
        points[i] = new Mesh(5 * sizeof(Vertex), sizeof(Vertex));
    }

    for (int i = 0; i < 2; i++)
    {
        supLines[i] = new Mesh(2 * sizeof(Vertex), sizeof(Vertex));
    }

    // ---------------------------------------

    BuildRootSignature();
    BuildPipelineState();

    // ---------------------------------------

    graphics->SubmitCommands();
}

// ------------------------------------------------------------------------------

void Bezier::Update()
{
    cx = float(window->CenterX());
    cy = float(window->CenterY());
    mx = float(input->MouseX());
    my = float(input->MouseY());

    // converte as coordenadas da tela para a faixa -1.0 a 1.0
    // cy e my foram invertidos para levar em consideração que 
    // o eixo y da tela cresce na direção oposta do cartesiano
    x = (mx - cx) / cx;
    y = (cy - my) / cy;



    // atualiza a contagem de vétices e cruvas
    if (input->KeyPress(VK_LBUTTON))
    {
        if (indexPoint == 3)
        {
            countCurve = (countCurve + 1) % MaxCurveVertex;
            indexPoint = 1;

            pointX[0] = curveX[19];
            pointY[0] = curveY[19];
            pointX[1] = x;
            pointY[1] = y;
        }
        indexPoint++;
    }

    // pontos acompanham as coordenadas do mouse
    if (indexPoint == 0)
    {
        pointX[0] = x;
        pointX[1] = x;
        pointX[2] = x;
        pointX[3] = x;

        pointY[0] = y;
        pointY[1] = y;
        pointY[2] = y;
        pointY[3] = y;
    }
    else if (indexPoint == 1) {

        pointX[1] = 2 * pointX[0] - x;
        pointX[2] = x;
        pointX[3] = x;
        pointY[1] = 2 * pointY[0] - y;
        pointY[2] = y;
        pointY[3] = y;

        // alocando linhas de suporte
        supLineVertex[0][0] = { XMFLOAT3(pointX[1],pointY[1], 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[0][1] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };

    }
    else if (indexPoint == 2) {

        pointX[2] = x;
        pointX[3] = x;
        pointY[2] = y;
        pointY[3] = y;

        // alocando linhas de suporte
        supLineVertex[0][0] = { XMFLOAT3(pointX[1],pointY[1], 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[0][1] = { XMFLOAT3(pointX[0],pointY[0], 0.0f), XMFLOAT4(Colors::Blue) };

    }
    else if (indexPoint == 3) {

        pointX[2] = 2 * pointX[3] - x;
        pointY[2] = 2 * pointY[3] - y;
        // alocando linhas de suporte
        supLineVertex[0][0] = { XMFLOAT3(pointX[1],pointY[1], 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[0][1] = { XMFLOAT3(pointX[0],pointY[0], 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[1][0] = { XMFLOAT3(pointX[2],pointY[2], 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[1][1] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };

    }

    if (indexPoint != 0 || countCurve > 0) {
        // calculo dos vertices da curva de bezier
        for (int i = 0; i < 20; i++)
        {
            t = 0.05f * i;
            curveX[i] = pointX[0] * (1 - t) * (1 - t) * (1 - t) + pointX[1] * 3 * t * (1 - t) * (1 - t) + pointX[2] * 3 * t * t * (1 - t) + pointX[3] * t * t * t;
            curveY[i] = pointY[0] * (1 - t) * (1 - t) * (1 - t) + pointY[1] * 3 * t * (1 - t) * (1 - t) + pointY[2] * 3 * t * t * (1 - t) + pointY[3] * t * t * t;

            curveVertex[countCurve][i] = { XMFLOAT3(curveX[i],curveY[i], 0.0f), XMFLOAT4(Colors::White) };
            if (countCurveVertex < MaxCurveVertex)
                ++countCurveVertex;
        }
    }


    // alocando quadrados nos pontos
    for (int i = 0; i < 4; i++)
    {
        if (i == 1 || i == 2) {
            pointsVertex[i][0] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Red) };
            pointsVertex[i][1] = { XMFLOAT3(pointX[i] + 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Red) };
            pointsVertex[i][2] = { XMFLOAT3(pointX[i] + 0.01f,pointY[i] + 0.02f, 0.0f), XMFLOAT4(Colors::Red) };
            pointsVertex[i][3] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] + 0.02f, 0.0f), XMFLOAT4(Colors::Red) };
            pointsVertex[i][4] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Red) };
        }
        else {
            pointsVertex[i][0] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Yellow) };
            pointsVertex[i][1] = { XMFLOAT3(pointX[i] + 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Yellow) };
            pointsVertex[i][2] = { XMFLOAT3(pointX[i] + 0.01f,pointY[i] + 0.02f, 0.0f), XMFLOAT4(Colors::Yellow) };
            pointsVertex[i][3] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] + 0.02f, 0.0f), XMFLOAT4(Colors::Yellow) };
            pointsVertex[i][4] = { XMFLOAT3(pointX[i] - 0.01f,pointY[i] - 0.02f, 0.0f), XMFLOAT4(Colors::Yellow) };
        }

    }


    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // deleta todas as curvas
    if (input->KeyPress(VK_DELETE)) {
        countCurve = 0;
        indexPoint = 0;
        supLineVertex[1][0] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[1][1] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[0][0] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[0][1] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
    }

    // salva o estado da curva
    if (input->KeyPress('S')) {

        for (int i = 0; i < 20; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                curveVertexSaved[i][j] = curveVertex[i][j];
            }
        }

        for (int i = 0; i < 4; i++)
        {
            pointXSaved[i] = pointX[i];
            pointYSaved[i] = pointY[i];
        }
        indexPointSaved = indexPoint;
        countCurveSaved = countCurve;

    }

    // retoma o estado da curva
    if (input->KeyPress('L') && indexPointSaved != NULL) {

        for (int i = 0; i < 20; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                curveVertex[i][j] = curveVertexSaved[i][j];
            }
        }

        for (int i = 0; i < 4; i++)
        {
            pointX[i] = pointXSaved[i];
            pointY[i] = pointYSaved[i];
        }
        indexPoint = indexPointSaved;
        countCurve = countCurveSaved;
        supLineVertex[1][0] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
        supLineVertex[1][1] = { XMFLOAT3(x,y, 0.0f), XMFLOAT4(Colors::Blue) };
    }

    // copia vértices para o buffer da GPU usando o buffer de Upload
    graphics->ResetCommands();


    graphics->Copy(curveVertex, bezier->vertexBufferSize, bezier->vertexBufferUpload, bezier->vertexBufferGPU);
    for (int i = 0; i < 4; i++)
    {
        graphics->Copy(pointsVertex[i], points[i]->vertexBufferSize, points[i]->vertexBufferUpload, points[i]->vertexBufferGPU);
    }
    for (int i = 0; i < 2; i++)
    {
        graphics->Copy(supLineVertex[i], supLines[i]->vertexBufferSize, supLines[i]->vertexBufferUpload, supLines[i]->vertexBufferGPU);
    }
    graphics->SubmitCommands();
    Display();


}

// ------------------------------------------------------------------------------

void Bezier::Display()
{
    // limpa backbuffer
    graphics->Clear(pipelineState);

    // submete comandos de configuração do pipeline
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
    graphics->CommandList()->IASetVertexBuffers(0, 1, bezier->VertexBufferView());
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

    // submete comandos de desenho
    if (indexPoint != 0) {
        graphics->CommandList()->DrawInstanced(20 * countCurve + 20, 1, 0, 0);
    }

    for (int i = 0; i < 4; i++)
    {
        graphics->CommandList()->IASetVertexBuffers(0, 1, points[i]->VertexBufferView());
        graphics->CommandList()->DrawInstanced(5, 1, 0, 0);
    }

    for (int i = 0; i < 2; i++)
    {
        graphics->CommandList()->IASetVertexBuffers(0, 1, supLines[i]->VertexBufferView());
        graphics->CommandList()->DrawInstanced(2, 1, 0, 0);
    }



    // apresenta backbuffer
    graphics->Present();
}

// ------------------------------------------------------------------------------

void Bezier::Finalize()
{
    rootSignature->Release();
    pipelineState->Release();
    for (int i = 0; i < 4; i++)
    {
        delete points[i];
    }
    for (int i = 0; i < 2; i++)
    {
        delete supLines[i];
    }
    delete bezier;
}


// ------------------------------------------------------------------------------
//                                     D3D                                      
// ------------------------------------------------------------------------------

void Bezier::BuildRootSignature()
{
    // descrição para uma assinatura vazia
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // serializa assinatura raiz
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &error));

    // cria uma assinatura raiz vazia
    ThrowIfFailed(graphics->Device()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));
}

// ------------------------------------------------------------------------------

void Bezier::BuildPipelineState()
{
    // --------------------
    // --- Input Layout ---
    // --------------------

    D3D12_INPUT_ELEMENT_DESC inputLayout[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertexShader;
    ID3DBlob* pixelShader;

    D3DReadFileToBlob(L"Shaders/Vertex.cso", &vertexShader);
    D3DReadFileToBlob(L"Shaders/Pixel.cso", &pixelShader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

    D3D12_RASTERIZER_DESC rasterizer = {};
    rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
    rasterizer.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ---------------------
    // --- Color Blender ---
    // ---------------------

    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    D3D12_DEPTH_STENCIL_DESC depthStencil = {};
    depthStencil.DepthEnable = TRUE;
    depthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencil.StencilEnable = FALSE;
    depthStencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depthStencil.FrontFace = defaultStencilOp;
    depthStencil.BackFace = defaultStencilOp;

    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = rootSignature;
    pso.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depthStencil;
    pso.InputLayout = { inputLayout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = graphics->Antialiasing();
    pso.SampleDesc.Quality = graphics->Quality();
    graphics->Device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState));

    vertexShader->Release();
    pixelShader->Release();
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    try
    {
        // cria motor e configura a janela
        Engine* engine = new Engine();
        engine->window->Mode(WINDOWED);
        engine->window->Size(1024, 600);
        engine->window->ResizeMode(ASPECTRATIO);
        engine->window->Color(30, 30, 30);
        engine->window->Title("Bezier");
        engine->window->Icon(IDI_ICON);
        engine->window->LostFocus(Engine::Pause);
        engine->window->InFocus(Engine::Resume);

        // cria e executa a aplicação
        engine->Start(new Bezier());

        // finaliza execução
        delete engine;
    }
    catch (Error& e)
    {
        // exibe mensagem em caso de erro
        MessageBox(nullptr, e.ToString().data(), "Bezier", MB_OK);
    }

    return 0;
}

// ----------------------------------------------------------------------------
