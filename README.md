# pfd-ts

a project to replace the WoPet electronics with a wifi enabled MCU for enhanced functionality

This repalces my pfd project which was a web server based API host that listened for requests. 
There are security issues created by hosting a web service on an IoT device on a private 
network when you attempt to make the device accessable via the internet. I changed the design
to use a web services to store the state, configuration, and trigger data. Thingspeak has libraries 
for devices running ESP82x modules that make the design simple and safe. 

I also added a PCF8574 i2c i/o expander board (https://www.amazon.com/dp/B08YNBZQ5S) to 
fix issues with using some of the GPIOs on the NodeMCU board that don't play well with 
digitalRead() / digitalWrite(). 

I'll post images of the new wiring and device layout when I get time. I may create a PCB for this 
project since it's a good canidate for a custom board and I've been meaning to try my hand at PCB 
design for a while.
