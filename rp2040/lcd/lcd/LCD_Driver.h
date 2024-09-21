/*****************************************************************************
* | File      	: LCD_Driver.h
* | Author      : Waveshare team
* | Function    : ILI9486 Drive function
* | Info        :
*----------------
* | This version: V1.0
* | Date        : 2018-01-11
* | Info        : Basic version
*
******************************************************************************/

/**************************Intermediate driver layer**************************/

#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "DEV_Config.h"

#define LCD_2_8 0x52
#define LCD_3_5 0x00

#define	COLOR  uint16_t // The variable type of the color (unsigned short) 
#define	POINT  uint16_t // The type of coordinate (unsigned short) 
#define	LENGTH uint16_t // The type of coordinate (unsigned short) 

/*******************************************************************************
function:	Define the full screen height length of the display
*******************************************************************************/
#define LCD_X_MAXPIXEL 480 // LCD width maximum memory 
#define LCD_Y_MAXPIXEL 320 // LCD height maximum memory
#define LCD_X 0
#define LCD_Y 0

#define LCD_3_5_WIDTH  (LCD_X_MAXPIXEL - 2 * LCD_X) // LCD width
#define LCD_3_5_HEIGHT LCD_Y_MAXPIXEL		    // LCD height

/*******************************************************************************
function:	scanning method
*******************************************************************************/
typedef enum {
	L2R_U2D  = 0, // display left to right, up to down
	L2R_D2U,
	R2L_U2D,
	R2L_D2U,

	U2D_L2R,
	U2D_R2L,
	D2U_L2R,
	D2U_R2L
} LCD_SCAN_DIR;

#define SCAN_DIR_DFT D2U_L2R // Default scan direction

/*******************************************************************************
function:	Defines the total number of rows in the display area
*******************************************************************************/
typedef struct {
	LENGTH LCD_Dis_Column;	// COLUMN
	LENGTH LCD_Dis_Page;	// PAGE
	LCD_SCAN_DIR LCD_Scan_Dir;
	POINT LCD_X_Adjust;	// LCD x actual display position calibration
	POINT LCD_Y_Adjust;	// LCD y actual display position calibration
} LCD_DIS;

extern LCD_DIS sLCD_DIS;
extern uint8_t id;

/******************************************************************************
function:	Write register address and data
******************************************************************************/
static inline void LCD_WriteReg(uint8_t Reg)
{
	DEV_Digital_Write(LCD_DC_PIN, 0);
	DEV_Digital_Write(LCD_CS_PIN, 0);
	SPI4W_Write_Byte(Reg);
	DEV_Digital_Write(LCD_CS_PIN, 1);
}

static inline void LCD_WriteData(uint16_t Data)
{
	DEV_Digital_Write(LCD_DC_PIN, 1);
	DEV_Digital_Write(LCD_CS_PIN, 0);
	SPI4W_Write_Byte(Data >> 8);
	SPI4W_Write_Byte(Data & 0XFF);
	DEV_Digital_Write(LCD_CS_PIN, 1);
}

/******************************************************************************
function:	Write register data
*******************************************************************************/
static inline void LCD_Write_AllData(uint16_t Data, uint32_t DataLen)
{
	register uint32_t i;

	DEV_Digital_Write(LCD_DC_PIN, 1);
	DEV_Digital_Write(LCD_CS_PIN, 0);
	for (i = 0; i < DataLen; i++) {
		SPI4W_Write_Byte(Data >> 8);
		SPI4W_Write_Byte(Data & 0XFF);
	}
	DEV_Digital_Write(LCD_CS_PIN, 1);
}

/*******************************************************************************
function:	Sets the start position and size of the display area
parameter:
	Xstart 	:   X direction Start coordinates
	Ystart  :   Y direction Start coordinates
	Xend    :   X direction end coordinates
	Yend    :   Y direction end coordinates
*******************************************************************************/
static inline void LCD_SetWindow(POINT Xstart, POINT Ystart,
				 POINT Xend, POINT Yend)
{	

	// set the X coordinates
	LCD_WriteReg(0x2A);
	LCD_WriteData(Xstart >> 8);       // Set the horizontal starting point
					  // to the high octet
	LCD_WriteData(Xstart & 0xff);	  // Set the horizontal starting point
					  // to the low octet
	LCD_WriteData((Xend - 1) >> 8);   // Set the horizontal end to the
					  // high octet
	LCD_WriteData((Xend - 1) & 0xff); // Set the horizontal end to the
					  // low octet

	// set the Y coordinates
	LCD_WriteReg(0x2B);
	LCD_WriteData(Ystart >> 8);
	LCD_WriteData(Ystart & 0xff );
	LCD_WriteData((Yend - 1) >> 8);
	LCD_WriteData((Yend - 1) & 0xff);

	LCD_WriteReg(0x2C);
}

/*******************************************************************************
function:	Set the display point (Xpoint, Ypoint)
parameter:
	xStart :   X direction Start coordinates
	xEnd   :   X direction end coordinates
*******************************************************************************/
static inline void LCD_SetCursor(POINT Xpoint, POINT Ypoint)
{
	LCD_SetWindow(Xpoint, Ypoint, Xpoint, Ypoint);
}

/*******************************************************************************
function:	Set show color
parameter:
		Color  :   Set show color, 16-bit depth
*******************************************************************************/
static inline void LCD_SetColor(COLOR Color, POINT Xpoint, POINT Ypoint)
{
	LCD_Write_AllData(Color, (uint32_t) Xpoint * (uint32_t) Ypoint);
}

/*******************************************************************************
function:	Point (Xpoint, Ypoint) Fill the color
parameter:
	Xpoint :   The x coordinate of the point
	Ypoint :   The y coordinate of the point
	Color  :   Set the color
*******************************************************************************/
static inline void LCD_SetPointColor(POINT Xpoint, POINT Ypoint, COLOR Color)
{
	if ((Xpoint <= sLCD_DIS.LCD_Dis_Column) &&
	    (Ypoint <= sLCD_DIS.LCD_Dis_Page)) {
		LCD_SetCursor (Xpoint, Ypoint);
		LCD_SetColor(Color, 1, 1);
	}
}

/*******************************************************************************
function:	Fill the area with the color
parameter:
	Xstart :   Start point x coordinate
	Ystart :   Start point y coordinate
	Xend   :   End point coordinates
	Yend   :   End point coordinates
	Color  :   Set the color
*******************************************************************************/
static inline void LCD_SetArealColor(POINT Xstart, POINT Ystart,
				     POINT Xend, POINT Yend, COLOR Color)
{
	if ((Xend > Xstart) && (Yend > Ystart)) {
		LCD_SetWindow(Xstart, Ystart, Xend, Yend);
		LCD_SetColor (Color, Xend - Xstart, Yend - Ystart);
	}
}

/*******************************************************************************
function:	Macro definition variable name
*******************************************************************************/
extern void LCD_Init(LCD_SCAN_DIR LCD_ScanDir, uint16_t LCD_BLval);
extern void LCD_SetGramScanWay(LCD_SCAN_DIR Scan_dir);

extern void LCD_SetBackLight(uint16_t value);
extern void LCD_Clear(COLOR Color);
extern uint8_t LCD_Read_Id(void);

#endif
