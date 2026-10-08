// Executa o sketch real com UART/relogio/GPTimer substituidos por dubles.
// Nao modela latencia, transistor, tensao ou integridade dos sinais.
#include <Arduino.h>
#include <cassert>
#include <algorithm>
HardwareSerial Serial,Serial1,Serial2;
int driveLevel=0;
bool alarmEnabled=false;
int64_t simulatedTime=0;
#include "../firmware/noise/noise.ino"

void fresh() {
  Serial.output.clear();Serial.input.clear();Serial1.input.clear();Serial2.input.clear();
  mode=activeMode=un=dn=count=eventCount=eventIndex=0;
  armed=busy=pulseFinished=timingFault=false;
  captureLastByte=armedAt=triggeredAt=0;driveLevel=0;alarmEnabled=false;
  simulatedTime=500000; // Reproduz inicio tardio, apos o timeout antigo.
}
void frameAndDebug(uint16_t number,uint8_t* bytes) {
  makeFrame(number,128,128,SUM8,bytes);makeDebug(number,128,128,bytes+8);
}
int main() {
  fresh();uint8_t bytes[14];frameAndDebug(0,bytes);
  for(uint8_t b:bytes) {
    simulatedTime+=87;upstream.input.push_back(b);downstream.input.push_back(b);loop();
  }
  assert(Serial.output=="0,0,-1,-1,0,na,na,na,BASELINE\n");
  assert(un==0 && dn==0);

  fresh();upstream.input.push_back(START0);loop();
  simulatedTime+=FRAME_TIMEOUT_US+1;loop();
  assert(un==0 && Serial.output.find("CAPTURA_INCOMPLETA")!=std::string::npos);

  fresh();frameAndDebug(0,up);frameAndDebug(1,down);un=dn=14;loop();
  assert(Serial.output.find("CAPTURA_INVALIDA")!=std::string::npos);
  assert(Serial.output.find("BASELINE")==std::string::npos);

  fresh();eventCount=2;events[0]={20,true};events[1]={26,false};busy=true;
  alarmEnabled=true;gptimer_alarm_event_data_t late={24};
  alarmCallback(pulseTimer,&late,nullptr);
  assert(timingFault && !busy && !alarmEnabled && driveLevel==0);

  fresh();eventCount=2;events[0]={20,true};events[1]={26,false};busy=true;
  gptimer_alarm_event_data_t first={20},last={26};
  alarmCallback(pulseTimer,&first,nullptr);assert(driveLevel==1 && busy);
  alarmCallback(pulseTimer,&last,nullptr);assert(driveLevel==0 && !busy && pulseFinished);
  puts("5 cenarios do injetor passaram (dubles; sem validacao fisica).");
}
