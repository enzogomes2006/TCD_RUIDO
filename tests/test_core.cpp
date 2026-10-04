#include "../libraries/Telemetry/src/Telemetry.h"
#include <cassert>
#include <initializer_list>
int main() {
  using namespace telemetry;
  const uint8_t vector[]={'1','2','3','4','5','6','7','8','9'};
  assert(crc8(vector,9)==0xF4);
  for(auto alg : {SUM8,CRC8}) {
    uint8_t frame[8]; makeFrame(1,128,128,alg,frame);
    assert(residue(frame+2,6,alg)==0);
    frame[5]=frame[6]=0;
    assert((residue(frame+2,6,alg)==0)==(alg==SUM8));
  }
}
