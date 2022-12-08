#include "Antenna.h"

#define DEBUG ;

namespace opportunisticcellularnetwork {

Define_Module(Antenna);

/* +-------------------------------------------------------------------------------+
 * | Definition of Antenna's methods.                                              |
 * +-------------------------------------------------------------------------------+
 */

void Antenna::initialize()
{
/* +-------------------------------------------------------------------------------+
 * | This method handles the Antenna's setup at the beginning of the simulation.   |
 * +-------------------------------------------------------------------------------+
 */

    // Retrieving the number of cellulars within the network (omnetpp.ini)
    population = par("population").intValue();

    // Retrieving timeslot duration (omnetpp.ini)
    timeslot = (simtime_t)par("timeslot").doubleValue();

    // Registering signals
    throughputSignal = registerSignal("throughputSignal");
    responseTimeSignal = registerSignal("responseTimeSignal");

    lostPackets = sentPackets = 0;

    switch(par("stage").intValue()) {
        case 1:
        {
            // Scenario 1: queues of fixed dimension.
            #ifdef DEBUG
            EV << "Antenna::initialize() - Scenario 1: queues of fixed dimension, population: " << population << endl;
            #endif

            userQueueDimension = par("queueDimension").intValue();

            for(int i = 0; i < population; i++)
                userQueues.push_back(new UserQueue(i, userQueueDimension));

            scheduleAt(simTime(), new cMessage("TIMER"));
        } break;
        default:
        {
            // Scenario 0: queues of infinite dimension.
            #ifdef DEBUG
            EV << "Antenna::initialize() - Scenario 0: queues of infinite dimension, population: " << population << endl;
            #endif

            for(int i = 0; i < population; i++)
                userQueues.push_back(new UserQueue(i));

            scheduleAt(simTime(), new cMessage("TIMER"));
        } break;
    }

    #ifdef DEBUG
    EV << "Antenna::initialize() - END" << endl;
    #endif
}

void Antenna::handleMessage(cMessage *msg)
{
    // A TIMESLOT expires
    if(msg->isSelfMessage())
    {
        handleSelfMessage(msg);
    }
    // A new CQI arrives
    else if(msg->arrivedOn("inCellular"))
    {
        handleCQI(msg);
    }
    // A new PACKET arrives
    else
    {
        handlePacket(msg);
    }
}

void Antenna::finish()
{
// +-----------------------------------------------------------------------------+
//  Packet loss (%): % of packets that cannot be queued
    if(par("stage").intValue() == 1)
        recordScalar("PacketLoss%", (lostPackets/sentPackets)*100);
// +-----------------------------------------------------------------------------+
}

void Antenna::handlePacket(cMessage *msg) {

/* +----------------------------------------------------------------------------------+
 * | AUTHOR : DANIEL                                                                  |
 * +----------------------------------------------------------------------------------+
 * | This metod is used to handle the packets incoming from the sources. Whenever a   |
 * | new packet arrives we check the gate number and the packet is stored in the queue|
 * | associated with the gate the packet came from.                                   |
 * +----------------------------------------------------------------------------------+
 */
    Packet *packet = check_and_cast<Packet*>(msg);

    int size = packet->getSize();
    int gateIndex = packet->getIndex();
    UserQueue *queue = getQueueById(gateIndex);

    // If we are in stage 1 (finite queues), we record a new lost packet and finish
    if (par("stage").intValue() && queue->getFreeSlots() == 0)
    {
        EV << "Antenna::handlePacket() - QUEUE (id=" << gateIndex << ") IS FULL : PACKET LOSS, SIZE=" << size << endl;
        lostPackets++;
    }
    else
    {
        #ifdef DEBUG
        EV << "Antenna::handlePacket() - New packet incoming from source (id="<< gateIndex << "), SIZE=" << size << endl;
        #endif

        // push packet into user queue
        queue->addPacket(size);

        #ifdef DEBUG
        queue->showQueue();
        #endif
    }

    // Since the message is no more useful, it will be 'deleted' to avoid any memory leak.
    delete(msg);
}

void Antenna::handleSelfMessage(cMessage *msg)
{
/* +--------------------------------------------------------------------------------+
 * | AUTHOR : FEDERICO                                                              |
 * +--------------------------------------------------------------------------------+
 * | This metod is used to handle the CQI request process. For each user registered |
 * | to the network, the Antenna requests a CQI at the beginning of each timeslot.  |
 * +--------------------------------------------------------------------------------+
 */
    #ifdef DEBUG
    EV << "Antenna::handleMessage() - NEW TIMESLOT" << endl;
    #endif

    for(int i = 0; i < population; i++)
        send(new cMessage("CQI"), "out", i);

    simtime_t delay = simTime() + timeslot;
    scheduleAt(delay, new cMessage("TIMER"));

    #ifdef DEBUG
    EV << "Antenna::handleMessage() - scheduleAt(" << delay << ", beep)" << endl;
    #endif

    // Since the message is no more useful, it will be 'deleted' to avoid any memory leak.
    delete(msg);
}

void Antenna::handleCQI(cMessage* msg)
{
/* +----------------------------------------------------------------------------------+
 * | AUTHOR : FEDERICO                                                                |
 * +----------------------------------------------------------------------------------+
 * | This method is used to manage CQIs. When a timeslot begins each user sends a CQI |
 * | packet. The Antenna stores CQIs in std::vector<CQIPacket> CQIs. Each element of  |
 * | CQIs is a couple whose fields are the user's id and the current CQI.             |
 * +----------------------------------------------------------------------------------+
 */

    CQIMessage *cqi = check_and_cast<CQIMessage*>(msg);
    int id = cqi->getId();
    int CQI = cqi->getCQI();

    #ifdef DEBUG
    EV << "Antenna::handleMessage() - A new CQI RESPONSE has just arrived! id=" << id << ", CQI=" << CQI << endl;
    #endif

    CQIPacket *tmp = new CQIPacket(id, CQI);
    CQIs.push_back(tmp);

    // Since the message is no more useful, it will be 'deleted' to avoid any memory leak.
    delete(msg);

    if(CQIs.size() == population)
    {
        // Each user has just sent his/her CQI packet. The Antenna must compose a new Frame.
        handleFrame();
        // To remove all the elements of the std::vector<CQIPacket*> CQIs.
        CQIs.clear();
    }
}


void Antenna::handleFrame()
{


/* +-----------------------------------------------------------------------------------+
 * | AUTHOR : FEDERICO                                                                 |
 * +-----------------------------------------------------------------------------------+
 * | This method is used to compose a new Frame. The Antenna composes a frame made up  |
 * | of 25 RBs. The antenna serves its users using an opportunistic policy: backlogged |
 * | users are served by decreasing CQI. When a user is considered for service, its    |
 * | queue is emptied, if the number of unallocated RBs is large enough.               |
 * +-----------------------------------------------------------------------------------+
 */

    // Opportunistic policy: users are served by decreasing CQI -> CQIs has to be sorted
    // in a decreasing order.
    std::sort(CQIs.begin(), CQIs.end(), [](CQIPacket *X, CQIPacket *Y){ return X->getCQI()>Y->getCQI();});

    // Once std::vector CQIs has been sorted the Antenna can pack a new Frame.
    int remainingRBs = par("maxFrameDim"), currentUser, currentCQI;

    // To store the total number of bytes sent in a timeslot.
    int bytesToSend = 0;


    int nRB = 25;
    int remainingRB = nRB;

    for(int i = 0; i < population; i++)
    {
        // Retrieving the user under service.
        currentUser = CQIs[i]->getId();
        // Retrieving the current CQI sent by the user under service.
        currentCQI = CQIs[i]->getCQI();

        int dimRB = CQI_to_BYTES(currentCQI);

        // get user queue
        UserQueue * queue = getQueueById(currentUser);
        std::pair<simtime_t,int> pkt;

        int remainingBytesFrame = remainingRB * dimRB;

        queue->showQueue();

        EV << "RB dimension is " << dimRB <<endl;

        while(!queue->getQueue()->empty()){

            pkt.second = queue->getQueue()->begin()->second;
            pkt.first = queue->getQueue()->begin()->first;

            if (pkt.second <= remainingBytesFrame){

                // serve packet
                queue->getQueue()->erase(queue->getQueue()->begin());
                remainingBytesFrame-=pkt.second;
                bytesToSend+=pkt.second;

                // send info to user
                Packet * packet = new Packet;
                packet->setSize(pkt.second);
                packet->setTimestamp(pkt.first);
                send(packet,"out",currentUser);

                EV << "Served user "<< currentUser << "packet size = " << pkt.second << endl;
                EV << "bytes that i can use for the user are " << remainingBytesFrame << endl;

            }
            else
            {
                EV << "Cannot serve user, packet size is too large packet_size = " << pkt.second << endl;
                break;
            }
         }

        // need to calculate used RBs
        int allocatedFrameBytes = remainingRB * dimRB - remainingBytesFrame;
        int allocatedRBs = (allocatedFrameBytes % dimRB) ? allocatedFrameBytes/dimRB + 1: allocatedFrameBytes/dimRB;

        EV << "Number of used bytes for user " << currentUser << "are " << allocatedFrameBytes << endl;
        EV << "Number of used RBs for user " << currentUser << "are " << allocatedRBs << endl;

        remainingRB -= allocatedRBs;

        EV << "Number or actual RB are " << remainingRB << endl;
    }

    // Recording throughtput's statistics
    emit(throughputSignal, bytesToSend);
}

int Antenna::CQI_to_BYTES(int CQI)
{
//  AUTHOR : FEDERICO
    switch(CQI)
    {
        case 1 : return 3;
        case 2 : return 3;
        case 3 : return 6;
        case 4 : return 11;
        case 5 : return 15;
        case 6 : return 20;
        case 7 : return 25;
        case 8 : return 36;
        case 9 : return 39;
        case 10: return 50;
        case 11: return 63;
        case 12: return 72;
        case 13: return 80;
        case 14: return 93;
        case 15: return 93;
        default: return 0;
    }
}

UserQueue* Antenna::getQueueById(int id)
{
//  AUTHOR : FEDERICO
    for(UserQueue *currentUser : userQueues)
    {
        if(currentUser->getId() == id)
            return currentUser;
    }
    return NULL;
}

}
