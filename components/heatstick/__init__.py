import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor, select, switch, uart
from esphome.const import CONF_CURRENT_TEMPERATURE, CONF_ID, CONF_MODE, CONF_POWER, CONF_TARGET_TEMPERATURE

CODEOWNERS = ["@coolrf"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["climate", "sensor", "select", "switch"]

CONF_DISPLAY = "display"
CONF_POWER_LEVEL = "power_level"
CONF_REQUEST_INTERVAL = "request_interval"

ns = cg.esphome_ns.namespace("heatstick")
HeatStick = ns.class_("HeatStick", cg.Component, uart.UARTDevice)
HeatStickClimate = ns.class_("HeatStickClimate", climate.Climate)
HeatStickPowerSelect = ns.class_("HeatStickPowerSelect", select.Select)
HeatStickModeSelect = ns.class_("HeatStickModeSelect", select.Select)
HeatStickDisplaySwitch = ns.class_("HeatStickDisplaySwitch", switch.Switch)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HeatStick),
    cv.Required("climate"): climate.climate_schema(HeatStickClimate),
    cv.Required(CONF_POWER): select.select_schema(HeatStickPowerSelect),
    cv.Required(CONF_MODE): select.select_schema(HeatStickModeSelect),
    cv.Required(CONF_DISPLAY): switch.switch_schema(HeatStickDisplaySwitch),
    cv.Required(CONF_CURRENT_TEMPERATURE): sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=0, device_class="temperature", state_class="measurement"),
    cv.Required(CONF_TARGET_TEMPERATURE): sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=0, device_class="temperature", state_class="measurement"),
    cv.Required(CONF_POWER_LEVEL): sensor.sensor_schema(accuracy_decimals=0, icon="mdi:radiator"),
    cv.Optional(CONF_REQUEST_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    hub = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(hub, config)
    await uart.register_uart_device(hub, config)
    cg.add(hub.set_request_interval(config[CONF_REQUEST_INTERVAL]))

    climate_var = cg.new_Pvariable(config["climate"][CONF_ID], hub)
    await climate.register_climate(climate_var, config["climate"])
    cg.add(hub.set_climate(climate_var))

    power = cg.new_Pvariable(config[CONF_POWER][CONF_ID], hub)
    await select.register_select(power, config[CONF_POWER], options=["Level 1", "Level 2", "Level 3", "Level 4", "Level 5", "Auto"])
    cg.add(hub.set_power_select(power))
    mode = cg.new_Pvariable(config[CONF_MODE][CONF_ID], hub)
    await select.register_select(mode, config[CONF_MODE], options=["Comfort", "Night", "No frost"])
    cg.add(hub.set_mode_select(mode))
    display = cg.new_Pvariable(config[CONF_DISPLAY][CONF_ID], hub)
    await switch.register_switch(display, config[CONF_DISPLAY])
    cg.add(hub.set_display_switch(display))

    value = await sensor.new_sensor(config[CONF_CURRENT_TEMPERATURE])
    cg.add(hub.set_current_temperature_sensor(value))
    value = await sensor.new_sensor(config[CONF_TARGET_TEMPERATURE])
    cg.add(hub.set_target_temperature_sensor(value))
    value = await sensor.new_sensor(config[CONF_POWER_LEVEL])
    cg.add(hub.set_power_level_sensor(value))
