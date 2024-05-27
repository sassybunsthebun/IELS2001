# Design project for IELS2001 spring 2024

This repository serves as an overview over the changes made to the repository as well as a guide for how the code works and how to implement the code yourself. 

## What does the code do? 

This is a code made up of three different segments. One of the three modules, "zumobil" is a code dedicated to the Zumo32u4 made by Pololu. The zumo car version used is the one with OLED screen, which can communicate to other devices via I2C. It is the master device and it requests information from the ESP32 connected to it and sends information to it. This ESP32 is also connected to three different sensors, a temperature sensor, a pressure sensor and a GPS. The averages of each of these sensor values are sent to a raspberry pi via MQTT which then displays the information on to a Node-RED website. Another mpdule "kontroller" acts as a controller with a joystick, and an ESP32 sends the direction the Zumo32u4 is to drive in, or if it is to follow a taped up line on the ground. This information is sent to the ESP32 connected to the car which sends the information to the Zumo32u4. 

## How do I set this up? 

You will need a Raspberry pi with mosquitto or some other MQTT broker installed and running and Node-RED installed. You will need two ESP32s and a Zumo32u4 with I2C communication capability. You will have to set up the following Node-RED flow in this link(insert link) as well as set up the MQTT broker to subscribe on the proper topics. 

The joystick used is the Parallax Inc 2-Axis Joystick Module. To set up the ESP32 with the joystick, you may follow the following circuit diagram: 

![kontroller](https://cdn.discordapp.com/attachments/324564514119090177/1243812190889443358/kontroller_kretsdiagram.png?ex=6652d5df&is=6651845f&hm=541aec8457c48a22e2cf8c1c798c708618e641432227ab58eb42bb555b584055&)

The GPS used is the Ultimate GPS Breackout Board by Adafruit. To set up the Zumo32u4 and ESP32 module with the attached sensors, you may follow the following circuit diagram: 

(insert)

Then you may upload the corresponding code to the corresponding devices, and you should be good to go!

## Got into any issues? 

No worries, we've got your back. Get in contact with us and we will help you with any issues or questions and concerns you may have.
