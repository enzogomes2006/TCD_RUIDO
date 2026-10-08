#pragma once
#include <stdint.h>
#include <cassert>
using gptimer_handle_t=void*;
constexpr int ESP_OK=0,GPTIMER_CLK_SRC_DEFAULT=0,GPTIMER_COUNT_UP=0;
#define ESP_ERROR_CHECK(result) assert((result)==ESP_OK)
struct gptimer_alarm_event_data_t {uint64_t count_value;};
struct gptimer_alarm_config_t {uint64_t alarm_count;};
struct gptimer_config_t {int clk_src,direction;uint32_t resolution_hz;};
struct gptimer_event_callbacks_t {
  bool (*on_alarm)(gptimer_handle_t,const gptimer_alarm_event_data_t*,void*);
};
extern bool alarmEnabled;
inline int gptimer_set_alarm_action(gptimer_handle_t,const gptimer_alarm_config_t* cfg) {
  alarmEnabled=cfg!=nullptr;return ESP_OK;
}
inline int gptimer_set_raw_count(gptimer_handle_t,uint64_t) {return ESP_OK;}
inline int gptimer_new_timer(const gptimer_config_t*,gptimer_handle_t* timer) {
  *timer=reinterpret_cast<void*>(1);return ESP_OK;
}
inline int gptimer_register_event_callbacks(gptimer_handle_t,const gptimer_event_callbacks_t*,void*) {return ESP_OK;}
inline int gptimer_enable(gptimer_handle_t) {return ESP_OK;}
inline int gptimer_start(gptimer_handle_t) {return ESP_OK;}
