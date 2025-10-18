/*
 * YOViewer.cpp
 *
 *  Created on: Sep 2, 2025
 *      Author: kurtz
 */
#include "YOViewer.h"
#include "YOKeys.h"
#include "YOXML.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/RenderPipeline/RenderPipeline.h>

#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/RenderPipeline/ShaderConsts.h>
#include <Urho3D/SystemUI/SystemUI.h>
#include <Urho3D/SystemUI/SystemUIEvents.h> // E_SYSTEMUI
#include <Urho3D/SystemUI/ImGui.h>
#include "plugins/YORecorderPlugin.h"

#include "YOTestPlugin.h"
#include "YOPlotterPlugin.h"
#include "YOPolylinePlugin.h"
#include "YOVideoPlugin.h"
#include "YOCameraPlugin.h"
#include "YOGPSPlugin.h"
#include "YOOBD2Plugin.h"
#include "YOCANPlugin.h"
#include "YODataViewerPlugin.h"

void sig_fn(int signal, void *data)
{
    std::cout << signal << " sig_fn " << std::endl;
    exit(0);
}

void YOViewer::Setup()
{
    engineParameters_[EP_WINDOW_TITLE] = "YOViewer";
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_MULTI_SAMPLE] = 4;   // 2/4/8, зависит от GPU
    engineParameters_[EP_VSYNC] = true;       // smoother motion
    engineParameters_[EP_WINDOW_RESIZABLE] = true;
    engineParameters_[EP_WINDOW_MAXIMIZE] = true;
    engineParameters_[EP_BORDERLESS] = false;
}

void YOViewer::Start()
{
    YOFlyController::RegisterObject(context_);
    YORootLogic::RegisterObject(context_);
    YONodeLogic::RegisterObject(context_);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_WIDTH);
    glEnable(GL_LINE_SMOOTH);

    CreateConfig();
    CreateCamera();
    CreateScene();
    CreateLight();
    GetSubsystem<Engine>()->SetMaxFps(30);

    plugin_bus_->SetConfig(&config_->get(yo::k::plugins));
    plugin_bus_->AddPlugin("Camera", new YOCameraPlugin(context_));
    plugin_bus_->AddPlugin("Plotter", new YOPlotterPlugin(context_));
    plugin_bus_->AddPlugin("Test", new YOTestPlugin(context_));
    plugin_bus_->AddPlugin("GPS", new YOGPSPlugin(context_));
    plugin_bus_->AddPlugin("ExternalData", new YOPolylinePlugin(context_));
    plugin_bus_->AddPlugin("InternalData", new YOPolylinePlugin(context_));
    plugin_bus_->AddPlugin("Video", new YOVideoPlugin(context_));
    plugin_bus_->AddPlugin("OBDII", new YOOBD2Plugin(context_));
    plugin_bus_->AddPlugin("CAN Viewer", new YOCANPlugin(context_));
    plugin_bus_->AddPlugin("Data Viewer", new YODataViewerPlugin(context_));
    plugin_bus_->AddPlugin("Recorder", new YORecorderPlugin(context_));
    plugin_bus_->OnStart(scene_);

    CreateXYGrid(world_, 200, 50, 1.0f, 0.0f);
    CreateXYGrid(world_, 20, 5, 10.0f, 0.0f);
    CreateXYGrid(world_, 4, 1, 50.0f, 0.0f);

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(YOViewer, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(YOViewer, HandleUpdate));
}

void YOViewer::CreateLight()
{
	Node *lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetPosition(Vector3(1.0f, 2.0f, 100.0f));
    lightNode->SetDirection(Vector3(10.0f, 20.0f, -100.0f));
    auto *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetBrightness(1.0f);

	Node *lightNodeOverlay = scene_overlay_->CreateChild("DirectionalLightOverlay");
	lightNodeOverlay->SetPosition(Vector3(0.0f, 0.0f, 300.0f));
	lightNodeOverlay->SetDirection(Vector3(10.0f, 50.0f, -100.0f));
	auto *lightOverlay = lightNodeOverlay->CreateComponent<Light>();
	lightOverlay->SetLightType(LIGHT_DIRECTIONAL);
	lightOverlay->SetBrightness(1.0f);
    //!!! light->SetLightMask(YO_LMASK_WORLD);
}

void YOViewer::CreateConfig(std::string fileName)
{
    config_ = std::make_shared<YOVariant>(yo::k::config);
    YOXML xml;

    if(!xml.readXML(fileName, *config_))
    {
        std::cout << "Error loading " << fileName << ". Creating default config." << std::endl;
        YOVariant &global = config_->get(yo::k::global);
        global[yo::k::broker][yo::k::sender]   = YOIPv4{ {127, 0, 0, 1}, 4444 };
        global[yo::k::broker][yo::k::receiver] = YOIPv4{ {127, 0, 0, 1}, 4445 };
        YOVariant &plugins = config_->get(yo::k::plugins);
    }
}

void YOViewer::CreateScene()
{
    technique_ = cache_->GetResource<Technique>("Techniques/NoTextureVColAddAlpha.xml");
    technique_overlay_ = cache_->GetResource<Technique>("Techniques/NoTextureAlpha.xml")->Clone("Overlay");

    for(int i = 0; i < technique_overlay_->GetNumPasses(); i++ )
    {
        std::cout << " PASS " << i << "  " << technique_overlay_->GetPassNames().at(i).c_str() << std::endl;
        Pass* p = technique_overlay_->GetPass(technique_overlay_->GetPassNames().at(i).c_str());
        p->SetDepthTestMode(CMP_ALWAYS);
        p->SetDepthWrite(false);
    }

    Node *zoneNode = scene_->CreateChild("Zone");
    auto *zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.03f, 0.03f, 0.03f));
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	Node *zoneNodeOverlay = scene_overlay_->CreateChild("ZoneOverlay");
	auto *zoneOverlay = zoneNodeOverlay->CreateComponent<Zone>();
	zoneOverlay->SetAmbientColor(Color(0.7f, 0.7f, 0.7f));
	zoneOverlay->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

   {
    	Urho3D::Node *myNode = world_->CreateChild("SampleNode");
        myNode->SetPosition(Urho3D::Vector3(0.0f, 0.0f, 0.0f));
        myNode->SetRotation(Urho3D::Quaternion(180, 0, -90));

        Urho3D::StaticModel* myObject = myNode->CreateComponent<Urho3D::StaticModel>();
        myObject->SetModel(cache_->GetResource<Urho3D::Model>("Models/Passat.mdl"));
        auto matp = cache_->GetResource<Urho3D::Material>("Materials/Passat.xml");
        myObject->SetMaterial(matp);
   }
}

void YOViewer::CreateCamera()
{
    auto *renderer = GetSubsystem<Renderer>();
	scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    context_->SetGlobalVar("YO_MAIN_SCENE", Variant(scene_));
    camera_ = scene_->CreateComponent<Camera>();
    camera_node_= scene_->CreateChild("CameraNode");
    controller_ = camera_node_->CreateComponent<YOFlyController>();
    controller_->SetCamera(camera_);
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera_));
    renderer->SetViewport(0, viewport);

    scene_overlay_ = new Scene(context_);
    scene_overlay_->CreateComponent<Octree>();
    context_->SetGlobalVar("YO_OVERLAY_SCENE", Variant(scene_overlay_));
    camera_overlay_ = scene_overlay_->CreateComponent<Camera>();
    camera_node_overlay_ = scene_overlay_->CreateChild("OverlayCameraNode");
    controller_overlay_ = camera_node_overlay_->CreateComponent<YOFlyController>();
    controller_overlay_->SetCamera(camera_overlay_);
    controller_overlay_->SetPosition({0, 0, 100});
    controller_overlay_->SetRotation(0, 0,0);
    controller_overlay_->EnableUpdate(false);

    SharedPtr<Viewport>viewport_overlay(new Viewport(context_, scene_overlay_, camera_overlay_));

    overlay_ = MakeShared<Texture2D>(context_);
    overlay_->SetSize(1920, 1080, TextureFormat::TEX_FORMAT_RGBA32_FLOAT, TextureFlag::BindRenderTarget);
    overlay_->GetRenderSurface()->SetViewport(0, viewport_overlay);
    overlay_->GetRenderSurface()->SetUpdateMode(SURFACE_UPDATEALWAYS);

    auto* ui = GetSubsystem<UI>();
	auto* root = ui->GetRoot();
	root->SetOpacity(1.0f);
	overlay_pad_ = root->CreateChild<Sprite>();
	overlay_pad_->SetTexture(overlay_);
	overlay_pad_->SetSize(overlay_->GetSize());
	overlay_pad_->SetBlendMode(BLEND_ADD);
	overlay_pad_->SetPosition(0,0);
	overlay_pad_->SetOpacity(1);
	overlay_pad_->SetColor(Color(1,1,1,1));

    world_ = scene_->CreateChild("Root");
    internal_ = scene_->CreateChild("Internal");
}

#define DEG2RAD(x) ((x) * 0.0174532925199433)

void YOViewer::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if(exit_)
    {
        YOXML xml;
        xml.writeXML("config.xml", *config_);
        plugin_bus_->OnStop();
        engine_->Exit();
    }

    using namespace Update;
    float dt = eventData[P_TIMESTEP].GetFloat();
    plugin_bus_->OnUpdate(dt);
    RenderUI();
    plugin_bus_->OnGui();
}

void YOViewer::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    if (eventData[P_KEY].GetInt() == KEY_ESCAPE)
    {
        exit_ = true;
    }
}

void YOViewer::CreateXYGrid(Node *parent, int cellsX, int cellsY, float spacing, float z)
{
    Node *grid = parent->CreateChild("Grid");

    auto *grid_cg = grid->CreateComponent<CustomGeometry>();

    SharedPtr<Material> grid_mat(new Material(context_));

    grid_mat->SetTechnique(0, technique_);

    Color grid_clr = Color(0.125f, 0.125f, 0.125f, 0.125f);

    grid_cg->BeginGeometry(0, LINE_LIST);
    for( float x = -cellsX * spacing; x <= cellsX * spacing; x+=spacing)
    {
        grid_cg->DefineNormal(Vector3::FORWARD);
        grid_cg->DefineVertex(Vector3(x, -cellsY * spacing, z));
        grid_cg->DefineColor(grid_clr);
        grid_cg->DefineVertex(Vector3( x, cellsY * spacing, z));
        grid_cg->DefineColor(grid_clr);
    }

    for( int y = -cellsY * spacing; y <= cellsY * spacing; y+=spacing)
    {
        grid_cg->DefineVertex(Vector3(-cellsX * spacing, y, z));
        grid_cg->DefineColor(grid_clr);
        grid_cg->DefineVertex(Vector3(cellsX * spacing, y, z));
        grid_cg->DefineColor(grid_clr);
    }
    grid_cg->Commit();

    grid_mat->SetCullMode(CULL_CW);
    grid_cg->SetMaterial(grid_mat);
}

void YOViewer::RenderUI()
{
    if (ui::BeginMainMenuBar())
    {
        if (ui::BeginMenu("File"))
        {
            if (ui::MenuItem("Open..."))
            {
                // TODO: open logic
            }
            if (ui::MenuItem("Save"))
            {
                // TODO: save logic
            }
            ui::EndMenu();
        }
        if (ui::BeginMenu("Plugins"))
        {
           plugin_bus_->OnMenu();
           ui::EndMenu();
        }
        if (ui::BeginMenu("Help"))
        {
            if (ui::MenuItem("About")) { /* … */ }
            ui::EndMenu();
        }
        ui::EndMainMenuBar();
    }
}
