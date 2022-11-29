//
// Generated file, do not edit! Created by opp_msgtool 6.0 from Packet.msg.
//

#ifndef __OPPORTUNISTICCELLULARNETWORK_PACKET_M_H
#define __OPPORTUNISTICCELLULARNETWORK_PACKET_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif


namespace opportunisticcellularnetwork {

class Packet;

}  // namespace opportunisticcellularnetwork


namespace opportunisticcellularnetwork {

/**
 * Class generated from <tt>Packet.msg:3</tt> by opp_msgtool.
 * <pre>
 * packet Packet
 * {
 *     int size;
 *     int index;
 *     simtime_t timestamp;
 * }
 * </pre>
 */
class Packet : public ::omnetpp::cPacket
{
  protected:
    int size = 0;
    int index = 0;
    ::omnetpp::simtime_t timestamp = SIMTIME_ZERO;

  private:
    void copy(const Packet& other);

  protected:
    bool operator==(const Packet&) = delete;

  public:
    Packet(const char *name=nullptr, short kind=0);
    Packet(const Packet& other);
    virtual ~Packet();
    Packet& operator=(const Packet& other);
    virtual Packet *dup() const override {return new Packet(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual int getSize() const;
    virtual void setSize(int size);

    virtual int getIndex() const;
    virtual void setIndex(int index);

    virtual ::omnetpp::simtime_t getTimestamp() const;
    virtual void setTimestamp(::omnetpp::simtime_t timestamp);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const Packet& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, Packet& obj) {obj.parsimUnpack(b);}


}  // namespace opportunisticcellularnetwork


namespace omnetpp {

template<> inline opportunisticcellularnetwork::Packet *fromAnyPtr(any_ptr ptr) { return check_and_cast<opportunisticcellularnetwork::Packet*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __OPPORTUNISTICCELLULARNETWORK_PACKET_M_H

