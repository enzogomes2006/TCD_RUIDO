#pragma once
#include <stdint.h>
#include <stddef.h>

// Leia docs/especificacao.md antes de alterar este formato.
namespace telemetry {
constexpr uint8_t START0=0xAA, START1=0x55, LENGTH=5;
constexpr uint8_t DEBUG0=0xD3, DEBUG1=0x3D;
constexpr size_t FRAME_SIZE=8, DEBUG_SIZE=6;
constexpr uint32_t BAUD=115200, INTERVAL_US=100000;
constexpr uint32_t GUARD_US=2000, FRAME_TIMEOUT_US=10000;
constexpr uint32_t ABSENCE_TIMEOUT_US=250000;
enum Algorithm : uint8_t { SUM8, CRC8 };

inline uint8_t crc8(const uint8_t* data, size_t size) {
  // CRC-8/SMBUS: poly=07, init=00, refin/refout=false, xorout=00.
  uint8_t crc=0;
  for(size_t i=0;i<size;++i) {
    crc ^= data[i];
    for(uint8_t bit=0;bit<8;++bit)
      crc=(crc & 0x80) ? uint8_t((crc<<1)^0x07) : uint8_t(crc<<1);
  }
  return crc;
}
inline uint8_t sum8(const uint8_t* data, size_t size) {
  uint8_t sum=0;
  for(size_t i=0;i<size;++i) sum=uint8_t(sum+data[i]);
  return sum;
}
inline uint8_t code(const uint8_t* data,size_t size,Algorithm algorithm) {
  return algorithm==SUM8 ? uint8_t(0-sum8(data,size)) : crc8(data,size);
}
inline uint8_t residue(const uint8_t* data,size_t size,Algorithm algorithm) {
  return algorithm==SUM8 ? sum8(data,size) : crc8(data,size);
}
inline void makeFrame(uint16_t seq,uint8_t p0,uint8_t p1,Algorithm algorithm,uint8_t* f) {
  f[0]=START0; f[1]=START1; f[2]=LENGTH;
  f[3]=uint8_t(seq>>8); f[4]=uint8_t(seq);
  f[5]=p0; f[6]=p1; f[7]=code(f+2,5,algorithm);
}
inline uint16_t sequence(const uint8_t* f) { return uint16_t(f[3])<<8|f[4]; }
inline void makeDebug(uint16_t seq,uint8_t p0,uint8_t p1,uint8_t* d) {
  d[0]=DEBUG0; d[1]=DEBUG1; d[2]=uint8_t(seq>>8); d[3]=uint8_t(seq); d[4]=p0; d[5]=p1;
}
// Distancia modular: <32768 para frente; >=32768 repetido/antigo/ambiguo.
inline uint16_t distance(uint16_t expected,uint16_t received) { return uint16_t(received-expected); }
}
