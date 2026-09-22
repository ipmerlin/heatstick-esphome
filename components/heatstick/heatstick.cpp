#include "heatstick.h"
#include <algorithm>
#include <cmath>
#include "esphome/core/log.h"

namespace esphome { namespace heatstick {
static const char *const TAG="heatstick";
static const std::vector<uint8_t> GET_STATE{0xAA,0x03,0x08,0x10,0x04,0xC9};
static const char *const POWERS[]={nullptr,"Level 1","Level 2","Level 3","Level 4","Level 5","Auto"};
static const char *const MODES[]={nullptr,"Comfort","Night","No frost"};

void HeatStick::setup(){ rx_.reserve(MAX_PACKET_SIZE); request_state_(); }
void HeatStick::loop(){
  while(available()){ uint8_t v; if(read_byte(&v)) consume_byte_(v); }
  const uint32_t now=millis(), interval=has_state_?request_interval_:std::min(request_interval_,uint32_t{5000});
  if(now-last_request_>=interval) request_state_();
}
void HeatStick::dump_config(){
  ESP_LOGCONFIG(TAG,"COOLRF HeatStick:");
  ESP_LOGCONFIG(TAG,"  State received: %s",YESNO(has_state_));
  ESP_LOGCONFIG(TAG,"  Valid/invalid packets: %" PRIu32 "/%" PRIu32,valid_packets_,invalid_packets_);
}
void HeatStick::consume_byte_(uint8_t v){
  if(rx_.empty()){ if(v==0xAA) rx_.push_back(v); return; }
  rx_.push_back(v); if(rx_.size()<2) return;
  const size_t expected=size_t(rx_[1])+3;
  if(expected<4||expected>MAX_PACKET_SIZE){ ESP_LOGW(TAG,"Invalid packet length: %u",rx_[1]); invalid_packets_++; rx_.clear(); return; }
  if(rx_.size()==expected){ process_packet_(rx_); rx_.clear(); }
}
void HeatStick::process_packet_(const std::vector<uint8_t>& p){
  if(p.back()!=checksum_(p)){ ESP_LOGW(TAG,"Checksum mismatch (%u bytes)",p.size()); invalid_packets_++; return; }
  valid_packets_++;
  if(p.size()!=STATE_SIZE) return;
  ESP_LOGI(TAG,"RX cmd=0x%02X status=%u target=%u mode=%u selected_power=%u actual_power=%u display=%u",
           p[CMD],p[STATUS],p[TARGET_TEMP],p[MODE],p[POWER],p[POWER_ACTUAL],p[DISPLAY_INDEX]);
  std::copy_n(p.begin(),STATE_SIZE,state_.begin()); state_[CMD]=CMD_SET; has_state_=true; publish_state_();
}
void HeatStick::publish_state_(){
  // publish_state() updates Home Assistant only. It deliberately does not call
  // control(), preventing the feedback loop in the original implementation.
  if(climate_){ climate_->mode=state_[STATUS]==0?climate::CLIMATE_MODE_OFF:climate::CLIMATE_MODE_HEAT; climate_->target_temperature=state_[TARGET_TEMP]; climate_->current_temperature=state_[CURRENT_TEMP]; climate_->publish_state(); }
  if(current_temperature_sensor_) current_temperature_sensor_->publish_state(state_[CURRENT_TEMP]);
  if(target_temperature_sensor_) target_temperature_sensor_->publish_state(state_[TARGET_TEMP]);
  if(power_level_sensor_) power_level_sensor_->publish_state(state_[POWER_ACTUAL]);
  const uint8_t p=state_[POWER]; if(power_select_){ if(p>=1&&p<=6) power_select_->publish_state(POWERS[p]); else ESP_LOGW(TAG,"Unknown power: 0x%02X",p); }
  const uint8_t m=state_[MODE]; if(mode_select_){ if(m>=1&&m<=3) mode_select_->publish_state(MODES[m]); else ESP_LOGW(TAG,"Unknown mode: 0x%02X",m); }
  if(display_switch_) display_switch_->publish_state(state_[DISPLAY_INDEX]==0);
}
void HeatStick::request_state_(){ write_array(GET_STATE); last_request_=millis(); }
void HeatStick::send_state_(){
  if(!has_state_){ ESP_LOGW(TAG,"State unknown; command not sent"); request_state_(); return; }
  std::vector<uint8_t> p(state_.begin(),state_.end()); p.back()=checksum_(p); state_.back()=p.back();
  ESP_LOGI(TAG,"TX cmd=0x%02X status=%u target=%u mode=%u selected_power=%u actual_power=%u display=%u",
           p[CMD],p[STATUS],p[TARGET_TEMP],p[MODE],p[POWER],p[POWER_ACTUAL],p[DISPLAY_INDEX]);
  write_array(p);
}
bool HeatStick::update_byte_(size_t i,uint8_t v){ if(!has_state_||state_[i]==v) return false; state_[i]=v; return true; }
uint8_t HeatStick::checksum_(const std::vector<uint8_t>& p){ uint8_t sum=0; for(size_t i=0;i+1<p.size();i++) sum+=p[i]; return sum; }
void HeatStick::set_power(bool on){ if(update_byte_(STATUS,on?1:0)) send_state_(); }
void HeatStick::set_target_temperature(float v){ auto t=uint8_t(std::clamp(std::lround(v),0L,30L)); if(update_byte_(TARGET_TEMP,t)) send_state_(); }
void HeatStick::set_power_level(const std::string& v){
  uint8_t n=0; for(uint8_t i=1;i<=6;i++) if(v==POWERS[i]) n=i;
  ESP_LOGI(TAG,"Power selection requested: %s (code %u)",v.c_str(),n);
  if(n&&update_byte_(POWER,n)) send_state_();
}
void HeatStick::set_operating_mode(const std::string& v){ uint8_t n=0; for(uint8_t i=1;i<=3;i++) if(v==MODES[i]) n=i; if(n&&update_byte_(MODE,n)) send_state_(); }
void HeatStick::set_display(bool on){ if(update_byte_(DISPLAY_INDEX,on?0:1)) send_state_(); }
climate::ClimateTraits HeatStickClimate::traits(){ climate::ClimateTraits t; t.set_supported_modes({climate::CLIMATE_MODE_OFF,climate::CLIMATE_MODE_HEAT}); t.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE); t.set_visual_min_temperature(0); t.set_visual_max_temperature(30); t.set_visual_temperature_step(1); return t; }
void HeatStickClimate::control(const climate::ClimateCall& c){ if(c.get_mode().has_value()) parent_->set_power(*c.get_mode()!=climate::CLIMATE_MODE_OFF); if(c.get_target_temperature().has_value()) parent_->set_target_temperature(*c.get_target_temperature()); }
void HeatStickPowerSelect::control(const std::string& v){ parent_->set_power_level(v); }
void HeatStickModeSelect::control(const std::string& v){ parent_->set_operating_mode(v); }
void HeatStickDisplaySwitch::write_state(bool v){ parent_->set_display(v); }
} }
