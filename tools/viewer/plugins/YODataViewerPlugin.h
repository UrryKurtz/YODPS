/*
 * YODataViewerPlugin.h
 *
 *  Created on: Oct 12, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YODATAVIEWERPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YODATAVIEWERPLUGIN_H_
#include <tinyxml2.h>
#include <shared_mutex>
#include "IPlugin.h"

using namespace tinyxml2;

struct YODataInfo
{
	std::string file;
	std::string topic;
	std::string type;
	uint32_t size;
	std::vector<uint8_t> data;
	YOTimestamp ts;
	YOVariant *config;
};

class YODataViewerPlugin: public IPlugin
{
	YOVariant *settings_ {nullptr};
	YOVariant *channels_ {nullptr};
	YOVariant *data_ {nullptr};
	std::map <std::string, std::shared_ptr<YODataInfo>> streams_;
	mutable std::shared_mutex mu_;
	YOGui gui_;

public:
	YODataViewerPlugin(Context *context);
	virtual ~YODataViewerPlugin();
	void LoadFile(const std::string &file_name, YOVariant &config);
	void LoadDatatypes( tinyxml2::XMLElement *root, YOVariant &config);
	void LoadEnums(tinyxml2::XMLElement *root, YOVariant &config);
	void LoadStructs(tinyxml2::XMLElement *root, YOVariant &config);

	std::shared_ptr<YODataInfo> GetOrCreateData(const std::string &topic);
	std::vector<std::pair<std::string, std::shared_ptr<YODataInfo>>> GetSnapshot() const;

	void Draw();
	void DrawInfo(std::shared_ptr<YODataInfo> &data);

	void DrawData(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name);
	void DrawEnum(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name);
	void DrawStruct(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name);

	void OnStart() override;
	void OnGui() override;


};

#endif /* TOOLS_VIEWER_PLUGINS_YODATAVIEWERPLUGIN_H_ */
