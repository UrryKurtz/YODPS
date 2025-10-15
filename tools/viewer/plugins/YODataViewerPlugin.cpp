/*
 * YODataViewerPlugin.cpp
 *
 *  Created on: Oct 12, 2025
 *      Author: kurtz
 */

#include "YODataViewerPlugin.h"
namespace yo::k
{
	YO_KEY(alignment, "alg")
	YO_KEY(arraysize, "arz")
	YO_KEY(bytepos, "bps")
	YO_KEY(byteorder, "bor")
	YO_KEY(channels, "chs")
	YO_KEY(channel, "chn")
	YO_KEY(datatypes, "dts")
	YO_KEY(description, "des")
	YO_KEY(enums, "ens")
	YO_KEY(element, "elm")
	YO_KEY(elements, "els")
	YO_KEY(files, "fls")
	YO_KEY(send, "snd")
	YO_KEY(structs, "srs")
	YO_KEY(streams, "sms")
	YO_KEY(stream, "srm")
}

uint32_t align_up(uint32_t field_end, uint32_t struct_align)
{
	return (field_end + (struct_align-1)) & ~(struct_align-1);
}


YODataViewerPlugin::YODataViewerPlugin(Context *context): IPlugin(context)
{

}

YODataViewerPlugin::~YODataViewerPlugin()
{

}

void YODataViewerPlugin::OnStart()
{
	settings_ =  &(*config_)[yo::k::settings];
	channels_ =  &(*config_)[yo::k::channels];
	data_ =      &(*config_)[yo::k::data];

	if(channels_->getTypeId()!=1)
		channels_->m_value = YOArray();
}

void YODataViewerPlugin::OnGui()
{
	ui::SeparatorText("Settings");
		gui_.draw(*settings_);
	ui::SeparatorText("Channels");
		gui_.draw(*channels_);

	ui::SeparatorText("Data Types");
		gui_.draw(*data_);


	if(ui::Button("Add"))
	{
		YOVariant channel(yo::k::channel);
		channel[yo::k::file] = "";
		channel[yo::k::topic] = "";
		channel[yo::k::stream] = "";
		channels_->push_back(channel);
	}

	if(ui::Button("Load"))
	{
		std::string file = (*channels_)[0][yo::k::file].getStr();
		std::string topic = (*channels_)[0][yo::k::topic].getStr();
		std::string stream = (*channels_)[0][yo::k::stream].getStr();

		LoadFile( file, (*config_)[yo::k::data][topic]);

		std::shared_ptr<YODataInfo> data = GetOrCreateData(topic);
		data->config =  &(*config_)[yo::k::data][topic];
		data->topic = topic;
		data->type = stream;
		data->file = file;
		data->size = (*data->config)[yo::k::structs][stream][yo::k::size].getU32();
		data->data.resize(data->size);

		std::cout <<  " Topic: " << topic
				<<  " Stream: " << stream
				<<  " Size: " << data->size
			    << std::endl;
	}
	ui::SeparatorText("Data");
	Draw();
}

void YODataViewerPlugin::Draw()
{
	std::vector<std::pair<std::string, std::shared_ptr<YODataInfo>>>  data = GetSnapshot();
	for(auto &pair : data)
	{
		DrawInfo(pair.second);
	}
}

void YODataViewerPlugin::DrawData(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name)
{
	YOVariant &vdata = config[yo::k::datatypes][type_name];
	//ui::Indent();
	ui::Text("DATA: %s : [%s] [%s] bytepos %d size: %d", name.c_str(), type_name.c_str(), vdata[yo::k::name].c_str(), bytepos, vdata[yo::k::size].getU32());
	//ui::Unindent();
}

void YODataViewerPlugin::DrawEnum(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &field_name, const std::string &type_name)
{
	YOVariant &venum = config[yo::k::enums][type_name];
	//ui::Indent();
	ui::Text("ENUM: %s : [%s] base [%s] bytepos %d size: %d", field_name.c_str(), type_name.c_str(), venum[yo::k::type].c_str(), bytepos, venum[yo::k::size].getU32());
	//ui::Unindent();
}

void YODataViewerPlugin::DrawStruct(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name)
{
	YOVariant &vstruct = config[yo::k::structs][type_name];

	if(ui::CollapsingHeader(name.c_str()))
	{
		ui::Indent();
		for( YOVariant &elem : vstruct[yo::k::elements].getArray())
		{
			uint32_t elem_bytepos = elem[yo::k::bytepos];
			uint32_t size = vstruct[yo::k::size];
			uint32_t arraysize = elem[yo::k::arraysize];
			uint32_t cursize = elem[yo::k::size];

			std::string elem_name = elem[yo::k::name].getStr();
			std::string elem_type = elem[yo::k::type].getStr();

			if(elem[yo::k::data].getStr() == yo::k::structs)
			{
				DrawStruct(data + elem_bytepos, config, bytepos + elem_bytepos, elem_name, elem_type);
			}
			else if(elem[yo::k::data].getStr() == yo::k::enums)
			{
				DrawEnum(data + elem_bytepos, config, bytepos + elem_bytepos, elem_name, elem_type);
			}
			else if(elem[yo::k::data].getStr() == yo::k::datatypes)
			{
				if(arraysize == 1)
				{
					DrawData(data + elem_bytepos, config, bytepos + elem_bytepos, elem_name, elem_type);
				}
				else
				{
					ui::PushID(name.c_str());
					if(ui::CollapsingHeader( (name + " " + elem_type + "[" + std::to_string(arraysize) +"]").c_str()))
					{
						ui::Indent();
						for(int i = 0; i < arraysize; i++)
						{
							DrawData(data + elem_bytepos + i * cursize, config, bytepos + elem_bytepos + i * cursize, elem_name + "[" + std::to_string(i) +"]", elem_type);
						}
						ui::Unindent();
					}
					ui::PopID();
				}
			}
		}
		ui::Unindent();
	}

}

void YODataViewerPlugin::DrawInfo(std::shared_ptr<YODataInfo> &data)
{
	ui::PushID(data->topic.c_str());
	if(ui::CollapsingHeader(data->topic.c_str()))
	{
		ui::Indent();
		DrawStruct(data->data.data(), (*data->config), 0, data->type, data->type);
		ui::Unindent();
	}
	ui::PopID();
}

void YODataViewerPlugin::LoadDatatypes(tinyxml2::XMLElement *root, YOVariant &config)
{
	tinyxml2::XMLElement *xdatatypes = root->FirstChildElement("datatypes");
	for(tinyxml2::XMLElement *xtype = xdatatypes->FirstChildElement("datatype") ; xtype ; xtype = xtype->NextSiblingElement("datatype"))
	{
		const char *xname = xtype->Attribute("name");
		std::cout << "load type " << xname << std::endl;
		YOVariant &vtype = config[yo::k::datatypes][xname];
		vtype[yo::k::name] = xname;
		vtype[yo::k::description] = xtype->Attribute("description");
		vtype[yo::k::min] = xtype->Attribute("min");
		vtype[yo::k::max] = xtype->Attribute("max");
		vtype[yo::k::size] = (uint32_t) xtype->Int64Attribute("size") / 8;
	}
}

void YODataViewerPlugin::LoadEnums(tinyxml2::XMLElement *root, YOVariant &config)
{
	tinyxml2::XMLElement *enums = root->FirstChildElement("enums");
	for(tinyxml2::XMLElement *xenum = enums->FirstChildElement("enum") ; xenum ; xenum = xenum->NextSiblingElement("enum"))
	{
		const char *xname = xenum->Attribute("name");
		const char *xtype = xenum->Attribute("type");
		std::cout << "load enum " << xname  << " " << xtype << std::endl;
		YOVariant &venum = config[yo::k::enums][xname];
		venum[yo::k::name] = xname;
		venum[yo::k::type] = xtype;
		venum[yo::k::size] = config[yo::k::datatypes][xtype][yo::k::size];
		for(tinyxml2::XMLElement *xelem = xenum->FirstChildElement("element") ; xelem ; xelem = xelem->NextSiblingElement("element"))
		{
			std::cout << "   load element " << xelem->Attribute("name")  << std::endl;
			venum[yo::k::elements][xelem->Attribute("name")] = (uint32_t) xelem->Int64Attribute("value");
		}
	}
}

void YODataViewerPlugin::LoadStructs(tinyxml2::XMLElement *root, YOVariant &config)
{
	tinyxml2::XMLElement *xstructs = root->FirstChildElement("structs");
	for(tinyxml2::XMLElement *xstruct = xstructs->FirstChildElement("struct") ; xstruct ; xstruct = xstruct->NextSiblingElement("struct"))
	{
		const char *xname = xstruct->Attribute("name");
		std::cout << "load struct " <<  xname << std::endl;

		YOVariant &vstruct = config[yo::k::structs][xname];
		uint32_t struct_alignment = (uint32_t) xstruct->Int64Attribute("alignment");
		vstruct[yo::k::alignment] = struct_alignment;
		vstruct[yo::k::version] = (uint32_t) xstruct->Int64Attribute("version");
		vstruct[yo::k::elements] = YOArray();
		uint32_t size = 0;
		uint32_t idx = 0;
		for(tinyxml2::XMLElement *xelem = xstruct->FirstChildElement("element") ; xelem ; xelem = xelem->NextSiblingElement("element"))
		{
			const char *xelname = xelem->Attribute("name");
			const char *xeltype = xelem->Attribute("type");
			uint32_t bytepos = (uint32_t) xelem->Int64Attribute("bytepos");
			uint32_t arraysize = (uint32_t) xelem->Int64Attribute("arraysize");
			uint32_t el_alignment = (uint32_t) xelem->Int64Attribute("alignment");
			YOVariant velem(yo::k::elements);
			velem[yo::k::id] = idx++;
			velem[yo::k::name] = xelname;
			velem[yo::k::bytepos] = bytepos;
			velem[yo::k::arraysize] = arraysize;
			velem[yo::k::alignment] = el_alignment;
			velem[yo::k::byteorder] = xelem->Attribute("byteorder");
			velem[yo::k::type] = xeltype;
			uint32_t cur_size = 0;
			if(config[yo::k::datatypes].hasChild(xeltype))
			{
				cur_size = config[yo::k::datatypes][xeltype][yo::k::size];
				velem[yo::k::data] = yo::k::datatypes;
			}
			else if(config[yo::k::enums].hasChild(xeltype))
			{
				cur_size = config[yo::k::enums][xeltype][yo::k::size];
				velem[yo::k::data] = yo::k::enums;
			}
			else if(config[yo::k::structs].hasChild(xeltype))
			{
				cur_size = config[yo::k::structs][xeltype][yo::k::size];
				velem[yo::k::data] = yo::k::structs;
			}
			else
			{
				std::cout << "   NO SELECT !!!!!!!!!!!!!!!!!!!!!!!!!! " << xeltype << std::endl;
			}
			velem[yo::k::size] = cur_size;
			vstruct[yo::k::elements].push_back(velem);
			size = bytepos + cur_size * arraysize;
			std::cout << "   load element " << xelname
					<< " type: " << xeltype
					<< " cur_size: " <<  cur_size
					<< " arraysize: " <<  arraysize
					<< " total size: " <<  size
					<< " byte pos: " << bytepos << std::endl;
		}
		vstruct[yo::k::size] = (uint32_t) align_up(size, struct_alignment);
	}
}

void YODataViewerPlugin::LoadFile(const std::string &file_name, YOVariant &config)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError er = doc.LoadFile(file_name.c_str());
	if(er != 0)
	{
		std::cout << "Can't load file " << file_name << ". Error: " << er << std::endl;
		return;
	}

	std::cout << "File " << file_name << ". Loaded Ok" << std::endl;
	tinyxml2::XMLElement *root = doc.FirstChildElement();
	std::cout << "Root element " << root->Name() << std::endl;
	LoadDatatypes(root, config);
	LoadEnums(root, config);
	LoadStructs(root, config);

	tCANData can;
	std::cout << " tCANData " << sizeof(tCANData) << std::endl;
	std::cout << " tMessageHeader " << sizeof(tMessageHeader) << std::endl;

	std::cout << " ui8Tag " << offsetof(tMessageHeader, ui8Tag) << std::endl;
	std::cout << " ui8Channel " << offsetof(tMessageHeader, ui8Channel) << std::endl;
	std::cout << " tmTimeStamp " << offsetof(tMessageHeader, tmTimeStamp) << std::endl;

	std::cout << " tData " << sizeof(tData) << std::endl;
}


std::shared_ptr<YODataInfo> YODataViewerPlugin::GetOrCreateData(const std::string &topic)
{
	std::unique_lock lk(mu_);                 // writer lock
	auto& ptr = streams_[topic];
	if (!ptr)
		ptr = std::make_shared<YODataInfo>();
	return ptr;
}

std::vector<std::pair<std::string, std::shared_ptr<YODataInfo>>> YODataViewerPlugin::GetSnapshot() const
{
	std::shared_lock lk(mu_);
	std::vector<std::pair<std::string, std::shared_ptr<YODataInfo>>> out;
	out.reserve(streams_.size());
	for (auto& kv : streams_)
		out.emplace_back(kv.first, kv.second);
	return out;
}
