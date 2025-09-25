/*
 * YOXML.cpp
 *
 *  Created on: Sep 13, 2025
 *      Author: kurtz
 */

#include "YOXML.h"
#include "YOKeys.h"

YOXML::YOXML()
{
    // TODO Auto-generated constructor stub
}

YOXML::~YOXML()
{
    // TODO Auto-generated destructor stub
}

bool YOXML::readXML(const std::string &file, YOVariant &cfg)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(file.c_str());
    if (err != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load file: " << file << " : " << doc.ErrorStr() << std::endl;
        return false;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    std::cout << "READ FILE. ROOT: " << root->Name() << std::endl;

    for (tinyxml2::XMLElement* node = root->FirstChildElement("map"); node != nullptr; node = node->NextSiblingElement("map"))
    {
        //if(strcmp(node->Attribute("name"), "cfg") == 0)
        {
            std::cout << "READ FILE. CONFIG: " <<  node->Attribute("name") << std::endl;
            readNode(cfg, node);
        }
    }
    return true;
}

void YOXML::readNode(YOVariant &cfg, tinyxml2::XMLElement* node,  const std::string &tab)
{
    //std::cout << tab << "readNode: START " <<  node->Attribute("name") << std::endl;
    if( strcmp( node->Name(), "map") == 0)
    {
        //std::cout << tab << "MAP: " <<  node->Attribute("name") << std::endl;
        cfg.m_name = node->Attribute("name")  ;
        cfg.m_value = YOMap();

        for (tinyxml2::XMLElement* sub = node->FirstChildElement(); sub != nullptr; sub = sub->NextSiblingElement())
        {
            //std::cout << tab << "  >> MAP: sub " <<  sub->Name() << " " << sub->Attribute("name") << std::endl;
            YOVariant &child = cfg[sub->Attribute("name")];
            readNode(child, sub, tab + "  ");
        }

    }
    else if( strcmp( node->Name(), "array") == 0)
    {
        //std::cout << tab << "ARRAY: " <<  node->Attribute("name") << std::endl;
        cfg.m_name = node->Attribute("name")  ;
        cfg.m_value = YOArray();

        for (tinyxml2::XMLElement* sub = node->FirstChildElement(); sub != nullptr; sub = sub->NextSiblingElement())
        {
            //std::cout << tab << " --- ARRAY: sub " <<  sub->Attribute("name") << std::endl;
            YOVariant child;
            readNode(child, sub, tab + "  ");
            cfg.push_back(child);
        }
    }
    else if(strcmp(node->Name(), "element") == 0)
    {
        //std::cout << tab << "ELEMENT: " <<  node->Attribute("name") << " TYPE: " <<  node->Attribute("type") << " !!!! "  << YOValue_index_of(node->Attribute("type")) <<  std::endl;
        cfg.m_name = node->Attribute("name");
        switch(YOValue_index_of(node->Attribute("type")))
        {

            case 2: //YOData,     //2
                cfg.m_value = node->Attribute("value");
               break;
           case 3: //YODataF,    //3
               cfg.m_value = YOFloatList();
               break;
           case 4: //std::string,//4
               cfg.m_value = node->Attribute("value");
               break;
           case 5: //YOStringList, //5
               cfg.m_value = YOStringList();
               {
                   YOStringList &sl = cfg.get<YOStringList>();
                   sl.select = node->IntAttribute("select");
                   for (tinyxml2::XMLElement* sub = node->FirstChildElement("sub"); sub != nullptr; sub = sub->NextSiblingElement("sub"))
                   {
                       sl.items.push_back(sub->Attribute("value"));
                   }
               }
               break;
           case 6: //bool,       //6
               cfg.m_value = node->BoolAttribute("value");
               break;
           case 7: //float,      //7
               cfg.m_value = node->FloatAttribute("value");
               break;
           case 8: //double,     //8
               cfg.m_value = node->DoubleAttribute("value");
               break;
           case 9: //int8_t,     //9
               cfg.m_value = (int8_t)node->IntAttribute("value");
               break;
           case 10: //uint8_t,    //10
               cfg.m_value = (uint8_t)node->IntAttribute("value");
               break;
           case 11: //int16_t,    //11
               cfg.m_value = (int16_t)node->IntAttribute("value");
               break;
           case 12: //uint16_t,   //12
               cfg.m_value = (uint16_t)node->IntAttribute("value");
               break;
           case 13: //int32_t,    //13
               cfg.m_value = (int32_t)node->IntAttribute("value");
               break;
           case 14: //uint32_t,   //14
               cfg.m_value = (uint32_t)node->IntAttribute("value");
               break;
           case 15: //int64_t,    //15
               cfg.m_value = node->Int64Attribute("value");
               break;
           case 16: //uint64_t,   //16
               cfg.m_value = node->Unsigned64Attribute("value");
               break;
           case 17: //YOIPv4,     //17
           {
               YOIPv4 v;
               v.ip[0] = node->IntAttribute("ip0");
               v.ip[1] = node->IntAttribute("ip1");
               v.ip[2] = node->IntAttribute("ip2");
               v.ip[3] = node->IntAttribute("ip3");
               v.port = (uint16_t)node->IntAttribute("port");
               cfg.m_value = v;
           }
               break;
           case 18: //YOIPv6,     //18
           {
               YOIPv6 v;
               v.ip[0] = node->IntAttribute("ip0");
               v.ip[1] = node->IntAttribute("ip1");
               v.ip[2] = node->IntAttribute("ip2");
               v.ip[3] = node->IntAttribute("ip3");
               v.ip[4] = node->IntAttribute("ip4");
               v.ip[5] = node->IntAttribute("ip5");
               v.port = (uint16_t)node->IntAttribute("port");
               cfg.m_value = v;
           }
               break;
           case 19: //YOVector2,  //19
           {
               YOVector2 v;
               v.x = node->FloatAttribute("x");
               v.y = node->FloatAttribute("y");
               cfg.m_value = v;
           }
               break;
           case 20: //YOVector2I, //20
           {
               YOVector2I v;
               v.x = node->IntAttribute("x");
               v.y = node->IntAttribute("y");
               cfg.m_value = v;
           }
               break;
           case 21: //YOVector2U, //21
           {
               YOVector2U v;
               v.x = node->UnsignedAttribute("x");
               v.y = node->UnsignedAttribute("y");
               cfg.m_value = v;
           }
               break;
           case 22: //YOVector3,  //22
           {
               YOVector3 v;
               v.x = node->FloatAttribute("x");
               v.y = node->FloatAttribute("y");
               v.z = node->FloatAttribute("z");
               cfg.m_value = v;
           }
               break;
           case 23: //YOVector4,  //23
           {
               YOVector4 v;
               v.x = node->FloatAttribute("x");
               v.y = node->FloatAttribute("y");
               v.z = node->FloatAttribute("z");
               v.w = node->FloatAttribute("w");
               cfg.m_value = v;
           }
               break;
           case 24: //YOColor3F,  //24
           {
               YOColor3F v;
               v.r = node->FloatAttribute("r");
               v.g = node->FloatAttribute("g");
               v.b = node->FloatAttribute("b");
               cfg.m_value = v;
           }
               break;
           case 25: //YOColor4F,  //25
           {
               YOColor4F v;
               v.r = node->FloatAttribute("r");
               v.g = node->FloatAttribute("g");
               v.b = node->FloatAttribute("b");
               v.a = node->FloatAttribute("a");
               cfg.m_value = v;
           }
               break;
           case 26: //YOColor3C,  //26
           {
               YOColor3C v;
               v.r = (uint8_t) node->UnsignedAttribute("r");
               v.g = (uint8_t) node->UnsignedAttribute("g");
               v.b = (uint8_t) node->UnsignedAttribute("b");
               cfg.m_value = v;
           }
               break;
           case 27: //YOColor4C,  //27
           {
               YOColor4C v;
               v.r = (uint8_t) node->UnsignedAttribute("r");
               v.g = (uint8_t) node->UnsignedAttribute("g");
               v.b = (uint8_t) node->UnsignedAttribute("b");
               v.a = (uint8_t) node->UnsignedAttribute("a");
               cfg.m_value = v;
           }
               break;

           case 28: //YOVector2List, //28
               break;

           case 29: //YOVector3List, //29
               break;

           case 30: //YOVector4List, //30
               break;

           case 31: //YOColor3FList, //31
               break;

           case 32: //YOColor4FList, //32
               break;

           case 33: //YOColor3CList, //33
               break;

           case 34: //YOColor4CList, //34
               break;

           case 35: //YOLimitF,  //35
           {
               YOLimitF v;
               v.value = node->FloatAttribute("value");
               v.min = node->FloatAttribute("min");
               v.max = node->FloatAttribute("max");
               v.speed = node->FloatAttribute("speed");
               cfg.m_value = v;
           }
               break;
           case 36: //YOLimitI32,  //36
           {
               YOLimitI32 v;
               v.value = node->IntAttribute("value");
               v.min = node->IntAttribute("min");
               v.max = node->IntAttribute("max");
               v.speed = node->IntAttribute("speed");
               cfg.m_value = v;
           }
               break;
           case 37: //YOLimitU32,  //37
           {
               YOLimitU32 v;
               v.value = node->UnsignedAttribute("value");
               v.min = node->UnsignedAttribute("min");
               v.max = node->UnsignedAttribute("max");
               v.speed = node->UnsignedAttribute("speed");
               cfg.m_value = v;
           }
               break;
        }
    }
    else
    {
        std::cout << tab << " UNKNOWN NODE " <<  node->Name() << std::endl;
    }
}

bool YOXML::writeXML(const std::string &file, YOVariant &input)
{
    tinyxml2::XMLDocument doc;
    std::cout << " ---------------------------------------------------------------- " << std::endl;
    //doc_.Clear();
    tinyxml2::XMLElement* root = doc.NewElement("xml");
    doc.InsertFirstChild(root);
    writeNode(input, root);
    //tinyxml2::XMLPrinter printer;
    //doc.Print(&printer);
    //std::cout << printer.CStr() << std::endl;
    doc.SaveFile(file.c_str());
    return true;
}

void YOXML::writeNode(YOVariant &node, tinyxml2::XMLElement* parent, const std::string &tab)
{
        //std::cout << tab << node.m_name << " !!! " << YOValue_type_name(node.m_value.index()) << " !!! " << parent << std::endl;

        tinyxml2::XMLElement *sub;

        if( YOValue_type_name(node.m_value.index()) == "YOMap" )
        {
            sub =  parent->InsertNewChildElement("map");

            for(auto &child: node.get<YOMap>())
            {
                writeNode(child.second, sub, tab + "  ");
            }
        }
        else if( YOValue_type_name(node.m_value.index()) == "YOArray" )
        {
            sub =  parent->InsertNewChildElement("array");
            for(auto &child: node.get<YOArray>())
            {
                writeNode(child, sub, tab + "  ");
            }
        }
        else
        {
            sub =  parent->InsertNewChildElement("element");

            switch (node.m_value.index())
            {
                case 2: //YOData,     //2
                    sub->SetAttribute("value", (const char *) (node.get<YOData>().data()));
                    break;
                case 3: //YODataF,    //3
                    break;
                case 4: //std::string,//4
                    sub->SetAttribute("value", node.get<std::string>().c_str());
                    break;
                case 5: //YOStringList, //5
                {
                    YOStringList &sl = node.get<YOStringList>();
                    sub->SetAttribute("select", sl.select);
                    for(auto &s : sl.items)
                    {
                        tinyxml2::XMLElement *subsub = sub->InsertNewChildElement("sub");
                        subsub->SetAttribute("value", s.c_str());
                    }
                }
                    break;
                case 6: //bool,       //6
                    sub->SetAttribute("value", node.get<bool>());
                    break;
                case 7: //float,      //7
                    sub->SetAttribute("value", node.get<float>());
                    break;
                case 8: //double,     //8
                    sub->SetAttribute("value", node.get<double>());
                    break;
                case 9: //int8_t,     //9
                    sub->SetAttribute("value", (int) node.get<int8_t>());
                    break;
                case 10: //uint8_t,    //10
                    sub->SetAttribute("value", node.get<uint8_t>());
                    break;
                case 11: //int16_t,    //11
                    sub->SetAttribute("value", (int32_t)node.get<int16_t>());
                    break;
                case 12: //uint16_t,   //12
                    sub->SetAttribute("value", (uint32_t)node.get<uint16_t>());
                    break;
                case 13: //int32_t,    //13
                    sub->SetAttribute("value", node.get<int32_t>());
                    break;
                case 14: //uint32_t,   //14
                    sub->SetAttribute("value", node.get<uint32_t>());
                    break;
                case 15: //int64_t,    //15
                    sub->SetAttribute("value", node.get<int64_t>());
                    break;
                case 16: //uint64_t,   //16
                    sub->SetAttribute("value", node.get<uint64_t>());
                    break;
                case 17: //YOIPv4,     //17
                {
                    YOIPv4 &v = node;
                    sub->SetAttribute("ip0", v.ip[0]);
                    sub->SetAttribute("ip1", v.ip[1]);
                    sub->SetAttribute("ip2", v.ip[2]);
                    sub->SetAttribute("ip3", v.ip[3]);
                    sub->SetAttribute("port", v.port);
                }
                    break;
                case 18: //YOIPv6,     //18
                {
                    YOIPv6 &v = node;
                    sub->SetAttribute("ip0", v.ip[0]);
                    sub->SetAttribute("ip1", v.ip[1]);
                    sub->SetAttribute("ip2", v.ip[2]);
                    sub->SetAttribute("ip3", v.ip[3]);
                    sub->SetAttribute("ip4", v.ip[4]);
                    sub->SetAttribute("ip5", v.ip[5]);
                    sub->SetAttribute("port", v.port);
                }
                    break;
                case 19: //YOVector2,  //19
                {
                    YOVector2 &v = node;
                    sub->SetAttribute("x", v.x);
                    sub->SetAttribute("y", v.y);
                }
                    break;
                case 20: //YOVector2I, //20
                {
                    YOVector2I &v = node;
                    sub->SetAttribute("x", v.x);
                    sub->SetAttribute("y", v.y);
                }
                    break;
                case 21: //YOVector2U, //21
                {
                    YOVector2U &v = node;
                    sub->SetAttribute("x", v.x);
                    sub->SetAttribute("y", v.y);
                }
                    break;
                case 22: //YOVector3,  //22
                {
                    YOVector3 &v = node;
                    sub->SetAttribute("x", v.x);
                    sub->SetAttribute("y", v.y);
                    sub->SetAttribute("z", v.z);
                }
                    break;
                case 23: //YOVector4,  //23
                {
                    YOVector4 &v = node;
                    sub->SetAttribute("x", v.x);
                    sub->SetAttribute("y", v.y);
                    sub->SetAttribute("z", v.z);
                    sub->SetAttribute("w", v.w);
                }
                    break;
                case 24: //YOColor3F,  //24
                {
                    YOColor3F &v = node;
                    sub->SetAttribute("r", v.r);
                    sub->SetAttribute("g", v.g);
                    sub->SetAttribute("b", v.b);
                }
                    break;
                case 25: //YOColor4F,  //25
                {
                    YOColor4F &v = node;
                    sub->SetAttribute("r", v.r);
                    sub->SetAttribute("g", v.g);
                    sub->SetAttribute("b", v.b);
                    sub->SetAttribute("a", v.a);
                }
                    break;
                case 26: //YOColor3C,  //26
                {
                    YOColor3C &v = node;
                    sub->SetAttribute("r", v.r);
                    sub->SetAttribute("g", v.g);
                    sub->SetAttribute("b", v.b);
                }
                    break;
                case 27: //YOColor4C,  //27
                {
                    YOColor4C &v = node;
                    sub->SetAttribute("r", v.r);
                    sub->SetAttribute("g", v.g);
                    sub->SetAttribute("b", v.b);
                    sub->SetAttribute("b", v.a);
                }
                    break;

                case 28: //YOVector2List, //28
                    break;

                case 29: //YOVector3List, //29
                    break;

                case 30: //YOVector4List, //30
                    break;

                case 31: //YOColor3FList, //31
                    break;

                case 32: //YOColor4FList, //32
                    break;

                case 33: //YOColor3CList, //33
                    break;

                case 34: //YOColor4CList, //34
                    break;

                case 35: //YOLimitF,  //35
                {
                    YOLimitF &v = node;
                    sub->SetAttribute("value", v.value);
                    sub->SetAttribute("min", v.min);
                    sub->SetAttribute("max", v.max);
                    sub->SetAttribute("speed", v.speed);
                }
                    break;
                case 36: //YOLimitI32,  //36
                {
                    YOLimitI32 &v = node;
                    sub->SetAttribute("value", v.value);
                    sub->SetAttribute("min", v.min);
                    sub->SetAttribute("max", v.max);
                    sub->SetAttribute("speed", v.speed);
                }
                    break;
                case 37: //YOLimitU32,  //37
                {
                    YOLimitU32 &v = node;
                    sub->SetAttribute("value", v.value);
                    sub->SetAttribute("min", v.min);
                    sub->SetAttribute("max", v.max);
                    sub->SetAttribute("speed", v.speed);
                }
                    break;
            };

        }
        sub->SetAttribute("name", node.m_name.c_str());
        sub->SetAttribute("type", YOValue_type_name(node.m_value.index()).c_str());

}
