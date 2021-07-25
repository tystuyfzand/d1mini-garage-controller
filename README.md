Garage Controller for Wemos D1 Mini
-----------------------------------

This is a simple project to control garage doors (like a smart controller) for a Wemos D1 Mini.

It is meant to be a cheap, easy to use alternative to existing implementations (like Genie's Aladdin Connect and Chamberlain's MyQ) if you're comfortable soldering and working with small electronics. Total cost should be somewhere around $15 (even less if you get the relay/sensors cheaper).

Note that it does not follow any basic safety regulations that commercial versions require, such as a warning sound/light - but if you don't have kids around or don't really care, this is for you. Please keep the obstruction sensors in place though!

Want a case? View the case I modified and use on [Thingiverse](https://www.thingiverse.com/thing:4916393)!

How it works
============

Using the [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient) library, it publishes available devices to the MQTT server used by HomeAssistant. It can report statuses based on the pin input, as well as toggle states via relay.

Configuration
=============

Copy `data/config-example.json` to `data/config.json` and add in your WiFi SSID, Passphrase, and MQTT Server. The pins in there are suggested, but do not use D3/D4 as they are used as boot pins and toggle randomly on boot.

After updating this, open the project in VS Code or whatever is supported by PlatformIO and build the filesystem image, then build the program itself. After building both, upload both the filesystem image and code to your D1 Mini.

Cost Breakdown
==============

Total cost: $18.04

| Item                 | Cost  | Link                                                    |
|----------------------|-------|---------------------------------------------------------|
| D1 Mini              | $3.07 | [Amazon](https://www.amazon.com/gp/product/B081PX9YFV/) |
| SainSmart 2CH Relay  | $7.99 | [Amazon](https://www.amazon.com/gp/product/B0057OC6D8/) |
| Magnetic Reed Switch | $6.98 | [Amazon](https://www.amazon.com/gp/product/B0154PTDFI/) |
