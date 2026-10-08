/* EXTRA experimental, NAO validado em hardware.
 * ESP32 classico + Arduino-ESP32 3.x / ESP-IDF 5.x. GPTimer programa pulsos.
 * USB 0..3 troca modo. Observe docs/extra_bancada.md e valide o timing.
 * TAP_UP=16 e EDGE=25 no TX antes do resistor; TAP_DOWN=26 na entrada RX.
 * DRIVE=27 -> resistor de base 1k -> BC547 NPN; emissor ao GND comum.
 * LOW no GPIO desliga o transistor. Conferir C/B/E do componente real.
 * Apenas payload/check sao sorteados. Nunca start/stop UART ou debug. */
#include <Arduino.h>
#include <Telemetry.h>
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_random.h"
using namespace telemetry;
constexpr int DRIVE=27,EDGE=25,TAP_UP=16,TAP_DOWN=26;
HardwareSerial &upstream=Serial1,&downstream=Serial2;
gptimer_handle_t pulseTimer=nullptr;
struct PulseEvent {uint32_t at;bool on;};
PulseEvent events[4]; uint8_t selected[8],count=0,eventCount=0,activeMode=0;
volatile uint8_t eventIndex=0;
volatile bool armed=false,busy=false,pulseFinished=false,timingFault=false;
volatile int64_t triggeredAt=0;
uint8_t mode=0,up[14],down[14],un=0,dn=0;uint16_t seq=0;
int64_t armedAt=0,captureLastByte=0;
// Trocar configuracao so entre quadros. Arrays ficam imutaveis enquanto busy.
bool IRAM_ATTR alarmCallback(gptimer_handle_t timer,const gptimer_alarm_event_data_t* data,void*) {
  uint8_t i=eventIndex;
  if(i>=eventCount)return false;
  // Atraso grande invalida a tentativa; nunca oculte o problema no CSV.
  if(data->count_value>events[i].at+3) {
    // Aborta tentativa atrasada para nao estender LOW ao stop/debug.
    timingFault=true;gpio_set_level((gpio_num_t)DRIVE,0);
    gptimer_set_alarm_action(timer,nullptr);busy=false;return false;
  }
  gpio_set_level((gpio_num_t)DRIVE,events[i].on?1:0);
  ++eventIndex;
  if(eventIndex<eventCount) {
    gptimer_alarm_config_t alarm={};alarm.alarm_count=events[eventIndex].at;
    if(gptimer_set_alarm_action(timer,&alarm)!=ESP_OK) {
      timingFault=true;gpio_set_level((gpio_num_t)DRIVE,0);
      gptimer_set_alarm_action(timer,nullptr);busy=false;
    }
  } else {gpio_set_level((gpio_num_t)DRIVE,0);busy=false;pulseFinished=true;}
  return false;
}
void IRAM_ATTR bodyStart() {
  if(!armed||busy)return;
  armed=false;busy=true;eventIndex=0;triggeredAt=esp_timer_get_time();
  gptimer_set_raw_count(pulseTimer,0);
  gptimer_alarm_config_t alarm={};alarm.alarm_count=events[0].at;
  if(gptimer_set_alarm_action(pulseTimer,&alarm)!=ESP_OK){timingFault=true;busy=false;gpio_set_level((gpio_num_t)DRIVE,0);}
}
uint32_t bitTime(double bit){return uint32_t(bit*1000000.0/BAUD+0.5);}
void prepare() {
  activeMode=mode;count=eventCount=0;timingFault=pulseFinished=false;
  armed=false;armedAt=esp_timer_get_time();triggeredAt=0;
  if(activeMode==0)return;
  if(activeMode==3) {
    uint8_t byte=esp_random()%3,size=3+esp_random()%6,start=esp_random()%(9-size);
    for(uint8_t k=0;k<size;++k)selected[count++]=byte*8+start+k;
  } else {
    selected[count++]=esp_random()%24;
    if(activeMode==2){uint8_t other;do{other=esp_random()%24;}while(other==selected[0]);selected[count++]=other;}
  }
  // Ordena bits LSB primeiro; junta adjacentes no mesmo byte num unico pulso.
  for(uint8_t i=0;i<count;++i)for(uint8_t j=i+1;j<count;++j)if(selected[j]<selected[i]){uint8_t t=selected[i];selected[i]=selected[j];selected[j]=t;}
  uint8_t i=0;
  while(i<count) {
    uint8_t first=selected[i],last=first;++i;
    while(i<count && selected[i]==last+1 && selected[i]/8==first/8)last=selected[i++];
    double start=(first/8)*10+1+(first%8)+0.1;
    double finish=(last/8)*10+1+(last%8)+0.9;
    events[eventCount++]={bitTime(start),true};events[eventCount++]={bitTime(finish),false};
  }
  armedAt=esp_timer_get_time();armed=true;
}
void collectUp(uint8_t b) {
  captureLastByte=esp_timer_get_time();
  if(!un){if(b==START0)up[un++]=b;return;}
  if(un==1 && b!=START1){un=b==START0?1:0;return;}
  up[un++]=b;
  if(un==5){seq=uint16_t(up[3])<<8|up[4];if(up[2]==LENGTH)prepare();}
}
void collectDown(uint8_t b) {
  captureLastByte=esp_timer_get_time();
  if(!dn){if(b==START0)down[dn++]=b;return;}
  if(dn==1&&b!=START1){dn=b==START0?1:0;return;}
  down[dn++]=b;
}
void logComplete() {
  // Nunca parear quadros diferentes ou uma referencia deslocada como alteracao.
  bool paired=up[0]==START0 && up[1]==START1 && up[2]==LENGTH
    && up[8]==DEBUG0 && up[9]==DEBUG1 && up[3]==up[10] && up[4]==up[11];
  for(uint8_t k=0;k<5;++k)if(up[k]!=down[k])paired=false;
  for(uint8_t k=8;k<14;++k)if(up[k]!=down[k])paired=false;
  if(!paired) {
    Serial.println("# CAPTURA_INVALIDA;cabecalho_referencia_ou_pareamento;reconciliar");
    un=dn=0;armed=false;return;
  }
  if(activeMode==0)Serial.printf("%u,0,-1,-1,0,na,na,na,BASELINE\n",seq);
  for(uint8_t k=0;k<count;++k) {
    uint8_t byte=5+selected[k]/8,bit=selected[k]%8;
    uint8_t before=(up[byte]>>bit)&1,after=(down[byte]>>bit)&1;
    Serial.printf("%u,%u,%u,%u,%lld,%u,%u,%s,%s\n",seq,activeMode,byte,bit,(long long)triggeredAt,
      before,after,before!=after?"sim":"nao",timingFault?"TIMING_INVALIDO":(!pulseFinished?"SEM_PULSO":"COMPARAR_COM_RX"));
  }
  un=dn=0;armed=false;
}
void setup() {
  Serial.begin(115200);pinMode(DRIVE,OUTPUT);digitalWrite(DRIVE,LOW);pinMode(EDGE,INPUT);
  // -1 pode manter um TX anterior. Reserva saidas nao ligadas para evitar conflito.
  upstream.begin(BAUD,SERIAL_8N1,TAP_UP,18);downstream.begin(BAUD,SERIAL_8N1,TAP_DOWN,19);
  upstream.setRxFIFOFull(1);upstream.setRxTimeout(1);downstream.setRxFIFOFull(1);downstream.setRxTimeout(1);
  gptimer_config_t config={};config.clk_src=GPTIMER_CLK_SRC_DEFAULT;config.direction=GPTIMER_COUNT_UP;config.resolution_hz=1000000;
  ESP_ERROR_CHECK(gptimer_new_timer(&config,&pulseTimer));
  gptimer_event_callbacks_t callbacks={};callbacks.on_alarm=alarmCallback;
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(pulseTimer,&callbacks,nullptr));
  ESP_ERROR_CHECK(gptimer_enable(pulseTimer));ESP_ERROR_CHECK(gptimer_start(pulseTimer));
  attachInterrupt(digitalPinToInterrupt(EDGE),bodyStart,FALLING);
  Serial.println("seq,modo,byte,bit,timestamp_us,antes,depois,mudou,validacao");
}
void loop() {
  while(Serial.available()){char c=Serial.read();if(c>='0'&&c<='3')mode=c-'0';}
  while(upstream.available()&&un<14)collectUp(uint8_t(upstream.read()));
  while(downstream.available()&&dn<14)collectDown(uint8_t(downstream.read()));
  if(un==14&&dn==14&&!busy)logComplete();
  if(armed && esp_timer_get_time()-armedAt>FRAME_TIMEOUT_US){armed=false;timingFault=true;digitalWrite(DRIVE,LOW);}
  if((un||dn) && !busy && esp_timer_get_time()-captureLastByte>FRAME_TIMEOUT_US) {
    Serial.println("# CAPTURA_INCOMPLETA;reconciliar_e_descartar_rodada_piloto");un=dn=0;armed=false;
  }
}
