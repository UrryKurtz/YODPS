/*
 * YOPolylinePlugin.cpp
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#include "YOPolylinePlugin.h"
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>

inline void createStyleCfg(uint32_t i, YOVariant &style, const std::string &name)
{
    //std::cout << " createStyleCfg " << name << std::endl;
    style[yo::k::enabled] = true;
    style[yo::k::replace] = true;

    style[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
    style[yo::k::comment] = "";

    style[yo::k::id] = i;

    style[yo::k::line][yo::k::enabled] = true;
    style[yo::k::line][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};
    style[yo::k::line][yo::k::width] = YOLimitF{1.0f, 0.0f, 16.0f, 0.1f};

    style[yo::k::fill][yo::k::enabled] = true;
    style[yo::k::fill][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};

    style[yo::k::text][yo::k::enabled] = true;
    style[yo::k::text][yo::k::color] = YOColor4F{1.0f, 1.0f, 1.0f, 1.0f};
    style[yo::k::text][yo::k::size]  = YOLimitF{12.0f, 1.0f, 32.0f, 0.1f};
    style[yo::k::text][yo::k::position] = YOVector3{0.0f, 0.0f, 0.0f};
    style[yo::k::text][yo::k::font]  = YOStringList{ .items = {"default", "Arial", "Courier", "Times", "Robo"}, .select = 0};

    style.m_name = name;
}

inline void createInputCfg(uint32_t i, YOVariant &input)
{
    input[yo::k::transform][yo::k::position] = YOVector3{};
    input[yo::k::transform][yo::k::rotation] = YOVector3{};
    input[yo::k::transform][yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
    input[yo::k::transform][yo::k::apply] = YOStringList {.items = {"Ignore", "From Config", "From Data", "Combine"}, .select = 1};

    createStyleCfg(i, input, yo::k::input);

    input[yo::k::types] = YOArray(YO_TYPE_NUM);

    for(int j = 0; j < YO_TYPE_NUM; j++)
    {
        YOVariant &type = input[yo::k::types][j];
        createStyleCfg(j, type, yo::k::type);
    }
}

SharedPtr<Texture2D> YOPolylinePlugin::CreateTexture()
{
	SharedPtr<Texture2D> tex = MakeShared<Texture2D>(context_);
    tex->SetNumLevels(1); // no MIPs
    tex->SetAddressMode(TextureCoordinate::U, ADDRESS_CLAMP);
    tex->SetAddressMode(TextureCoordinate::V, ADDRESS_CLAMP);
    tex->SetSize(YO_INPUT_NUM, YO_TYPE_NUM, TextureFormat::TEX_FORMAT_RGBA32_FLOAT, TextureFlag::BindRenderTarget | TextureFlag::BindUnorderedAccess );
    tex->SetFilterMode(FILTER_NEAREST);
    return tex;
}

YOPolylinePlugin::YOPolylinePlugin(Context* context) : IPlugin(context)
{
    cache_ = GetSubsystem<ResourceCache>();
	auto* renderer = GetSubsystem<Renderer>();
	scene_overlay_ = (Scene*) context_->GetGlobalVar("YO_OVERLAY_SCENE").GetVoidPtr();
	node_overlay_ = scene_overlay_->CreateChild(name_.c_str());
}

YOPolylinePlugin::~YOPolylinePlugin()
{

}

void YOPolylinePlugin::OnSetConfig(YOVariant *config)
{
	params_cfg_ = &(*config_)[yo::k::config];
	inputs_cfg_ = &(*config_)[yo::k::inputs];

	if(inputs_cfg_->getTypeId() == 1 &&  inputs_cfg_->getArraySize() == YO_INPUT_NUM)
	{
		std::cout << " YOPolylinePlugin Config Loaded " << std::endl;
	}
	else
	{
		std::cout << " YOPolylinePlugin Create Config" << 1 << std::endl;
		(*params_cfg_)[yo::k::subscribe] = true;
		inputs_cfg_->m_value = YOArray(YO_INPUT_NUM);
	    for(int i = 0; i < YO_INPUT_NUM; i++)
	    {
	        createInputCfg(i, (*inputs_cfg_)[i]);
	    }
	}
}

void YOPolylinePlugin::OnStart()
{
	std::cout << " YOPolylinePlugin::OnStart() " << std::endl;
    texture_param_ = CreateTexture();
    texture_line_ = CreateTexture();
    texture_fill_ = CreateTexture();
    texture_text_ = CreateTexture();

    material_line_ = cache_->GetResource<Material>("Materials/YOTextureOnly.xml")->Clone();
    material_line_->SetTexture(ShaderResources::Albedo, texture_line_);
    material_line_->SetTexture(ShaderResources::Properties, texture_param_);
    material_line_->SetCullMode(CULL_NONE);

    material_fill_ = cache_->GetResource<Material>("Materials/YOTextureOnly.xml")->Clone();
    material_fill_->SetTexture(ShaderResources::Albedo, texture_fill_);
    material_fill_->SetTexture(ShaderResources::Properties, texture_param_);
    material_fill_->SetCullMode(CULL_NONE);

    material_text_ = cache_->GetResource<Material>("Materials/YOTextureOnly.xml")->Clone();
    material_text_->SetTexture(ShaderResources::Albedo, texture_text_);
    material_text_->SetTexture(ShaderResources::Properties, texture_param_);
    material_text_->SetCullMode(CULL_NONE);

    YOVariant &wrld = config_->get(yo::k::inputs);
    for(int input = 0; input < wrld.getArraySize(); input++)
	{
		YOVariant &inp_cfg = wrld[input][yo::k::types];
		for(int type = 0; type < inp_cfg.getArraySize(); type++)
		{
			YOVariant &type_cfg = inp_cfg[type];
			if(!type_cfg.hasChild(yo::k::mask))
			{
				type_cfg[yo::k::mask] = 0xFFFFFFFFu;
			}

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
		std::string topic = "INPUT" + std::to_string(input);

		if((*params_cfg_)[yo::k::subscribe].getBool())
			RegisterTopic(topic, input);
	}
}

void YOPolylinePlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	//std::cout << " YOPolylinePlugin::OnData() " << topic << std::endl;
    std::shared_ptr<YOVariant> frame = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
    AddFrame(frame, GetTopicId(topic));
}

void YOPolylinePlugin::AddFrame(std::shared_ptr<YOVariant> frame, int input_id)
{
    data_lock_[input_id].lock();
    data_in_[input_id] = frame;
    data_lock_[input_id].unlock();
}

void YOPolylinePlugin::OnUpdate(float timeStep)
{
	std::vector<std::shared_ptr<YOInputData>> tmp;
    for (std::size_t i = 0; i < data_in_.size(); ++i)
    {
       data_lock_[i].lock();
       std::shared_ptr<YOVariant> frame = data_in_[i];
       data_in_[i] = nullptr;
       data_lock_[i].unlock();

       if(!data_[i]) {
    	   data_[i] = std::make_shared<YOInputData>();
    	   data_[i]->root = node_->CreateChild();
    	   data_[i]->root->SetTemporary(true);
    	   data_[i]->root_overlay_ = node_overlay_->CreateChild();
		   data_[i]->root_overlay_->SetTemporary(true);
    	   data_[i]->logic = data_[i]->root->CreateComponent<YORootLogic>();

    	   data_[i]->logic->setMaterials(material_fill_, material_line_, material_text_);
    	   data_[i]->logic->SetInputConfig((*inputs_cfg_)[i]);
    	   data_[i]->logic->SetOverlayRoot(data_[i]->root_overlay_);
       }

       data_[i]->logic->UpdateConfig();

       if(frame != nullptr )
       {
    	   data_[i]->data = frame;
    	   data_[i]->logic->ConvertRoot(frame);
       }
    }
}

void YOPolylinePlugin::SetLineEnabled(const std::string &path, YOVariant *cfg, int input, int type)
{

}

void YOPolylinePlugin::SetLineColor(const std::string &path, YOVariant *cfg, int input, int type)
{
	texture_line_->SetData(0, input, type, 1, 1, &cfg->get<YOColor4F>());
}

void YOPolylinePlugin::SetLineWidth(const std::string &path, YOVariant *cfg, int input, int type)
{
	YOLimitF &line_width = cfg->get<YOLimitF>();
	YOColor4F width{line_width.value / 16.0f, 1.0f, 1.0f, 1.0f};
	texture_param_->SetData(0, input, type, 1, 1, &width);
}

void YOPolylinePlugin::SetFillEnabled(const std::string &path, YOVariant *cfg, int input, int type)
{

}

void YOPolylinePlugin::SetFillColor(const std::string &path, YOVariant *cfg, int input, int type)
{
	texture_fill_->SetData(0, input, type, 1, 1, &cfg->get<YOColor4F>());
}

void YOPolylinePlugin::SetTextEnabled(const std::string &path, YOVariant *cfg, int input, int type)
{

}

void YOPolylinePlugin::SetTextColor(const std::string &path, YOVariant *cfg, int input, int type)
{
	texture_text_->SetData(0, input, type, 1, 1, &cfg->get<YOColor4F>());
}

void YOPolylinePlugin::OnGui()
{
	//if(config_)
	int start = config_->m_name.length() + 1;

	if(gui_.draw(*config_))
	{
		std::string path = gui_.getPath().substr(start);
		YOVariant *res = gui_.getConfig();
		std::vector<int> addr = gui_.getIndex();
		OnGuiChanged(path, addr, res);
	}
	int i = 0;
	if (ui::CollapsingHeader("Data"))
	{
		if(ui::BeginTable("TwoColsData", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp ))
		{
			for(auto &data : data_)
			{
				ui::PushID(i);
				if(data && data->data)
				{
					ui::TableNextRow();
					ui::TableSetColumnIndex(0);
					ui::Text("INPUT%d", i);
					ui::TableSetColumnIndex(1);
					gui_.draw(*(data->data));
				}
				ui::PopID();
				i++;
			}
			ui::EndTable();
		}
	}
}

void YOPolylinePlugin::OnGuiChanged(const std::string &path, std::vector<int> &addr,  YOVariant *cfg)
{
	int input = addr.size() > 0 ? addr[0] : -1;
	int type = addr.size() > 1 ? addr[1] : -1;

	std::cout << " OnGuiChanged " <<  path << " Input: " << input << " Type: " <<  type << " Value: " << cfg->m_value ;

	if(input < 0 || !data_[input] )
	{
		return;
	}

	YORootLogic *logic = data_[addr[0]]->logic;

	if(path == "/inputs/#0/enabled")
	{
		//logic->SetEnable(type, cfg->get<bool>());
	}
	else if(path == "/inputs/#0/comment")
	{

	}
	else if(path == "/inputs/#0/name")
	{

	}
	else if(path == "/inputs/#0/transform/apply")
	{
		std::cout << cfg->getTypeName() << std::endl;
	}
	else if(path == "/inputs/#0/transform/position")
	{

	}
	else if(path == "/inputs/#0/transform/rotation")
	{

	}
	else if(path == "/inputs/#0/transform/scale")
	{

	}
	else if(path == "/inputs/#0/fill/enabled") // INPUT
	{
		//logic->SetFillEnabled(type, cfg->get<bool>());
	}
	else if(path == "/inputs/#0/fill/color")
	{

	}
	else if(path == "/inputs/#0/line/enabled")
	{

	}
	else if(path == "/inputs/#0/line/color")
	{

	}
	else if(path == "/inputs/#0/line/width")
	{

	}
	else if(path == "/inputs/#0/text/enabled")
	{

	}
	else if(path == "/inputs/#0/text/color")
	{

	}
	else if(path == "/inputs/#0/text/font")
	{

	}
	else if(path == "/inputs/#0/text/position")
	{

	}
	else if(path == "/inputs/#0/text/size")
	{

	}
	else if(path == "/inputs/#0/types/#1/enabled") // TYPE
	{
		//logic->SetEnabled(type, cfg->get<bool>());
	}
	else if(path == "/inputs/#0/types/#1/fill/enabled")
	{

	}
	else if(path == "/inputs/#0/types/#1/fill/color")
	{
		SetFillColor(path, cfg, addr[0], addr[1]);
	}
	else if(path == "/inputs/#0/types/#1/line/enabled")
	{

	}
	else if(path == "/inputs/#0/types/#1/line/color")
	{
		SetLineColor(path, cfg, addr[0], addr[1]);
	}
	else if(path == "/inputs/#0/types/#1/line/width")
	{
		SetLineWidth(path, cfg, addr[0], addr[1]);
	}
	else if(path == "/inputs/#0/types/#1/text/enabled")
	{

	}
	else if(path == "/inputs/#0/types/#1/text/color")
	{
		SetTextColor(path, cfg, addr[0], addr[1]);
	}
	else if(path == "/inputs/#0/types/#1/text/font")
	{

	}
	else if(path == "/inputs/#0/types/#1/text/position")
	{

	}
	else if(path == "/inputs/#0/types/#1/text/size")
	{

	}

}
