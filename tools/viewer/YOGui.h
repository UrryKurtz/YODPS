/*
 * YOGui.h
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOGUI_H_
#define UTILS_YOGUI_H_
#include <Urho3D/UI/UI.h>
#include <Urho3D/SystemUI/SystemUI.h>
#include <Urho3D/SystemUI/SystemUIEvents.h> // E_SYSTEMUI
#include "YOVariant.h"
#include "YOKeys.h"
#include <map>

class YOGui
{
    YOVariant *current_;
    std::string param_;
    std::string path_;
    std::string path_code_;
    bool changed_;
    std::vector<int> index_;
    std::vector<int> index_out_;

public:
    YOGui();
    virtual ~YOGui();
    std::string getParam() {return param_;}
    std::string getPath() {return path_;}
    YOVariant *getConfig() {return current_;}
    std::vector<int> getIndex(){return index_out_;}

    bool draw(YOVariant &cfg);
    void drawCfg(YOVariant &cfg, const std::string &name = "", bool add = true, bool show_name = true, int *select = 0, int val = 0);

};

#endif /* UTILS_YOGUI_H_ */
