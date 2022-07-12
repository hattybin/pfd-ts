# pfd-ts

a project to replace the WoPet electronics with a wifi enabled MCU for enhanced functionality

This repalces my pfd project which was a web server based API host that listened for requests. 
There aresecurity issues created by hosting a web service on an IoT device on a private 
network when you attempt to make the device accessable via the internet. I decided to port the
project to a more IoT compatible by using web services to store the state, configuration,
and trigger data on a IoT data platform such as thingspeak. There are libraries for devices 
running ESP82x modules that make the design simple and safe. 
