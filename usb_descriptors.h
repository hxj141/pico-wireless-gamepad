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
 */
#include "common/tusb_common.h"
#include "device/usbd.h"

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_CONSUMER_CONTROL,
  REPORT_ID_GAMEPAD,
  REPORT_ID_COUNT
};

// Gamepad Report Descriptor Template
// with 16 buttons and 2 joysticks with following layout
// | Button Map (2 bytes) |  X | Y | Z | Rz
#define GAMECON_REPORT_DESC_GAMEPAD(...)                   \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                \
        HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),              \
        HID_COLLECTION(HID_COLLECTION_APPLICATION),        \
        __VA_ARGS__                                        \
            HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),         \
        HID_USAGE_MIN(1),                                  \
        HID_USAGE_MAX(16),                                 \
        HID_LOGICAL_MIN(0),                                \
        HID_LOGICAL_MAX(1),                                \
        HID_REPORT_COUNT(16),                              \
        HID_REPORT_SIZE(1),                                \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),            \
        HID_LOGICAL_MIN(0x00),                             \
        HID_LOGICAL_MAX_N(0x00ff, 2),                      \
        HID_USAGE(HID_USAGE_DESKTOP_Z),                    \
        HID_USAGE(HID_USAGE_DESKTOP_RX),                   \
        HID_USAGE(HID_USAGE_DESKTOP_RY),                   \
        HID_USAGE(HID_USAGE_DESKTOP_RZ),                   \
        HID_REPORT_COUNT(4),                               \
        HID_REPORT_SIZE(8),                                \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        HID_COLLECTION_END
#endif /* USB_DESCRIPTORS_H_ */

