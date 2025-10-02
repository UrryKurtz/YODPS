/*
 * YOGPSUtils.cpp
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */

#include "YOGPSUtils.h"
#include <cmath>
#include <fstream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

YOGPSUtils::YOGPSUtils()
{

}

YOGPSUtils::~YOGPSUtils()
{

}

bool YOGPSUtils::SetCoord(float lat, float lon, int32_t scale)
{
	bool ret = false;
	bearing_ = Bearing(lat_, lon_, lat, lon);
	lat_ = lat;
	lon_ = lon;
	map_scale_ = scale;
	YOVector2I tmp = YOVector2I{ Lon2X(lon_, scale), Lat2Y(lat_, scale)};
	float x = Lon2Xf(lon, scale);
	float y = Lat2Yf(lat, scale);
	img_pos_.x = ( x - floor(x)) * 256;
	img_pos_.y = ( y - floor(y)) * 256;
	if( (tmp.x && tmp.x != tile_id_.x ) || (tmp.y && tmp.y != tile_id_.y ) )
	{
		tile_id_ = tmp;
	    ret = true;
	}
	return ret;
}

const YOVector2I &YOGPSUtils::GetTileId()
{
	return tile_id_;
}

const YOVector2I &YOGPSUtils::GetImagePos()
{
	return img_pos_;
}

float YOGPSUtils::GetBearing()
{
	return bearing_;
}

float YOGPSUtils::Bearing(float lat1, float lon1, float lat2, float lon2)
{
    float F1 = lat1 * M_PI/180.0;
    float F2 = lat2 * M_PI/180.0;
    float L1 = lon1 * M_PI/180.0;
    float L2 = lon2 * M_PI/180.0;
    float y = std::sin(L2-L1) * std::cos(F2);
    float x = std::cos(F1)*std::sin(F2) - std::sin(F1)*std::cos(F2)*std::cos(L2-L1);
    return std::fmod(std::atan2(y,x) * 180.0/M_PI + 360.0, 360.0);
}

int32_t YOGPSUtils::Lon2X(float lon_deg, int32_t scale) {
	float n = std::pow(2.0, scale);
    return ((lon_deg + 180.0) / 360.0 * n);
}

int32_t YOGPSUtils::Lat2Y(float lat_deg, int32_t scale) {
	float lat_rad = lat_deg * M_PI / 180.0;
	float n = std::pow(2.0, scale);
    return ((1.0 - std::log(std::tan(lat_rad) + 1.0/std::cos(lat_rad)) / M_PI) / 2.0 * n);
}

float YOGPSUtils::Lon2Xf(float lon, int32_t scale) {
    return (lon + 180.0) / 360.0 * (1 << scale);
}

float YOGPSUtils::Lat2Yf(float lat, int32_t scale) {
	float lat_rad = lat * M_PI / 180.0;
    return (1.0 - std::log(std::tan(lat_rad) + 1.0/std::cos(lat_rad)) / M_PI) / 2.0 * (1 << scale);
}

bool YOGPSUtils::GetOsmTile(const fs::path& root, int scale, int x, int y, std::vector<uint8_t>& data)
{
	if(LoadTile(root, scale, x, y, data))
	{
		return true;
	}

	if(FetchOsmTile(scale, x, y, data))
	{
		SaveTile(root, scale, x, y, data);
		return true;
	}

	return false;
}

bool YOGPSUtils::FetchOsmTile(int scale, int x, int y, std::vector<uint8_t>& data)
{
	httplib::SSLClient cli("a.tile.openstreetmap.org", 443);
	cli.set_follow_location(true);
	cli.set_connection_timeout(2, 0);  // 2s connect
	cli.set_read_timeout(10, 0);       // 10s read
	cli.set_default_headers({
	{ "User-Agent", "YODPS/1.0 (+tiles)" },
	{ "Accept", "image/png" },
	{ "Connection", "close" }});
	//cli.set_ca_cert_path("/etc/ssl/certs/ca-certificates.crt");
	cli.enable_server_certificate_verification(false);

	std::string path = "/" + std::to_string(scale) + "/" + std::to_string(x) + "/" + std::to_string(y) + ".png";

	if (auto res = cli.Get(path.c_str()); res && res->status == 200)
	{
		std::cout << path << " " << res->body.size() << std::endl;
		data.clear();
		data.assign(res->body.begin(), res->body.end());
		return true;
	}
	else
	{
		std::cout << "Can't fetch " << path << " Status: " << res->status << std::endl;
	}
	return false;
}

bool YOGPSUtils::SaveTile(const fs::path& root, uint32_t z, uint32_t x, uint32_t y, std::vector<uint8_t>& data)
{
	fs::path file = root / std::to_string(z) / std::to_string(x)/ (std::to_string(y) + ".png");
	std::error_code ec;
	fs::create_directories(file.parent_path(), ec);

	std::ofstream out(file, std::ios::binary | std::ios::trunc);
	if (!out) return false;

	out.write((const char*)data.data(), data.size());
	return (bool) out;
}

bool YOGPSUtils::LoadTile(const fs::path& root, uint32_t z, uint32_t x, uint32_t y, std::vector<uint8_t>& out)
{
    out.clear();
    fs::path file = root / std::to_string(z)  / std::to_string(x) / (std::to_string(y) + ".png");

    std::ifstream in(file, std::ios::binary);
    if (!in) return false;

    in.seekg(0, std::ios::end);
    std::streamsize sz = in.tellg();

    if (sz <= 0)
    	return false;

    out.resize(sz);
    in.seekg(0, std::ios::beg);
    return (bool)(in.read((char*)out.data(), sz));
}


