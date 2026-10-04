/* TX - ESP32 classico, Arduino-ESP32 3.x. Prototipo para validar em bancada.
 * Monitor USB: s=soma; c=CRC; g=iniciar 1000 quadros; p=parar.
 * Configure RX e ruido antes de g. Nao muda tecnica durante uma rodada. */
#include <Arduino.h>
#include <Telemetry.h>
#include "esp_timer.h"
using namespace telemetry;
HardwareSerial link(2);
Algorithm algorithm=SUM8;
uint16_t seq=0, remaining=0;
int64_t nextAt=0, lastAt=0;
void setup() {
  Serial.begin(115200);
  link.begin(BAUD,SERIAL_8N1,16,17);
  Serial.println("seq,tecnica,timestamp_us,intervalo_us,calculo_us,tx_hex");
}
void loop() {
  while(Serial.available()) {
    char c=Serial.read();
    if(c=='p') remaining=0;
    if(!remaining && (c=='s'||c=='c')) algorithm=c=='s'?SUM8:CRC8;
    if(!remaining && c=='g') {seq=0;remaining=1000;nextAt=esp_timer_get_time();lastAt=0;}
  }
  if(!remaining || esp_timer_get_time()<nextAt) return;
  uint8_t f[8],d[6];
  // Telemetria artificial conhecida no TX; o RX nao reproduz este gerador.
  uint8_t p0=uint8_t(seq*17+128),p1=uint8_t(seq*29+128);
  int64_t t0=esp_timer_get_time();makeFrame(seq,p0,p1,algorithm,f);
  int64_t calc=esp_timer_get_time()-t0;
  makeDebug(seq,p0,p1,d);
  int64_t sentAt=esp_timer_get_time();
  link.write(f,5);link.flush();
  delay(GUARD_US/1000); // Janela de armamento do injetor; nao temporiza pulsos.
  link.write(f+5,3);link.flush();
  delay(1); // Impede uma injecao pendente de atingir a referencia.
  link.write(d,6);link.flush();
  Serial.printf("%u,%s,%lld,%lld,%lld,",seq,algorithm==SUM8?"sum8":"crc8",
    (long long)sentAt,(long long)(lastAt?sentAt-lastAt:0),(long long)calc);
  for(uint8_t b:f) Serial.printf("%02X",b);
  Serial.println();lastAt=sentAt;nextAt=sentAt+INTERVAL_US;
  seq=uint16_t(seq+1);--remaining;
}
