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
    }


class YOVariant;

using YOVariantPtr = std::shared_ptr<YOVariant>;
using YOArray = std::vector<YOVariant>;
using YOMap = std::map<std::string, YOVariant>;

YO_DECLARE_VARIANT(YOValue,
        YOMap,
        YOArray,
        YOData,
        YODataF,
        std::string,
        bool,
        float,
        double,
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        YOIPv4,
        YOIPv6,
        YOVector2,
        YOVector2I,
        YOVector2U,
        YOVector3,
        YOVector4,
        YOColor3F,
        YOColor4F,
        YOColor3C,
        YOColor4C,
        YOVector2List,
        YOVector3List,
        YOVector4List,
        YOColor3FList,
        YOColor4FList,
        YOColor3CList,
        YOColor4CList)

inline std::ostream& operator<<(std::ostream& os, const YOData& v)    {  return os << "size: " << v.size() ; }
inline std::ostream& operator<<(std::ostream& os, const YODataF& v)    {  return os << "size: " << v.size() ; }

inline std::ostream& operator<<(std::ostream& os, const YOVector2& v) {  return os << "(" << v.x << ", " << v.y << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector2I& v) { return os << "(" << v.x << ", " << v.y << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector2U& v) { return os << "(" << v.x << ", " << v.y << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOVector3& v) {  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector4& v) {  return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOColor3F& v) {  return os << "(" << v.r << ", " << v.g << ", " << v.b << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4F& v) {  return os << "(" << v.r << ", " << v.g << ", " << v.b << ", " << v.a << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3C& v) {  return os << "(" << HEX(v.r) << ", " << HEX(v.g) << ", " << HEX(v.b) << std::dec << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4C& v) {  return os << "(" << HEX(v.r) << ", " << HEX(v.g) << ", " << HEX(v.b) << ", " << HEX(v.a) << ")"; }

inline std::ostream& operator<<(std::ostream& os, const YOVector2List& v) {  return os << "YOVector2List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector3List& v) {  return os << "YOVector3List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOVector4List& v) {  return os << "YOVector4List(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3CList& v) {  return os << "YOColor3CList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4CList& v) {  return os << "YOColor4CList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor3FList& v) {  return os << "YOColorFCList(" << v.size() << ")"; }
inline std::ostream& operator<<(std::ostream& os, const YOColor4FList& v) {  return os << "YOColorFCList(" << v.size() << ")"; }

std::ostream& operator<<(std::ostream& os, const YOValue& v);

class YOVariant
{
public:
    std::string m_name;
    YOValue m_value;
    MSGPACK_DEFINE(m_name, m_value);
    void pack(msgpack::sbuffer &buffer);
    bool unpack(const char *data, size_t size);

    YOVariant();
    YOVariant(const std::string&);
    YOVariant(const std::string&, YOValue val);
    YOVariant(const char *data, size_t size);
    virtual ~YOVariant();

    size_t getTypeId();
    YOVariant& get(size_t idx);
    YOVariant& get(const std::string &key);
    std::string getKey(size_t idx);
    size_t getArraySize();
    void setArraySize(size_t size);
    size_t getMapSize();
    void print(const std::string &tab = "");

    void erase(size_t idx);
    void erase(const std::string &key);

    YOVariant& operator[](size_t idx) { return get(idx); }
    YOVariant& operator[](const std::string &key) { return get(key); }
    YOVariant& operator=(const YOValue &val) { m_value = val; return *this; }

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
