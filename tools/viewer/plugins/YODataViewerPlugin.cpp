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
	YO_KEY(primitive, "prm")
	YO_KEY(send, "snd")
	YO_KEY(reverse, "rev")
	YO_KEY(structs, "srs")
	YO_KEY(streams, "sms")
	YO_KEY(stream, "srm")
}

enum class YODatatype
{
    UInt8,
	Int8,
	UInt16,
	Int16,
	UInt32,
	Int32,
	UInt64,
	Int64,
	Float32,
	Float64,
	Bool,
	Unknown
};

std::map <std::string, YODatatype> type_map_ {
    {"tUInt8", YODatatype::UInt8},
	{"tInt8", YODatatype::Int8},
	{"tUInt16", YODatatype::UInt16},
	{"tInt16", YODatatype::Int16},
	{"tUInt32", YODatatype::UInt32},
	{"tInt32", YODatatype::Int32},
	{"tUInt64", YODatatype::UInt64},
	{"tInt64", YODatatype::Int64},
	{"tFloat32", YODatatype::Float32},
	{"tFloat64", YODatatype::Float64},
	{"tBool", YODatatype::Bool},
	{"tUnknown", YODatatype::Unknown}
};

uint32_t align_up(uint32_t field_end, uint32_t struct_align)
{
	return (field_end + (struct_align-1)) & ~(struct_align-1);
}

template<typename T>
inline T load_unaligned(const void *data)
{
    T v;
    std::memcpy(&v, data, sizeof(T));
    return v;
}

float get_float(const void *data, uint32_t type)
{
	float v = 0;
	switch(type)
	{
	case (uint32_t)YODatatype::UInt8:
		v = load_unaligned<uint8_t>(data); break;
	case (uint32_t)YODatatype::Int8:
		v = load_unaligned<int8_t>(data); break;
	case (uint32_t) YODatatype::UInt16:
		v = load_unaligned<uint16_t>(data); break;
	case (uint32_t) YODatatype::Int16:
		v = load_unaligned<int16_t>(data); break;
	case (uint32_t) YODatatype::UInt32:
		v = load_unaligned<uint32_t>(data); break;
	case (uint32_t) YODatatype::Int32:
		v = load_unaligned<int32_t>(data); break;
	case (uint32_t) YODatatype::UInt64:
		v = load_unaligned<uint64_t>(data); break;
	case (uint32_t) YODatatype::Int64:
		v = load_unaligned<int64_t>(data); break;
	case (uint32_t) YODatatype::Float32:
		v = load_unaligned<float>(data); break;
	case (uint32_t) YODatatype::Float64:
		v = load_unaligned<double>(data); break;
	case (uint32_t) YODatatype::Bool:
		v = load_unaligned<bool>(data); break;
	}
	return v;
}

std::string get_string(const void *data, uint32_t type)
{
	std::string v;
	switch(type)
	{
	case (uint32_t)YODatatype::UInt8:
		v = std::to_string(load_unaligned<uint8_t>(data)); break;
	case (uint32_t)YODatatype::Int8:
		v = std::to_string(load_unaligned<int8_t>(data)); break;
	case (uint32_t) YODatatype::UInt16:
		v = std::to_string(load_unaligned<uint16_t>(data)); break;
	case (uint32_t) YODatatype::Int16:
		v = std::to_string(load_unaligned<int16_t>(data)); break;
	case (uint32_t) YODatatype::UInt32:
		v = std::to_string(load_unaligned<uint32_t>(data)); break;
	case (uint32_t) YODatatype::Int32:
		v = std::to_string(load_unaligned<int32_t>(data)); break;
	case (uint32_t) YODatatype::UInt64:
		v = std::to_string(load_unaligned<uint64_t>(data)); break;
	case (uint32_t) YODatatype::Int64:
		v = std::to_string(load_unaligned<int64_t>(data)); break;
	case (uint32_t) YODatatype::Float32:
		v = std::to_string(load_unaligned<float>(data)); break;
	case (uint32_t) YODatatype::Float64:
		v = std::to_string(load_unaligned<double>(data)); break;
	case (uint32_t) YODatatype::Bool:
		v = load_unaligned<bool>(data ? "true" : "false"); break;
	}
	return v;
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

	if(!settings_->hasChild(yo::k::topic))
		(*settings_)[yo::k::topic] = "PLOTTER";

	Advertise((*settings_)[yo::k::topic].getStr());

	if(channels_->getTypeId()!=1)
		channels_->m_value = YOArray();

	for(auto &ch : channels_->getArray())
	{
		std::string file = ch[yo::k::file].getStr();
		std::string topic = ch[yo::k::topic].getStr();
		std::string stream = ch[yo::k::stream].getStr();

		LoadFile( file, (*config_)[yo::k::data][file]);

		std::shared_ptr<YODataInfo> data = GetOrCreateData(topic);
		data->config =  &(*config_)[yo::k::data][file];
		data->topic = topic;
		data->type = stream;
		data->file = file;
		data->size = (*data->config)[yo::k::structs][stream][yo::k::size].getU32();
		data->data.resize(data->size);

		std::cout <<  "Loading Topic: " << topic
				<<  " Stream: " << stream
				<<  " Size: " << data->size
			    << std::endl;
		Subscribe(topic);
	}
}
void YODataViewerPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	//std::cout << " YODataViewerPlugin " << topic << std::endl;
	std::shared_ptr<YODataInfo> data = GetOrCreateData(topic);
	data->ts = message->getTimestamp();
	data->data.resize( message->getDataSize());
	memcpy(data->data.data(), message->getData(), message->getDataSize());
	for(auto &plt : data->plot)
	{
		YOVariant msg;
		msg[plt.second.field] = get_float(message->getData() + plt.second.bytepos, plt.second.primitive);
		Transmit((*settings_)[yo::k::topic].getStr(), msg);
	}
}

void YODataViewerPlugin::OnGui()
{
	if (ImGui::BeginTabBar("Data Viewer"))
	{
		if (ImGui::BeginTabItem("Data"))
		{
			Draw();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Settings"))
		{
			ui::SeparatorText("Settings");
				gui_.draw(*settings_);
			ui::SeparatorText("Channels");
				gui_.draw(*channels_);
			if(ui::Button("Add"))
			{
				YOVariant channel(yo::k::channel);
				channel[yo::k::file] = "";
				channel[yo::k::topic] = "";
				channel[yo::k::stream] = "";
				channels_->push_back(channel);
			}
			ui::SeparatorText("Data Types");
				gui_.draw(*data_);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("About"))
		{
			ImGui::Text("Data Viewer Plugin");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void YODataViewerPlugin::Draw()
{
	std::vector<std::pair<std::string, std::shared_ptr<YODataInfo>>>  data = GetSnapshot();
	if (ImGui::BeginTable("datatable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
	{
		for(auto &pair : data)
		{
			DrawInfo(pair.second);
		}
		ImGui::EndTable();
	}
}

void YODataViewerPlugin::PlotData(uint32_t bytepos, const std::string &field_name, uint32_t primitive)
{
	std::shared_ptr<YODataInfo> data = GetOrCreateData(current_topic_);
	data->plot[bytepos] = { bytepos, field_name, primitive};
	std::cout << "Plot data "
			<< current_topic_ << " " << field_name
			<< " pos " << bytepos
			<< " prim " << primitive << std::endl;
}

void YODataViewerPlugin::DrawData(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &field_name, const std::string &type_name)
{
	YOVariant &vdata = config[yo::k::datatypes][type_name];
	uint32_t primitive = vdata[yo::k::primitive].getU32();
	std::string value =  get_string(data, primitive);
	ui::PushID(field_name.c_str());
	if(ui::Button("Plot"))
	{
		PlotData(bytepos, field_name, primitive);
	}
	ui::PopID();
	ui::SameLine();
	ui::Text("%s: %s", field_name.c_str(), value.c_str());
	ui::SetItemTooltip("%s: %s\nType: %s\nBytepos: %d\nSize: %d",  field_name.c_str(), value.c_str(), type_name.c_str(),  bytepos, vdata[yo::k::size].getU32());
}

void YODataViewerPlugin::DrawEnum(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &field_name, const std::string &type_name)
{
	YOVariant &venum = config[yo::k::enums][type_name];
	uint32_t primitive = venum[yo::k::primitive].getU32();
	std::string value = get_string(data, primitive);
	ui::PushID(field_name.c_str());
	if(ui::Button("Plot"))
	{
		PlotData(bytepos, field_name, primitive);
	}
	ui::PopID();
	ui::SameLine();
	ui::Text("%s: %s (%s)", field_name.c_str(), value.c_str() , venum[yo::k::reverse][value].c_str());
	ui::SetItemTooltip("%s: %s\nType: %s\nBase type %s\nBytepos %d \nSize: %d", field_name.c_str(), value.c_str(), type_name.c_str(), venum[yo::k::type].c_str(), bytepos, venum[yo::k::size].getU32());
}

void YODataViewerPlugin::DrawStruct(const uint8_t *data, YOVariant &config, uint32_t bytepos, const std::string &name, const std::string &type_name)
{
	YOVariant &vstruct = config[yo::k::structs][type_name];
	ui::PushID(name.c_str());
	if(ui::CollapsingHeader((name + " : " + type_name).c_str()))
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
			bool show = true;
			if(arraysize > 1)
			{
				show = ui::CollapsingHeader( (elem_name + " : " + elem_type + "[" + std::to_string(arraysize) +"]").c_str());
				ui::Indent();
			}
			if(show)
			{
				for(int i = 0; i < arraysize; i++)
				{
					uint32_t offset = elem_bytepos + i * cursize;
					std::string cur_elem_name = arraysize > 1 ? elem_name + "[" + std::to_string(i) +"]" : elem_name;

					if(elem[yo::k::data].getStr() == yo::k::structs)
					{
						DrawStruct(data + offset, config, bytepos + offset, cur_elem_name, elem_type);
					}
					else if(elem[yo::k::data].getStr() == yo::k::enums)
					{
						DrawEnum(data + offset, config, bytepos + offset, cur_elem_name, elem_type);
					}
					else if(elem[yo::k::data].getStr() == yo::k::datatypes)
					{
						DrawData(data + offset, config, bytepos + offset, cur_elem_name, elem_type);
					}
				}
			}
			if(arraysize > 1)
			{
				ui::Unindent();
			}
		}
		ui::Unindent();
	}
	ui::PopID();
}

void YODataViewerPlugin::DrawInfo(std::shared_ptr<YODataInfo> &data)
{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ui::Text("%llu", data->ts);
		ImGui::TableSetColumnIndex(1);
		ui::PushID(data->topic.c_str());
		current_topic_ = data->topic;
		DrawStruct(data->data.data(), (*data->config), 0,  data->topic, data->type);
		std::string plot;
		for(auto &plt : data->plot)
		{
			plot += " " + std::to_string(plt.first);
		}
		ui::Text("Plot : %s", plot.c_str());
		current_topic_ = "";
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
		vtype[yo::k::primitive] = (uint32_t) type_map_[xname];
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
		std::cout << "load enum " << xname  << " " << xtype << " " << (uint32_t) type_map_[xtype] << std::endl;
		YOVariant &venum = config[yo::k::enums][xname];
		venum[yo::k::name] = xname;
		venum[yo::k::type] = xtype;
		venum[yo::k::size] = config[yo::k::datatypes][xtype][yo::k::size];
		venum[yo::k::primitive] = (uint32_t )type_map_[xtype];
		for(tinyxml2::XMLElement *xelem = xenum->FirstChildElement("element") ; xelem ; xelem = xelem->NextSiblingElement("element"))
		{
			std::cout << "   load element " << xelem->Attribute("name")  << std::endl;
			venum[yo::k::elements][xelem->Attribute("name")] = (uint32_t) xelem->Int64Attribute("value");
			venum[yo::k::reverse][xelem->Attribute("value")] = xelem->Attribute("name");
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
				std::cout << "   WRONG TYPE: " << xeltype << std::endl;
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
