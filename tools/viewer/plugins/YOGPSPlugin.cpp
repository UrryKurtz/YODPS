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

Color SEA(170.0f/256.0f, 211.0f/256.0f, 223.0f/256.0f, 1.0f);

YOGPSPlugin::YOGPSPlugin(Context *context) : IPlugin(context)
{
	map_ = MakeShared<Texture2D>(context_);
	map_->SetNumLevels(1);
	map_->SetSize(256 * 6, 256 * 6, TextureFormat::TEX_FORMAT_RGBA8_UINT);
	auto systemUI = GetSubsystem<SystemUI>();
	systemUI->ReferenceTexture(map_);

	full_img_ = MakeShared<Image>(context_);
	full_img_->SetSize(256 * 6, 256 * 6, 4);
	full_img_->Clear(SEA);

	empty_ = MakeShared<Image>(context_);
	empty_->SetSize(256, 256, 4);
	empty_->Clear(SEA);
}

YOGPSPlugin::~YOGPSPlugin()
{

}

void YOGPSPlugin::OnStart()
{
	if (!config_->hasChild(yo::k::subscribe))
	{
		(*config_)[yo::k::subscribe] = true;
		(*config_)[yo::k::receiver] = false;
		(*config_)[yo::k::direction] = false;
		(*config_)[yo::k::topic] = "GPS";
		(*config_)[yo::k::image] = YOVector2I { 0u, 0u };
		(*config_)[yo::k::tile] = YOVector2I { 0u, 0u };
		(*config_)[yo::k::coord] = YOVector2{ 0.0f, 0.0f };
		(*config_)[yo::k::scale] = YOLimitI32
		{ .value = 1, .min = 1, .max = 19, .speed = 1 };
		(*config_)[yo::k::rotation] = 0.0f;
	}

	topic_ = (*config_)[yo::k::topic].get<std::string>();

	if ((*config_)[yo::k::subscribe].get<bool>())
		subs_.push_back(topic_);
		subs_.push_back("COORDINATES");

	ads_.push_back(topic_);
	update_ = true;
}

void YOGPSPlugin::OnUpdate(float timeStep)
{

}

inline constexpr bool TileIsValid(int scale, int tile) {
    const int n = 1 << scale;
    return tile >= 0 && tile < n;
}

void YOGPSPlugin::RequestTile(int scale, int x, int y)
{
	if( !TileIsValid(scale, x) || !TileIsValid(scale, y) || scale < 0 || scale > 19) //wrong request
	{
		printf("Invalid tile request. Scale: %d  X: %d  Y: %d\n", scale, x, y);
		return;
	}

	cache_[scale][x][y].counter %= 30;
	if(cache_[scale][x][y].counter == 0)
	{
		YOVariant gps("GPS");
		gps[yo::k::tile] = YOVector2I {x, y};
		gps[yo::k::scale] = scale;
		Transmit("GPS", gps);
	}
	cache_[scale][x][y].counter++;
}

void YOGPSPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::shared_ptr<YOVariant> gps = std::make_shared<YOVariant>(message->getDataSize(), (const char*) message->getData());
	if(topic == "COORDINATES")
	{
		//std::cout << "OnData " << topic << " " << *gps << std::endl;
		if(!(*config_)[yo::k::receiver].get<bool>())
		{
			return;
		}

		YOLimitI32 scale = (*config_)[yo::k::scale];
		YOVector2 coord = (*gps)[yo::k::coord];
		(*config_)[yo::k::coord] = coord;
		gps_.SetCoord(scale.value, coord.x, coord.y);
		update_ = true;
		return;
	}

	YOVector2I tile = (*gps)[yo::k::tile];
	int32_t scale = (*gps)[yo::k::scale];

	std::vector<uint8_t> data;
	bool res = gps_.GetOsmTile(path_, scale , tile.x, tile.y, data);
	if(res && data.size())
	{
		MemoryBuffer mb(data.data(), data.size());
		cache_[scale][tile.x][tile.y].image = MakeShared<Image>(context_);
		cache_[scale][tile.x][tile.y].image->Load(mb);
		cache_[scale][tile.x][tile.y].image = cache_[scale][tile.x][tile.y].image->ConvertToRGBA();
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


inline YOVector2 TilePointToLatLon(int z, int x, int y, int px, int py) {
    const float n = float(1 << z);
    const float xf = (float)x + px / 256.0f;
    const float yf = (float)y + py / 256.0f;
    float lon_deg = xf / n * 360.0f - 180.0f;
    const float a = M_PI * (1.0f - 2.0f * yf / n);
    float lat_deg = 180.0f / M_PI * std::atan(std::sinh(a));
    return YOVector2 {lat_deg, lon_deg};
}

void YOGPSPlugin::OnGui()
{
	ImVec2 s = ImGui::GetWindowSize();
	float w = s.x > 1024.0f ? 1024.0f : s.x;
	float h = s.y > 1024.0f ? 1024.0f : s.y;
	ImGui::SetWindowSize(ImVec2(w, h));

	int32_t scale = (*config_)[yo::k::scale].get<YOLimitI32>().value;
	bool gui_update = false;
	if(setup_)
	{
		ui::Begin("Settings", &setup_);
			gui_update = gui_.draw(*config_);
		ui::End();
	}

	if (gui_update)
	{
		std::string path = gui_.getPath();
		YOVariant *cfg = gui_.getConfig();
		std::vector<int> addr = gui_.getIndex();
		std::cout << "GUI changed: " << path << " " << topic_ << std::endl;

		if (path == "/GPS/topic")
		{
			topic_ = cfg->get<std::string>();
		}
		else if (path == "/GPS/coord")
		{

		}
		else if (path == "/GPS/scale")
		{
			full_img_->Clear(SEA);
		}

		else if (path == "/GPS/tile")
		{
			//RequestTile(scale, tile.x, tile.y);
		}
	}

	static YOVector2 coord_old = (*config_)[yo::k::coord];

	YOVector2 &coord = (*config_)[yo::k::coord];
	coord.x = Clamp(coord.x, -82.0f, 82.0f);
	coord.y = Clamp(coord.y, -179.999f, 179.999f);
	//(*config_)[yo::k::coord] = coord;

	if(coord.x != coord_old.x || coord.y != coord_old.y || update_ )
	{
		update_ = true;
		coord_old = coord;
		gps_.SetCoord(coord.x, coord.y, scale);
		(*config_)[yo::k::bearing] = gps_.GetBearing();
	}

	YOVector2I tile = gps_.GetTileId();
	(*config_)[yo::k::tile] = tile;

	YOVector2I img = gps_.GetImagePos();
	img.x = Clamp(img.x, 0, 255);
	img.y = Clamp(img.y, 0, 255);
	(*config_)[yo::k::image] = img;

	bool zero = false;

	if((gui_update || update_ ))
	{
		//std::cout << "UPDATE "  << std::endl;
		for(int x = 0 ; x < 5 ; x++)
		{
			for(int y = 0 ; y < 5 ; y++)
			{
				int tile_x = tile.x - 2 + x;
				int tile_y = tile.y - 2 + y;
				IntRect posRect (256 * x + 256 - img.x, 256 * y + 256 - img.y,  256 * x - img.x + 512 , 256 * y - img.y + 512 );

				if(cache_[scale][tile_x][tile_y].image.GetPointer())
				{
					full_img_->SetSubimage(cache_[scale][tile_x][tile_y].image, posRect);
				}
				else
				{
					full_img_->SetSubimage(empty_, posRect);
					RequestTile(scale, tile_x, tile_y);
					zero = true;
				}
			}
		}
		map_->SetData(full_img_);
	}
	update_ = zero;

	ui::SetNextWindowSizeConstraints(ImVec2(64, 32), ImVec2(1024.f, 1024.f));

	//ui::Begin("MAP");
	static const ImVec2 size{1024, 1024};

	ImVec2 avail = ui::GetContentRegionAvail();
	if (ui::BeginChild("Canvas", avail, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))
	{
		const bool hovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByActiveItem );
		if(hovered)
		{
	        if(float d = ImGui::GetMouseWheel())
	        {
	        	YOLimitI32 &scale = (*config_)[yo::k::scale].get<YOLimitI32>();
	        	scale.value += d;
	        	if(scale.value > scale.max ) scale.value  = scale.max;
	        	if(scale.value < scale.min ) scale.value  = scale.min;
	        	update_ = true;
	        }
			if(ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				if(!(*config_)[yo::k::receiver].get<bool>() && ( d.x || d.y))
				{
					(*config_)[yo::k::coord] = TilePointToLatLon(scale, tile.x, tile.y, img.x - d.x, img.y - d.y);
					update_ = true;
				}
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			}
		}
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 center = (p0 + avail)/2;
		DrawViewport(map_, avail, 256.0f * 3, 256.0f * 3);
		dl->AddLine( p0 + avail/2 + ImVec2( -10, -10), p0 + avail/2 + ImVec2(10, 10), IM_COL32(256,64,64,128), 3);
		dl->AddLine( p0 + avail/2 + ImVec2( -10, 10), p0 + avail/2 + ImVec2(10, -10), IM_COL32(256,64,64,128), 3);
		dl->AddRectFilled(p0 + ImVec2(5, 5) , p0 + ImVec2(280, 30), IM_COL32(255,255,255,128));
		static char coord_buf[256];
		sprintf(coord_buf, "Coordinates: %f, %f", coord.x, coord.y);
		dl->AddText( p0 + ImVec2(10,10), IM_COL32(0,0,0,255), coord_buf);

		ui::SetCursorScreenPos(p0 + ImVec2(avail.x - 75, 5));
		if (ImGui::Button("Settings", ImVec2(70,20)))
		{	setup_ = !setup_; }
		//float a =  gps_.GetBearing();
		//dl->AddLine( p0 + avail/2 , p0 + avail/2 + ImVec2(Sin(a)*50, -Cos(a)*50), IM_COL32(256,64,64,128), 3);
		ui::EndChild();
	}


	//ui::End();


}
