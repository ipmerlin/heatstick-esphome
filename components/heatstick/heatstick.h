#pragma once
#include <array>
#include <string>
#include <vector>
#include "esphome/components/climate/climate.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome { namespace heatstick {
class HeatStick;
class HeatStickClimate : public climate::Climate {
 public: explicit HeatStickClimate(HeatStick *parent) : parent_(parent) {}
 protected: climate::ClimateTraits traits() override; void control(const climate::ClimateCall &call) override; HeatStick *parent_;
};
class HeatStickPowerSelect : public select::Select {
 public: explicit HeatStickPowerSelect(HeatStick *parent) : parent_(parent) {}
 protected: void control(const std::string &value) override; HeatStick *parent_;
};
class HeatStickModeSelect : public select::Select {
 public: explicit HeatStickModeSelect(HeatStick *parent) : parent_(parent) {}
 protected: void control(const std::string &value) override; HeatStick *parent_;
};
class HeatStickDisplaySwitch : public switch_::Switch {
 public: explicit HeatStickDisplaySwitch(HeatStick *parent) : parent_(parent) {}
 protected: void write_state(bool state) override; HeatStick *parent_;
};

class HeatStick : public Component, public uart::UARTDevice {
 public:
  HeatStick() = default;
  void setup() override; void loop() override; void dump_config() override;
  void set_request_interval(uint32_t v) { request_interval_ = v; }
  void set_climate(HeatStickClimate *v) { climate_ = v; }
  void set_power_select(HeatStickPowerSelect *v) { power_select_ = v; }
  void set_mode_select(HeatStickModeSelect *v) { mode_select_ = v; }
  void set_display_switch(HeatStickDisplaySwitch *v) { display_switch_ = v; }
  void set_current_temperature_sensor(sensor::Sensor *v) { current_temperature_sensor_ = v; }
  void set_target_temperature_sensor(sensor::Sensor *v) { target_temperature_sensor_ = v; }
  void set_power_level_sensor(sensor::Sensor *v) { power_level_sensor_ = v; }
  void set_power(bool on); void set_target_temperature(float value);
  void set_power_level(const std::string &value); void set_operating_mode(const std::string &value); void set_display(bool on);
 protected:
  static constexpr size_t STATE_SIZE=15, MAX_PACKET_SIZE=64;
  static constexpr uint8_t CMD=2, STATUS=3, TARGET_TEMP=4, MODE=5, POWER=6, CURRENT_TEMP=10, POWER_ACTUAL=11, DISPLAY_INDEX=13, CMD_SET=0x0A;
  void consume_byte_(uint8_t value); void process_packet_(const std::vector<uint8_t> &packet); void publish_state_();
  void request_state_(); void send_state_(); bool update_byte_(size_t index, uint8_t value);
  static uint8_t checksum_(const std::vector<uint8_t> &packet);
  std::vector<uint8_t> rx_; std::array<uint8_t, STATE_SIZE> state_{}; bool has_state_{false};
  uint32_t request_interval_{60000}, last_request_{0}, valid_packets_{0}, invalid_packets_{0};
  bool pending_auto_{false}; uint32_t auto_transition_at_{0}; uint8_t last_valid_actual_power_{0};
  HeatStickClimate *climate_{nullptr}; HeatStickPowerSelect *power_select_{nullptr}; HeatStickModeSelect *mode_select_{nullptr};
  HeatStickDisplaySwitch *display_switch_{nullptr}; sensor::Sensor *current_temperature_sensor_{nullptr};
  sensor::Sensor *target_temperature_sensor_{nullptr}; sensor::Sensor *power_level_sensor_{nullptr};
};
} }
