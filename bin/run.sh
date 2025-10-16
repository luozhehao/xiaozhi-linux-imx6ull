#!/bin/sh

./lvgl_xiaozhi_gui  >/dev/null 2>&1 &
./sound_app >/dev/null 2>&1 &
./control_center &
