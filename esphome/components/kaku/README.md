# KAKU Protocol External Component

This is an external component for ESPHome that provides support for the KAKU remote control protocol.
Algorithm based on https://github.com/1technophile/NewRemoteSwitch from 1technophile.  

## Installation

Add the following to your ESPHome configuration file:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [kaku]
```

Or, if you have this component in a Git repository:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/FrancoisAltieri/esphome
      ref: main
    components: [kaku]
```

## Usage

### Remote Receiver

```yaml
kaku: 
  # empty component, forces loading new protocol to remote_base
remote_receiver:
  pin: GPIO14
  
  triggers:
    - platform: kaku
      name: "Bedroom Light"
      address: 0x123456
      group: false
      switch_type: 0  # 0=off, 1=on, 2=dimming
      unit: 0
```

### Remote Transmitter

```yaml
kaku: 
  # empty component, forces loading new protocol to remote_base
remote_transmitter:
  pin: GPIO32
  carrier_duty_percent: 50

on_boot:
  then:
    - remote_transmitter.transmit_kaku:
        address: 0x123456
        group: false
        switch_type: 1
        unit: 0
```

### Binary Sensor

```yaml
binary_sensor:
  - platform: remote_receiver
    kaku:
      address: 0x123456
      group: false
      switch_type: 0
      unit: 0
    name: "Light Binary Sensor"
```

## Configuration Variables

- **period** (*Optional*, int): The period of the signal in microseconds. Default is 0 (auto-detect).
- **address** (*Required*, int): The 26-bit KAKU address.
- **group** (*Optional*, boolean): Group bit. Default is `false`.
- **switch_type** (*Optional*, int): The switch type (0=off, 1=on, 2=dim). Default is `0`.
- **unit** (*Optional*, int): 4-bit unit number. Default is `0`.
- **dim_level_present** (*Optional*, boolean): Whether the dim level is present. Default is `false`.
- **dim_level** (*Optional*, int): 4-bit dim level value. Default is `0`.

## See Also

- [ESPHome Remote Base](https://esphome.io/components/remote_base/)
- [ESPHome External Components](https://esphome.io/components/external_components/)
