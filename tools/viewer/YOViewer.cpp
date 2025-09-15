/*
 * YOViewer.cpp
 *
 *  Created on: Sep 2, 2025
 *      Author: kurtz
 */
#include "YOViewer.h"
#include "UI/Text3D.h"
#include "UI/Font.h"
#include "YOKeys.h"
#include "YOXML.h"

#include <Urho3D/SystemUI/SystemUI.h>
#include <Urho3D/SystemUI/SystemUIEvents.h> // E_SYSTEMUI
//#include <Urho3D/SystemUI/imgui.h>

int fn_input(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
    YOViewer *viewer = (YOViewer *)param;
    //std::cout  << topic << " fn_hdl32 !!! data size: "  << message->getDataSize() << std::endl;
    std::shared_ptr<YOVariant> frame = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
    //frame->print();
    viewer->AddFrame(frame, 0);
    return 0;
}
void sig_fn(int signal, void *data)
{
    std::cout << signal << " sig_fn " << std::endl;
    exit(0);
}

void *fn_thread_input(void *param)
{
    std::cout << "START!!! "<< std::endl;

    YONode node("RECEIVER");
    node.connect();
    node.addSignalFunction(SIGINT, sig_fn, &node);

    node.subscribe("INPUT0", fn_input, param);
    node.subscribe("INPUT1", fn_input, param);
    node.subscribe("INPUT2", fn_input, param);
    node.subscribe("INPUT3", fn_input, param);
    node.subscribe("INPUT4", fn_input, param);
    node.subscribe("INPUT5", fn_input, param);
    node.subscribe("INPUT6", fn_input, param);
    node.subscribe("INPUT7", fn_input, param);
    node.subscribe("INPUT8", fn_input, param);
    node.subscribe("INPUT9", fn_input, param);
    std::cout << "START RECEIVER "<< std::endl;
    node.start();
    std::cout << "STOP RECEIVER "<< std::endl;
    node.disconnect();
    node.shutdown();
    return  param;
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

    CreateConfig();

    CreateCamera();

    CreateScene();
    CreateLight();

    CreateXYGrid(world_, 200, 50, 1.0f, 0.0f);
    CreateXYGrid(world_, 20, 5, 10.0f, 0.0f);
    CreateXYGrid(world_, 4, 1, 50.0f, 0.0f);

    auto *renderer = GetSubsystem<Renderer>();
    SharedPtr<Viewport>viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_WIDTH);
    glEnable(GL_LINE_SMOOTH);

    GetSubsystem<Engine>()->SetMaxFps(30);   // 0 = без лимита
        // Не забудь отключить VSync, иначе он сильнее:
    //GetSubsystem<Graphics>()->SetVSync(false);

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(YOViewer, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(YOViewer, HandleUpdate));
    pthread_create(&thread_inputs_, NULL, fn_thread_input, this);
}

void YOViewer::CreateLight()
{
    Node *lightNode = world_->CreateChild("DirectionalLight");
    lightNode->SetPosition(Vector3(1.0f, 2.0f, 100.0f));
    lightNode->SetDirection(Vector3(0.15f, 0.15f, -0.5f));
    auto *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetBrightness(1.2f);
    light->SetLightMask(YO_LMASK_WORLD);
    {
        Node *lightNode = overlay_->CreateChild("DirectionalLight");
        lightNode->SetPosition(Vector3(1.0f, 2.0f, 100.0f));
        lightNode->SetDirection(Vector3(0.15f, 0.15f, 0.50f));
        auto *light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetBrightness(1.2f);
        light->SetLightMask(YO_LMASK_OVERLAY);
    }
}

void YOViewer::AddFrame(std::shared_ptr<YOVariant> frame, int frame_id)
{
    data_lock_[frame_id].lock();
    data_in_[frame_id] = frame;
    data_lock_[frame_id].unlock();
}

void YOViewer::CreateConfig()
{
    config_ = std::make_shared<YOVariant>(yo::k::config);
    YOXML xml;

    if(!xml.readXML("config.xml", *config_))
    {
        std::cout << "ERROR LOADING CONFIG" << std::endl;

        YOVariant &world = config_->get(yo::k::world);
        YOVariant &internal = config_->get(yo::k::internal);
        YOVariant &global = config_->get(yo::k::global);

        internal.m_name = yo::k::internal;
        internal.m_value = YOArray(YO_INPUT_NUM);

        world.m_name = yo::k::world;
        world.m_value = YOArray(YO_INPUT_NUM);

        global[yo::k::broker][yo::k::sender]   = YOIPv4{ {127, 0, 0, 1}, 4444 };
        global[yo::k::broker][yo::k::receiver] = YOIPv4{ {127, 0, 0, 1}, 4445 };

        for(int i = 0; i < YO_INPUT_NUM; i++)
        {
            createInputCfg(i, world[i]);
            createInputCfg(i, internal[i]);
        }
        //config_->print();
    }

    gui_ = std::make_shared<YOGui>(config_);
}

void YOViewer::CreateScene()
{
    ResourceCache *cache = GetSubsystem<ResourceCache>();
    technique_ = cache->GetResource<Technique>("Techniques/NoTextureVColAddAlpha.xml");

    technique_overlay_ = cache->GetResource<Technique>("Techniques/NoTextureAlpha.xml")->Clone("Overlay");

    for(int i = 0; i < technique_overlay_->GetNumPasses(); i++ )
    {
        std::cout << " PASS " << i << "  " << technique_overlay_->GetPassNames().at(i).c_str() << std::endl;
        Pass* p = technique_overlay_->GetPass(technique_overlay_->GetPassNames().at(i).c_str());
        p->SetDepthTestMode(CMP_ALWAYS);
        p->SetDepthWrite(false);
    }

    Node *zoneNode = world_->CreateChild("Zone");
    auto *zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.03f, 0.03f, 0.03f));
//        zone->SetFogColor(Color(0.25f, 0.25f, 0.25f));
//        zone->SetFogStart(50.0f);
//        zone->SetFogEnd(200.0f);
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

    {
        Urho3D::Node *myNode = world_->CreateChild("SampleNode");
        myNode->SetPosition(Urho3D::Vector3(0.0f, 0.0f, 0.0f));
        myNode->SetRotation(Urho3D::Quaternion(180, 0, -90));

        Urho3D::StaticModel* myObject = myNode->CreateComponent<Urho3D::StaticModel>();
        myObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Passat.mdl"));
        auto matp = cache->GetResource<Urho3D::Material>("Materials/Passat.xml");
        myObject->SetMaterial(matp);
    }
    {
        Node *boxNode = overlay_->CreateChild("Plane");

        auto *boxModel = boxNode->CreateComponent<StaticModel>();
        boxModel->SetModel(cache->GetResource<Model>("Models/box.mdl"));
        boxNode->SetPosition(Vector3(10,20,0));
        boxNode->Rotate(Quaternion(0,45,45));

        //SharedPtr<Material>mat(new Material(context_));
        auto mat = cache->GetResource<Material>("Materials/DefaultGrey.xml")->Clone("R");
        mat->SetTechnique(0, technique_overlay_);
        mat->SetRenderOrder(250);
        mat->SetShaderParameter("MatDiffColor", Color(1, 1, 1, 0.95));
        //mat->SetCullMode(CULL_NONE);
        boxModel->SetMaterial(mat);
        boxModel->SetLightMask(YO_LMASK_OVERLAY);
        //boxModel->SetViewMask();

        //Node* label = boxNode->CreateChild("Label");
        //label->SetPosition(Vector3(-2,2,0));
        auto* txt = boxNode->CreateComponent<Text3D>();
        txt->SetLightMask(YO_LMASK_OVERLAY);
        //auto* cache = GetSubsystem<ResourceCache>();
        txt->SetText("    OVERLAY #9");
        txt->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        txt->SetFaceCameraMode(FC_ROTATE_XYZ);
        txt->SetFixedScreenSize(true);
        txt->SetDepthTest(false);
        //txt->SetMaterial(mat->Clone(":"));
        //txt->SetColor(Color(Color::WHITE));
        //txt->SetMaterial(mat->Clone("TXT"));
    }
}

void YOViewer::CreateCamera()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    world_ = scene_->CreateChild("WorldRoot");
    internal_ = scene_->CreateChild("Internal");

    cameraNode_ = world_->CreateChild("Camera");
    overlay_ = cameraNode_->CreateChild("Overlay");
    overlay_->SetPosition(Vector3(0, 0, 50));
    overlay_->Rotate(Quaternion(0, 0, 90));

    cameraNode_->SetPosition(Vector3(0, 0, 100));
    cameraNode_->LookAt(Vector3(0, 0, 0), Vector3(1, 0, 0), TS_WORLD);
    controller_ = cameraNode_->CreateComponent<YOFlyController>();
    // Camera
    camera_ = cameraNode_->CreateComponent<Camera>();
    //camera_->SetOrthographic(true);
}

void YOViewer::createMaterial(int input, int type, YOVariant &style)
{
    materials_update[input][type] = true;
    std::cout << "-------------CREATE MATERIAL " << input << " " << type << std::endl;
    materials_[input][type] = SharedPtr<Material>(new Material(context_));
    auto *cache = GetSubsystem<ResourceCache>();
    auto tech = cache->GetResource<Technique>("Techniques/YOPoints.xml");
    YOLimitF &limWidth = style[yo::k::line][yo::k::width];

     if (materials_[input][type] && tech)
     {
         materials_[input][type]->SetTechnique(0, tech);
         materials_[input][type]->SetCullMode(CULL_NONE);
         materials_[input][type]->SetShaderParameter("PointSize", limWidth.value, true);
     }
}

#define DEG2RAD(x) ((x) * 0.0174532925199433)

Node* YOViewer::ConvertFrame(std::shared_ptr<YOVariant> frame, int frame_id)
{
    YOVariant &input_cfg = config_->get(yo::k::world)[frame_id];

    Node *fnode = world_->CreateChild();
    fnode->SetTemporary(true);
    YOVector3 &rot = input_cfg[yo::k::transform][yo::k::rotation].get<YOVector3>();

    fnode->SetPosition( *(Vector3*)&input_cfg[yo::k::transform][yo::k::position].get<YOVector3>());
    fnode->SetRotation(Quaternion(rot.x, rot.y, rot.z));
    fnode->SetScale(*(Vector3*)&input_cfg[yo::k::transform][yo::k::scale].get<YOVector3>());

    YOVariant &objects = frame->get(yo::k::objects);
    YOColor4CList &colors = frame->get(yo::k::colors);

    for(int i = 0; i < objects.getArraySize(); i++)
    {
        YOVariant &obj = objects[i];
        uint32_t type = obj[yo::k::style_id];
        YOVariant &type_cfg = input_cfg[yo::k::types][type];

        Node *node = fnode->CreateChild();
        node->SetTemporary(true);
        auto *cg = node->CreateComponent<CustomGeometry>();
        cg->BeginGeometry(0, POINT_LIST);

        YOColor4F &clr = type_cfg[yo::k::line][yo::k::color];
        YOVector3List &vertices = obj[yo::k::vertices];
        for( auto &vertex : vertices)
        {
            cg->DefineNormal(Vector3::FORWARD);
            cg->DefineVertex((Urho3D::Vector3&)vertex);
            cg->DefineColor(*(Color*)&clr);
        }

        if(!materials_update[frame_id][type])
        {
            std::cout << "!!!!!!!! materials_update " << frame_id << " " << type << std::endl;
            createMaterial(frame_id, type, type_cfg);
        }
        cg->SetMaterial(materials_[frame_id][type]);
        cg->Commit();
    }
    return fnode;
}

void YOViewer::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if(exit_)
    {
        YOXML xml;
        xml.writeXML("config.xml", *config_);
        engine_->Exit();
    }

    using namespace Update;
    float dt = eventData[P_TIMESTEP].GetFloat();

    auto *cache = GetSubsystem<ResourceCache>();

    for (std::size_t i = 0; i < data_in_.size(); ++i)
    {
       data_lock_[i].lock();
       std::shared_ptr<YOVariant> frame = data_in_[i];
       data_in_[i] = nullptr;
       data_lock_[i].unlock();

       //std::cout << "UPDATE !!!! " << i << " : " << std::endl;
       if(frame != nullptr )
       {
           //std::cout << "UPDATE !!!! " << i << " : " << data_in_[i]->m_name << std::endl;
           if(data_[i] != nullptr)
               data_[i]->Remove();

           data_[i] = ConvertFrame(frame, i);
       }
    }
    RenderUI();

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

YOVariant *getConfig(YOVariant  &config, const std::string &path)
{
    YOVariant *res = &config;
    auto plist = split_by_string(path.c_str() + strlen("/config/"), "/");
    int32_t addr[2] = {-1, -1};
    uint32_t pos = 0;
    for(auto &pelem : plist)
    {
        if(res->m_value.index() == 0) // map
        {
            res = &res->get(yo_keys_rev[pelem]);
        }
        else if(res->m_value.index() == 1) // array
        {
            uint32_t n = std::atoi(pelem.c_str());
            addr[pos++] = n;
            res = &res->get(n);
        }
    }
    printf("Changed Input: %d, Type: %d\n", addr[0], addr[1]);
    return res;
}

void YOViewer::ProcessChange()
{
    std::string path = gui_->getPath();
    std::string param = gui_->getParam();
    YOVariant *cfg = getConfig(*config_, path);
    std::cout << " Got config " << cfg->m_name << " type: " <<  YOValue_type_name(cfg->m_value.index())  << " Value :" << cfg->m_value << std::endl;
}

void YOViewer::RenderUI(){

    ui::SetNextWindowSize(ImVec2(550, 500), ImGuiCond_FirstUseEver);
    ui::SetNextWindowPos(ImVec2(350, 50), ImGuiCond_FirstUseEver);

    ui::Begin("YO::HUD");
    bool changed = gui_->draw();
    ui::End();
    if(changed)
    {
        ProcessChange();
    }
}
