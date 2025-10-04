/*
 * YOPlotterPlugin.cpp
 *
 *  Created on: Oct 3, 2025
 *      Author: kurtz
 */

#include <Urho3D/UI/UI.h>
#include "YOPlotterPlugin.h"
#include "YONode.h"

YOPlotterPlugin::YOPlotterPlugin(Context *context) : IPlugin(context)
{

}

YOPlotterPlugin::~YOPlotterPlugin()
{

}

void YOPlotterPlugin::OnStart()
{
	subs_.push_back(topic_);
}


void YOPlotterPlugin::AddValue(const std::string &name, YOTimestamp ts, const float &value)
{
	lock_[name].lock();
	data_[name].stream.push_front({ts, value});
	if(data_[name].stream.size() > buf_size_)
	{
		data_[name].stream.pop_back();
	}
	lock_[name].unlock();
}


void YOPlotterPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::shared_ptr<YOVariant> frame = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
	for( auto &stream : frame->get<YOMap>())
	{
		YOTimestamp ts = YONode::getTimestamp();
		std::string stream_name = stream.first;
		YOValue &value = stream.second.m_value;

		switch (stream.second.m_value.index())
		{
		case 6: //bool
			AddValue(stream_name, ts, std::get<bool>(value));
			break;
		case 7: //float
			AddValue(stream_name, ts, std::get<float>(value));
			break;
		case 8: //double
			AddValue(stream_name, ts, std::get<double>(value));
			break;
		case 9: //int8_t
			AddValue(stream_name, ts, std::get<int8_t>(value));
			break;
		case 10: //uint8_t
			AddValue(stream_name, ts, std::get<uint8_t>(value));
			break;
		case 11: //int16_t
			AddValue(stream_name, ts, std::get<int16_t>(value));
			break;
		case 12: //uint16_t
			AddValue(stream_name, ts, std::get<uint16_t>(value));
			break;
		case 13: //int32_t
			AddValue(stream_name, ts, std::get<int32_t>(value));
			break;
		case 14: //uint32_t
			AddValue(stream_name, ts, std::get<uint32_t>(value));
			break;
		case 15: //int64_t
			AddValue(stream_name, ts, std::get<int64_t>(value));
			break;
		case 16: //uint64_t
			AddValue(stream_name, ts, std::get<uint64_t>(value));
			break;

		case 19: //YOVector2
			AddValue(stream_name + ".x", ts, std::get<YOVector2>(value).x);
			AddValue(stream_name + ".y", ts, std::get<YOVector2>(value).y);
			break;
		case 20: //YOVector2I
			AddValue(stream_name + ".x", ts, std::get<YOVector2I>(value).x);
			AddValue(stream_name + ".y", ts, std::get<YOVector2I>(value).y);
			break;

		case 21: //YOVector2U
			AddValue(stream_name + ".x", ts, std::get<YOVector2U>(value).x);
			AddValue(stream_name + ".y", ts, std::get<YOVector2U>(value).y);
			break;

		case 22: //YOVector3
			AddValue(stream_name + ".x", ts, std::get<YOVector3>(value).x);
			AddValue(stream_name + ".y", ts, std::get<YOVector3>(value).y);
			AddValue(stream_name + ".z", ts, std::get<YOVector3>(value).z);
			break;

		case 23: //YOVector4
			AddValue(stream_name + ".x", ts, std::get<YOVector4>(value).x);
			AddValue(stream_name + ".y", ts, std::get<YOVector4>(value).y);
			AddValue(stream_name + ".y", ts, std::get<YOVector4>(value).z);
			AddValue(stream_name + ".w", ts, std::get<YOVector4>(value).w);
			break;
		};

		//std::cout << "Got " << stream.first << " value: " << stream.second.m_value << std::endl;
	}
}

void YOPlotterPlugin::OnUpdate(float timeStep)
{

}

void YOPlotterPlugin::OnGui()
{
	YOTimestamp ts = YONode::getTimestamp();
	if(pause_)
		ts = last_ts_;
	else
		ts = YONode::getTimestamp();

	ui::Begin("Plotter");
	ImVec2 size = ui::GetContentRegionAvail();
	ui::BeginChild("plotter", size  - ImVec2(250, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImVec2 avail = ImGui::GetContentRegionAvail();

	ImVec2 startI = p0 + ImVec2(avail.x, avail.y/2);
	ImVec2 endI = ImVec2(p0.x, p0.y + avail.y/2);

	const bool hovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByActiveItem );

	ImVec2 pos = ui::GetMousePos();
	ImRect box = {pos - ImVec2(20,20), pos + ImVec2(20,20)};

	for(auto &stream : data_)
	{
		if (!stream.second.stream.size())
			continue;

		lock_[stream.first].lock();

		float startV = stream.second.stream.front().value;
		float startT = stream.second.stream.front().timestamp;
		ImVec2 A = startI + ImVec2((startT - ts) / time_period_, -startV);
		for( auto &data : stream.second.stream)
		{
			ImVec2 B = startI + ImVec2((data.timestamp - ts) / time_period_, - data.value);
			if(hovered)
			{
				if( pos.x <= A.x && pos.x >=B.x ) //box.Contains(A)
				{
					char text[256];
					std::sprintf(text, "%s : %f", stream.first.c_str(), data.value);
					dl->AddText(A + ImVec2(0,10),  stream.second.color, text);
				}
			}
			dl->AddLine(A, B, stream.second.color, stream.second.width);
   		    A = B;
		}
		lock_[stream.first].unlock();
	}

	dl->AddLine(ImVec2(pos.x , p0.y), ImVec2(pos.x, p0.y + avail.y), 0xFF00FFFF); //mouse line
	dl->AddLine(endI, startI, IM_COL32(64,64,64,128), 1); //X axis

	float sec_x = ts % 1000000000 / time_period_;

	for (int x = startI.x - sec_x ; x > p0.x; x -= 1000000000 / time_period_ )
	{
		ImVec2 s0 (x, p0.y);
		ImVec2 s1 (x, p0.y + avail.y);
		dl->AddLine(s0, s1, IM_COL32(64,64,64,128), 1);
	}

	ui::EndChild();
	ui::SameLine();
	ui::BeginChild("Settings", ImVec2(240, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

	ui::Checkbox("Pause", &pause_ );
	ui::DragInt("Buffer size", &buf_size_, 32, 1, 1024);
	ui::SliderInt("Time scale", &time_period_, 1000000, 100000000);

	for(auto &stream : data_)
	{
		if (!stream.second.stream.size())
			continue;

		lock_[stream.first].lock();
		float startV = stream.second.stream.front().value;
		float startT = stream.second.stream.front().timestamp;
		ui::PushID(&stream);
		ui::ColorEdit4("##color", (float *)&stream.second.color, ImGuiColorEditFlags_NoInputs); ui::SameLine(); ui::Text("%s :: %f",stream.first.c_str(), startV );
		ui::SliderFloat("width", &stream.second.width, 1.0f, 12.0);
		ui::PopID();
		lock_[stream.first].unlock();

	}
	ui::EndChild();
	ui::End();
	last_ts_ = ts;
}

