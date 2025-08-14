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

#include <a_util/strings.h>

#include <adtf_file/standard_adtf_file_reader.h>

#include <adtf_file/file_extensions.h>
#include <adtf_file/adtf_file_writer.h>
#include <adtf_file/standard_factories.h>
#include <adtf_file/default_sample.h>

#define ADTF2_STREAM_META_TYPE "adtf2/legacy"
#define ADTF2_MEDIA_TYPE_CAN            0x0200
#define ADTF2_MEDIA_SUBTYPE_CAN_DATA    0x0003

int main(int argc, char **argv) {
    std::cout << "Starting Reader. v" << YO_READER_VERSION_MAJOR << "." << YO_READER_VERSION_MINOR << std::endl;

    a_util::filesystem::Path filename = "test.dat";
    std::shared_ptr<adtf_file::ADTFDatFileWriter> writer(new adtf_file::ADTFDatFileWriter(filename, std::chrono::microseconds(0), adtf_file::adtf2::StandardTypeSerializers(), adtf_file::ADTFDatFileWriter::adtf2));

    //define CAN stream type for ADTF 2.x.
    adtf_file::DefaultStreamType can_stream_type(ADTF2_STREAM_META_TYPE);
    can_stream_type.setProperty("major", "tUInt32", a_util::strings::format("%ul", ADTF2_MEDIA_TYPE_CAN));
    can_stream_type.setProperty("sub", "tUInt32", a_util::strings::format("%ul", ADTF2_MEDIA_SUBTYPE_CAN_DATA));

    //create stream for can messages
    //by default the copy serializer is adequate enough
    size_t stream_id = writer->createStream("rawcan", can_stream_type, std::make_shared<adtf_file::adtf2::AdtfCoreMediaSampleSerializer>());


    //std::shared_ptr<OutputStream> ostream = getExtensionStream(const std::string& name, uint32_t user_id, uint32_t type_id, uint32_t file_version_id);

    std::shared_ptr<adtf_file::OutputStream> ostream = writer->getExtensionStream("testfile.bin", 0, 0, 0);
    uint8_t data[255];

    for(uint8_t i = 0 ; i < 255; i++)
    {
        data[i] = i;
    }

    ostream->write(data, 255);

    return 0;
}

