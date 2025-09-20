/*
 * YOViewer.cpp
 *
 *  Created on: Sep 2, 2025
 *      Author: kurtz
 */
#include "YOViewer.h"
#include "YOKeys.h"
#include "YOXML.h"
#include <turbojpeg.h>

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

int fn_input(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
    YOViewer *viewer = (YOViewer *)param;
    //std::cout  << topic << " fn_hdl32 !!! data size: "  << message->getDataSize() << std::endl;
    std::shared_ptr<YOVariant> frame = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
    //frame->print();
    viewer->AddFrame(frame, viewer->GetTopicId(topic));
    return 0;
}

inline bool DecodeJpegToRGBA(const uint8_t* data, size_t size, SharedPtr<Image> rgba, int& width, int& height)
{
    tjhandle tj = tjInitDecompress();
    if (!tj)
    	return false;

    int subsamp = 0, colorspace = 0;
    if (tjDecompressHeader3(tj, data, (unsigned long)size, &width, &height, &subsamp, &colorspace) != 0)
    {
    	tjDestroy(tj);
    	return false;
    }
    rgba->SetSize(width,height, 4);
    const int flags = TJFLAG_FASTDCT; //TJFLAG_ACCURATEDCT; //TJFLAG_FASTDCT;
    if (tjDecompress2(tj, data, (unsigned long)size, rgba->GetData(), width, 0, height, TJPF_RGBA, flags) != 0)
    {
    	tjDestroy(tj);
    	return false;
    }
    tjDestroy(tj);
    return true;
}

int fn_video(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
    YOViewer *viewer = (YOViewer *)param;
	YOImageData *img_info = (YOImageData *)message->getData();
	if(img_info->format == YOFrameFormat::YO_JPEG)
	{
		int width, height;
		auto rgba = MakeShared<Image>(viewer->GetContext());
		if(DecodeJpegToRGBA(message->getExtData(), message->getExtDataSize(), rgba, width, height))
		{
			int input = viewer->GetTopicId(topic);

			viewer->AddVideo(rgba, input);
		}
	}
    //viewer->AddVideo(message, viewer->GetTopicId(topic));
    return 0;
}

void sig_fn(int signal, void *data)
{
    std::cout << signal << " sig_fn " << std::endl;
    exit(0);
}

void *fn_thread_input(void *param)
{
    YONode node("GEOM");
    node.connect();
    node.addSignalFunction(SIGINT, sig_fn, &node);
    for(int i = 0; i < YO_INPUT_NUM; i++)
    {
    	std::string topic = "INPUT" + std::to_string(i);
    	YOViewer *viewer = (YOViewer *)param;
    	viewer->RegisterTopic(topic, i);
    	node.subscribe(topic.c_str(), fn_input, param);
    }
    node.start();
    node.disconnect();
    node.shutdown();
    return  param;
}

void *fn_thread_video(void *param)
{
    YONode node("VIDEO");
    node.connect();
    node.addSignalFunction(SIGINT, sig_fn, &node);
    for(int i = 0; i < YO_VIDEO_NUM; i++)
    {
    	std::string topic = "VIDEO" + std::to_string(i);
    	YOViewer *viewer = (YOViewer *)param;
    	viewer->RegisterTopic(topic, i);
    	node.subscribe(topic.c_str(), fn_video, param);
    }
    node.start();
    node.disconnect();
    node.shutdown();
    return  param;
}

void YOViewer::RegisterTopic(const std::string &name, int num)
{
	topics_[name] = num;
}

int YOViewer::GetTopicId(const std::string &name)
{
	return topics_[name];
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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_WIDTH);
    glEnable(GL_LINE_SMOOTH);

    CreateConfig();
    CreateCamera();
    CreateScene();
    CreateLight();
    CreateTextures();

    CreateXYGrid(world_, 200, 50, 1.0f, 0.0f);
    CreateXYGrid(world_, 20, 5, 10.0f, 0.0f);
    CreateXYGrid(world_, 4, 1, 50.0f, 0.0f);

    auto *renderer = GetSubsystem<Renderer>();
    SharedPtr<Viewport>viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    GetSubsystem<Engine>()->SetMaxFps(30);

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(YOViewer, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(YOViewer, HandleUpdate));
    pthread_create(&thread_inputs_, NULL, fn_thread_input, this);
    pthread_create(&thread_videos_, NULL, fn_thread_video, this);
}

void YOViewer::CreateLight()
{
    Node *lightNode = world_->CreateChild("DirectionalLight");
    lightNode->SetPosition(Vector3(1.0f, 2.0f, 100.0f));
    lightNode->SetDirection(Vector3(0.15f, 0.15f, -0.5f));
    auto *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetBrightness(1.2f);
    //!!! light->SetLightMask(YO_LMASK_WORLD);
    {
        Node *lightNode = overlay_->CreateChild("DirectionalLight");
        lightNode->SetPosition(Vector3(1.0f, 2.0f, 100.0f));
        lightNode->SetDirection(Vector3(0.15f, 0.15f, 0.50f));
        auto *light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetBrightness(1.2f);
        //!!!!light->SetLightMask(YO_LMASK_OVERLAY);
    }
}

void YOViewer::AddFrame(std::shared_ptr<YOVariant> frame, int frame_id)
{
    data_lock_[frame_id].lock();
    data_in_[frame_id] = frame;
    data_lock_[frame_id].unlock();
}

void YOViewer::AddVideo(SharedPtr<Image> frame, int frame_id)
{
    video_lock_[frame_id].lock();
    video_img_[frame_id] = frame;
    video_lock_[frame_id].unlock();
}

void YOViewer::CreateConfig()
{
    config_ = std::make_shared<YOVariant>(yo::k::config);
    YOXML xml;

    if(!xml.readXML("config.xml", *config_))
    {
        std::cout << "ERROR LOADING CONFIG" << std::endl;

        YOVariant &world = config_->get(yo::k::world);
        world.m_name = yo::k::world;
        world.m_value = YOArray(YO_INPUT_NUM);

        YOVariant &internal = config_->get(yo::k::internal);
        internal.m_name = yo::k::internal;
        internal.m_value = YOArray(YO_INPUT_NUM);

        for(int i = 0; i < YO_INPUT_NUM; i++)
        {
            createInputCfg(i, world[i]);
            createInputCfg(i, internal[i]);
        }

        YOVariant &global = config_->get(yo::k::global);
        global[yo::k::broker][yo::k::sender]   = YOIPv4{ {127, 0, 0, 1}, 4444 };
        global[yo::k::broker][yo::k::receiver] = YOIPv4{ {127, 0, 0, 1}, 4445 };

        YOVariant &video = config_->get(yo::k::video);
        video.m_name = yo::k::video;
        video.m_value = YOArray(YO_INPUT_NUM);
        for(int i = 0; i < YO_VIDEO_NUM; i++)
        {
            createVideoCfg(i, video[i], yo::k::video);
        }
        //config_->print();
    }


    gui_ = std::make_shared<YOGui>(config_);
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
        myObject->SetModel(cache_->GetResource<Urho3D::Model>("Models/Passat.mdl"));
        auto matp = cache_->GetResource<Urho3D::Material>("Materials/Passat.xml");
        myObject->SetMaterial(matp);
    }
    {
        Node *boxNode = overlay_->CreateChild("Plane");

        auto *boxModel = boxNode->CreateComponent<StaticModel>();
        boxModel->SetModel(cache_->GetResource<Model>("Models/box.mdl"));
        boxNode->SetPosition(Vector3(15,20,0));
        boxNode->Rotate(Quaternion(0,45,45));

        //SharedPtr<Material>mat(new Material(context_));
        auto mat = cache_->GetResource<Material>("Materials/DefaultGrey.xml")->Clone("R");
        mat->SetTechnique(0, technique_overlay_);
        mat->SetRenderOrder(250);
        mat->SetShaderParameter("MatDiffColor", Color(1, 1, 1, 0.95));
        //mat->SetCullMode(CULL_NONE);
        boxModel->SetMaterial(mat);
        //!!!boxModel->SetLightMask(YO_LMASK_OVERLAY);
        //boxModel->SetViewMask();

        //Node* label = boxNode->CreateChild("Label");
        //label->SetPosition(Vector3(-2,2,0));
        auto* txt = boxNode->CreateComponent<Text3D>();
        //!!!txt->SetLightMask(YO_LMASK_OVERLAY);
        //auto* cache = GetSubsystem<ResourceCache>();
        txt->SetText("    OVERLAY #9");
        txt->SetFont(cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        txt->SetFaceCameraMode(FC_ROTATE_XYZ);
        txt->SetFixedScreenSize(true);
        txt->SetDepthTest(false);
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

SharedPtr<Texture2D> YOViewer::CreateTexture()
{
	SharedPtr<Texture2D> tex = MakeShared<Texture2D>(context_);
    tex->SetNumLevels(1); // no MIPs
    tex->SetAddressMode(TextureCoordinate::U, ADDRESS_CLAMP);
    tex->SetAddressMode(TextureCoordinate::V, ADDRESS_CLAMP);
    tex->SetSize(YO_INPUT_NUM, YO_TYPE_NUM, TextureFormat::TEX_FORMAT_RGBA32_FLOAT, TextureFlag::BindRenderTarget | TextureFlag::BindUnorderedAccess );
    tex->SetFilterMode(FILTER_NEAREST);
    return tex;
}

void YOViewer::CreateSprite(SharedPtr<Texture2D> tex, Vector2 pos)
{
    auto* ui = GetSubsystem<UI>();
    auto* root = ui->GetRoot();
	Sprite* spr = root->CreateChild<Sprite>();
	spr->SetTexture(tex);
	spr->SetSize(tex->GetWidth(), tex->GetHeight());
	spr->SetBlendMode(BLEND_ADDALPHA);
	spr->SetPosition(pos);
	spr->SetPriority(0);
	spr->SetScale(8);
}

void YOViewer::CreateTextures()
{
    YOVariant &wrld = config_->get(yo::k::world);

    texture_param_ = CreateTexture();
    texture_line_ = CreateTexture();
    texture_fill_ = CreateTexture();
    texture_text_ = CreateTexture();

    material_line_ =  cache_->GetResource<Material>("Materials/YOTextureOnly.xml");
    material_line_->SetTexture(ShaderResources::Albedo, texture_line_);
    material_line_->SetTexture(ShaderResources::Properties, texture_param_);
    material_line_->SetCullMode(CULL_NONE);
    //material_line_->SetShaderParameter("PointSize", 3.0f, true);

    for(int input = 0; input < wrld.getArraySize(); input++)
    {
        YOVariant &inp_cfg = wrld[input][yo::k::types];
        for(int type = 0; type < inp_cfg.getArraySize(); type++)
        {
            YOVariant &type_cfg = inp_cfg[type];
            YOVariant &line_cfg = type_cfg[yo::k::line];
            texture_line_->SetData(0, input, type, 1, 1, &line_cfg[yo::k::color].get<YOColor4F>());

            YOLimitF &line_width = line_cfg[yo::k::width].get<YOLimitF>();
            YOColor4F width{line_width.value / 16.0f, 1.0f, 1.0f, 1.0f};
            texture_param_->SetData(0, input, type, 1, 1, &width);

            YOVariant &fill_cfg = type_cfg[yo::k::fill];
            texture_fill_->SetData(0, input, type, 1, 1, &fill_cfg[yo::k::color].get<YOColor4F>());

            YOVariant &text_cfg = type_cfg[yo::k::text];
            texture_text_->SetData(0, input, type, 1, 1, &text_cfg[yo::k::color].get<YOColor4F>());
        }
    }

    CreateSprite(texture_line_, Vector2{20,20});
    CreateSprite(texture_fill_, Vector2{120,20});
    CreateSprite(texture_text_, Vector2{220,20});
    CreateSprite(texture_param_, Vector2{320,20});

    auto* ui = GetSubsystem<UI>();
	auto* root = ui->GetRoot();

	YOVariant &video_cfg = config_->get(yo::k::video);
	int idx = 0;
	for(auto &v : video_)
	{
		v = MakeShared<Texture2D>(context_);
		video_img_[idx] = MakeShared<Image>(context_);

		YOVariant &cur_vid = video_cfg[idx];
		YOVariant &transform = cur_vid[yo::k::transform];
		YOVector3 &pos = transform[yo::k::position];
		YOVector3 &scale = transform[yo::k::scale];

		video_pad_[idx] = root->CreateChild<Sprite>();
		video_pad_[idx]->SetTexture(v);
		video_pad_[idx]->SetSize(v->GetSize());
		video_pad_[idx]->SetBlendMode(BLEND_ALPHA);
		video_pad_[idx]->SetPosition(pos.x, pos.y);
		video_pad_[idx]->SetScale(scale.x, scale.y);
		idx++;
	}
}

void YOViewer::CreateMaterial(int input, int type, YOVariant &style)
{

}

#define DEG2RAD(x) ((x) * 0.0174532925199433)


void YOViewer::ConvertVideos()
{
	for(int i = 0; i < YO_VIDEO_NUM; i++)
	{
		video_lock_[i].lock();
		SharedPtr<Image> img = video_img_[i];
		video_img_[i].Reset();
		if(img)
		{
			video_[i]->SetData(img);
			if(img->GetWidth() != video_pad_[i]->GetWidth() || img->GetHeight() != video_pad_[i]->GetHeight())
			{
				 std::cout << "RESIZE " << video_[i]->GetWidth() << " " << video_[i]->GetHeight() << std::endl;
				 video_pad_[i]->SetSize(img->GetSize().x_, img->GetSize().y_);
				 video_pad_[i]->SetHotSpot(img->GetSize().x_/2, img->GetSize().y_/2);
				 video_pad_[i]->SetTexture(video_[i]);
			}
		}
		video_lock_[i].unlock();
	}
}

std::shared_ptr<YOInputData> YOViewer::ConvertFrame(std::shared_ptr<YOVariant> frame, int frame_id)
{
    YOVariant &input_cfg = config_->get(yo::k::world)[frame_id];
    YOVariant &transform = input_cfg[yo::k::transform];
    YOVector3 &rot = transform[yo::k::rotation].get<YOVector3>();

    std::shared_ptr<YOInputData> fdata = std::make_shared<YOInputData>();

    fdata->root = world_->CreateChild();
    fdata->root->SetTemporary(true);

    fdata->root->SetPosition((Vector3&)transform[yo::k::position].get<YOVector3>());
    fdata->root->SetRotation(Quaternion(rot.x, rot.y, rot.z));
    fdata->root->SetScale((Vector3&)transform[yo::k::scale].get<YOVector3>());

    YOVariant &objects = frame->get(yo::k::objects);
    YOColor4CList &colors = frame->get(yo::k::colors);
    YOVariant &types_list = input_cfg[yo::k::types];

    for(int i = 0; i < objects.getArraySize(); i++)
    {
        YOVariant &obj = objects[i];
        uint32_t type = obj[yo::k::style_id];
        YOVariant &type_cfg = types_list[type];

        Node *node = fdata->root->CreateChild();
        fdata->types[type].push_back(node);
        fdata->enabled[type] = type_cfg[yo::k::enabled].get<bool>();

		node->SetEnabled(type_cfg[yo::k::enabled].get<bool>());
        node->SetTemporary(true);
        auto *cg = node->CreateComponent<CustomGeometry>();
        cg->BeginGeometry(0, POINT_LIST);

        YOColor4F &clr = type_cfg[yo::k::line][yo::k::color];
        YOVector3List &vertices = obj[yo::k::vertices];

        for( auto &vertex : vertices)
        {
            cg->DefineNormal(Vector3::FORWARD);
            cg->DefineVertex((Urho3D::Vector3&)vertex);
            cg->DefineTexCoord(Vector2( (float) frame_id / YO_INPUT_NUM, (float) type / YO_TYPE_NUM));
        }
        cg->SetMaterial(material_line_);
        cg->Commit();
    }
    //fnode->SetEnabledRecursive(input_cfg[yo::k::enabled].get<bool>());
    //fnode->ApplyAttributes();
    return fdata;
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

    ConvertVideos();

    for (std::size_t i = 0; i < data_in_.size(); ++i)
    {
       data_lock_[i].lock();
       std::shared_ptr<YOVariant> frame = data_in_[i];
       data_in_[i] = nullptr;
       data_lock_[i].unlock();

       if(frame != nullptr )
       {
           //std::cout << "UPDATE !!!! " << i << " : " << frame->m_name << std::endl;
           if(data_[i] != nullptr)
               data_[i]->root->Remove();

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

YOVariant *YOViewer::GetConfig(YOVariant  &config, const std::string &path)
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

    std::string &param = plist[plist.size() - 1];
    std::string &subsystem = plist[0];

    if(subsystem == "world" && addr[0]>-1 && addr[1]>-1)
    {
    	if(ends_with(path, "/line/enabled"))
        {

        }
        else if(ends_with(path, "/line/color"))
        {
            texture_line_->SetData(0, addr[0], addr[1], 1, 1, &res->get<YOColor4F>());
        }
        else if(ends_with(path, "/line/width"))
        {
            YOLimitF &line_width = res->get<YOLimitF>();
            YOColor4F width{line_width.value / 16.0f, 1.0f, 1.0f, 1.0f};
            texture_param_->SetData(0, addr[0], addr[1], 1, 1, &width);
        }
        else if(ends_with(path, "/text/enabled"))
        {

        }
        else if(ends_with(path, "/text/color"))
        {
            texture_text_->SetData(0, addr[0], addr[1], 1, 1, &res->get<YOColor4F>());
        }
        else if(ends_with(path, "/fill/enabled"))
        {

        }
        else if(ends_with(path, "/fill/color"))
        {
        	texture_fill_->SetData(0, addr[0], addr[1], 1, 1, &res->get<YOColor4F>());
        }
        else if(ends_with(path, "/enabled"))
        {
        	if(data_[addr[0]])
				for( Node *n : data_[addr[0]]->types[addr[1]])
				{
					n->SetEnabled(res->get<bool>());
				}
        }
    }

    if(subsystem == "world" && addr[0]>-1 && addr[1]==-1)
    {
        if(ends_with(path, "/enabled") && data_[addr[0]])
        {
			for( Node *n : data_[addr[0]]->types[addr[1]])
			{
				n->SetEnabled(res->get<bool>());
			}
        }
    }

    if(plist[0] == "video" && addr[0]>-1 )
    {

        if(ends_with(path, "/enabled"))
        {
        	std::cout << " VIDEO " << res->get<bool>() << std::endl;
        	video_pad_[addr[0]]->SetVisible(res->get<bool>());
        }
        else if(ends_with(path, "/transform/position"))
        {
        	YOVector3 pos = res->get<YOVector3>();

        	video_pad_[addr[0]]->SetHotSpot(video_pad_[addr[0]]->GetWidth()/2, video_pad_[addr[0]]->GetHeight()/2);
        	video_pad_[addr[0]]->SetPosition(pos.x, pos.y);
        }
        else if(ends_with(path, "/transform/scale"))
        {
        	YOVector3 pos = res->get<YOVector3>();
        	video_pad_[addr[0]]->SetScale(pos.x, pos.y);
        }
        else if(ends_with(path, "/transform/rotation"))
        {
        	YOVector3 pos = res->get<YOVector3>();
        	video_pad_[addr[0]]->SetRotation(pos.z);
        }
        else if(ends_with(path, "/opacity"))
        {
        	YOLimitF op = res->get<YOLimitF>();
        	video_pad_[addr[0]]->SetOpacity(op.value);
        }
    }
    printf("CFG: %s Changed [%s] Input: %d, Type: %d\n", plist[0].c_str(), path.c_str(), addr[0], addr[1]);
    return res;
}

void YOViewer::ProcessChange()
{
    std::string path = gui_->getPath();
    std::string param = gui_->getParam();
    YOVariant *cfg = GetConfig(*config_, path);
    std::cout << " Got config path: " << path << " name: " << cfg->m_name << " type: " <<  YOValue_type_name(cfg->m_value.index())  << " Value :" << cfg->m_value << std::endl;
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
