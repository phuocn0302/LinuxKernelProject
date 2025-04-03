# Linux Controller to Mouse Kernel

## A kernel that turn your controller to a mouse (kind of)

## How to run

``` sh
# Compile module
make

# Enable module
sudo insmod controller_module.ko

# Disable module
sudo rmmod controller_module

# See log
sudo dmseg -wH
```

## Usage

| Joystick   | Mouse          |
| ---------- | -------------- |
| L Joystick | Mouse Movement |
| R Joystick | Scroll Wheel   |
| LS Button  | Decrease Speed |
| RS Button  | Increase Speed |
| A          | Left Click     |
| B          | Right Click    |
| X          | ESC            |
| Y          | Enter          |
| LB         | Backward       |
| RB         | Forward        |
| D-Pad      | Arrow Key      |
