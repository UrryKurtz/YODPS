/*
 * YOVariant.h
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOVARIANT_H_
#define UTILS_YOVARIANT_H_

#include "YOTypes.h"

#include <memory>
#include <string>
#include <variant>
#include <map>
#include <vector>
#include <array>

#include <iostream>

#define HEX(X) "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int32_t) X

std::vector<std::string> split_by_string(const std::string& str, const std::string& delim);

#define YO_DECLARE_VARIANT(Name, ...)                                                    \
    using Name = std::variant<__VA_ARGS__>;                                              \
    inline const std::vector<std::string>& Name##_type_names() {                         \
        static const std::vector<std::string> names = split_by_string(#__VA_ARGS__, ", "); \
        return names;                                                                    \
    }                                                                                    \
    inline const std::string& Name##_type_name(size_t index) {                           \
        static const std::string unknown = "unknown";                                    \
        const auto& names = Name##_type_names();                                         \
        return (index < names.size()) ? names[index] : unknown;                          \
    }                                                                                    \
    inline int Name##_index_of(const std::string& typeName) {                            \
       const auto& names = Name##_type_names();                                          \
       auto it = std::find(names.begin(), names.end(), typeName);                        \
       return (it != names.end()) ? static_cast<int>(std::distance(names.begin(), it)) : -1; \
   }

class YOVariant;

using YOVariantPtr = std::shared_ptr<YOVariant>;
using YOArray = std::vector<YOVariant>;
using YOMap = std::map<std::string, YOVariant>;

YO_DECLARE_VARIANT(YOValue,
        YOMap,      //0
        YOArray,    //1
        YOData,     //2
        YOFloatList, //3
        std::string,//4
        YOStringList, //5
        bool,       //6
        float,      //7
        double,     //8
        int8_t,     //9
        uint8_t,    //10
        int16_t,    //11
        uint16_t,   //12
        int32_t,    //13
        uint32_t,   //14
        int64_t,    //15
        uint64_t,   //16
        YOIPv4,     //17
        YOIPv6,     //18
        YOVector2,  //19
        YOVector2I, //20
        YOVector2U, //21
        YOVector3,  //22
        YOVector4,  //23
        YOColor3F,  //24
        YOColor4F,  //25
        YOColor3C,  //26
        YOColor4C,  //27
        YOVector2List, //28
        YOVector3List, //29
        YOVector4List, //30
        YOColor3FList, //31
        YOColor4FList, //32
        YOColor3CList, //33
        YOColor4CList, //34
        YOLimitF, //35
        YOLimitI32, //36
        YOLimitU32, //37
		YOVector3U //38

)

inline std::ostream& operator<<(std::ostream& os, const YOStringList& v)    {
	os << "YOStringList: " << v.items.size() << " {";
	for (auto &str : v.items) os << str << " ";  os << "} select:" << v.select ;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const YOMap & v);
inline std::ostream& operator<<(std::ostream& os, const YOArray &v);

inline std::ostream& operator<<(std::ostream& os, const YOData& v)    {  return os << "size: " << v.size() ; }
inline std::ostream& operator<<(std::ostream& os, const YOFloatList& v)    {  return os << "size: " << v.size() ; }

inline std::ostream& operator<<(std::ostream& os, const YOVector2& v) {  return os << "(" << v.x << ", " << v.y << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector2I& v) { return os << "(" << v.x << ", " << v.y << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector2U& v) { return os << "(" << v.x << ", " << v.y << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOVector3& v) {  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector3U& v) {  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector4& v) {  return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOColor3F& v) {  return os << "(" << v.r << ", " << v.g << ", " << v.b << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4F& v) {  return os << "(" << v.r << ", " << v.g << ", " << v.b << ", " << v.a << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3C& v) {  return os << "(" << HEX(v.r) << ", " << HEX(v.g) << ", " << HEX(v.b) << std::dec << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4C& v) {  return os << "(" << HEX(v.r) << ", " << HEX(v.g) << ", " << HEX(v.b) << ", " << HEX(v.a) << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOIPv4& v) {  return os << "IPv4(" << (uint32_t)v.ip[0] << "." << (uint32_t)v.ip[1] << "." << (uint32_t)v.ip[2] << "." << (uint32_t)v.ip[3] << " : " << v.port << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOVector2List& v) {  return os << "YOVector2List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector3List& v) {  return os << "YOVector3List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector4List& v) {  return os << "YOVector4List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3CList& v) {  return os << "YOColor3CList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4CList& v) {  return os << "YOColor4CList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3FList& v) {  return os << "YOColorFCList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4FList& v) {  return os << "YOColorFCList(" << v.size() << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOLimitF& v)   {  return os << "YOLimitF( value:" << v.value << " min:" << v.min << " max:"  << v.max << " speed:" << v.speed << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOLimitI32& v) {  return os << "YOLimitI32( value:" << v.value << " min:" << v.min << " max:"  << v.max << " speed:" << v.speed << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOLimitU32& v) {  return os << "YOLimitU32( value:" << v.value << " min:" << v.min << " max:"  << v.max << " speed:" << v.speed << ")"; }

std::ostream& operator<<(std::ostream& os, const YOValue& v);

constexpr float inv = 1.0f / 255.0f;

inline YOColor3F convert(const YOColor3C &in)
{
    return YOColor3F {inv * in.r, inv * in.g, inv * in.b};
}

inline YOColor4F convert(const YOColor4C &in)
{
    return YOColor4F {inv * in.r, inv * in.g, inv * in.b, inv * in.a};
}

inline YOColor3C convert(const YOColor3F &in)
{
    return YOColor3C {(uint8_t)(255 * in.r  + 0.5f), (uint8_t)(255 * in.g  + 0.5f), (uint8_t)(255 * in.b  + 0.5f)};
}

inline YOColor4C convert(const YOColor4F &in)
{
    return YOColor4C {(uint8_t)(255 * in.r  + 0.5f), (uint8_t)(255 * in.g  + 0.5f), (uint8_t)(255 * in.b  + 0.5f), (uint8_t)(255 * in.a  + 0.5f)};
}

class YOVariant
{
public:
    YOValue m_value;
    std::string m_name;
    MSGPACK_DEFINE(m_name, m_value);
    void pack(msgpack::sbuffer &buffer);
    bool unpack(const char *data, size_t size);

    YOVariant();
    YOVariant(const std::string&);
    YOVariant(const std::string&, YOValue val);
    YOVariant(const size_t &size, const char *data);

    virtual ~YOVariant();

    size_t getTypeId();
    const std::string &getTypeName();
    void push_back(const YOVariant& node);
    YOVariant& back();

    YOVariant& get(size_t idx);
    YOVariant& get(const std::string &key);
    std::string getKey(size_t idx);
    size_t getArraySize();
    void setArraySize(size_t size);
    size_t getMapSize();
    void print(const std::string &tab = "");
    bool hasChild(const std::string &key);

    void erase(size_t idx);
    void erase(const std::string &key);

    YOVariant& operator[](size_t idx) { return get(idx); }
    YOVariant& operator[](const std::string &key) { return get(key); }
    YOVariant& operator=(const YOValue &val) { m_value = val; return *this; }
    friend std::ostream& operator<<(std::ostream& os, const YOVariant& v)
    {

    	return os << v.m_name  << " " << v.m_value.index() << " " << v.m_value;

    }

    template<typename T>
    inline T& get() { return std::get < T > (m_value); }

    template<typename T>
    inline const T& get() const { return std::get < T > (m_value); }

    template<typename T>
    inline operator T&() { return std::get < T > (m_value); }

    template<typename T>
    inline operator const T&() const { return std::get < T > (m_value); }
};

#endif /* UTILS_YOVARIANT_H_ */
