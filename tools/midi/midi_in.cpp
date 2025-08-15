#include <iostream>
#include <pthread.h>
#include "YONode.h"
#include <rtmidi/RtMidi.h>
#include <getopt.h>

YONode *g_node;
std::string g_name;
std::string g_message;
std::vector<uint8_t> g_data;

RtMidiIn *g_midiin = 0;
RtMidiOut *g_midiout = 0;

enum YOMidiType
{
    YONoteOff = 0x80, //release velocity is ignored
    YONoteOn = 0x90, //velocity 0 = note off
    YOKeyPressure = 0xA0,
    YOController = 0xB0,
    YOProgramChange = 0xC0,
    YOChannelPressure = 0xD0,
    YOPitchWheel = 0xE0, // ll mm l = lsb, m = msb
    YOSysEx = 0xF0,

};

std::string notes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

std::string getNote(unsigned char note)
{
    int octave = note / 12 - 1;
    return notes[note % 12] + std::to_string(octave);
}

void mycallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    YOVariant out("MIDI Receiver");
    out["Objects"].setArraySize(1);
    //out["Objects"][0]["Type"] = (int) YO_MIDI;
    //out["Position"] = YOUtils::YOVector3(0,0,0);
    //out["Rotation"] = YOUtils::YOVector3(0,0,0);

    unsigned int nBytes = message->size();

    int channel = (message->at(0) & 0x0F);

    switch ((message->at(0) & 0xF0))
    {
        case YONoteOn:
            std::cout << "NOTE ON " << getNote(message->at(1));
            out["Objects"][0]["NoteOn"] = (int) message->at(1);
            break;
        case YONoteOff:
            std::cout << "NOTE OFF " << getNote(message->at(1));
            out["Objects"][0]["NoteOff"] = (int) message->at(1);
            break;

        case YOController:
            std::cout << "YOController " << (int) message->at(1) << " ";
            out["Objects"][0]["CC"] = (int) message->at(1);
            out["Objects"][0]["Value"] = (int) message->at(2);
            break;

        case YOKeyPressure:
            std::cout << "PRESSURE ";
            out["Objects"][0]["Pressure"] = (int) message->at(1);
            break;
        case YOProgramChange:
        {
            out["Objects"][0]["ProgramChange"] = (int) message->at(1);
            std::cout << " PROG CHANGE ";
            unsigned char msg[] = { 0xF0, 0x18, 0x04, 0x00, 0x04, 0xF7 };
            g_midiout->sendMessage(msg, 6);
        }
            break;
        case YOPitchWheel:
            std::cout << " PITCH WHEEL ";
            out["Objects"][0]["PitchWheel"] = (int) message->at(2) * 127 + message->at(1);
            break;

        case YOSysEx:
            std::cout << " SYSEX ";
            for (int i = 0; i < nBytes && i < 16; i++)
                printf(" 0x%02X", message->at(i));

            std::cout << " " << std::endl;
            return;
            break;

        default:
            std::cout << " DUNNO ";

    }

    ///for (unsigned int i = 0; i < nBytes; i++)
    {
        //std::cout << " Byte " << i << " = " << (int) message->at(i) << ", ";
    }

    //if (nBytes > 0)
        //std::cout << " stamp = " << deltatime << std::endl;

    //Serio::ByteArray str = Serio::serialize(out);

    YOMessage msg(out);
    g_node->sendMessage("MIDI_IN", msg);
}

void openMIDI(const char *iname, const char *oname)
{
    g_midiin = new RtMidiIn(RtMidi::UNIX_JACK, iname);
    std::cout << " INPUT " << iname << " " << g_midiin->getPortCount() << std::endl;
    g_midiin->openVirtualPort(iname);
    g_midiin->setBufferSize(165536, 165536);

    g_midiout = new RtMidiOut(RtMidi::UNIX_JACK, oname);
    std::cout << " OUTPUT " << oname << " " << g_midiout->getPortCount() << std::endl;
    g_midiout->openVirtualPort(oname);
}

void receiveMidi()
{
    YOVariant midi("Midi Receiver");
    int status;
    const char *portname = "MidiReceiver";
    //Serio::ByteArray str = Serio::serialize(midi);
    //g_node->Transmit("MIDI_IN", str.data(), str.size());
}

void parse_midi(const std::string &line)
{
    std::string str = line + " ";

    int pos=-1;
    while( (pos = str.find(" ")) >= 0)
    {
        g_data.push_back(strtol(str.substr(0, pos).c_str(), NULL, 16));
        //std::cout  << " find " << pos << " [" << str.substr( pos + 1, str.length()) << "]  ["  <<str.substr(0, pos) << "] " << std::endl;
        str = str.substr( pos + 1, str.length());
    }
}

int fn_get_midi(const std::string &topic, std::shared_ptr<YOMessage>message, void *data)
{
    std::cout << " GOT YO MIDI MESSAGE " << (const char *) message->getData() << std::endl;

    g_data.clear();
    parse_midi((const char *)message->getData());
    g_midiout->sendMessage(g_data.data(), g_data.size());

    return 0;
}

void processMIDI(void *ev)
{

}

int main(int argc, char **argv)
{

    static struct option long_options[] = {
    { "name", optional_argument, NULL, 'n' },
    { "message", optional_argument, NULL, 'm' },
    { NULL, 0, NULL, 0 } };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:m:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'n':
                g_name = optarg;
                break;
            case 'm':
                g_message = optarg;
                parse_midi(g_message);
                break;
        }
    }

    g_node = new YONode("MIDI_SUB");
    g_node->advertise("MIDI_IN");

    openMIDI("YO_MIDI_IN", "YO_MIDI_OUT");

    g_midiin->setCallback(&mycallback, 0);
    g_midiin->ignoreTypes(false, false, false);

    g_node->subscribe("MIDI_OUT", fn_get_midi, 0);
    g_node->start();
    return 0;
}
