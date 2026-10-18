#include <cinttypes>
#include <mod/amlmod.h>
#include <mod/iaml.h>
#include <mod/logger.h>
#include <mod/config.h>

MYMODCFGNAME(test.sampfuncs, SAMP FUNCS TEST, 1.0, StevenY, test_sampfuncs)

#include "isampfuncs.h"
#include <raknet/RakNetTypes.h>
#include <raknet/BitStream.h>

ISAMPFUNCS* isampfuncs = nullptr;

void OnPacket(uint8_t packetId, RakNet::BitStream* bs)
{
    logger->Info("[TestPacket] >> Packet ID: %u (Payload Bits: %u)", packetId, bs ? bs->GetNumberOfUnreadBits() : 0);

    if (packetId == 100 || packetId == 252)
    {
        uint8_t action = 0;
        if (bs && bs->Read(action))
        {
            logger->Info("[TestPacket] -> Terdeteksi Packet %u! Action: %u", packetId, action);
        }
    }
}

void OnRawPacket(RakNet::Packet* pkt)
{

}

void OnReceiveRPC(int rpcId, RakNet::BitStream* bs)
{
    logger->Info("[TestRPC] >> Incoming Custom RPC ID: %d (Payload Bits: %u)", rpcId, bs ? bs->GetNumberOfUnreadBits() : 0);
}

void OnCustomNativeRPC(RakNet::RPCParameters* pRpcParams)
{
    if (!pRpcParams || !pRpcParams->input)
        return;

    logger->Info("[NativeRPC] >> Custom Registered RPC triggered! (Bits: %u)", pRpcParams->numberOfBitsOfData);
}

ON_MOD_PRELOAD()
{
    logger->SetTag("SAMPFUNCSTest");
}

ON_ALL_MODS_LOAD()
{
    isampfuncs = (ISAMPFUNCS*)GetInterface("AMLSAMP");
    if (!isampfuncs)
    {
        logger->Error("[AMLSAMP-Test] sampfuncs interface not found!");
        return;
    }

    isampfuncs->AddReceivePacketListener(OnPacket);
    isampfuncs->AddReceiveRawPacketListener(OnRawPacket);
    isampfuncs->AddReceiveRPCListener(OnReceiveRPC);
    isampfuncs->RegisterRPC(250, OnCustomNativeRPC);

    logger->Info("[AMLSAMP-Test] All Packet & RPC listeners registered successfully!");
}

ON_MOD_UNLOAD()
{
    logger->Info("[AMLSAMP-Test] Example Mod unloaded.");
}