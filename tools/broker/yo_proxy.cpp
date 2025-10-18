/*
 * yo_proxy.cpp
 *
 *  Created on: Oct 18, 2025
 *      Author: kurtz
 */

// proxy.cpp
#include "YOTypes.h"
#include <zmq.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

int main()
{
    void* ctx = zmq_ctx_new();

    void* xsub = zmq_socket(ctx, ZMQ_XSUB);   // front
    void* xpub = zmq_socket(ctx, ZMQ_XPUB);   // back
    void* cap  = zmq_socket(ctx, ZMQ_PUB);    // capture

    zmq_bind(xsub, YO_PUB_DATA_SRV); // for PUBLISHERs
    zmq_bind(xpub, YO_SUB_DATA_SRV); // for SUBSCRIBERs

    zmq_bind(xsub, YO_PUB_SYS_SRV); // for SYS PUBLISHERs
    zmq_bind(xpub, YO_SUB_SYS_SRV); // for SYS SUBSCRIBERs

    zmq_bind(cap,  YO_REC_DATA_SRV); // capture

    int rc = zmq_proxy(xpub, xsub, cap);

    if (rc != 0)
    	std::perror("zmq_proxy");

    zmq_close(cap);
    zmq_close(xpub);
    zmq_close(xsub);
    zmq_ctx_term(ctx);
    return rc;
}


