#pragma once
#include <vector>
#include <inttypes.h>

#include <raknet/RakNetTypes.h>
#include <raknet/BitStream.h>

// Callback untuk Packet Masuk User-Friendly (otomatis dikupas timestamp & packet ID, bs langsung isi payload!)
typedef void (*ReceivePacketFn)(uint8_t packetId, RakNet::BitStream* bs);

// Callback untuk Raw Packet mentah (akses langsung ke RakNet::Packet*)
typedef void (*ReceiveRawPacketFn)(RakNet::Packet* pkt);
typedef ReceiveRawPacketFn ReceiveIgnoreRPCFn;

// Callback untuk RPC Masuk (otomatis di-parse RPC ID & BitStream payload-nya)
typedef void (*ReceiveRPCFn)(int rpcId, RakNet::BitStream* bs);

// Callback untuk registrasi RPC native ke RakClient
typedef void (*RPCFunction)(RakNet::RPCParameters* pRpcParams);

class ISAMPFUNCS {
public:
    virtual ~ISAMPFUNCS() = default;

    // Mendaftarkan callback untuk menerima packet (user-friendly: packetId + payload BitStream)
    virtual void AddReceivePacketListener(ReceivePacketFn fn) = 0;

    // Mendaftarkan callback untuk menerima raw packet jaringan (akses RakNet::Packet*)
    virtual void AddReceiveRawPacketListener(ReceiveRawPacketFn fn) = 0;
    virtual void AddReceiveIgnoreRPCListener(ReceiveRawPacketFn fn) = 0; // Alias

    // Mendaftarkan callback untuk menerima RPC yang masuk (otomatis di-parse)
    virtual void AddReceiveRPCListener(ReceiveRPCFn fn) = 0;
    
    // Registrasi Handler RPC native ke RakClient (RegisterAsRemoteProcedureCall)
    virtual bool RegisterRPC(int rpcId, RPCFunction fn) = 0;
    virtual bool UnregisterRPC(int rpcId) = 0;

    // Mengirim RPC (Remote Procedure Call) ke server SA-MP
    virtual bool SendRPC(int rpcId, RakNet::BitStream* bs = nullptr, RakNet::PacketPriority priority = RakNet::HIGH_PRIORITY, RakNet::PacketReliability reliability = RakNet::RELIABLE_ORDERED, char orderingChannel = 0, bool shiftTimestamp = false) = 0;

    // Mengirim raw RakNet packet ke server SA-MP
    virtual bool Send(RakNet::BitStream* bs, RakNet::PacketPriority priority = RakNet::HIGH_PRIORITY, RakNet::PacketReliability reliability = RakNet::RELIABLE_ORDERED, char orderingChannel = 0) = 0;
};