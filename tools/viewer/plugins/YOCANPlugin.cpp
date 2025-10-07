/*
 * YOCANPlugin.cpp
 *
 *  Created on: Oct 7, 2025
 *      Author: kurtz
 */

#include "YOCANPlugin.h"
#include <libdbc/message.hpp>
#include <libdbc/dbc.hpp>

YOCANPlugin::YOCANPlugin(Context *context) : IPlugin(context)
{
	// TODO Auto-generated constructor stub

}

YOCANPlugin::~YOCANPlugin()
{
	// TODO Auto-generated destructor stub
}

void YOCANPlugin::OnStart()
{
	dbc_.m_name = "DBC";
	dbc_.m_value = YOMap();

	Libdbc::DbcParser parser;
	parser.parse_file("vw_mqb.dbc");

	std::string version = parser.get_version();
	std::cout << "version " << version << std::endl;
	std::vector<std::string> nodes = parser.get_nodes();
	std::cout << "nodes " << nodes.size() << std::endl;
	for( auto &node : nodes)
	{
		std::cout << "node " << node << std::endl;
		//dbc_["Nodes"][node] = node.c_str();
	}
	//std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
	std::vector<Libdbc::Message> msgs = parser.get_messages();
	for( auto &msg : msgs)
	{
		//std::cout << "message " << msg.id() << std::endl;
		std::string msg_id = std::to_string( msg.id());
		YOVariant &cur_msg = dbc_["MQB"][msg_id];
		std::vector<Libdbc::Signal> signals = msg.get_signals();
		for( auto &sig : signals)
		{
			cur_msg[sig.name] = 0u;
		}
	}
}

void YOCANPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{

}

void YOCANPlugin::OnGui()
{
		ui::Begin("CAN Viewer");
		gui_.drawCfg(dbc_);
		ui::End();
}
