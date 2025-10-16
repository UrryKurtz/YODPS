/*
 * yo_broker.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */
#include <assert.h>
#include <iostream>
#include "config.h"
#include "zmq.h"
#include "YOTypes.h"
#include "YONode.h"
#include <queue>
#include <mutex>
#include <pthread.h>

struct YOSample
{
    YOHeaderBase *base = nullptr;
    uint32_t topic_len;

    YOMessageData data;
    YOMessageData ext_data;
};

bool cmp_by_ts(const std::shared_ptr<YOSample> &a, const std::shared_ptr<YOSample> &b)
{
    return a->base->timestamp < b->base->timestamp; // sort asc ts
}

std::map<uint64_t, std::vector<std::shared_ptr<YOSample>>> data_map_;
std::vector<std::vector<std::shared_ptr<YOSample>>> tmp_data_;

std::map<std::string, int> sub_map_;
std::mutex mutex_;
uint64_t last_slot_;
uint64_t last_time_;

#define MCAP_IMPLEMENTATION
#define MCAP_COMPRESSION_NO_LZ4
#define MCAP_COMPRESSION_NO_ZSTD
#include <mcap/writer.hpp>
#include <fstream>


void* system(void *arg)
{
	YONode node("BROKER");
	node.start();
	return arg;
}

void* worker(void *arg)
{
    std::cout << " WORKER START " << sizeof(YOSample) << std::endl;
    mcap::McapWriter w;
    mcap::McapWriterOptions opts("");
    opts.compression = mcap::Compression::None;
    //opts.chunkSize = 1024;
    opts.noChunking = true;
    mcap::Status x = w.open("demo.mcap", opts);

    mcap::Schema schema;
    schema.name = "XRAW";
    schema.encoding = "none";
    schema.data = {};
    schema.id = 1;
    w.addSchema(schema);

    mcap::Channel ch;
    ch.id = 1;
    ch.topic = "TEST";
    ch.schemaId = 1;       // empty scheme
    ch.messageEncoding = "binary";
    w.addChannel(ch);

    while (1)
    {
        mutex_.lock();
        while (!data_map_.empty() && last_slot_ - data_map_.begin()->first > 3000000000)
        {
            auto begin_s = data_map_.begin();
            tmp_data_.push_back(begin_s->second);
            data_map_.erase(begin_s);
        }
        mutex_.unlock();
        std::cout << " LAST SLOT " << last_slot_ << std::endl;

        for (auto &vec : tmp_data_)
        {
            std::sort(vec.begin(), vec.end(), cmp_by_ts);

            for (auto &s : vec)
            {
                //std::cout << " TS : " << s->base->timestamp << " data size: " << s->data.size << " " << s->ext_data.size << std::endl;
                mcap::Message msg;
                msg.logTime = msg.publishTime = s->base->timestamp;
                msg.channelId = 1;
                msg.dataSize = s->data.size;
                msg.data = (std::byte*) s->data.buffer;

                mcap::Status x = w.write(msg);
                //std::cout << " TS : " << s->base->timestamp << " data size: " << s->data.size << " " << s->ext_data.size << " RES : " << (int) x.code << " " << x.message << std::endl;
                zmq_msg_close((zmq_msg_t*) &s->data.message);
                zmq_msg_close((zmq_msg_t*) &s->ext_data.message);
            }

        }
        tmp_data_.clear();
        std::cout << " QUEUE: " << data_map_.size() << " FILE SIZE: "  << w.statistics().messageCount << std::endl;

        usleep(500000);
    }
    w.close();
    return 0;
}


void my_free_fn (void *data_, void *hint_)
{
    std::cout << " my_free_fn " << std::endl;
}

void set_socket(void *sock)
{
	int size = 512 * 1024 * 1024; // 512MB
	int hwm = 100000; // 100k msgs
    zmq_setsockopt(sock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(sock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(sock, ZMQ_RCVBUF, &size, sizeof(size));
    zmq_setsockopt(sock, ZMQ_SNDBUF, &size, sizeof(size));
}

void send_back(void *sock_sub, void *sock_pub)
{
    zmq_msg_t sub;
    zmq_msg_init(&sub);
    if (zmq_msg_recv(&sub, sock_pub, 0) != -1)
    {
        std::string topic((char*) zmq_msg_data(&sub) + 1);
        std::cout << " SUB/UNSUB " << zmq_msg_size(&sub) << " : " << topic << std::endl;
        sub_map_[topic] = 0;
        zmq_msg_send(&sub, sock_sub, 0);
    }
    zmq_msg_close(&sub);
}

std::shared_ptr<YOSample> get_sample(void *sock_sub, void *sock_pub)
{
	zmq_msg_t data;
	zmq_msg_init(&data);
	int rcv = zmq_msg_recv(&data, sock_sub, 0);
	std::shared_ptr<YOSample> sample = 0;
	//std::cout << " DATA " << rcv << std::endl;
	if (rcv != -1)
	{
		sample = std::make_shared<YOSample>();
		zmq_msg_init((zmq_msg_t*)&sample->data.message);
		zmq_msg_copy((zmq_msg_t*) &sample->data.message, &data);
		int more;
		size_t more_size = sizeof(more);
		zmq_getsockopt(sock_sub, ZMQ_RCVMORE, &more, &more_size);
		zmq_msg_send(&data, sock_pub, more ? ZMQ_SNDMORE : 0);
		zmq_msg_close(&data);

		if (more)
		{
			zmq_msg_t ext_data;
			zmq_msg_init(&ext_data);
			zmq_msg_recv(&ext_data, sock_sub, 0);
			//std::cout << " EXT DATA : " << more << std::endl;
			zmq_msg_init((zmq_msg_t*) &sample->ext_data.message);
			zmq_msg_copy((zmq_msg_t*) &sample->ext_data.message, &ext_data);
			sample->ext_data.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &sample->ext_data.message);
			sample->ext_data.size = zmq_msg_size((zmq_msg_t*) &sample->ext_data.message);
			zmq_msg_send(&ext_data, sock_pub, 0);
			zmq_msg_close(&ext_data);
		}
		sample->data.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &sample->data.message);
		sample->topic_len = std::strlen((const char*) sample->data.buffer);
		sample->base = (YOHeaderBase*) (sample->data.buffer + sample->topic_len + 1);
		sample->data.size = zmq_msg_size((zmq_msg_t*) &sample->data);
	}
	return sample;
}

void start_proxy()
{
    void *context = zmq_ctx_new();
    void *xsub_socket_data = zmq_socket(context, ZMQ_XSUB);
    void *xpub_socket_data = zmq_socket(context, ZMQ_XPUB);
    set_socket(xsub_socket_data);
    set_socket(xpub_socket_data);
    int res_fd = zmq_bind(xsub_socket_data, YO_SUB_DATA_SRV);
    int res_bd = zmq_bind(xpub_socket_data, YO_PUB_DATA_SRV);

    void *xsub_socket_sys = zmq_socket(context, ZMQ_XSUB);
    void *xpub_socket_sys = zmq_socket(context, ZMQ_XPUB);
    set_socket(xsub_socket_sys);
    set_socket(xpub_socket_sys);
    int res_fs = zmq_bind(xsub_socket_sys, YO_SUB_SYS_SRV);
    int res_bs = zmq_bind(xpub_socket_sys, YO_PUB_SYS_SRV);

    zmq_pollitem_t items[] = {
    		{xsub_socket_sys, 0, ZMQ_POLLIN, 0},
			{xpub_socket_sys, 0, ZMQ_POLLIN, 0},
    		{xsub_socket_data, 0, ZMQ_POLLIN, 0},
			{xpub_socket_data, 0, ZMQ_POLLIN, 0}};

    uint64_t msg_num = 0;
    while (1)
    {
        int rc = zmq_poll(items, 4, 1000);
        if (rc == -1)
            break;

        if (items[0].revents & ZMQ_POLLIN) //got SYS msg, send a moved copy to the SYS backend
        {
        	get_sample(xsub_socket_sys, xpub_socket_sys);
        }
        if (items[1].revents & ZMQ_POLLIN) // send subscribe events from subscribers SYS
        {
        	send_back(xsub_socket_sys, xpub_socket_sys);
        }
        if (items[2].revents & ZMQ_POLLIN) //got DATA msg, send a moved copy to the DATA backend and another copy to DATA recorder
        {
        	std::shared_ptr<YOSample> sample = get_sample(xsub_socket_data, xpub_socket_data);
        	if(sample)
        	{
                uint64_t slot = (uint64_t) sample->base->timestamp & ~0xFFFFFULL; //cut 1.048.575 from ns ts
                if (slot > last_slot_)
                    last_slot_ = slot;
                mutex_.lock();
                data_map_[slot].push_back(sample);
                mutex_.unlock();
        	}
        }
        if (items[3].revents & ZMQ_POLLIN) // send subscribe events from subscribers DATA
        {
        	send_back(xsub_socket_data, xpub_socket_data);
        }
    }
}

int main(int argc, char **argv)
{
    std::cout << "Starting Broker Version " << YO_BROKER_VERSION_MAJOR << "." << YO_BROKER_VERSION_MINOR << std::endl;
    std::cout << " SUB: " << YO_SUB_DATA_SRV << "   PUB: " << YO_PUB_DATA_SRV << std::endl;

    pthread_t tid;
    int thread_id = 1;

    if (pthread_create(&tid, nullptr, worker, &thread_id) != 0)
    {
        std::cerr << "Thread create error\n";
        return 1;
    }

    start_proxy();

    if (pthread_join(tid, nullptr) != 0)
    {
        std::cerr << "Thread join error\n";
        return 1;
    }
    /*
     void *context = zmq_ctx_new();

     // Create frontend and backend sockets
     void *frontend_sub = zmq_socket(context, ZMQ_XSUB);
     //assert (frontend_sub);
     void *backend_pub = zmq_socket(context, ZMQ_XPUB);
     //assert (backend_pub);

     // Bind both sockets to TCP ports
     int res_f = zmq_bind(frontend_sub, YO_SUB_SRV);
     //assert(res_f == 0);
     int res_b = zmq_bind(backend_pub, YO_PUB_SRV);
     //assert(res_b == 0);

     // Start the queue proxy, which runs until ETERM
     zmq_proxy(frontend_sub, backend_pub, NULL);

     */
    return 0;

}

