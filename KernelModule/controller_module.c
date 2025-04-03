#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/timer.h>

#define MAX_CONTROLLER_VAL 32768

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nhom. 8");
MODULE_DESCRIPTION("Controller Mouse Module");
MODULE_VERSION("4.2.0");

struct input_dev *virtual_device;
struct input_handle *current_controller_handle;

int joystick_x = 0;
int joystick_y = 0;
int joystick_rx = 0;
int joystick_ry = 0;

int deadzone = 5000; // total of MAX_CONTROLLER_VAL
int mouse_min_sens = 4;
int mouse_max_sens = 32;
int mouse_sens = 8;
int scroll_sentivity_hires = 30;

struct timer_list timer; // for checking joystick value
int timer_interval = 10; // in ms

void controller_handle_event(struct input_handle *handle, unsigned int type,
                             unsigned int code, int value);
int controller_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id);
void controller_disconnect(struct input_handle *handle);

void on_timer_callback(struct timer_list *t);

int create_virtual_device(void);

void virtual_dev_press_button(unsigned int key);

const struct input_device_id controller_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
        .evbit = {BIT_MASK(EV_ABS)},
        .absbit = {BIT_MASK(ABS_X) | BIT_MASK(ABS_Y) | BIT_MASK(ABS_RX) |
                   BIT_MASK(ABS_RY)},
    },
    {}};

struct input_handler controller_handler = {
    .event = controller_handle_event,
    .connect = controller_connect,
    .disconnect = controller_disconnect,
    .name = "controller",
    .id_table = controller_ids,
};

void on_timer_callback(struct timer_list *t) {
  if (!virtual_device) {
    return;
  }

  int x_move = 0;
  int y_move = 0;
  int scroll_x = 0;
  int scroll_y = 0;

  if (abs(joystick_x) > deadzone) {
    x_move = (joystick_x > 0)
                 ? (joystick_x - deadzone) * mouse_sens / MAX_CONTROLLER_VAL
                 : (joystick_x + deadzone) * mouse_sens / MAX_CONTROLLER_VAL;
    ;
  }

  if (abs(joystick_y) > deadzone) {
    y_move = (joystick_y > 0)
                 ? (joystick_y - deadzone) * mouse_sens / MAX_CONTROLLER_VAL
                 : (joystick_y + deadzone) * mouse_sens / MAX_CONTROLLER_VAL;
  }

  if (abs(joystick_rx) > deadzone) {
    scroll_x = (joystick_rx > 0)
                   ? (joystick_rx - deadzone) * 2 / MAX_CONTROLLER_VAL
                   : (joystick_rx + deadzone) * 2 / MAX_CONTROLLER_VAL;
  }

  if (abs(joystick_ry) > deadzone) {
    scroll_y = (joystick_ry > 0)
                   ? (joystick_ry - deadzone) * 2 / MAX_CONTROLLER_VAL
                   : (joystick_ry + deadzone) * 2 / MAX_CONTROLLER_VAL;
    scroll_y *= -1;
  }

  if (x_move != 0 || y_move != 0) {
    input_report_rel(virtual_device, REL_X, x_move);
    input_report_rel(virtual_device, REL_Y, y_move);
    input_sync(virtual_device);
    printk(KERN_INFO "MOUSE: X=%d, Y=%d\n", x_move, y_move);
  }

  if (scroll_x != 0 || scroll_y != 0) {
    input_report_rel(virtual_device, REL_HWHEEL, scroll_x);
    input_report_rel(virtual_device, REL_WHEEL, scroll_y);

    input_report_rel(virtual_device, REL_HWHEEL_HI_RES,
                     scroll_x * scroll_sentivity_hires);
    input_report_rel(virtual_device, REL_WHEEL_HI_RES,
                     scroll_y * scroll_sentivity_hires);

    input_sync(virtual_device);
    printk(KERN_INFO "WHEEL: X=%d, Y=%d\n", scroll_x, scroll_y);
  }

  mod_timer(&timer, jiffies + msecs_to_jiffies(timer_interval));
}

void virtual_dev_press_button(unsigned int key) {
  input_report_key(virtual_device, key, 1);
  input_sync(virtual_device);

  input_report_key(virtual_device, key, 0);
  input_sync(virtual_device);
}

void controller_handle_event(struct input_handle *handle, unsigned int type,
                             unsigned int code, int value) {
  if (!virtual_device)
    return;

  if (type == EV_ABS) {
    switch (code) {
    case ABS_X:
      joystick_x = value;
      break;

    case ABS_Y:
      joystick_y = value;
      break;

    case ABS_RX:
      joystick_rx = value;
      break;

    case ABS_RY:
      joystick_ry = value;
      break;

    case ABS_HAT0Y:
      if (value == -1) {
        virtual_dev_press_button(KEY_UP);
        printk(KERN_INFO "Key: UP\n");
      } else if (value == 1) {
        virtual_dev_press_button(KEY_DOWN);
        printk(KERN_INFO "Key: DOWN\n");
      }
      break;

    case ABS_HAT0X:
      if (value == -1) {
        virtual_dev_press_button(KEY_LEFT);
        printk(KERN_INFO "Key: LEFT\n");
      } else if (value == 1) {
        virtual_dev_press_button(KEY_RIGHT);
        printk(KERN_INFO "Key: RIGHT\n");
      }
      break;
    }
  }

  if (type == EV_KEY) {
    switch (code) {
    case BTN_A:
      input_report_key(virtual_device, BTN_LEFT, value);
      printk(KERN_INFO "LEFT Mouse Button: %d\n", value);
      break;

    case BTN_B:
      input_report_key(virtual_device, BTN_RIGHT, value);
      printk(KERN_INFO "RIGHT Mouse Button: %d\n", value);
      break;

    case BTN_X:
      if (value == 1) {
        virtual_dev_press_button(KEY_ESC);
        printk(KERN_INFO "Key: ESC\n");
      }
      break;

    case BTN_Y:
      if (value == 1) {
        virtual_dev_press_button(KEY_ENTER);
        printk(KERN_INFO "Key: ENTER\n");
      }
      break;

    case BTN_TL:
      if (value == 1) {
        virtual_dev_press_button(BTN_SIDE);
        printk(KERN_INFO "BACKWARD Mouse Button\n");
      }
      break;

    case BTN_TR:
      if (value == 1) {
        virtual_dev_press_button(BTN_EXTRA);
        printk(KERN_INFO "FORWARD Mouse Button\n");
      }
      break;

    case BTN_THUMBL:
      if (value == 1) {
        mouse_sens = max(mouse_sens - 2, mouse_min_sens);
      }
      break;

    case BTN_THUMBR:
      if (value == 1) {
        mouse_sens = min(mouse_sens + 2, mouse_max_sens);
      }
      break;
    }

    input_sync(virtual_device);
  }
}

int controller_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id) {
  struct input_handle *handle;

  handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);

  handle->dev = dev;
  handle->handler = handler;
  handle->name = "controller";

  input_register_handle(handle);
  input_open_device(handle);

  current_controller_handle = handle;

  printk(KERN_ALERT "Controller connected to %s!\n", dev->name);
  return 0;
}

void controller_disconnect(struct input_handle *handle) {
  if (handle == current_controller_handle) {
    current_controller_handle = NULL;
  }

  input_close_device(handle);
  input_unregister_handle(handle);
  kfree(handle);
}

int create_virtual_device(void) {
  virtual_device = input_allocate_device();
  virtual_device->name = "Controller Virtual Mouse";
  virtual_device->id.bustype = BUS_VIRTUAL;

  __set_bit(EV_KEY, virtual_device->evbit);
  __set_bit(EV_REL, virtual_device->evbit);

  __set_bit(REL_X, virtual_device->relbit);
  __set_bit(REL_Y, virtual_device->relbit);
  __set_bit(REL_WHEEL, virtual_device->relbit);
  __set_bit(REL_HWHEEL, virtual_device->relbit);
  __set_bit(REL_WHEEL_HI_RES, virtual_device->relbit);
  __set_bit(REL_HWHEEL_HI_RES, virtual_device->relbit);

  __set_bit(BTN_LEFT, virtual_device->keybit);
  __set_bit(BTN_RIGHT, virtual_device->keybit);
  __set_bit(BTN_MIDDLE, virtual_device->keybit);

  __set_bit(BTN_SIDE, virtual_device->keybit);
  __set_bit(BTN_EXTRA, virtual_device->keybit);

  __set_bit(KEY_UP, virtual_device->keybit);
  __set_bit(KEY_DOWN, virtual_device->keybit);
  __set_bit(KEY_LEFT, virtual_device->keybit);
  __set_bit(KEY_RIGHT, virtual_device->keybit);

  __set_bit(KEY_ENTER, virtual_device->keybit);
  __set_bit(KEY_ESC, virtual_device->keybit);

  input_register_device(virtual_device);

  return 0;
}

static int __init controller_module_init(void) {
  create_virtual_device();
  int input;
  input = input_register_handler(&controller_handler);

  timer_setup(&timer, on_timer_callback, 0);
  mod_timer(&timer, jiffies + msecs_to_jiffies(timer_interval));

  printk(KERN_ALERT "Controller Module Loaded: %d \n", input);
  return 0;
}

static void __exit controller_module_exit(void) {
  del_timer_sync(&timer);

  input_unregister_handler(&controller_handler);

  if (virtual_device) {
    input_unregister_device(virtual_device);
  }

  printk(KERN_ALERT "Controller Module Exited \n");
}

module_init(controller_module_init);
module_exit(controller_module_exit);
