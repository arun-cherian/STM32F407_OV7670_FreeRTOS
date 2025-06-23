/*
 * OV7670_control.c
 *
 *  Created on: Jun 7, 2025
 *      Author: Arun
 */




/*
 * OV7670_control.c
 *
 *  Created on: Jun 5, 2025
 *      Author: Arun
 */



/*
 *	==========================================================================
 *   OV7670_control.c
 *   (c) 2014, Petr Machala
 *
 *   Description:
 *   OV7670 camera configuration and control file.
 *   Optimized for 32F429IDISCOVERY board.
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Camera wiring:
 *   3V3		-	3V		;		GND		-	GND
 *   SIOC	-	PB8		;		SIOD	-	PB9
 *   VSYNC -	PB7		;		HREF	-	PA4
 *   PCLK	-	PA6		;		XCLK	-	PA8
 *   D7		-	PE6		;		D6		-	PE5
 *   D5		-	PB6		;		D4		-	PE4
 *   D3		-	PC9		;		D2		-	PC8
 *   D1		-	PC7		;		D0		-	PC6
 *   RESET	-	/			;		PWDN	-	/
 *
 *	==========================================================================
 */


#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "OV7670_reg.h"
#include "OV7670_control.h"
#include "stdio.h"
#include "retarget.h"
#include "stm32f4xx_ll_i2c.h"
// Image buffer
// -------- ADDED FOR UART LOGGING -------- /
extern volatile uint16_t frame_buffer[IMG_ROWS*IMG_COLUMNS];
const uint8_t OV7670_reg_org [OV7670_REG_NUM][2] = {
   {0x12, 0x80},		//Reset registers

   // Image format
   {0x12, 0x14},		//QVGA size, RGB mode

   {0x40, 0xd0},		//RGB565
   {0xb0, 0x84},		//Color mode

   // Hardware window
   {0x11, 0x01},		//PCLK settings, 15fps
   {0x32, 0x80},		//HREF
   {0x17, 0x17},		//HSTART
   {0x18, 0x05},		//HSTOP
   {0x03, 0x0a},		//VREF
   {0x19, 0x02},		//VSTART
   {0x1a, 0x7a},		//VSTOP
   /*
   {0x17,0x16},
   {0x18,0x04},
   {0x32,0x24},
   {0x19,0x02},
   {0x1a,0x7a},
   {0x03,0x0a},
*/
   // Scalling numbers
   {0x70, 0x3a},		//X_SCALING
   {0x71, 0x35},		//Y_SCALING
   /*{0x72, 0x11},		//DCW_SCALING
   {0x73, 0xf0},		//PCLK_DIV_SCALING*/
   /*{0x72, 0x11},
   {0x73, 0xf1},*/
   {0x72, 0x22}, //20190712
   {0x73, 0xf2}, //20190712
   {0xa2, 0x02},		//PCLK_DELAY_SCALING

   // Matrix coefficients
   {0x4f, 0x80},		{0x50, 0x80},
   {0x51, 0x00},		{0x52, 0x22},
   {0x53, 0x5e},		{0x54, 0x80},
   {0x58, 0x9e},{0x56,0x40},

   // Gamma curve values
   {0x7a, 0x20},		{0x7b, 0x10},
   {0x7c, 0x1e},		{0x7d, 0x35},
   {0x7e, 0x5a},		{0x7f, 0x69},
   {0x80, 0x76},		{0x81, 0x80},
   {0x82, 0x88},		{0x83, 0x8f},
   {0x84, 0x96},		{0x85, 0xa3},
   {0x86, 0xaf},		{0x87, 0xc4},
   {0x88, 0xd7},		{0x89, 0xe8},

   // AGC and AEC parameters
   {0xa5, 0x05},		{0xab, 0x07},
   {0x24, 0x95},		{0x25, 0x33},
   {0x26, 0xe3},		{0x9f, 0x78},
   {0xa0, 0x68},		{0xa1, 0x03},
   {0xa6, 0xd8},		{0xa7, 0xd8},
   {0xa8, 0xf0},		{0xa9, 0x90},
   {0xaa, 0x94},		{0x10, 0x00},

   // AWB parameters
   {0x43, 0x0a},		{0x44, 0xf0},
   {0x45, 0x34},		{0x46, 0x58},
   {0x47, 0x28},		{0x48, 0x3a},
   {0x59, 0x88},		{0x5a, 0x88},
   {0x5b, 0x44},		{0x5c, 0x67},
   {0x5d, 0x49},		{0x5e, 0x0e},
   {0x6c, 0x0a},		{0x6d, 0x55},
   {0x6e, 0x11},		{0x6f, 0x9f},

   {0x6a, 0x40},		{0x01, 0x40},
   {0x02, 0x60},		{0x13, 0xe7},

   // Additional parameters
   {0x34, 0x11},		{0x3f, 0x00},
   {0x75, 0x05},		{0x76, 0xe1},
   {0x4c, 0x00},		{0x77, 0x01},
   {0xb8, 0x0a},		{0x41, 0x18},
   {0x3b, 0x12},		{0xa4, 0x88},
   {0x96, 0x00},		{0x97, 0x30},
   {0x98, 0x20},		{0x99, 0x30},
   {0x9a, 0x84},		{0x9b, 0x29},
   {0x9c, 0x03},		{0x9d, 0x4c},
   {0x9e, 0x3f},		{0x78, 0x04},
   {0x0e, 0x61},		{0x0f, 0x4b},
   {0x16, 0x02},		{0x1e, 0x00},
   {0x21, 0x02},		{0x22, 0x91},
   {0x29, 0x07},		{0x33, 0x0b},
   {0x35, 0x0b},		{0x37, 0x1d},
   {0x38, 0x71},		{0x39, 0x2a},
   {0x3c, 0x78},		{0x4d, 0x40},
   {0x4e, 0x20},		{0x69, 0x00},
   {0x6b, 0x3a},		{0x74, 0x10},
   {0x8d, 0x4f},		{0x8e, 0x00},
   {0x8f, 0x00},		{0x90, 0x00},
   {0x91, 0x00},		{0x96, 0x00},
   {0x9a, 0x00},		{0xb1, 0x0c},
   {0xb2, 0x0e},		{0xb3, 0x82},
   {0x4b, 0x01},
/*
   {0x0c, 0x04},
   {0x3e, 0x19}*/
   {0x3e, 0x1a}, //20190712
   {0x0c, 0x04}, //20190712
};
const uint8_t OV7670_QQVGA_160x120_RGB565_FULL[][2] = {

    // --- RESET & CORE FORMAT ---
    {0x12, 0x80},   // COM7:  Reset all registers to default values.
    {0x12, 0x04},   // COM7:  Select QVGA resolution preset and RGB color space.
    {0x40, 0xD0},   // COM15: Set output format to RGB565 (Full 0-255 range).
    {0xb0, 0x84},   // Reserved register, but commonly set.
    {0x11, 0x01},   // CLKRC: PCLK prescaler (Input clock / 2). A value of 0x01 is common for ~15fps.

    // --- HARDWARE WINDOWING (Standard QVGA 320x240 Window) ---
    {0x17, 0x13},   // HSTART: Horizontal Frame Start edge
    {0x18, 0x01},   // HSTOP:  Horizontal Frame Stop edge
    {0x32, 0xb6},   // HREF:   HREF Control (timing reference for window)
    {0x19, 0x02},   // VSTART: Vertical Frame Start line
    {0x1a, 0x7a},   // VSTOP:  Vertical Frame Stop line
    {0x03, 0x0a},   // VREF:   Vertical Reference control

    // --- SCALING TO 160x120 ---
    {0x0c, 0x04},   // COM3: Enable camera DSP scaling. CRITICAL.
    {0x72, 0x22},   // SCALING_DCWCTR: **Downsample by 2 horizontally and vertically.**
    {0x73, 0xf1},   // SCALING_PCLK_DIV: PCLK divider control for scaling.
    {0x70, 0x3a},   // SCALING_XSC: Horizontal scaling factor.
    {0x71, 0x35},   // SCALING_YSC: Vertical scaling factor.
    {0xa2, 0x02},   // SCALING_PCLK_DELAY: PCLK delay control.

    // --- IMAGE QUALITY: COLOR, BRIGHTNESS, CONTRAST ---
    // Gamma Curve
    {0x7a, 0x20}, {0x7b, 0x10}, {0x7c, 0x1e}, {0x7d, 0x35}, {0x7e, 0x5a},
    {0x7f, 0x69}, {0x80, 0x76}, {0x81, 0x80}, {0x82, 0x88}, {0x83, 0x8f},
    {0x84, 0x96}, {0x85, 0xa3}, {0x86, 0xaf}, {0x87, 0xc4}, {0x88, 0xd7},

    // Color Matrix (for YUV to RGB conversion accuracy)
    {0x4f, 0xb3}, {0x50, 0xb3}, {0x51, 0x00}, {0x52, 0x3d}, {0x53, 0xa7},
    {0x54, 0xe4}, {0x58, 0x9e},

    // General Controls
    {0x3d, 0x40},   // COM13: Gamma enable, UV auto-adjust enable.
    {0x3a, 0x1c},   // TSLB: YUV an UYVY options.
    {0x14, 0x38},   // COM9: Max AGC value (gain ceiling), 4x.
    {0x0e, 0x61},   // COM5: General control.
    {0x0f, 0x4b},   // COM6: General control.

    // --- AUTO ADJUSTMENTS: GAIN, EXPOSURE, WHITE BALANCE ---
    {0x13, 0xe7},   // COM8: Enable AGC, AEC, AWB.
    {0x00, 0x00},   // GAIN: AGC - Gain control
    {0x10, 0x00},   // AECH: AEC - Exposure value
    {0x01, 0x40},   // BAVG: U/B Average Level
    {0x02, 0x60},   // GAVG: V/G Average Level
    {0x24, 0x95},   // AGC/AEC stable range (upper limit)
    {0x25, 0x33},   // AGC/AEC stable range (lower limit)
    {0x26, 0xe3},   // Fast AGC/AEC algorithm region
    {0xa5, 0x05},   // BD50MAX: Max exposure for 50Hz lighting
    {0xab, 0x07},   // BD60MAX: Max exposure for 60Hz lighting

    // Auto White Balance (AWB)
    {0x41, 0x08},   // COM16
    {0x42, 0x00},   // COM17
    {0x43, 0x0a},   // AWB Control
    {0x44, 0xf0},   // AWB Control
    {0x45, 0x34},   // AWB Control
    {0x46, 0x58},   // AWB Control
    {0x47, 0x28},   // AWB Control
    {0x48, 0x3a},   // AWB Control
    {0x59, 0x88}, {0x5a, 0x88}, {0x5b, 0x44}, {0x5c, 0x67}, {0x5d, 0x49}, {0x5e, 0x0e},
    {0x6c, 0x0a}, {0x6d, 0x55}, {0x6e, 0x11}, {0x6f, 0x9e},

    // --- LENS CORRECTION AND DENOISE ---
    {0x3b, 0x12},   // COM11: Denoise, Edge enhancement.
    {0x75, 0x05},   // Denoise strength
    {0x76, 0xe1},   // Edge enhancement factor
    {0x77, 0x01},   // Denoise/Edge control
    {0x8d, 0x4f},   // Lens Correction Option 1
    {0x8e, 0x00},   // Lens Correction Option 2
    {0x8f, 0x00},   // Lens Correction Option 3
    {0x90, 0x00},   // Lens Correction Option 4
    {0x91, 0x00},   // Lens Correction Option 5

    // --- MISC / UNDOCUMENTED / RESERVED (Standard values) ---
    {0x16, 0x02}, {0x1e, 0x07}, {0x21, 0x02}, {0x22, 0x91}, {0x29, 0x07},
    {0x33, 0x0b}, {0x34, 0x11}, {0x35, 0x0b}, {0x37, 0x1d}, {0x38, 0x71},
    {0x39, 0x2a}, {0x3f, 0x00}, {0x4c, 0x00}, {0x4d, 0x40}, {0x4e, 0x20},
    {0x69, 0x00}, {0x78, 0x04}, {0x96, 0x00}, {0x97, 0x30}, {0x98, 0x20},
    {0x99, 0x30}, {0x9a, 0x84}, {0x9b, 0x29}, {0x9c, 0x03}, {0x9d, 0x4c},
    {0x9e, 0x3f}, {0xa4, 0x82}, {0xb1, 0x0c}, {0xb2, 0x0e}, {0xb3, 0x82},
    {0xb8, 0x0a}, {0x4b, 0x01},
};
const uint8_t OV7670_reg [OV7670_REG_NUM][2] = {
   {0x12, 0x80},		//Reset registers

   // Image format
   {0x12, /*0x14*/0x04},		//QVGA size, RGB mode //20190725

   {0x40, 0xd0},		//RGB565
   {0xb0, 0x84},		//Color mode

   // Hardware window
   {0x11, 0x01},		//PCLK settings, 15fps
   /*{0x32, 0x80},		//HREF
   {0x17, 0x17},		//HSTART
   {0x18, 0x05},		//HSTOP
   {0x03, 0x0a},		//VREF
   {0x19, 0x02},		//VSTART
   {0x1a, 0x7a},		//VSTOP
*/
   /*
                         {0x17, 0x13},	{0x18, 0x01},
                         {0x32, 0xb6},	{0x19, 0x02},
                         {0x1a, 0x7a},	{0x03, 0x0a},
			 */

   {0x17,0x16},
   {0x18,0x04},
   {0x32,0x24},
   {0x19,0x02},
   {0x1a,0x7a},
   {0x03,0x0a},

   // Scalling numbers
   {0x70, 0x3a},		//X_SCALING
   {0x71, 0x35},		//Y_SCALING
      /*{0x70, 0x80},		//X_SCALING test
   {0x71, 0x80},		//Y_SCALING test*/

   /*{0x72, 0x11},		//DCW_SCALING
   {0x73, 0xf0},		//PCLK_DIV_SCALING*/
   /*{0x72, 0x11},
   {0x73, 0xf1},*/
   {0x72, 0x22}, //20190712
   {0x73, 0xf2}, //20190712
   {0xa2, 0x01},		//PCLK_DELAY_SCALING

   // Matrix coefficients
   /*{0x4f, 0x80},		{0x50, 0x80},
   {0x51, 0x00},		{0x52, 0x22},
   {0x53, 0x5e},		{0x54, 0x80},*/	//20190725
    {0x4f, 0xb3},		 /* "matrix coefficient 1" */
    {0x50, 0xb3},		 /* "matrix coefficient 2" */
    {0x51, 0},		 /* vb */
    {0x52, 0x3d},		 /* "matrix coefficient 4" */
    {0x53, 0xa7},		 /* "matrix coefficient 5" */
    {0x54, 0xe4},		 /* "matrix coefficient 6" */
   {0x58, 0x9e},

   // Gamma curve values
   {0x7a, 0x20},		{0x7b, 0x10},
   {0x7c, 0x1e},		{0x7d, 0x35},
   {0x7e, 0x5a},		{0x7f, 0x69},
   {0x80, 0x76},		{0x81, 0x80},
   {0x82, 0x88},		{0x83, 0x8f},
   {0x84, 0x96},		{0x85, 0xa3},
   {0x86, 0xaf},		{0x87, 0xc4},
   {0x88, 0xd7},		{0x89, 0xe8},

   // AGC and AEC parameters
   {0xa5, 0x05},		{0xab, 0x07},
   {0x24, 0x95},		{0x25, 0x33},
   {0x26, 0xe3},		{0x9f, 0x78},
   {0xa0, 0x68},		{0xa1, 0x03},
   {0xa6, 0xd8},		{0xa7, 0xd8},
   {0xa8, 0xf0},		{0xa9, 0x90},
   {0xaa, 0x94},		{0x10, 0x00},

   // AWB parameters
   {0x43, 0x0a},		{0x44, 0xf0},
   {0x45, 0x34},		{0x46, 0x58},
   {0x47, 0x28},		{0x48, 0x3a},
   {0x59, 0x88},		{0x5a, 0x88},
   {0x5b, 0x44},		{0x5c, 0x67},
   {0x5d, 0x49},		{0x5e, 0x0e},
   {0x6c, 0x0a},		{0x6d, 0x55},
   {0x6e, 0x11},		{0x6f, /*0x9f*/0x9e}, //20190725

   {0x6a, 0x40},		{0x01, 0x40},
   {0x02, 0x60},		{0x13, 0xe7},

   // Additional parameters
   {0x34, 0x11},		{0x3f, 0x00},
   {0x75, 0x05},		{0x76, 0xe1},
   {0x4c, 0x00},		{0x77, 0x01},
   {0xb8, 0x0a},		{0x41, 0x08},
   {0x3b, 0x12},		{0xa4, 0x82},
   {0x96, 0x00},		{0x97, 0x30},
   {0x98, 0x20},		{0x99, 0x30},
   {0x9a, 0x84},		{0x9b, 0x29},
   {0x9c, 0x03},		{0x9d, 0x4c},
   {0x9e, 0x3f},		{0x78, 0x04},
   {0x0e, 0x61},		{0x0f, 0x4b},
   {0x16, 0x02},		{0x1e, 0x07},
   {0x21, 0x02},		{0x22, 0x91},
   {0x29, 0x07},		{0x33, 0x0b},
   {0x35, 0x0b},		{0x37, 0x1d},
   {0x38, 0x71},		{0x39, 0x2a},
   {0x3c, 0x78},		{0x4d, 0x40},
   {0x4e, 0x20},		{0x69, 0x00},
   {0x6b, 0x3a},		{0x74, 0x10},
   {0x8d, 0x4f},		{0x8e, 0x00},
   {0x8f, 0x00},		{0x90, 0x00},
   {0x91, 0x00},		{0x96, 0x00},
   {0x9a, 0x00},		{0xb1, 0x0c},
   {0xb2, 0x0e},		{0xb3, 0x82},
   {0x4b, 0x01},
/*
   {0x0c, 0x04},
   {0x3e, 0x19}*/
   {0x3e, 0x1a}, //20190712
   {0x0c, 0x04}, //20190712
   {0x3d,0x40}, // //20190725
   {0x14,0x6A}, // //20190725
   {0x30,0x0}, // //20190725
   {0x31,0x0}, // //20190725
   {0x15, 0x02}, // //20190726
};

const uint8_t OV7670_reg2 [OV7670_REG_NUM][2] = {
   {0x12, 0x80},		//Reset registers

   // Image format
   {0x12, 0x14},		//QVGA size, RGB mode
   {0x40, 0xd0},		//RGB565
   {0xb0, 0x84},		//Color mode

   // Hardware window
   {0x11, 0x01},		//PCLK settings, 15fps
   {0x32, 0x80},		//HREF
   {0x17, 0x17},		//HSTART
   {0x18, 0x05},		//HSTOP
   {0x03, 0x0a},		//VREF
   {0x19, 0x02},		//VSTART
   {0x1a, 0x7a},		//VSTOP

   // Scalling numbers
   {0x70, 0x3a},		//X_SCALING
   {0x71, 0x35},		//Y_SCALING
   {0x72, 0x11},		//DCW_SCALING
   {0x73, 0xf0},		//PCLK_DIV_SCALING
   {0xa2, 0x02},		//PCLK_DELAY_SCALING

   // Matrix coefficients
   {0x4f, 0x80},		{0x50, 0x80},
   {0x51, 0x00},		{0x52, 0x22},
   {0x53, 0x5e},		{0x54, 0x80},
   {0x58, 0x9e},

   // Gamma curve values
   {0x7a, 0x20},		{0x7b, 0x10},
   {0x7c, 0x1e},		{0x7d, 0x35},
   {0x7e, 0x5a},		{0x7f, 0x69},
   {0x80, 0x76},		{0x81, 0x80},
   {0x82, 0x88},		{0x83, 0x8f},
   {0x84, 0x96},		{0x85, 0xa3},
   {0x86, 0xaf},		{0x87, 0xc4},
   {0x88, 0xd7},		{0x89, 0xe8},

   // AGC and AEC parameters
   {0xa5, 0x05},		{0xab, 0x07},
   {0x24, 0x95},		{0x25, 0x33},
   {0x26, 0xe3},		{0x9f, 0x78},
   {0xa0, 0x68},		{0xa1, 0x03},
   {0xa6, 0xd8},		{0xa7, 0xd8},
   {0xa8, 0xf0},		{0xa9, 0x90},
   {0xaa, 0x94},		{0x10, 0x00},

   // AWB parameters
   {0x43, 0x0a},		{0x44, 0xf0},
   {0x45, 0x34},		{0x46, 0x58},
   {0x47, 0x28},		{0x48, 0x3a},
   {0x59, 0x88},		{0x5a, 0x88},
   {0x5b, 0x44},		{0x5c, 0x67},
   {0x5d, 0x49},		{0x5e, 0x0e},
   {0x6c, 0x0a},		{0x6d, 0x55},
   {0x6e, 0x11},		{0x6f, 0x9f},
   {0x6a, 0x40},		{0x01, 0x40},
   {0x02, 0x60},		{0x13, 0xe7},

   // Additional parameters
   {0x34, 0x11},		{0x3f, 0x00},
   {0x75, 0x05},		{0x76, 0xe1},
   {0x4c, 0x00},		{0x77, 0x01},
   {0xb8, 0x0a},		{0x41, 0x18},
   {0x3b, 0x12},		{0xa4, 0x88},
   {0x96, 0x00},		{0x97, 0x30},
   {0x98, 0x20},		{0x99, 0x30},
   {0x9a, 0x84},		{0x9b, 0x29},
   {0x9c, 0x03},		{0x9d, 0x4c},
   {0x9e, 0x3f},		{0x78, 0x04},
   {0x0e, 0x61},		{0x0f, 0x4b},
   {0x16, 0x02},		{0x1e, 0x00},
   {0x21, 0x02},		{0x22, 0x91},
   {0x29, 0x07},		{0x33, 0x0b},
   {0x35, 0x0b},		{0x37, 0x1d},
   {0x38, 0x71},		{0x39, 0x2a},
   {0x3c, 0x78},		{0x4d, 0x40},
   {0x4e, 0x20},		{0x69, 0x00},
   {0x6b, 0x3a},		{0x74, 0x10},
   {0x8d, 0x4f},		{0x8e, 0x00},
   {0x8f, 0x00},		{0x90, 0x00},
   {0x91, 0x00},		{0x96, 0x00},
   {0x9a, 0x00},		{0xb1, 0x0c},
   {0xb2, 0x0e},		{0xb3, 0x82},
   {0x4b, 0x01},
};



bool SCCB_write_reg(uint8_t reg_addr, uint8_t value)
{
    uint32_t timeout;

    printf("loaded data %d %d\r\n", reg_addr, value);

    LL_I2C_GenerateStartCondition(I2C1);

    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if (--timeout == 0) return true;
    }

    LL_I2C_TransmitData8(I2C1, OV7670_I2C_ADDR << 1);  // write mode

    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if (--timeout == 0) return true;
    }
    (void)I2C1->SR2;  // clear ADDR flag

    LL_I2C_TransmitData8(I2C1, reg_addr);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
        if (--timeout == 0) return true;
    }

    LL_I2C_TransmitData8(I2C1, value);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
        if (--timeout == 0) return true;
    }

    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_BTF(I2C1)) {
        if (--timeout == 0) return true;
    }

    LL_I2C_GenerateStopCondition(I2C1);
    return false;  // false = success, true = error
}
/*
 * ==========================================================================
 *   SCCB (I2C-Compatible) Read Function
 * ==========================================================================
 */

/**
 * @brief  Reads a value from a specific register on an SCCB device like the OV7670.
 *         This function is designed to be robust and handle the specific timing
 *         required for SCCB reads using a standard I2C peripheral.
 * @param  reg_addr: The 8-bit address of the register to read from.
 * @param  value: Pointer to an 8-bit variable to store the read value.
 * @retval bool: false on success (HAL_OK), true on error (HAL_ERROR).
 */
/**
 * @brief  Reads a value from a specific register on an SCCB device like the OV7670.
 *         This version is built to MIMIC THE EXACT TIMING AND FLAG-CHECKING
 *         of the provided working write function.
 * @param  reg_addr: The 8-bit address of the register to read from.
 * @param  value: Pointer to an 8-bit variable to store the read value.
 * @retval bool: false on success, true on error.
 */
bool SCCB_read_reg(uint8_t reg_addr, uint8_t *value)
{
    uint32_t timeout;

    // =======================================================================
    // Phase 1: Dummy Write - Set the register address we want to read from.
    // This phase follows your write function's logic exactly.
    // =======================================================================

    // 1. Generate START condition
    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if (--timeout == 0) return true;
    }

    // 2. Send camera slave address in WRITE mode
    LL_I2C_TransmitData8(I2C1, OV7670_I2C_ADDR << 1);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if (--timeout == 0) {
            LL_I2C_GenerateStopCondition(I2C1); // Important: Release bus on failure
            return true;
        }
    }
    (void)I2C1->SR2;  // Clear ADDR flag

    // 3. Send the register address
    LL_I2C_TransmitData8(I2C1, reg_addr);
    timeout = 0xFFFF;
    // Wait for the transmission to complete
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
        if (--timeout == 0) {
            LL_I2C_GenerateStopCondition(I2C1);
            return true;
        }
    }
    // Note: No second TXE wait, no BTF wait, to match the simple write function.


    // =======================================================================
    // Phase 2: Actual Read Operation
    // =======================================================================

    // 4. Generate a REPEATED START condition to switch to read mode
    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if (--timeout == 0) return true;
    }

    // 5. Send camera slave address in READ mode (SLA+R)
    LL_I2C_TransmitData8(I2C1, (OV7670_I2C_ADDR << 1) | 0x01);
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if (--timeout == 0) {
            LL_I2C_GenerateStopCondition(I2C1);
            return true;
        }
    }

    // 6. **CRITICAL SEQUENCE for single-byte read**
    //    To receive only one byte, the I2C peripheral must be told NOT to
    //    acknowledge the byte it receives. This signals the end of the
    //    transfer to the slave device.

    // a) Disable Acknowledge
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

    // b) Clear the ADDR flag (this is a key step)
    (void)I2C1->SR2;

    // c) Program the STOP condition to be sent after the byte is received.
    LL_I2C_GenerateStopCondition(I2C1);


    // 7. Wait for the data to be received into the data register (RXNE flag)
    timeout = 0xFFFF;
    while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) {
        if (--timeout == 0) return true;
    }

    // 8. Read the data. This also clears the RXNE flag.
    *value = LL_I2C_ReceiveData8(I2C1);

    // 9. Wait for the STOP condition to be cleared from the bus
    timeout = 0xFFFF;
    while (LL_I2C_IsActiveFlag_STOP(I2C1)) {
        if (--timeout == 0) return true;
    }

    return false;  // false = success
}

bool OV7670_init(void) {


   uint8_t data, i = 0;
   bool err = false;
   for(i = 0; i <sizeof(OV7670_reg) / sizeof(OV7670_reg[0]); i++) {
      data = OV7670_reg[i][1];
      // Note the direct passing of 'data', not its address
      uint32_t timeout;

      printf("loaded data %d %d\r\n", OV7670_reg[i][0], data);

      LL_I2C_GenerateStartCondition(I2C1);

      timeout = 0xFFFF;
      while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
          if (--timeout == 0) return true;
      }

      LL_I2C_TransmitData8(I2C1, OV7670_I2C_ADDR << 1);  // write mode

      timeout = 0xFFFF;
      while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
          if (--timeout == 0) return true;
      }
      (void)I2C1->SR2;  // clear ADDR flag

      LL_I2C_TransmitData8(I2C1, OV7670_reg[i][0]);
      timeout = 0xFFFF;
      while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
          if (--timeout == 0) return true;
      }

      LL_I2C_TransmitData8(I2C1, data);
      timeout = 0xFFFF;
      while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
          if (--timeout == 0) return true;
      }

      timeout = 0xFFFF;
      while (!LL_I2C_IsActiveFlag_BTF(I2C1)) {
          if (--timeout == 0) return true;
      }

      LL_I2C_GenerateStopCondition(I2C1);
      err=false;
      printf("i: %d\r\n",i);
   }
	/*uint8_t id;
	if (!SCCB_read_reg(0x0A, &id)) {
	    printf("MIDH: 0x%02X\r\n", id);  // Should be 0x7F for OV7670
	}*/
   return err;
}






bool OV7670_init3(void){
   uint8_t data, i = 0;
   bool err;
   // Configure camera registers
   for(i=0; i<OV7670_REG_NUM ;i++){
      data = OV7670_reg2[i][1];
      SCCB_write_reg(OV7670_reg2[i][0], &data);

      /*
      LCD_ILI9341_DrawLine(99+i, 110, 99+i, 130, ILI9341_COLOR_WHITE);*/
      OVDelay(0xFFFF);
   }

   return err;
}

bool OV7670_init_v2(void)
{
   OVDelay(0xFFF);SCCB_write_reg(REG_COM7, 0x80);OVDelay(0xFFF);

        // QQVGA RGB444
        SCCB_write_reg(REG_CLKRC,0x80);OVDelay(0xFFF);
        SCCB_write_reg(REG_COM11,0x0A) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_TSLB,0x04);OVDelay(0xFFF);
        SCCB_write_reg(REG_COM7,0x04) ;OVDelay(0xFFF);

        //SCCB_write_reg(REG_RGB444, 0x02);OVDelay(0xFFF);
        //SCCB_write_reg(REG_COM15, 0xd0);OVDelay(0xFFF);
        SCCB_write_reg(REG_RGB444, 0x00);OVDelay(0xFFF);     // Disable RGB 444?
        SCCB_write_reg(REG_COM15, 0xD0);OVDelay(0xFFF);      // Set RGB 565?

        SCCB_write_reg(REG_HSTART,0x16) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_HSTOP,0x04) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_HREF,0x24) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_VSTART,0x02) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_VSTOP,0x7a) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_VREF,0x0a) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_COM10,0x02) ;OVDelay(0xFFF);
        SCCB_write_reg(REG_COM3, 0x04);OVDelay(0xFFF);
        SCCB_write_reg(REG_COM14, 0x1a);OVDelay(0xFFF);
        SCCB_write_reg(REG_MVFP,0x27) ;OVDelay(0xFFF);
        SCCB_write_reg(0x72, 0x22);OVDelay(0xFFF);
        SCCB_write_reg(0x73, 0xf2);OVDelay(0xFFF);

        // COLOR SETTING
        SCCB_write_reg(0x4f,0x80);OVDelay(0xFFF);
        SCCB_write_reg(0x50,0x80);OVDelay(0xFFF);
        SCCB_write_reg(0x51,0x00);OVDelay(0xFFF);
        SCCB_write_reg(0x52,0x22);OVDelay(0xFFF);
        SCCB_write_reg(0x53,0x5e);OVDelay(0xFFF);
        SCCB_write_reg(0x54,0x80);OVDelay(0xFFF);
        SCCB_write_reg(0x56,0x40);OVDelay(0xFFF);
        SCCB_write_reg(0x58,0x9e);OVDelay(0xFFF);
        SCCB_write_reg(0x59,0x88);OVDelay(0xFFF);
        SCCB_write_reg(0x5a,0x88);OVDelay(0xFFF);
        SCCB_write_reg(0x5b,0x44);OVDelay(0xFFF);
        SCCB_write_reg(0x5c,0x67);OVDelay(0xFFF);
        SCCB_write_reg(0x5d,0x49);OVDelay(0xFFF);
        SCCB_write_reg(0x5e,0x0e);OVDelay(0xFFF);
        SCCB_write_reg(0x69,0x00);OVDelay(0xFFF);
        SCCB_write_reg(0x6a,0x40);OVDelay(0xFFF);
        SCCB_write_reg(0x6b,0x0a);OVDelay(0xFFF);
        SCCB_write_reg(0x6c,0x0a);OVDelay(0xFFF);
        SCCB_write_reg(0x6d,0x55);OVDelay(0xFFF);
        SCCB_write_reg(0x6e,0x11);OVDelay(0xFFF);
        SCCB_write_reg(0x6f,0x9f);OVDelay(0xFFF);

        SCCB_write_reg(0xb0,0x84);OVDelay(0xFFF);
	return true;
}

void OV7670_init_v3(void)
{
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM7, 0x80); /* reset to default values */
    OVDelay(0xFFFF);SCCB_write_reg(REG_CLKRC, 0x80);
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM11, 0x0A);
    OVDelay(0xFFFF);SCCB_write_reg(REG_TSLB, 0x04);
    OVDelay(0xFFFF);SCCB_write_reg(REG_TSLB, 0x04);
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM7, 0x04); /* output format: rgb */

    OVDelay(0xFFFF);SCCB_write_reg(REG_RGB444, 0x00); /* disable RGB444 */
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM15, 0xD0); /* set RGB565 */

    /* not even sure what all these do, gonna check the oscilloscope and go
     * from there... */
    OVDelay(0xFFFF);SCCB_write_reg(REG_HSTART, 0x16);
    OVDelay(0xFFFF);SCCB_write_reg(REG_HSTOP, 0x04);
    OVDelay(0xFFFF);SCCB_write_reg(REG_HREF, 0x24);
    OVDelay(0xFFFF);SCCB_write_reg(REG_VSTART, 0x02);
    OVDelay(0xFFFF);SCCB_write_reg(REG_VSTOP, 0x7a);
    OVDelay(0xFFFF);SCCB_write_reg(REG_VREF, 0x0a);
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM10, 0x02);
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM3, 0x04);
    OVDelay(0xFFFF);SCCB_write_reg(REG_MVFP, 0x3f);

    /* 160x120, i think */
    OVDelay(0xFFFF);SCCB_write_reg(REG_COM14, 0x1a); // divide by 4
    OVDelay(0xFFFF);SCCB_write_reg(0x72, 0x22); // downsample by 4
    OVDelay(0xFFFF);SCCB_write_reg(0x73, 0xf2); // divide by 4

    /* 320x240: */
   // OVDelay(0xFFFF);SCCB_write_reg(REG_COM14, 0x19);
   // OVDelay(0xFFFF);SCCB_write_reg(0x72, 0x11);
  //  OVDelay(0xFFFF);SCCB_write_reg(0x73, 0xf1);

    // test pattern
    //OVDelay(0xFFFF);SCCB_write_reg(0x70, 0xf0);
    //OVDelay(0xFFFF);SCCB_write_reg(0x71, 0xf0);

    // COLOR SETTING
    OVDelay(0xFFFF);SCCB_write_reg(0x4f, 0x80);
    OVDelay(0xFFFF);SCCB_write_reg(0x50, 0x80);
    OVDelay(0xFFFF);SCCB_write_reg(0x51, 0x00);
    OVDelay(0xFFFF);SCCB_write_reg(0x52, 0x22);
    OVDelay(0xFFFF);SCCB_write_reg(0x53, 0x5e);
    OVDelay(0xFFFF);SCCB_write_reg(0x54, 0x80);
    OVDelay(0xFFFF);SCCB_write_reg(0x56, 0x40);
    OVDelay(0xFFFF);SCCB_write_reg(0x58, 0x9e);
    OVDelay(0xFFFF);SCCB_write_reg(0x59, 0x88);
    OVDelay(0xFFFF);SCCB_write_reg(0x5a, 0x88);
    OVDelay(0xFFFF);SCCB_write_reg(0x5b, 0x44);
    OVDelay(0xFFFF);SCCB_write_reg(0x5c, 0x67);
    OVDelay(0xFFFF);SCCB_write_reg(0x5d, 0x49);
    OVDelay(0xFFFF);SCCB_write_reg(0x5e, 0x0e);
    OVDelay(0xFFFF);SCCB_write_reg(0x69, 0x00);
    OVDelay(0xFFFF);SCCB_write_reg(0x6a, 0x40);
    OVDelay(0xFFFF);SCCB_write_reg(0x6b, 0x0a);
    OVDelay(0xFFFF);SCCB_write_reg(0x6c, 0x0a);
    OVDelay(0xFFFF);SCCB_write_reg(0x6d, 0x55);
    OVDelay(0xFFFF);SCCB_write_reg(0x6e, 0x11);
    OVDelay(0xFFFF);SCCB_write_reg(0x6f, 0x9f);

    OVDelay(0xFFFF);SCCB_write_reg(0xb0, 0x84);
}



