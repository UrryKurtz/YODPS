/*
 * YOGui.cpp
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#include "YOGui.h"

YOGui::YOGui()
{
    changed_ = false;
    current_ = 0;
}

YOGui::~YOGui()
{
}

bool YOGui::draw(YOVariant &cfg)
{
    changed_ = false;
    param_ = "";
    path_ = "";
    index_.clear();
    drawCfg(cfg);
    return changed_;
}

void YOGui::drawCfg(YOVariant &cfg, const std::string &path, bool add, bool show_name, int *select, int value)
{
    bool changed = false;
    ui::PushID(&cfg);

    //std::string name =  yo_keys.count(cfg.m_name) ? yo_keys[cfg.m_name] : cfg.m_name;
    std::string name = yo_key(cfg.m_name);
    ui::AlignTextToFramePadding();

    std::string new_path = path;

    if(add)
        new_path += "/" + name;

    if(cfg.hasChild(yo::k::id))
    {
        name += " " + std::to_string(cfg[yo::k::id].get<uint32_t>());
    }

    if(cfg.hasChild(yo::k::name))
    {
        name += " [" + cfg[yo::k::name].get<std::string>() + "]";
    }
    ui::SetNextItemWidth(250);

    switch (size_t xtype = cfg.m_value.index())
    {
        case 0: //map
		if(cfg.hasChild(yo::k::enabled))
		{
			drawCfg(cfg[yo::k::enabled], new_path, true, false);
			ui::SameLine();
		}

		if (ui::CollapsingHeader(name.c_str()))
        {
        	ui::Indent();

			for (auto &pair : cfg.get<YOMap>() )
			{
				drawCfg(pair.second, new_path, true, true, select, value);
			}
			ui::Unindent();
        }
            break;
        case 1: //array
        {
            name += "[" + std::to_string(cfg.getArraySize()) + "]";
            if (ui::CollapsingHeader(name.c_str()))
            {
            	ui::Indent();
	        	int select = 0;
            	for (int i = 0; i < cfg.getArraySize(); i++)
                {
    				if(cfg[i].hasChild(yo::k::select) && cfg[i][yo::k::select])
    				{
    					select = i;
    				}
                }
            	bool check = false;
            	for (int i = 0; i < cfg.getArraySize(); i++)
                {
    				if(cfg[i].hasChild(yo::k::select))
    				{
    					if( i != select)
    					{
    						cfg[i][yo::k::select] = false;
    					}
    					drawCfg(cfg[i][yo::k::select], new_path + "/#" + std::to_string(index_.size()), true, false, &select, i);
						ui::SameLine();
    				}
    				index_.push_back(i);
    				drawCfg(cfg[i], new_path + "/#" + std::to_string(index_.size()-1), false, false, &select, i);
    				if(changed_ && !check)
    				{
    					index_out_ = index_;
    					check = true;
    				}
    				else
    				{
    					index_.pop_back();
    				}
                }
                ui::Unindent();
            }
        }
            break;

        case 4: //std::string
        {
            char buf[256];
            strncpy(buf, cfg.get<std::string>().c_str(), sizeof(buf));
            buf[sizeof(buf)-1] = '\0';
            if (ui::InputText(name.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue ))
            {
                cfg.get<std::string>() = buf; // copy back when edited
                changed = true;
            }
        }
            break;

       case 5: //YOStringList
       {
           YOStringList &sl = cfg.get<YOStringList>();
           if (ui::BeginCombo((name + "##" + new_path) .c_str(), sl.items[sl.select].c_str()))
           {
               for (int n = 0; n < (int)sl.items.size(); n++) {
                   bool isSelected = (sl.select == n);
                   if (ui::Selectable(sl.items[n].c_str(), isSelected))
                       { sl.select = n;
                         changed = true; }
                   if (isSelected)
                       ui::SetItemDefaultFocus();
               }
               ui::EndCombo();
           }
       }
       break;

        case 6: //bool
        if(ui::Checkbox(show_name ? name.c_str() : "", ( bool*) &cfg.m_value))
        {
        	changed = true;
        	if( select ) *select = value;
        }
        break;

        case 7: //float
        {
            if(ui::DragScalar(name.c_str(), ImGuiDataType_Float,  &cfg.get<float>(), 0.01f))
            {
                changed = true;
            }
        }
            break;

        case 13: //int32_t
            if(ui::DragScalar(name.c_str(), ImGuiDataType_S32, &cfg.m_value, 1, 0, 0, "%u", (name == "id") ? ImGuiSliderFlags_ReadOnly : 0))
            {
                changed = true;
            }
            break;

        case 14: //uint32_t
            if(ui::DragScalar(name.c_str(), ImGuiDataType_U32, &cfg.m_value, 1, 0, 0, "%u", (name == "id") ? ImGuiSliderFlags_ReadOnly : 0))
            {
                changed = true;
            }
            break;

        case 17: //IPv4
        {
            YOIPv4 &ip = cfg;
            if(ui::DragScalarN("ip", ImGuiDataType_U8, &ip.ip, 4, 1.0f))
            {
                changed = true;
            }
            ui::SameLine();
            ui::SetNextItemWidth(100);
            ui::DragScalar("port", ImGuiDataType_U16, &ip.port);
            ui::SameLine(); ui::Text(" [%s]" ,  name.c_str());
        }
            break;

        case 22: //YOVector3
            if(ui::DragFloat3(name.c_str(), (float *)&cfg.get<YOVector3>()))
            {
                changed = true;
            }
            break;
        case 23: //YOVector4
            if(ui::DragFloat4(name.c_str(), (float *)&cfg.get<YOVector4>()))
            {
                changed = true;
            }
            break;

        case 24: //YOColor3F
            if(ui::ColorEdit3(name.c_str(), (float *)&cfg.get<YOColor3F>(), ImGuiColorEditFlags_NoInputs))
            {
                changed = true;
            }
            break;

        case 25: //YOColor4F
            if(ui::ColorEdit4(name.c_str(), (float *)&cfg.get<YOColor4F>(), ImGuiColorEditFlags_NoInputs))
            {
                changed = true;
            }
            break;

        case 26: //YOColor3C
            {
                YOColor3F clr = convert(cfg.get<YOColor3C>());
                if(ui::ColorEdit3(name.c_str(), (float *)&cfg.get<YOColor3C>(), ImGuiColorEditFlags_NoInputs))
                {
                    cfg.m_value = clr;
                    changed = true;
                }
            }
            break;

        case 27: //YOColor4C
            {
                YOColor4F clr = convert(cfg.get<YOColor4C>());
                if(ui::ColorEdit4(name.c_str(), (float *)&clr, ImGuiColorEditFlags_NoInputs))
                {
                    cfg.m_value = clr;
                    changed = true;
                }
            }
            break;

        case 35: //YOLimitF
            {
                YOLimitF &lim = cfg;
                if(ui::DragScalar(name.c_str(), ImGuiDataType_Float, &lim.value, lim.speed, &lim.min, &lim.max, "%0.02f", 0))
                {
                    changed = true;
                }
            }
            break;

        default:
        {
            ui::Text("[%s] TYPE %s (%d) PATH: %s" ,  name.c_str(), YOValue_type_name(cfg.m_value.index()).c_str(), cfg.m_value.index(), new_path.c_str() );
        }
    };

    if(changed)
    {
        param_ = name;
        path_ = path + "/" + name;
        changed_ = true;
        current_ = &cfg;
        //std::cout << "GUI changed " << new_path  << "  param : " << param_ << std::endl;
    }
    ui::PopID();
}
