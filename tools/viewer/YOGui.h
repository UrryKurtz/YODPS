/*
 * YOGui.h
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOGUI_H_
#define UTILS_YOGUI_H_
#include "YOVariant.h"
#include "YOKeys.h"
#include <map>

class YOGui
{
    std::shared_ptr<YOVariant> config_;
    YOVariant limits_ = YOVariant(yo::k::limits);

    std::string param_;
    std::string path_;
    bool changed_;

public:
    YOGui(std::shared_ptr<YOVariant> config);
    virtual ~YOGui();
    std::string getParam() {return param_;}
    std::string getPath() {return path_;}

    bool draw();
    void drawCfg(YOVariant &cfg, const std::string &name = "", bool add = true);

};

#endif /* UTILS_YOGUI_H_ */
