/*
 * YOPlotterPlugin.cpp
 *
 *  Created on: Oct 3, 2025
 *      Author: kurtz
 */
#include <algorithm>
#include <Urho3D/UI/UI.h>
#include "YOPlotterPlugin.h"
#include "YONode.h"
#include "YOKeys.h"

YOPlotterPlugin::YOPlotterPlugin(Context *context) :
		IPlugin(context)
{

}

YOPlotterPlugin::~YOPlotterPlugin()
{

}

void YOPlotterPlugin::OnStart()
{
	settings_ = &(*config_)["Settings"];
	if (!settings_->getMapSize())
	{
		(*settings_)["Buffer Size"] = 1024u;
		(*settings_)["Time scale"] = 1000000u;
		(*settings_)["Topic"] = "PLOTTER";
		(*settings_)["Pause"] = false;
		(*settings_)["Receive"] = true;
	}
	settings_ = &(*config_)["Settings"];
	streams_ = &(*config_)["Streams"];
	for (auto &stream : streams_->get<YOMap>())
	{
		data_[stream.first].strean_cfg = &stream.second;
		std::cout << " STREAM: " << stream.first << std::endl;
	}
	subs_.push_back(topic_);
}

YOVariant StreamConfig(const std::string &name)
{
	YOVariant config(name);
	config["Name"] = name.c_str();
	config["Color"] = YOColor4C { 0xFF, 0xFF, 0xFF, 0xFF };
	config["Width"] = 1.0f;
	config["Receive"] = true;
	config["Scale"] = 1.0f;
	config["Auto Scale"] = false;
	config["Smooth"] = false;
	config["Show"] = true;
	return config;
}

void YOPlotterPlugin::AddValue(const std::string &name, YOTimestamp ts, const float &value)
{
	if (!streams_->hasChild(name))
	{
		(*streams_)[name] = StreamConfig(name);
		data_[name].strean_cfg = &(*streams_)[name];
	}

	YOStreamInfo &stream = data_[name];
	if (!stream.strean_cfg->get("Receive"))
	{
		return;
	}

	lock_[name].lock();
	stream.stream.push_front({ ts, value });
	stream.last = value;
	if (stream.stream.size() > buf_size_)
	{
		stream.stream.pop_back();
	}
	lock_[name].unlock();
}

void YOPlotterPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::shared_ptr<YOVariant> frame = std::make_shared<YOVariant>(message->getDataSize(), (const char*) message->getData());
	for (auto &stream : frame->get<YOMap>())
	{
		YOTimestamp ts = YONode::getTimestamp();
		std::string stream_name = yo_key(stream.first);
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

inline float Nice125Unit(float v)
{
	float sign = 1.0f;
	if (v < 0.0f)
		sign = -1.0f;

	float a = std::fabs(v);
	if (a == 0.0f)
		return sign * 1.0f;
	float e = std::floor(std::log10(a));
	float base = std::pow(10.0f, e);
	float m = a / base;
	float mul = (m < 2.0f) ? 1.0f : (m < 5.0f ? 2.0f : 5.0f);
	return sign * mul * base;
}

inline float DecadeUnit(float v)
{
	float sign = 1.0f;

	if (v < 0.0f)
		sign = -1.0f;

	float a = std::fabs(v);
	if (a == 0.0f)
		return 1.0f;
	float e = std::floor(std::log10(a));
	return sign * std::pow(10.0f, e);  // ...10, 1, 0.1, 0.01, ...
}

void YOPlotterPlugin::OnGui()
{
	YOTimestamp ts = YONode::getTimestamp();
	if (settings_->get("Pause"))
		ts = last_ts_;
	else
		ts = YONode::getTimestamp();

	ui::Begin("Plotter");
	ImVec2 size = ui::GetContentRegionAvail();

	ui::BeginChild("plotter", size - ImVec2(300,0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ImDrawList *dl = ui::GetWindowDrawList();
	ImVec2 p0 = ui::GetCursorScreenPos();
	ImVec2 avail = ui::GetContentRegionAvail();

	ImVec2 startI = p0 + ImVec2(avail.x, avail.y / 2);
	ImVec2 endI = ImVec2(p0.x, p0.y + avail.y / 2);

	const bool hovered = ui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

	ImVec2 pos = ui::GetMousePos();
	ImRect box =
	{ pos - ImVec2(20, 20), pos + ImVec2(20, 20) };
	uint32_t &time_period = settings_->get("Time scale");
	for (auto &stream : data_)
	{
		if (!stream.second.stream.size())
			continue;

		lock_[stream.first].lock();

		bool show = stream.second.strean_cfg->get("Show");
		if (!show)
		{
			lock_[stream.first].unlock();
			continue;
		}

		float startV = stream.second.stream.front().value;
		float startT = stream.second.stream.front().timestamp;

		YOColor4C clr = stream.second.strean_cfg->get("Color");
		ImU32 clrc = IM_COL32(clr.r, clr.g, clr.b, clr.a);

		float width = stream.second.strean_cfg->get("Width");
		float &scale = stream.second.strean_cfg->get("Scale");
		bool smooth = stream.second.strean_cfg->get("Smooth");
		bool auto_scale = stream.second.strean_cfg->get("Auto Scale");
		bool dec_scale = false;
		ImVec2 A = startI + ImVec2((startT - ts) / time_period, -startV * scale);
		float min = startV;
		float max = startV;
		for (auto &data : stream.second.stream)
		{
			if (data.value < min)
				min = data.value;
			if (data.value > max)
				max = data.value;

			ImVec2 B = startI + ImVec2((data.timestamp - ts) / time_period, -data.value * scale);
			if (auto_scale && (B.y < p0.y || B.y > p0.y + avail.y))
			{
				dec_scale = true;
			}

			if (hovered && pos.x < A.x && pos.x >= B.x)
			{
				char text[256];
				std::sprintf(text, "%s : %f", stream.first.c_str(), data.value);
				dl->AddText(ImVec2(pos.x, A.y) + ImVec2(5, 10),  clrc  , text);
			}
			if (smooth)
			{
				dl->AddLine(A, B, clrc, width);
			}
			else
			{
				ImVec2 C(A.x, B.y);
				dl->AddLine(A, C, clrc, width);
				dl->AddLine(C, B, clrc, width);
			}
			A = B;
		}

		float mmax = Nice125Unit(max * 1.1);
		float mmin = Nice125Unit(min * 1.1);

		dl->AddLine(ImVec2(p0.x, startI.y - mmin * scale), ImVec2(p0.x + avail.x, startI.y - mmin * scale), clrc, 1);
		char text_min[256];
		std::sprintf(text_min, "%.0f", stream.first.c_str(), mmin);
		dl->AddText(ImVec2(p0.x + avail.x, startI.y - mmin * scale) + ImVec2(-25, 5), clrc, text_min);

		dl->AddLine(ImVec2(p0.x, startI.y - mmax * scale), ImVec2(p0.x + avail.x, startI.y - mmax * scale), clrc, 1);
		char text_max[256];
		std::sprintf(text_max, "%.0f", stream.first.c_str(), mmax);
		dl->AddText(ImVec2(p0.x + avail.x, startI.y - mmax * scale) + ImVec2(-25, 5), clrc, text_max);

		if (dec_scale)
		{
			scale *= 0.95f;
		}

		lock_[stream.first].unlock();
	}

	dl->AddLine(ImVec2(pos.x, p0.y), ImVec2(pos.x, p0.y + avail.y), 0xFF00FFFF); //mouse line
	dl->AddLine(endI, startI, IM_COL32(64, 64, 64, 128), 1); //X axis

	float sec_x = ts % 1000000000 / time_period;
	for (int x = startI.x - sec_x; x > p0.x; x -= 1000000000 / time_period)
	{
		ImVec2 s0(x, p0.y);
		ImVec2 s1(x, p0.y + avail.y);
		dl->AddLine(s0, s1, IM_COL32(64, 64, 64, 128), 1);
	}
	ui::EndChild();
	ui::SameLine();
	ui::BeginChild("Settings", ImVec2(290, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ui::SeparatorText("Settings");
	DrawSettings((*settings_));

	ui::SeparatorText("Streams");
	for (auto &stream : streams_->get<YOMap>())
	{
		DrawStream(stream.second, stream.first, data_[stream.first].last);
	}
	ui::EndChild();
	ui::End();
	last_ts_ = ts;
}

void YOPlotterPlugin::DrawSettings(YOVariant &config)
{
	bool &pause = (*settings_)["Pause"];
	ui::Checkbox("Pause", &pause);
	uint32_t bufsize = (*settings_)["Buffer Size"];
	ui::DragScalar("Buffer Size", ImGuiDataType_U32, &bufsize);
	uint32_t &timescale = (*settings_)["Time scale"];
	uint32_t min = 10000;
	uint32_t max = 20000000;
	ui::SliderScalar("Time scale", ImGuiDataType_U32, &timescale, &min, &max);

	std::string &topic = (*settings_)["Topic"];

	char tmp[256];
	strcpy(tmp,topic.c_str());
	if(ui::InputText("Topic", tmp, 256, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		topic = tmp;
	}
	//(*settings_)["Topic"] = "PLOTTER";
	//(*settings_)["Receive"] = true;
}


void YOPlotterPlugin::DrawStream(YOVariant &config, const std::string &name, float last)
{
	ui::PushID(&config);
	YOColor4C &clr = config["Color"];
	ImVec4 clrvec((float)clr.r / 255.0f, (float)clr.g / 255.0f, (float)clr.b / 255.0f, (float)clr.a / 255.0f);
	bool &en = config["Show"];
	ui::Checkbox("##check", &en);
	if (ui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
	{
    	ui::SetTooltip("Show");
	}
	ui::SameLine();
	if(ui::ColorEdit4("##color", (float*)&clrvec, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
	{
			clr.r = clrvec.x * 255;
			clr.g = clrvec.y * 255;
			clr.b = clrvec.z * 255;
			clr.a = clrvec.w * 255;
	}
	ui::SameLine();
	ui::PushStyleColor(ImGuiCol_Text, clrvec);
 	ui::Text("%s : %f", name.c_str(), last);
	ui::PopStyleColor();
  if (ui::CollapsingHeader(name.c_str()))
  {
		ui::Indent();
		bool &rcv = config["Receive"];
		ui::Checkbox("Receive", &rcv);
		bool &auto_scl = config["Auto Scale"];
		ui::Checkbox("Auto Scale", &auto_scl);
		bool &smooth = config["Smooth"];
		ui::Checkbox("Smooth", &smooth);
		float &width = config["Width"];
		ui::SliderFloat("Width", &width, 1, 10, "%0.01f");
		float &scale = config["Scale"];
		ui::DragFloat("Scale", &scale);
		ui::Unindent();
  }
	ui::PopID();
}
