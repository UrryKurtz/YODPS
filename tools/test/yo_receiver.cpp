/*
 * yo_broker.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <iostream>
#include "config.h"
#include "YONode.h"


uint32_t errors = 0;

int processData(const std::string &topic, YOMessage *message, void *data)
{
    std::cout << " processData received " << topic << std::endl;
    return 0;
}
#include <fstream>


int fnum = 0;

int processSharedTest(const std::string &topic, std::shared_ptr<YOMessage>message, void *data)
{
    tCANData *can = (tCANData *)message->getData();
    uint64_t* counter = (uint64_t*) can->sData.aui8Data;
    static uint64_t counter_old = *counter;

    if(*counter - counter_old > 1)
    {
        std::cout << " !!!!! EEEROR " << *counter - counter_old << std::endl;
        errors++;
    }

    if(message->getExtDataSize())
    {
        std::cout << " ExtDataSize " << message->getExtDataSize() << std::endl;

        std::ostringstream oss;
        oss << "OUT//file_" << std::setw(5) << std::setfill('0') << fnum++ << ".jpeg";

       std::ofstream file(oss.str(), std::ios::binary);
       file.write(reinterpret_cast<const char*>(message->getExtData()), message->getExtDataSize());
    }

    std::cout << message->getTimestamp() << " " << topic << " Errors: " << errors << " num: " <<  *counter << std::endl;
    counter_old = *counter;
    return 0;
}

int processSharedData(const std::string &topic, std::shared_ptr<YOMessage>message, void *data)
{
    std::cout << " processSharedData received " << topic << " size: " << message->getDataSize() << " ext size: " << message->getExtDataSize() << std::endl;
    return 0;
}

int main(int argc, char **argv) {
	std::cout << "Starting Test Version " << YO_TEST_VERSION_MAJOR << "." << YO_TEST_VERSION_MINOR << std::endl;
    std::cout << " SUB: " << YO_SUB_DATA_SRV  << "   PUB: " <<  YO_PUB_DATA_SRV << std::endl;

    char *sub = argv[1];
    std::cout << " SUBSCRIBE: " << sub << std::endl;

    YONode node("receiver");
    //node.subscribe("TEST", processSharedTest, 0);
    node.subscribe(sub, processSharedTest, 0);
    node.start();
    std::cout << " EXIT " << std::endl;
    return 0;
}



