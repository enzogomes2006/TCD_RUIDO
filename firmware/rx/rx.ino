/* RX: decide sobre LEN+SEQ+PAYLOAD+CHECK antes de ler a referencia.
 * USB: s=soma, c=CRC, 0..3=metadado de modo, r=limpa estado antes da rodada.
 * Eventos # nao sao linhas de resultado. SEM_REFERENCIA exige reconciliacao. */
#include <Arduino.h>
#include <Telemetry.h>
#include "esp_timer.h"
using namespace telemetry;
HardwareSerial &uartLink=Serial2;
Algorithm algorithm=SUM8; uint8_t mode=0;
uint8_t buf[8],ref[6],n=0,dn=0,match=0;
enum State { SEEK,FRAME,DEBUG }; State state=SEEK;
bool pending=false,checked=false,accepted=false,haveExpected=false,absenceLogged=false;
uint8_t res=0; uint16_t expected=0;
int64_t lastByte=0,lastFrame=0,verifyTime=0;

void resetParser(){state=SEEK;n=dn=match=0;}
void emit(const char* status,uint16_t seq,const char* agrees) {
  Serial.printf("REAL,%s,%u,%u,%u,%d,%s,%s,%s,%lld,",algorithm==SUM8?"sum8":"crc8",mode,seq,
    pending?sequence(buf):0,checked?res:-1,checked?(accepted?"ACEITO":"DESCARTADO"):"SEM_CHECAGEM",agrees,status,(long long)verifyTime);
  if(pending) for(uint8_t b:buf) Serial.printf("%02X",b);
  Serial.println();
}
void completeFrame() {
  pending=true;checked=true;
  int64_t t0=esp_timer_get_time();res=residue(buf+2,6,algorithm);verifyTime=esp_timer_get_time()-t0;
  accepted=(res==0);lastFrame=esp_timer_get_time();absenceLogged=false;
  if(accepted) {
    uint16_t got=sequence(buf);
    if(haveExpected) {
      uint16_t gap=distance(expected,got);
      if(gap && gap<32768) Serial.printf("# LACUNA_OBSERVADA,%u,%u;reconciliar_descartados\n",expected,gap);
      if(gap>=32768) Serial.printf("# REPETIDO_OU_ANTIGO,%u\n",got);
      else expected=uint16_t(got+1);
    } else {expected=uint16_t(got+1);haveExpected=true;}
  }
  // A decisao ja esta congelada. Referencia so determina a classe experimental.
  resetParser();
}
void completeDebug() {
  uint16_t truthSeq=uint16_t(ref[2])<<8|ref[3];
  if(!checked) emit("PERDIDO",truthSeq,"N/A");
  else {
    bool same=buf[5]==ref[4] && buf[6]==ref[5];
    emit(!accepted?"CORROMPIDO":(same?"OK":"NAO_DETECTADO"),truthSeq,same?"sim":"nao");
  }
  pending=checked=accepted=false;verifyTime=0;resetParser();
}
void consume(uint8_t b) {
  lastByte=esp_timer_get_time();
  if(state==FRAME) {
    buf[n++]=b;
    if(n==3 && buf[2]!=LENGTH){pending=true;checked=false;resetParser();return;}
    if(n==8) completeFrame();
    return;
  }
  if(state==DEBUG) {ref[dn++]=b;if(dn==6)completeDebug();return;}
  if(match==START0 && b==START1) {
    if(checked) {emit("SEM_REFERENCIA",sequence(buf),"N/A");pending=checked=false;}
    buf[0]=START0;buf[1]=START1;n=2;match=0;state=FRAME;return;
  }
  if(match==DEBUG0 && b==DEBUG1) {ref[0]=DEBUG0;ref[1]=DEBUG1;dn=2;match=0;state=DEBUG;return;}
  match=(b==START0||b==DEBUG0)?b:0;
}
void setup() {
  Serial.begin(115200);uartLink.setRxBufferSize(1024);
  uartLink.begin(BAUD,SERIAL_8N1,16,17);uartLink.setRxFIFOFull(1);uartLink.setRxTimeout(1);
  lastFrame=esp_timer_get_time();
  Serial.println("origem,tecnica,modo,seq,seq_recebida,residuo,decisao,payload_confere,status,verificacao_us,rx_hex");
}
void loop() {
  while(Serial.available()) {
    char c=Serial.read();
    if(c=='s'||c=='c')algorithm=c=='s'?SUM8:CRC8;
    if(c>='0'&&c<='3')mode=c-'0';
    if(c=='r') {resetParser();pending=checked=haveExpected=false;lastFrame=esp_timer_get_time();absenceLogged=false;}
  }
  while(uartLink.available())consume(uint8_t(uartLink.read()));
  int64_t now=esp_timer_get_time();
  if(state!=SEEK && now-lastByte>FRAME_TIMEOUT_US) {
    Serial.println("# TIMEOUT_PARCIAL;aguardar_referencia_ou_reconciliar_TX");
    if(state==FRAME) {pending=true;checked=false;}
    resetParser();
  }
  if(!absenceLogged && now-lastFrame>ABSENCE_TIMEOUT_US) {
    Serial.println("# AUSENCIA;nao_contar_duas_vezes;reconciliar_TX_RX");absenceLogged=true;
  }
}
