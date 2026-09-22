# Installation through ESPHome Device Builder

Extract the archive contents directly into `/config/esphome/` on the Home
Assistant host. The resulting paths must be:

```text
/config/esphome/coolrf-heatstick.yaml
/config/esphome/components/heatstick/__init__.py
/config/esphome/components/heatstick/heatstick.h
/config/esphome/components/heatstick/heatstick.cpp
```

Add these entries to the existing `/config/esphome/secrets.yaml` file (or edit
their existing values):

```yaml
wifi_ssid: "your network name"
wifi_password: "your network password"
fallback_password: "your fallback access point password"
```

Restart ESPHome Device Builder or refresh its dashboard. Open the
`ballu-heatstick` device and select `Install`, then `Wirelessly`. If ESPHome
cannot resolve the device hostname, temporarily add `use_address:` with the
device's current IP address under `wifi:`. Do not interrupt heater power during
the update.
