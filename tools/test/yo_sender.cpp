/*
 * yo_broker.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <iostream>
#include "config.h"
#include "YOTypes.h"
#include "YONode.h"

int main(int argc, char **argv)
{

    char *sub = argv[1];
    std::cout << " SUBSCRIBE" <<  sub << std::endl;

    YOVariant v("Color", YOVector3List());

    YOVector3List &m = std::get<YOVector3List>(v.m_value);
    std::cout << " size " << m.size() << std::endl;
    m.resize(22);
    std::cout << " size " << m.size() << std::endl;

    YOVector3List &mx = v.get<YOVector3List>();
    std::cout << " size " << mx.size() << std::endl;

    YOVariant test("Color");
    test["TEST0"] = YOVariant("TEST0", YOVector3 {1, 2, 3});
    test["TEST1"] = YOVector3 {1, 2, 3};

    std::cout << " TEST A " << std::endl;
    YOVariant &xx = test["TEST1"];
    std::cout << " TEST A " << xx.getTypeId() << std::endl;

    test["TEST2"]["Color"] = YOColor3C {1, 2, 3};
    std::cout << " TEST A " << std::endl;

    YOColor3C &x = test["TEST2"]["Color"];

    test["TEST_ARR"].setArraySize(5);

    for (int i = 0; i < test["TEST_ARR"].getArraySize(); i++)
    {
        test["TEST_ARR"][i]["NAME"] = "TEST";
        test["TEST_ARR"][i]["ID"] = 42u;
        test["TEST_ARR"][i]["POS"] = YOVector3 {11.0, 21.0, 43.0};
        std::cout << i << "   ARR " << test["TEST_ARR"][i].getTypeId() << std::endl;
    }

    std::cout << " " << x.r << " " << x.g << " " << x.b << std::endl;

    for (int i = 0; i < test.getMapSize(); i++)
    {
        std::cout << i << " key " << test.getKey(i) << std::endl;
    }

    msgpack::sbuffer *buf = new msgpack::sbuffer();
    test.pack(*buf);

    std::cout << " pack " << buf->size() << std::endl;

    YOVariant out(buf->data(), buf->size());

    std::cout << " unpack " << out.m_name << std::endl << std::endl << std::endl;

    out.print();

    std::cout << " MSG START " << std::endl;

    YONode node("TEST NODE");
    node.advertise(sub, 0xAA, 0xBB);
    node.connect();

    tCANData can;
    //can.sData.aui8Data[0] = 0xA9;
    //can.sData.aui8Data[7] = 0x9A;

    uint64_t *data = (uint64_t *)can.sData.aui8Data;

    while(node.isRunning())
    {
        (*data)++;
        YOMessage msgA(can);
        printf("0x%08X DATA %llX\n", msgA.getTimestamp(), *data);
        node.sendMessage(sub, msgA);
        //usleep(100);
    }
}

