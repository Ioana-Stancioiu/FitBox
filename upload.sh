#!/bin/bash

PORT=/dev/ttyUSB0
BOARD=nano
CPU=atmega328old

arduino --board arduino:avr:$BOARD:cpu=$CPU --port $PORT --upload FitBox.ino
