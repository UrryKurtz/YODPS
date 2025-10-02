/*
 * YOGPSPlugin.cpp
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "YOGPSPlugin.h"
#include "YOPluginBus.h"
#include <Urho3D/SystemUI/Widgets.h>


Color SEA(170.0f/256.0f, 211.0f/256.0f, 223.0f/256.0f);

YOGPSPlugin::YOGPSPlugin(Context *context) : IPlugin(context)
{
	map_ = MakeShared<Texture2D>(context_);
	map_->SetNumLevels(1);
	map_->SetSize(256 * 5, 256 * 5, TextureFormat::TEX_FORMAT_RGBA8_UNORM);

	full_img_ = MakeShared<Image>(context_);
	full_img_->SetSize(256 * 6, 256 * 6, 3);
	full_img_->Clear(SEA);
}

YOGPSPlugin::~YOGPSPlugin()
{

}

void YOGPSPlugin::OnStart()
{
	if (!config_->hasChild(yo::k::subscribe))
	{
		(*config_)[yo::k::subscribe] = true;
		(*config_)[yo::k::direction] = false;
		(*config_)[yo::k::topic] = "GPS";
		(*config_)[yo::k::image] = YOVector2I { 0u, 0u };
		(*config_)[yo::k::tile] = YOVector2I { 0u, 0u };
		(*config_)[yo::k::coord] = YOVector2{ 0.0f, 0.0f };
		(*config_)[yo::k::scale] = YOLimitI32
		{ .value = 1, .min = 1, .max = 20, .speed = 1 };
		(*config_)[yo::k::rotation] = 0.0f;
	}

	topic_ = (*config_)[yo::k::topic].get<std::string>();

	if ((*config_)[yo::k::subscribe].get<bool>())
		subs_.push_back(topic_);

	ads_.push_back(topic_);
}

void YOGPSPlugin::OnUpdate(float timeStep)
{

}

//#include <fstream>

void YOGPSPlugin::RequestTile(int scale, int x, int y)
{
	if (cache_[scale][x][y].GetPointer() || x < 0 || y < 0 )
	{
		return;
	}
	YOVariant gps("GPS");
	gps[yo::k::tile] = YOVector2I {x, y};
	gps[yo::k::scale] = scale;
	Transmit("GPS", gps);
}

void YOGPSPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::shared_ptr<YOVariant> gps = std::make_shared<YOVariant>(message->getDataSize(), (const char*) message->getData());
	std::cout << *gps << std::endl;
	YOVector2I tile = (*gps)[yo::k::tile];
	int32_t scale = (*gps)[yo::k::scale];

	std::vector<uint8_t> data;
	bool res = gps_.GetOsmTile(path_, scale , tile.x, tile.y, data);
	if(res && data.size())
	{
		MemoryBuffer mb(data.data(), data.size());
		cache_[scale][tile.x][tile.y] = MakeShared<Image>(context_);
		cache_[scale][tile.x][tile.y]->Load(mb);
		update_ = true;
	}
}

static void DrawViewport(Texture2D* tex, ImVec2 avail, const float& cx, const float& cy)
{
    if (!tex || avail.x <= 0 || avail.y <= 0) return;

    const float tw = (float)tex->GetWidth();
    const float th = (float)tex->GetHeight();

    float x0 = cx - avail.x * 0.5f;
    float y0 = cy - avail.y * 0.5f;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 + avail.x > tw) x0 = tw - avail.x;
    if (y0 + avail.y > th) y0 = th - avail.y;

    ImVec2 uv0(x0 / tw,       y0 / th);
    ImVec2 uv1((x0+avail.x)/tw, (y0+avail.y)/th);

    ui::Image(ToImTextureID(tex), avail, uv0, uv1);
}

void YOGPSPlugin::OnGui()
{
	bool update = gui_.draw(*config_);
	int32_t scale = (*config_)[yo::k::scale].get<YOLimitI32>().value;

	if (update)
	{
		std::cout << "update" << std::endl;
		std::string path = gui_.getPath();
		YOVariant *cfg = gui_.getConfig();
		std::vector<int> addr = gui_.getIndex();

		YOVector2 coord = (*config_)[yo::k::coord];
		coord.x = Clamp(coord.x, -82.0f, 82.0f);
		coord.y = Clamp(coord.y, -179.999f, 179.999f);
		(*config_)[yo::k::coord] = coord;

		bool change = gps_.SetCoord(coord.x, coord.y, scale);

		YOVector2I tile = gps_.GetTileId();
		//TODO add clamp
		(*config_)[yo::k::tile] = tile;

		YOVector2I img = gps_.GetImagePos();
		img.x = Clamp(img.x, 0, 255);
		img.y = Clamp(img.y, 0, 255);
		(*config_)[yo::k::image] = img;

		std::cout << "GUI changed: " << path << " " << topic_ << std::endl;

		if (path == "/GPS/topic")
		{
			topic_ = cfg->get<std::string>();
		}
		else if (path == "/GPS/coord" || path == "/GPS/scale")
		{

		}
		else if (path == "/GPS/tile")
		{
			RequestTile(scale, tile.x, tile.y);
		}
	}

	YOVector2 coord = (*config_)[yo::k::coord];
	YOVector2I tile = gps_.GetTileId();
	YOVector2I img = gps_.GetImagePos();

	ui::SetNextWindowSizeConstraints(ImVec2(64, 32), ImVec2(1024.f, 1024.f));

	ui::Begin("MAP");
		const ImVec2 size{ 256 * 4, 256 * 4};
		full_img_->Clear(SEA);
		if (map_->GetSize().x_ && map_->GetSize().x_)
		{
			for(int x = 0 ; x < 5 ; x++)
			{
				for(int y = 0 ; y < 5 ; y++)
				{
					int tile_x = tile.x - 2 + x;
					int tile_y = tile.y - 2 + y;

					if(cache_[scale][tile_x][tile_y].GetPointer())
					{
						if(update || update_)
						{
							IntRect posRect (256 * x + 256 - img.x, 256 * y + 256 - img.y,  256 * x - img.x + 512 , 256 * y - img.y + 512 );
							full_img_->SetSubimage(cache_[scale][tile_x][tile_y], posRect);
						}
					}
					else
					{
						RequestTile(scale, tile_x, tile_y);
					}
				}
			}

			if(update || update_)
			{
				map_->SetData(full_img_);
				update_ = false;
			}

			auto systemUI = GetSubsystem<SystemUI>();
			systemUI->ReferenceTexture(map_);
			ImVec2 avail = ui::GetContentRegionAvail();

			if (ui::BeginChild("Canvas", avail, false, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))
			{
				DrawViewport(map_, avail, 256.0f * 3, 256.0f * 3);
			}
			ui::EndChild();
		}
		ui::End();
}
