/*
 * YOCANPlugin.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: kurtz
 */

#include "YOCANPlugin.h"
#include <Urho3D/IO/FileSystem.h>
#include "YOKeys.h"
#include "YONode.h"
#include <libdbc/exceptions/error.hpp>

namespace yo::k
{
YO_KEY(channels, "chs")
YO_KEY(channel, "chn")
YO_KEY(files, "fls")
YO_KEY(send, "snd")
}

void CreateHex(const uint8_t *data, int size, std::string &hex)
{
	if (size > 0)
	{
		// "AA BB CC" => length 3*size-1
		hex.resize(size * 3 - 1);
		static const char *lut = "0123456789ABCDEF";
		char *p = hex.data();
		for (int i = 0; i < size; ++i)
		{
			uint8_t v = data[i];
			*p++ = lut[v >> 4];
			*p++ = lut[v & 0xF];
			if (i + 1 != size)
				*p++ = ' ';
		}
	}
}

YOCANPlugin::YOCANPlugin(Context *context) :
		IPlugin(context)
{

}

YOCANPlugin::~YOCANPlugin()
{
	// TODO Auto-generated destructor stub
}

void YOCANPlugin::OnStart()
{
	settings_ = &(*config_)[yo::k::settings];
	channels_ = &(*config_)[yo::k::channels];
	send_ = &(*config_)[yo::k::send];

	if (send_->getTypeId() != 1)
	{
		send_->m_value = YOArray();
		send_->setArraySize(8);
	}

	if (!settings_->hasChild(yo::k::topic))
		(*settings_)[yo::k::topic] = "PLOTTER";

	Advertise( (*settings_)[yo::k::topic].getStr() );

	if (channels_->getTypeId() != 1)
	{
		channels_->m_value = YOArray();
	}

	for (uint32_t ch = 0; ch < 8; ch++)
	{
		if (channels_->getArraySize() < 8)
		{
			YOVariant channel(yo::k::channel);
			channel[yo::k::id] = ch;
			channel[yo::k::enabled] = true;
			channel[yo::k::name] = std::string("CAN" + std::to_string(ch)).c_str();
			channel[yo::k::files] = YOArray();
			channels_->push_back(channel);
		}
		YOVariant &channel = (*channels_)[ch];

		for (auto &file : channel[yo::k::files].getArray())
		{
			try
			{
				std::shared_ptr<Libdbc::DbcParser> parser = std::make_shared<Libdbc::DbcParser>();
				parser->parse_file(file.getStr());
				parsers_[file.getStr()].parser = parser;

				std::cout << " Channel " << ch << ": loaded DBC file :" << file.getStr() << " Total messages: " << parser->get_messages().size() << std::endl;
				parsers_[file.getStr()].messages = parser->get_messages();

				for (Libdbc::Message &msg : parsers_[file.getStr()].messages)
				{
					uint32_t id = msg.id();
					if (data_[ch].count(id))
					{
						std::cout << file.getStr() << " duplicate message " << id << std::endl;
					}
					char header[256];
					sprintf(header, "ID %u (0x%02X) ", id, id);
					data_[ch][id].header = header;
					data_[ch][id].id = id;
					data_[ch][id].message = &msg;
					data_[ch][id].signals = msg.get_signals();
					data_[ch][id].send.resize(msg.get_signals().size());
					int sid = 0;
					for (auto &signal : data_[ch][id].signals)
					{
						if ((*send_)[ch].hasChild(header) && (*send_)[ch][header].hasChild(signal.name))
						{
							data_[ch][id].send[sid] = (*send_)[ch][header][signal.name];
						}
						sid++;
					}
					std::cout << "   message " << id << " " << msg.name() << std::endl;
				}
				std::cout << file.getStr() << " messages: " << data_[ch].size() << std::endl;
			} catch (const Libdbc::ValidityError e)
			{
				std::cout << "ERROR: " << file.getStr() << " " << e.what() << std::endl;
			}
		}
		RegisterTopic(channel[yo::k::name].getStr(), ch);
	}
	/*  //TODO gen list of DBC files
	 FileSystem *fs = context_->GetSubsystem<FileSystem>();
	 const ea::string path = "Data/DBC";
	 fs->ScanDir(files_, path, "*.*", SCAN_FILES);
	 for (auto &file : files_)
	 std::cout << "DBC: " << file.c_str() << std::endl;
	 */
}

void YOCANPlugin::SendToPlotter(const std::string &pid, const std::string &signal, float value)
{
	YOVariant msg;
	msg[pid + signal] = value;
	Transmit((*settings_)[yo::k::topic].getStr(), msg);
}

void YOCANPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	//std::cout << topic << " ID " << GetTopicId(topic) << " size " << message->getDataSize() << std::endl;
	tCANFDData *canfd = (tCANFDData*) message->getData();

	int ch = GetTopicId(topic);
	uint32_t id = canfd->sData.ui32Id;

	std::vector<uint8_t> data(canfd->sData.aui8Data, canfd->sData.aui8Data + canfd->sData.ui8Length);
	YOCANInfo &info = data_[ch][id];
	info.mutex.lock();
	info.ts = YONode::getTimestamp();
	info.values.clear();
	info.canfd = *canfd;

	if (info.header.empty())
	{
		char header[256];
		sprintf(header, "ID %u (0x%02X) ", id, id);
		info.header = header;
	}

	if (info.message)
		info.message->parse_signals(data, info.values);
	if (info.values.size() != info.send.size())
		info.send.resize(info.values.size());

	for (int i = 0; i < info.signals.size(); i++)
	{
		if (info.send[i])
		{
			SendToPlotter(info.header, info.signals[i].name, info.values[i]);
		}
	}
	CreateHex(canfd->sData.aui8Data, canfd->sData.ui8Length, info.hex);
	//std::cout << topic << " " << ch << " " << id  << " " << info.hex << std::endl;
	info.mutex.unlock();
	//parse_message(canfd->sData.ui32Id, data, data_[ch][canfd->sData.ui32Id].values);
}

void YOCANPlugin::DrawChannelData(YOVariant &channel, int ch)
{
	ui::SeparatorText(channel[yo::k::name].c_str());
	if (ImGui::BeginTable("tbl2x2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		for (auto &can : data_[ch])
		{
			if (can.second.hex.size())
			{
				can.second.mutex.lock();
				ui::PushID(&can);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ui::Text("%llu", can.second.ts);
				ImGui::TableSetColumnIndex(1);
				bool collapse = false;
				if (can.second.message) //got DBC
				{
					collapse = ui::CollapsingHeader(can.second.header.c_str());
					ui::SameLine();
					ui::Text(" Size: %d  Data:[%s]", can.second.canfd.sData.ui8Length, can.second.hex.c_str());
				}
				else
				{
					ui::Text("   %s Size: %d  Data:[%s]", can.second.header.c_str(), can.second.canfd.sData.ui8Length, can.second.hex.c_str());
				}
				if (collapse)
				{
					int i = 0;
					for (auto &sig : can.second.message->get_signals())
					{
						ui::Text("  %s [%f] %s ", sig.name.c_str(), can.second.values[i], sig.unit.c_str());
						ui::SameLine();
						if (ui::Checkbox("Plot", (bool*) &can.second.send[i]))
						{
							(*send_)[ch][can.second.header][sig.name] = can.second.send[i];
						}
						i++;
					}
				}
				ui::PopID();
				can.second.mutex.unlock();
			}
		}
		ImGui::EndTable();
	}
}

void YOCANPlugin::OnGui()
{
	if (ImGui::BeginTabBar(name_.c_str()))
	{
		for (int ch = 0; ch < 8; ch++)
		{
			YOVariant &channel = (*channels_)[ch];
			if (!channel[yo::k::enabled].getBool())
				continue;
			if (ImGui::BeginTabItem(channel[yo::k::name].c_str()))
			{
				DrawChannelData(channel, ch);
				ImGui::EndTabItem();
			}
		}
		if (ImGui::BeginTabItem("Config"))
		{
			ui::SeparatorText("Config");
			gui_.draw(*settings_);
			ui::SeparatorText("Channels");
			for (auto &channel : channels_->getArray())
			{
				uint32_t id = channel[yo::k::id];
				char name[256];
				sprintf(name, "Channel %u [%s]", id, channel[yo::k::name].c_str());
				ui::PushID(&channel);
				ui::Checkbox("", &channel[yo::k::enabled].getBool());
				ui::SameLine();
				if (ui::CollapsingHeader(name))
				{
					ui::Indent();
					gui_.draw(channel[yo::k::name]);

					if (ui::CollapsingHeader("DBC files"))
					{
						int idx = 0;
						int del = -1;
						for (auto &file : channel[yo::k::files].getArray())
						{
							ui::Indent();
							gui_.draw(file);
							ui::SameLine();
							ui::PushID(&file);
							if (ui::Button("Delete"))
							{
								del = idx;
								std::cout << "delete BTN " << del << " " << idx << std::endl;
							}
							ui::PopID();
							idx++;
							ui::Unindent();
						}
						if (del >= 0)
						{
							std::cout << "delete " << del << std::endl;
							channel[yo::k::files].erase(del);
						}
					}
					ui::SetNextItemWidth(250);
					ui::InputText("Add file", files_tmp_[id], 256);
					ui::SameLine();
					if (ui::Button("Add"))
					{
						std::cout << files_tmp_[id] << std::endl;
						channel[yo::k::files].push_back(YOVariant(yo::k::file, files_tmp_[id]));
						files_tmp_[id][0] = 0;
					}
					ui::Unindent();
				}
				ui::PopID();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("About"))
		{
			ImGui::Text("CAN Plugin");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
