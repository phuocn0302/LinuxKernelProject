#!/bin/bash
DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus notify-send "Reminder" "This is a notification every minute."
