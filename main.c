/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp/board.h"
#include "tusb.h"
#include "nrf_receiver.h"


#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
int button_coord(int button_raw);
int dpad_coord(uint16_t val);
int *button_parse(uint16_t val);
int js_coord(uint8_t val);


/*------------- MAIN -------------*/
int main(void)
{
  //Initialize TUSB, NRF, and board
  board_init();
  tusb_init();
  nrf_rx_init();
  

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn, uint8_t *buf)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;
      

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

     //Process the Rx
      uint16_t rx_buttons = ((uint16_t)buf[0] << 8) | buf[1]; //Send buttons as 16-bit
      struct RxReport rx = {
      	.buttons = button_parse(rx_buttons),
      	.dpad = dpad_coord(rx_buttons),
      	.lstick_x = js_coord(buf[2]),
      	.lstick_y = js_coord(buf[3]),
      	.rstick_x = js_coord(buf[4]),
      	.rstick_y = js_coord(buf[5])
      };

      if ( btn )
      {
      	if (rx.buttons[0] == -1) {
      		report.x = rx.lstick_x;
	      	report.y = rx.lstick_y;
	      	report.rx = rx.rstick_x;
	      	report.ry = rx.rstick_y;
	      	report.hat = rx.dpad;
	      	has_gamepad_key = true;
      	}
      	else {
      		int btn_size = sizeof(rx.buttons);
	      	for (int i = 0; i < btn_size; ++i) {
		      	report.x = rx.lstick_x;
		      	report.y = rx.lstick_y;
		      	report.rx = rx.rstick_x;
		      	report.ry = rx.rstick_y;
		      	report.buttons = button_coord(rx.buttons[i]);
		      	report.hat = rx.dpad;
		        has_gamepad_key = true;
	        }
        }
      }
      else
      {
      	//HAT IS DPAD, START PLANNING
      	
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint8_t *buf;
  int btn = read_rx_report(&buf);
  
  
  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_GAMEPAD, btn, buf);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
// void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
// {
  // (void) instance;
  // (void) len;
// 
  // uint8_t next_report_id = report[0] + 1;
// 
  // if (next_report_id < REPORT_ID_COUNT)
  // {
    // send_hid_report(next_report_id, board_button_read());
  // }
// }

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

//Take JS coord (which is normally 0 to 256 and scale it to -128 to 128)
int js_coord(uint8_t val) {
	// Convert to int, 
	int parse_val = (int) val;
	parse_val = parse_val - 128;
	return parse_val;	
}

//Take button inputs and send as array
int *button_parse(uint16_t val) {
	uint16_t checker = (uint16_t) 1; // 1000 0000 0000 0000
	int array_size;
	int *dio_parse = (int *) malloc(sizeof(int)*11);
	
	// Do an AND shift over to see which buttons are there
	//Our packet will be backwards
	for (int i = 0; i < 11; ++i) {
		if (((int)(checker & val)) == pow(2,i)) {
			dio_parse[array_size] = i;
			array_size++; 
		}
		checker = (checker<<1);
	}
	if (array_size == 0) {
		dio_parse[0] = -1;
	}
	return dio_parse;
}
int button_coord(int button_raw) {
	switch(button_raw) {
		case 0:
			return GAMEPAD_BUTTON_B;
		case 1:
			return GAMEPAD_BUTTON_A;
		case 2:
			return GAMEPAD_BUTTON_Y;
		case 3:
			return GAMEPAD_BUTTON_X;
		case 4:
			return GAMEPAD_BUTTON_A;
		case 5:
			return GAMEPAD_BUTTON_TL;
		case 6:
			return GAMEPAD_BUTTON_TR;
		case 7:
			return GAMEPAD_BUTTON_SELECT;
		case 8:
			return GAMEPAD_BUTTON_START;
		case 9:
			return GAMEPAD_BUTTON_THUMBL;
		case 10:
			return GAMEPAD_BUTTON_THUMBR;
	}
}

int dpad_coord(uint16_t val) {
	uint16_t checker = (uint16_t) 2048; // 0000 1000 0000 0000
	int net_y = 0;
	int net_x = 0;
	
	// Case 11 - UP
	// Case 12 - RIGHT
	// Case 13 - DOWN
	// Case 14 - LEFT

	// Do an AND shift over to see which buttons are there
	//Our packet will be backwards

	//Figure out how each coordinate cancels
	for (int i = 11; i < 16; ++i) {
		if (((int)(checker & val)) == pow(2,i)) {
			switch(i) {
				case 11:
					net_y++;
				case 12:
					net_x++;
				case 13:
					net_y--;
				case 14:
					net_x--;
			}			
		}
		checker = (checker<<1);
	} 
	int t_net = net_y + net_x;

	switch (t_net) {
		case -2:
			return GAMEPAD_HAT_DOWN_LEFT;
		case -1:
			if (net_y == 0) {
				return GAMEPAD_HAT_LEFT;
			}
			if (net_y == -1) {
				return GAMEPAD_HAT_DOWN;
			}
		case 0:
			if (net_y == -1) {
				return GAMEPAD_HAT_DOWN_RIGHT;
			}
			if (net_y == 0) {
				return GAMEPAD_HAT_CENTERED;
			}
			if (net_y == 1) {
				return GAMEPAD_HAT_UP_LEFT;
			}
		case 1:
			if (net_y == 0) {
				return GAMEPAD_HAT_RIGHT;
			}
			if (net_y == 1) {
				return GAMEPAD_HAT_UP;
			}
		case 2:
			return GAMEPAD_HAT_UP_RIGHT;
	}
}
