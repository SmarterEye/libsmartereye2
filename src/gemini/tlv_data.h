//
// Created by xtp on 2021/1/20.
//

#ifndef TLV_DATA_H
#define TLV_DATA_H

#ifdef _WIN64
#pragma warning(disable: 4200)
#endif

#include <cstdint>

#pragma pack(push, 1)

namespace libsmartereye2 {

static const uint32_t kSerialConnectionToken(0xAA5555AA);
static const uint32_t kMinValidTypeValue(50000);
static const uint32_t kMaxValidTypeValue(60000);
static const uint32_t kMaxTlvDataLength(4096 * 1024);

struct TLVStruct {
  uint32_t type;
  uint32_t length;
  uint8_t data[0];

  TLVStruct() : type(0), length(0), data() {}
};

enum SerialConnection {
  SerialConnection_None = kMinValidTypeValue,
  SerialConnection_Sync,
  SerialConnection_Heartbeat,
  SerialConnection_Disconnect,
  SerialConnection_Ack,
};

enum SerialCommand {
  SerialCommand_None = SerialConnection_None + 8,
  SerialCommand_RequirePerception,   /**< Require perception data from device. */
  SerialCommand_RequireUserFiles,
  SerialCommand_RequireIntrinsics,
  SerialCommand_RespondIntrinsics,
  SerialCommand_RequireExtrinsics,
  SerialCommand_RespondExtrinsics
};

enum SerialDataUnit {
  SerialDataUnit_None = SerialCommand_None + 64,
  SerialDataUnit_FileHeader,
  SerialDataUnit_FileTail,
  SerialDataUnit_FileData,
  SerialDataUnit_FileResp,
  SerialDataUnit_Speed,
  SerialDataUnit_J2Perception,
  SerialDataUnit_Obstacle,
  SerialDataUnit_Lane,
  SerialDataUnit_AlgorithmResult,
  SerialDataUnit_FreeSpace,
  SerialDataUnit_TrafficSign,
  SerialDataUnit_TrafficLight,
  SerialDataUnit_VehicleRealTimeInfo
};

struct SerialFileHeader {
  uint32_t fileSize;
  inline const char *fileName() const {
    return reinterpret_cast<const char *>(this) + sizeof(SerialFileHeader);
  }
};

struct SerialFileResp {
  uint32_t received;
  uint16_t continued;
  inline const char *fileName() {
    return (const char *) this + sizeof(SerialFileResp);
  }
};

}  // namespace libsmartereye2

#pragma pack(pop)

#endif //TLV_DATA_H
