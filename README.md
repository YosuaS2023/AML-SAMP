# AMLSAMP Mobile

**AMLSAMP Mobile** adalah plugin AndroidModLoader (AML) sekaligus SDK yang memungkinkan modder AML dapat **berkomunikasi langsung dengan server SA-MP melalui jaringan RakNet dan Remote Procedure Call (RPC)**.

---

## 🌟 Kegunaan & Fitur Utama

- **Komunikasi Jaringan SA-MP**: Membuka akses bagi mod AML untuk membaca, dan memproses packet jaringan yang dikirim oleh server ke client.
- **RakNet & RPC Handler**: Memungkinkan mod menangkap packet masuk melalui event jaringan `ReceiveIgnoreRPC`.
- **Sistem Interface Modular AML**: Menggunakan standar `GetInterface("AMLSAMP")` dari AML, sehingga banyak mod berbeda dapat mendengarkan packet jaringan secara bersamaan tanpa saling bentrok.
- **Arsitektur 32-bit & 64-bit**: Mendukung `armeabi-v7a` dan `arm64-v8a`.

---

## 📦 Isi Repositori (SDK)

- **`isampfuncs.h`**: Header interface utama yang perlu di-include ke dalam project mod AML Anda.
- **`raknet/`**: Header data type RakNet (`BitStream`, `RakNetTypes.h`, dll) untuk parsing payload packet.
- **`libs/`**: Binary prebuilt plugin (`libSAMPAml.so` & `libSAMPAml64.so`) yang siap dipasang di game.
- **`test.cpp`**: Contoh nyata implementasi mod client AML yang menggunakan interface AMLSAMP.

---

## 🚀 Panduan Penggunaan untuk Developer

### 1. Persiapan Project Mod
Salin file/folder berikut ke dalam direktori project mod AML Anda:
- `isampfuncs.h`
- Folder `raknet/`

Pastikan include path pada `Android.mk` Anda sudah mengarah ke folder tersebut:
```makefile
LOCAL_C_INCLUDES += $(LOCAL_PATH)/raknet
```

### 2. Include Header di Kode Mod
Tambahkan include interface AMLSAMP dan RakNet pada file source code mod Anda:

```cpp
#include "isampfuncs.h"
#include <raknet/RakNetTypes.h>
#include <raknet/BitStream.h>
```

### 3. Dapatkan Interface & Daftarkan Callback
Ambil instance `ISAMPFUNCS` pada callback `ON_ALL_MODS_LOAD()` AML dan daftarkan listener:

```cpp
#include <mod/amlmod.h>
#include <mod/iaml.h>
#include <mod/logger.h>

#include "isampfuncs.h"
#include <raknet/RakNetTypes.h>
#include <raknet/BitStream.h>

MYMOD(com.example.mymod, My AMLSAMP Mod, 1.0, AuthorName)

ISAMPFUNCS* amlsamp = nullptr;

// 1. Callback untuk menangani Packet umum (User-Friendly: Payload langsung siap dibaca)
void OnReceivePacket(uint8_t packetId, RakNet::BitStream* bs)
{
    logger->Info("[MyMod] Diterima Packet ID: %u (Payload: %u bits)", packetId, bs ? bs->GetNumberOfUnreadBits() : 0);

    if (packetId == 100 || packetId == 252)
    {
        uint8_t action = 0;
        if (bs && bs->Read(action))
        {
            logger->Info("[MyMod] Custom Packet action: %u", action);
        }
    }
}

// 2. Callback untuk menangani RPC dari server
void OnReceiveRPC(int rpcId, RakNet::BitStream* bs)
{
    logger->Info("[MyMod] Incoming RPC ID: %d", rpcId);
}

// 3. Hubungkan ke AMLSAMP saat semua mod selesai dimuat
ON_ALL_MODS_LOAD()
{
    amlsamp = (ISAMPFUNCS*)GetInterface("AMLSAMP");
    if (!amlsamp)
    {
        logger->Error("AMLSAMP tidak ditemukan! Pastikan libSAMPAml.so terpasang.");
        return;
    }

    // Daftarkan listener
    amlsamp->AddReceivePacketListener(OnReceivePacket);
    amlsamp->AddReceiveRPCListener(OnReceiveRPC);
    logger->Info("Berhasil terhubung ke AMLSAMP Mobile!");
}
```

---

## 📚 Referensi API (`ISAMPFUNCS`)

### 1. `AddReceivePacketListener` (User-Friendly Packet)
```cpp
virtual void AddReceivePacketListener(ReceivePacketFn fn) = 0;
```
* **Deskripsi**: Mendaftarkan fungsi callback untuk menerima paket non-RPC (seperti custom packet 100/252, sync, dll.). `AMLSAMP` otomatis membuang timestamp & packet ID, sehingga `bs` langsung siap membaca payload data server!
* **Tipe Callback**:
  ```cpp
  typedef void (*ReceivePacketFn)(uint8_t packetId, RakNet::BitStream* bs);
  ```

### 2. `AddReceiveRawPacketListener` (Advanced Raw Packet)
```cpp
virtual void AddReceiveRawPacketListener(ReceiveRawPacketFn fn) = 0;
virtual void AddReceiveIgnoreRPCListener(ReceiveRawPacketFn fn) = 0; // Alias kompatibilitas
```
* **Deskripsi**: Akses langsung ke pointer `RakNet::Packet*` asli untuk kebutuhan tingkat lanjut (seperti membaca `pkt->systemAddress`, `pkt->playerId`, dll.).
* **Tipe Callback**:
  ```cpp
  typedef void (*ReceiveRawPacketFn)(RakNet::Packet* pkt);
  ```

### 3. `AddReceiveRPCListener`
```cpp
virtual void AddReceiveRPCListener(ReceiveRPCFn fn) = 0;
```
* **Deskripsi**: Mendaftarkan listener khusus untuk **Remote Procedure Call (RPC)** masuk dari server. `AMLSAMP` otomatis mem-parse timestamp dan kompresi data, sehingga callback langsung menerima `rpcId` dan `BitStream` isinya.
* **Tipe Callback**:
  ```cpp
  typedef void (*ReceiveRPCFn)(int rpcId, RakNet::BitStream* bs);
  ```

### 4. `RegisterRPC` / `UnregisterRPC`
```cpp
virtual bool RegisterRPC(int rpcId, RPCFunction fn) = 0;
virtual bool UnregisterRPC(int rpcId) = 0;
```
* **Deskripsi**: Mendaftarkan handler RPC secara native ke tabel `RegisterAsRemoteProcedureCall` milik RakClient. Jika dipanggil sebelum game connect, pendaftaran akan diantrekan secara otomatis dan diinjeksikan begitu instance jaringan aktif.
* **Tipe Callback**:
  ```cpp
  typedef void (*RPCFunction)(RakNet::RPCParameters* pRpcParams);
  ```

### 5. `SendRPC`
```cpp
virtual bool SendRPC(int rpcId, 
                     RakNet::BitStream* bs = nullptr, 
                     RakNet::PacketPriority priority = RakNet::HIGH_PRIORITY, 
                     RakNet::PacketReliability reliability = RakNet::RELIABLE_ORDERED, 
                     char orderingChannel = 0, 
                     bool shiftTimestamp = false) = 0;
```
* **Deskripsi**: Mengirim Remote Procedure Call (RPC) ke server SA-MP melalui client RakNet.
* **Return**: `true` jika RPC berhasil dikirim, `false` jika client belum terhubung.

### 6. `Send`
```cpp
virtual bool Send(RakNet::BitStream* bs, 
                  RakNet::PacketPriority priority = RakNet::HIGH_PRIORITY, 
                  RakNet::PacketReliability reliability = RakNet::RELIABLE_ORDERED, 
                  char orderingChannel = 0) = 0;
```
* **Deskripsi**: Mengirim raw packet RakNet ke server SA-MP.
* **Return**: `true` jika packet berhasil dikirim, `false` jika client belum terhubung.

### Struktur Data `RakNet::Packet`
| Anggota | Tipe | Deskripsi |
| :--- | :--- | :--- |
| `data` | `unsigned char*` | Raw buffer byte dari packet jaringan. |
| `data[0]` | `uint8_t` | ID pengenal jenis packet (RakNet / SA-MP Packet ID). |
| `length` | `unsigned int` | Ukuran total packet dalam satuan byte. |
| `bitSize` | `unsigned int` | Ukuran total packet dalam satuan bit. |
| `systemAddress` | `SystemAddress` | Alamat IP dan Port endpoint server. |
| `playerIndex` | `PlayerIndex` | Index internal koneksi player. |

---

## 🎮 Panduan Pemasangan untuk Pemain (User)

Bagi pemain yang menggunakan mod yang membutuhkan **AMLSAMP Mobile**:

1. Pastikan game GTA:SA sudah terpasang **AndroidModLoader (AML)**.
2. Salin file library dari folder `libs/`:
   * Untuk GTA 32-bit: `libs/armeabi-v7a/libSAMPAml.so`
   * Untuk GTA 64-bit: `libs/arm64-v8a/libSAMPAml64.so`
3. Masukkan file `.so` tersebut ke folder mods AML di storage perangkat Anda:
   ```
   Android/data/<package.name>/files/mods/
   ```
4. Jalankan game bersama mod yang Anda gunakan.