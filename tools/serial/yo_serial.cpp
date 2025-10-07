/*
 * yo_serial.cpp
 *
 *  Created on: Oct 6, 2025
 *      Author: kurtz
 */
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <getopt.h>

#include "config.h"
#include "YOTypes.h"
#include "YONode.h"
#include "YOKeys.h"

uint16_t counter_ = 0;
const char *name_ = "SERIAL";
const char *topic_in_ = "SERIAL_REQUEST";
const char *topic_out_ = "SERIAL_RESPONSE";
const char *device_ = "/dev/ttyUSB0";
int baud_ = B115200;
int fd_;
YONode *node_;

int open_serial(const char* device, int baud = B115200)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // Set baud rate
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    // 8 data bits, no parity, 1 stop bit
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // no flow control
    tty.c_lflag = 0;                         // no signaling chars, no echo
    tty.c_oflag = 0;                         // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                     // read returns after 1 byte
    tty.c_cc[VTIME] = 1;                     // timeout (0.1 s)

    tty.c_cflag |= (CLOCAL | CREAD);         // enable receiver
    tty.c_cflag &= ~(PARENB | PARODD);       // no parity
    tty.c_cflag &= ~CSTOPB;                  // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                 // no HW flow ctrl

    // Apply settings
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    return fd;
}

// Send string with \r at the end
bool send_cmd(int fd, const std::string& cmd)
{
    std::string s = cmd + "\r";
    ssize_t n = write(fd, s.c_str(), s.size());
    return (n == (ssize_t)s.size());
}

// Read response until '>' prompt or timeout
std::string read_resp(int fd)
{
    std::string buf;
    char c;
    while (true) {
        int n = read(fd, &c, 1);
        static char last = c;
        if (n <= 0 || c == '>')
        {
        	if(buf.size() && buf[buf.size() - 1] == '\n')
        		buf[buf.size() - 1] = '\0';
        	break;
        }
        if (c == '\n' && last == '\n')
        	continue;
        buf.push_back(c);
        last = c;
    }
    return buf;
}

std::string query(const std::string& cmd, int fd)
{
       send_cmd(fd, cmd);
       usleep(20000); // 20 ms delay between commands
       std::string resp = read_resp(fd);
       //std::cout << "REQUEST [" << cmd << "]" << std::endl;
	   //std::cout << "RESPONSE[" << resp << "]" << std::endl;
       return resp;
};

int fn_request(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
	std::shared_ptr<YOVariant> request = std::make_shared<YOVariant>(message->getDataSize(), (const char*) message->getData());
	for (auto &req : request->get<YOArray>())
	{
		//std::cout << topic << " " << req << std::endl;
		std::string cmd = req[yo::k::request].get<std::string>().c_str();
		req[yo::k::response] = query(cmd, fd_).c_str();
	}
	YOMessage msg(*request);
	node_->sendMessage(topic_out_, msg);
	return 0;
}

static struct option long_options[] = {
        {"name", optional_argument, NULL, 'n'},
        {"subscribe", optional_argument, NULL, 's'},
		{"advertise", optional_argument, NULL, 'a'},
        {"device", optional_argument, NULL, 'd'},
        {"baud", optional_argument, NULL, 'b'},
        {NULL, 0, NULL, 0}};

int main(int argc, char **argv)
{
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "n:s:a:d:b:", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'n': name_ = optarg; break;
			case 's': topic_in_ = optarg; break;
			case 'a': topic_out_ = optarg; break;
			case 'd': device_ = optarg; break;
			case 'b': baud_ = std::atoi(optarg); break;
		}
	}

	std::cout << " Node name: " <<  name_  << std::endl;
	std::cout << " Subscribe: " <<  topic_in_  << std::endl;
	std::cout << " Advertise: " <<  topic_out_  << std::endl;
	std::cout << " Device   : " <<  device_  << std::endl;
	std::cout << " Baud     : " <<  baud_  << std::endl;

	fd_ = open_serial(device_, baud_);
    if (fd_ < 0)
    	return 1;

    std::cout << "Connected to " << device_ << std::endl;
    node_ = new YONode(name_);
    node_->advertise(topic_out_ , 0x0, 0x0);
    node_->subscribe(topic_in_, fn_request, 0);
    node_->start();
    std::cout << "Closing " << device_ << std::endl;
    close(fd_);
}
