#include <YOTypes.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <tins/tins.h>
#include "YONode.h"
#include <getopt.h>

std::string g_name = "PCAP_NODE";
std::string g_topic;
std::string g_filter;
std::string g_interface = "lo";

std::shared_ptr<YONode> g_node;
std::shared_ptr<Tins::Sniffer> sniffer_;

Tins::IPv4Reassembler reassembler_;
Tins::IPv4Reassembler::PacketStatus status_;

void sig_fn(int signal, void *data)
{
    std::cout << signal << " sig_fn " << std::endl;
    exit(0);
}

void run_tins()
{
    while (g_node->isRunning())
    {
        Tins::Packet pkt = sniffer_->next_packet();
        if (!pkt)
        {
            continue; // timeout, check running_ again
        }

        //status_ = reassembler_.process(pdu);
        const Tins::IP &ip = pkt.pdu()->rfind_pdu<Tins::IP>();

        switch (ip.protocol())
        {
            case 6:
                {
                //const Tins::TCP &tcp = pdu.rfind_pdu<Tins::TCP>();
                //std::cout << " TCP PROTO: " << (uint32_t) ip.protocol() << " size: " << tcp.size() << " " <<  ip.src_addr() << ':' << tcp.sport() << " -> " << ip.dst_addr() << ':' << tcp.dport() << std::endl;
            }
                break;

            case 17:
                {
                const Tins::RawPDU &raw = ip.rfind_pdu<Tins::RawPDU>();
                YOMessage msg;
                msg.setData((uint8_t*) raw.payload().data(), raw.payload().size());
                g_node->sendMessage(g_topic.c_str(), msg);
            }
                break;

            default:
                {
                //std::cout << " IP PROTO: " << (uint32_t) ip.protocol() << " size: " << ip.size() << " " <<  ip.src_addr() << " -> " << ip.dst_addr() << std::endl;
            }
        }
    }
}

bool callback(Tins::PDU &pdu)
{
    const Tins::IP &ip = pdu.rfind_pdu<Tins::IP>();
    status_ = reassembler_.process(pdu);
    // std::cout  << ip.flags() << " status " << status_  << " Type: " <<  pdu.pdu_type() << std::endl;

    switch (ip.protocol())
    {
        case 6:
        {
            //const Tins::TCP &tcp = pdu.rfind_pdu<Tins::TCP>();
            //std::cout << " TCP PROTO: " << (uint32_t) ip.protocol() << " size: " << tcp.size() << " " <<  ip.src_addr() << ':' << tcp.sport() << " -> " << ip.dst_addr() << ':' << tcp.dport() << std::endl;
        }
        break;

        case 17:
        {
            //const Tins::UDP &udp = pdu.rfind_pdu<Tins::UDP>();
            const Tins::RawPDU &raw = pdu.rfind_pdu<Tins::RawPDU>();
            YOMessage msg;
            msg.setData((uint8_t*) raw.payload().data(), raw.payload().size());
            g_node->sendMessage(g_topic.c_str(), msg);
        }
        break;

        default:
        {
            //std::cout << " IP PROTO: " << (uint32_t) ip.protocol() << " size: " << ip.size() << " " <<  ip.src_addr() << " -> " << ip.dst_addr() << std::endl;
        }
    }
    // Find the TCP layer
    //const Tins::TCP &tcp = pdu.rfind_pdu<Tins::TCP>();
    //std::cout << " PROTO: " << (uint32_t) ip.protocol() << " " <<  ip.src_addr() << ':' << tcp.sport() << " -> " << ip.dst_addr() << ':' << tcp.dport() << std::endl;
    return g_node->isRunning();
}

static struct option long_options[] = {
        {"name", optional_argument, NULL, 'n'},
        {"topic", required_argument, NULL, 't'},
        {"interface", required_argument, NULL, 'i'},
        {"filter", required_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}};

int main(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:t:i:f:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'n':
                g_name = optarg;
                break;
            case 't':
                g_topic = optarg;
                break;
            case 'i':
                g_interface = optarg;
                break;
            case 'f':
                g_filter = optarg;
                break;
        }
    }

    std::cout << "g_name = " << g_name << std::endl;
    std::cout << "g_topic = " << g_topic << std::endl;
    std::cout << "g_interface = " << g_interface << std::endl;
    std::cout << "g_filter = " << g_filter << std::endl;

    reassembler_.clear_streams();
    Tins::SnifferConfiguration scfg;
    scfg.set_buffer_size(128 * 1024 * 1024);
    scfg.set_snap_len(2048);
    scfg.set_promisc_mode(true);
    scfg.set_immediate_mode(true);
    scfg.set_timeout(10);
    scfg.set_filter(g_filter);

    sniffer_ = std::make_shared<Tins::Sniffer>(g_interface, scfg);
    sniffer_->stop_sniff();

    g_node = std::make_shared<YONode>(g_name.c_str());
    g_node->addSignalFunction(SIGINT, sig_fn, 0);
    g_node->advertise(g_topic.c_str());
    g_node->connect();

    run_tins();
    //sniffer_->sniff_loop(callback);

    g_node->disconnect();
    g_node->shutdown();
    return 0;
}
