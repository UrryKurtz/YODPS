/*
 * YOGPSUtils.h
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_GPS_YOGPSUTILS_H_
#define TOOLS_GPS_YOGPSUTILS_H_

#include "YOVariant.h"
#include <filesystem>
namespace fs = std::filesystem;
class YOGPSUtils
{
	float lon_ {0.0f};
	float lat_ {0.0f};
	int map_scale_ {0};
	float bearing_ {0.0f};

	YOVector2I tile_id_{0, 0};
	YOVector2I img_pos_{0, 0};

	std::vector<uint8_t> tiles_[3][3];

public:
	YOGPSUtils();
	virtual ~YOGPSUtils();

	bool SetCoord(float lat, float lon, int32_t scale);

	const YOVector2I &GetTileId();
	const YOVector2I &GetImagePos();
	int32_t GetMapScale();
	float Bearing(float lat1, float lon1, float lat2, float lon2);
	float GetBearing();

	//void GetTiles();
	bool FetchTile(int32_t scale, int32_t x, int32_t y, std::vector<unsigned char> &out);

	int32_t Lon2X(float lon_deg, int32_t scale);
	int32_t Lat2Y(float lat_deg, int32_t scale);
	float Lon2Xf(float lon, int32_t scale);
	float Lat2Yf(float lat, int32_t scale);

	bool SaveTile(const fs::path& root, uint32_t z, uint32_t x, uint32_t y, std::vector<uint8_t>& data);
	bool LoadTile(const fs::path& root, uint32_t z, uint32_t x, uint32_t y, std::vector<uint8_t>& data);
	bool FetchOsmTile(int scale, int x, int y, std::vector<uint8_t>& data);
	bool GetOsmTile(const fs::path& root, int scale, int x, int y, std::vector<uint8_t>& data);
};

#endif /* TOOLS_GPS_YOGPSUTILS_H_ */
