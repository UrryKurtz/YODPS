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

struct YOSocketInfo;

class YONode {
    void* m_context;
    std::map<std::string, void*> m_user_data;
    void *m_poll;

    YOSocketInfo *m_sock_data;
    YOSocketInfo *m_sock_sys;
    //YOSocketInfo *m_sock_log;

    std::string m_name;
    YOVariant m_config;
    std::unordered_map<std::string, YOSubData> m_sub_map;
    std::unordered_map<std::string, YOPubData> m_pub_map;
    std::unordered_map<int, YOSigData> m_sig_map;
    YOSubSharedFn m_sys_fn;
    void *m_sys_param;

    std::shared_ptr<YOMessage> readMessage(void *sock_sub);
    void sendMessage(const std::string &topic, YOMessage &message, void *pub_sock);

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

	void sendMessage(const std::string &topic, YOMessage &message);
	void sendMessage(const std::string &topic, std::shared_ptr<YOMessage> message);

	void sendMessageSys(const std::string &topic, YOMessage &message);
	void subscribeSys(const std::string &topic);
	void unsubscribeSys(const std::string &topic);
	void subscribeSysFn(YOSubSharedFn fn, void *param);

	void subscribe(const std::string &topic, YOSubFn fn, void *data);
	void subscribe(const std::string &topic, YOSubSharedFn fn, void *data);
	void unsubscribe(const std::string &topic);
	void advertise(const std::string &topic, uint16_t type = 0, uint16_t subtype = 0);
	void unadvertise(const std::string &topic);

	void addPollFunction(YOPollFn, void *data);
	void addSignalFunction(int signal, YOSigFn fn, void *data);
	YOSigData *getSignalFunction(int signal);

	void setUserData(const std::string &name, void *data);
	void *getUserData(const std::string &name);

	void logInfo(const char* fmt, ...);
	void logWarning(const char* fmt, ...);
	void logError(const char* fmt, ...);

	static YOTimestamp getTimestamp();
};

#endif /* UTILS_YONODE_H_ */
