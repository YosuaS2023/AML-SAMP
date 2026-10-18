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

// Helper untuk mengambil Packet ID (mendukung jika paket diawali ID_TIMESTAMP)
uint8_t GetPacketId(RakNet::Packet* pkt)
{
    if (!pkt || !pkt->data || pkt->length == 0)
        return 0xFF;

    if ((uint8_t)pkt->data[0] == RakNet::ID_TIMESTAMP)
    {
        if (pkt->length > sizeof(uint8_t) + sizeof(uint32_t))
            return (uint8_t)pkt->data[sizeof(uint8_t) + sizeof(uint32_t)];
        return 0xFF;
    }
    return (uint8_t)pkt->data[0];
}

void ExampleSendRPC(int rpcId, const char* text)
{
    if (!isampfuncs)
        return;

    RakNet::BitStream bs;
    if (text)
    {
        uint32_t len = strlen(text);
        bs.Write(len);
        bs.Write(text, len);
    }

    bool success = isampfuncs->SendRPC(rpcId, &bs);
    logger->Info("[TestRPC] SendRPC (ID: %d) status: %s", rpcId, success ? "Berhasil" : "Gagal");
}

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
    logger->Info("[TestRPC] >> Incoming RPC ID: %d (Payload Bits: %u)", rpcId, bs ? bs->GetNumberOfUnreadBits() : 0);

    switch (rpcId)
    {
        case 251: // Contoh custom RPC dari server
            ExampleSendRPC(251, "test");
            break;

        case RakNet::RPC_ScrClientMessage:
            logger->Info("[TestRPC] -> Terdeteksi: RPC_ScrClientMessage");
            break;

        case RakNet::RPC_ScrShowDialog:
            logger->Info("[TestRPC] -> Terdeteksi: RPC_ScrShowDialog");
            break;

        case RakNet::RPC_ScrInitGame:
            logger->Info("[TestRPC] -> Terdeteksi: RPC_ScrInitGame");
            break;

        default:
            break;
    }
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