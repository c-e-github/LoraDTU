# Device Profiles

It is possible to change hardware settings like pin assignments or ethernet support using a json file. The json file can be uploaded using the configuration management in the web interface. Just select "Pin Mapping (pin_mapping.json)" in the recovery section.

When the file is uploaded the ESP performs a reboot. This is required as the pin settings could have changed within the file. By default all the pin assignments are used as compiled into the firmware.

To change the device profile, navigate to the "Device Manager" and selected the appropriate profile. You can see the current (Active) and the new (Selected) in assignment in the table below the combobox.

## Structure of the json file

```json
[
    {
        "name": "ttgo-lora32-v21",
        "nrf24": {
            "miso": 12,
            "mosi": 13,
            "clk": 14,
            "irq": 0,
            "en": 4,
            "cs": 15
        },
        "eth": {
            "enabled": false,
            "phy_addr": 0,
            "power": -1,
            "mdc": 23,
            "mdio": 18,
            "type": 0,
            "clk_mode": 0
        }
        "display": {
            "type": 2,
            "data": 21,
            "clk": 22
        }
    },
    {
        "name": "Your ESP32 Board",
        "nrf24": {
            "miso": 15,
            "mosi": 2,
            "clk": 14,
            "irq": 13,
            "en": 16,
            "cs": 5
        },
        "eth": {
            "enabled": true,
            "phy_addr": 0,
            "power": 12,
            "mdc": 23,
            "mdio": 18,
            "type": 0,
            "clk_mode": 3
        }
    }
]
```

The json file can contain multiple profiles. Each profile requires a name and different parameters. If one parameter is not set, the default value, as compiled into the firmware is used.

## Implemented configuration values

| Parameter     | Data Type | Description |
| ------------- | --------- | ----------- |
| name          | string    | Unique name of the profile (max 63 characters) |
| nrf24.miso    | number    | MISO Pin |
| nrf24.mosi    | number    | MOSI Pin |
| nrf24.clk     | number    | Clock Pin |
| nrf24.irq     | number    | Interrupt Pin |
| nrf24.en      | number    | Enable Pin |
| nrf24.cs      | number    | Chip Select Pin |
| cmt.sdio      | number    | SDIO Pin |
| cmt.clk       | number    | CLK Pin |
| cmt.cs        | number    | CS Pin |
| cmt.fcs       | number    | FCS Pin |
| cmt.gpio2     | number    | GPIO2 Pin (optional) |
| cmt.gpio3     | number    | GPIO3 Pin (optional) |
| eth.enabled   | boolean   | Enable/Disable the ethernet stack |
| eth.phy_addr  | number    | Unique PHY addr |
| eth.power     | number    | Power Pin (if available). Use -1 for not assigned pins. |
| eth.mdc       | number    | Serial Management Interface MDC Pin. Use -1 for not assigned pins. |
| eth.mdio      | number    | Serial Management Interface MDIO Pin. Use -1 for not assigned pins. |
| eth.type      | number    | Possible values:<br>* 0 = ETH_PHY_LAN8720<br>* 1 = ETH_PHY_TLK110<br>* 2 = ETH_PHY_RTL8201<br>* 3 = ETH_PHY_DP83848<br>* 4 = ETH_PHY_DM9051<br>* 5 = ETH_PHY_KSZ8041<br>* 6 = ETH_PHY_KSZ8081 |
| eth.clk_mode  | number    | Possible values:<br>* 0 = ETH_CLOCK_GPIO0_IN<br>* 1 = ETH_CLOCK_GPIO0_OUT<br>* 2 = ETH_CLOCK_GPIO16_OUT<br>* 3 = ETH_CLOCK_GPIO17_OUT |
| display.type  | number    | Specify type of display. Possible values:<br>* 0 = None (default)<br>* 1 = PCD8544 <br>* 2 = SSD1306 <br>* 3 = SH1106 |
| display.data  | number    | Data Pin (e.g. SDA for i2c displays) required for all displays. Use 255 for not assigned pins. |
| display.clk   | number    | Clock Pin (e.g. SCL for i2c displays) required for SSD1306 and SH1106. Use 255 for not assigned pins. |
| display.cs    | number    | Chip Select Pin required for PCD8544. Use 255 for not assigned pins. |
| display.reset | number    | Reset Pin required for PCD8544, optional for all other displays. Use 255 for not assigned pins. |
| led.led0      | number    | LED pin for network indication. Blinking = WLAN connected but NTP disconnected. On = WLAN, NTP connected. Off = Network not connected |
| led.led1      | number    | LED pin for inverter indication. Off = Inverter reachable & producing. Blinking = Inverter reachable but not producing. On = Inverter is not reachable. |
