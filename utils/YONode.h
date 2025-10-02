/*
 * YONode.h
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YONODE_H_
#define UTILS_YONODE_H_

#include "YOMessage.h"
#include "YOVariant.h"
#include <unordered_map>
#include <pthread.h>

#include <cstdarg>
#include <cstdio>

#define YOCFG_NAME "Name"
#define YOCFG_PROTO "Proto"

using YOSubFn = int(*)(const std::string &topic, YOMessage *message, void *data);
using YOSubSharedFn = int(*)(const std::string &topic, std::shared_ptr<YOMessage> message, void *data);

using YOPollFn = int(*)(void *data);

using YOSigFn = void(*)(int signal, void *data);
struct YOSigData
{
    int signal;
    YOSigFn fn;
    void *data;
};


struct YOSubData
{
    std::string topic;
    YOSubFn fn;
    YOSubSharedFn shared_fn;
    void *data;
};

struct YOPollData
{
    pthread_t thread;
    YOPollFn fn;
    void *data;
};

struct YOPubData
{
    uint16_t type;
    uint16_t subtype;
};

class YONode {
    void* m_context;
    std::map<std::string, void*> m_user_data;
    void *m_socket_pub;
    void *m_socket_sub;
    void *m_poll;

    std::string m_name;
    YOVariant m_config;
    std::unordered_map<std::string, YOSubData> m_sub_map;
    std::unordered_map<std::string, YOPubData> m_pub_map;
    std::unordered_map<int, YOSigData> m_sig_map;

public:
	YONode(const char *node_name = "");
	virtual ~YONode();

	void setConfig(YOVariant &config);
	YOVariant &getConfig();

	int connect();
	int disconnect();
    int shutdown();

	int receive();
	int start();
	int getMessage(int wait = 1000);
	int stop();
	bool isRunning();

	void sendMessage(const char *topic, YOMessage &message);
	void sendMessage(const char *topic, std::shared_ptr<YOMessage> message);

	void subscribe(const char *topic, YOSubFn fn, void *data);
	void subscribe(const char *topic, YOSubSharedFn fn, void *data);
	void unsubscribe(const char *topic);
	void advertise(const char *topic, uint16_t type = 0, uint16_t subtype = 0);

	void addPollFunction(YOPollFn, void *data);
	void addSignalFunction(int signal, YOSigFn fn, void *data);
	YOSigData *getSignalFunction(int signal);

	void setUserData(const char *name, void *data);
	void *getUserData(const char *name);

	void logInfo(const char* fmt, ...);
	void logWarning(const char* fmt, ...);
	void logError(const char* fmt, ...);

	static YOTimestamp getTimestamp();
};

#endif /* UTILS_YONODE_H_ */
