/*
 * YOVariant.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */
#include "YOVariant.h"

std::vector<std::string> split_by_string(const std::string &str, const std::string &delim)
{
    std::vector<std::string> out;
    size_t start = 0;
    size_t end;
    while ((end = str.find(delim, start)) != std::string::npos)
    {
        out.push_back(str.substr(start, end - start));
        start = end + delim.length();
    }
    out.push_back(str.substr(start));
    return out;
}

YOVariant::YOVariant() : m_name(""), m_value(YOMap())
{
}

YOVariant::YOVariant(const std::string &name, YOValue val) : m_name(std::move(name)), m_value(std::move(val))
{
}

YOVariant::YOVariant(const std::string &name) : m_name(std::move(name)), m_value(YOMap())
{
}

YOVariant::YOVariant(const size_t &size, const char *data)
{
    unpack(data, size);
}

YOVariant::~YOVariant()
{
}

size_t YOVariant::getTypeId()
{
    return m_value.index();
}

const std::string &YOVariant::getTypeName()
{
	return YOValue_type_name(m_value.index());
}


YOVariant& YOVariant::get(size_t idx)
{
    return std::get<YOArray>(m_value).at(idx);
}

std::string YOVariant::getKey(size_t idx)
{
    auto it = std::get<YOMap>(m_value).begin();
    std::advance(it, idx);
    return it->first;
}

void YOVariant::push_back(const YOVariant &node)
{
    if (m_value.index() != 1)
        m_value = YOArray();

    std::get<YOArray>(m_value).push_back(node);
}

YOVariant& YOVariant::back()
{
    return std::get<YOArray>(m_value).back();
}

YOVariant& YOVariant::get(const std::string &key)
{
    std::get<YOMap>(m_value).try_emplace(key, key, YOMap());
    return std::get<YOMap>(m_value)[key];
}

size_t YOVariant::getArraySize()
{
    return std::get<YOArray>(m_value).size();
}

void YOVariant::setArraySize(size_t size)
{
    m_value = YOArray(size);
}

size_t YOVariant::getMapSize()
{
    return std::get<YOMap>(m_value).size();
}

void YOVariant::pack(msgpack::sbuffer &buffer)
{
    msgpack::pack(buffer, *this);
}

bool YOVariant::unpack(const char *data, size_t size)
{
    try
    {
        msgpack::object_handle oh = msgpack::unpack(data, size);
        oh.get().convert(*this);
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool YOVariant::hasChild(const std::string &key)
{
    if(m_value.index() == 0 && get<YOMap>().count(key))
        return true;
    else
        return false;
}

void YOVariant::print(const std::string &tab)
{
    std::cout << tab << "(" << m_name << ") ";
    switch (size_t xtype = m_value.index())
    {
        case 0: //map
            std::cout << "[" << YOValue_type_name(m_value.index()) << "]" << std::endl;
            for (int i = 0; i < getMapSize(); i++)
            {
                std::get<YOMap>(m_value)[getKey(i)].print(tab + "  ");
            }
            break;
        case 1: //array
            std::cout << "[" << YOValue_type_name(m_value.index()) << "]" << std::endl;
            for (int i = 0; i < getArraySize(); i++)
            {
                std::get<YOArray>(m_value)[i].print(tab + "  ");
            }
            break;
        default:
            std::cout << m_value;
    };
}

void YOVariant::erase(size_t idx)
{
    std::get<YOArray>(m_value).erase(std::get<YOArray>(m_value).begin() + idx);
}

void YOVariant::erase(const std::string &key)
{
    std::get<YOMap>(m_value).erase(key);
}

std::ostream& operator<<(std::ostream &os, const YOValue &v)
{
    std::visit([&v, &os](auto &value)
    {   os << "[" << YOValue_type_name(v.index()) << "] " << value << "\n";}, v);
    return os;
}


inline std::ostream& operator<<(std::ostream& os, const YOMap& v)
{
	os << "YOMap size: " << v.size() << std::endl;
	for(const auto &m : v)
	{
		os << m.first << " : " << m.second;
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const YOArray& v)
{
	os << "YOArray size: " << v.size() << std::endl;
	int i = 0;
	for(const auto &m : v)
	{
		os << "[" << i++ << "] : " << m << std::endl;
	}
	return os;
}

