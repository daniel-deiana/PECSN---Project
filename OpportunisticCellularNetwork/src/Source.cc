#include "Source.h"
#include "Packet_m.h"


#define DEBUG ;
namespace opportunisticcellularnetwork {

    Define_Module(Source);

    /* +-------------------------------------------------------------------------------+
     * | Definition of Source's methods.                                              |
     * +-------------------------------------------------------------------------------+
     */

    // When a source gets initialized it starts by setting a timer
    void Source::initialize()
    {
    //  AUTHOR : DANIEL

        // schedule first packet to send
        double mean = 1 / ( par("rate").doubleValue() * 1000);
        simtime_t delay = exponential(mean, 0);
        scheduleAt(simTime() + delay, timerMessage);
    }


    void Source::handleMessage(cMessage *msg)
    {
    //  AUTHOR : DANIEL

        // When timer message arrives, a packet gets forwarded out
        Packet *packet = new Packet("packet");

        // generating packet size
        int size = intuniform(1,par("maxPacketSize"), 1);

        // testing purposes
        if (par("testing"))
            size = par("fixedPacketSize");

        // setting packet infos
        packet->setSize(size);
        packet->setTimestamp(simTime());
        packet->setIndex(par("id"));

        // debug
        #ifdef DEBUG
        EV << getName() << par("id").intValue()*2 << "::handleMessage() - Packet with size=" << packet->getSize() << " sent at " << packet->getTimestamp() << endl;
        #endif

        // sending out the message
        send(packet, "out");

       // setting exponential i.a.t. value
       double mean = 1 / ( par("rate").doubleValue() * 1000);
       EV << "MEAN INTER PACKET ARRIVAL = " << mean << "RATE = " <<  par("rate").doubleValue() << endl;
       simtime_t delay = exponential(mean, 0);

       //scheduling timer
       scheduleAt(simTime() + mean , msg);

    }
}

