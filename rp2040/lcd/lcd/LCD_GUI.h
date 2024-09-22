/*****************************************************************************
* | File      	: LCD_GUI.h
* | Author      : Waveshare team
* | Function    : Achieve drawing: draw points, lines, boxes, circles and
*                 their size, solid dotted line, solid rectangle hollow
*		  rectangle, solid circle hollow circle.
* | Info        : Achieve display characters: Display a single character,
*		  string, number
*		  Achieve time display: adaptive size display time minutes
*		  and seconds
*----------------
* | This version: V1.0
* | Date        : 2017-08-16
* | Info        : Basic version
*
******************************************************************************/

/****************************Upper application layer**************************/
#ifndef __LCD_GUI_H
#define __LCD_GUI_H

#include "LCD_Driver.h"
#include "fonts.h"

#define LOW_Speed_Show 0
#define HIGH_Speed_Show 1

/*******************************************************************************
function:	dot pixel
*******************************************************************************/
typedef enum {
	DOT_PIXEL_1X1  = 1,		// dot pixel 1 x 1
	DOT_PIXEL_2X2,	 		// dot pixel 2 X 2
	DOT_PIXEL_3X3,			// dot pixel 3 X 3
	DOT_PIXEL_4X4,			// dot pixel 4 X 4
	DOT_PIXEL_5X5,	 		// dot pixel 5 X 5
	DOT_PIXEL_6X6,	 		// dot pixel 6 X 6
	DOT_PIXEL_7X7,	 		// dot pixel 7 X 7
	DOT_PIXEL_8X8,	 		// dot pixel 8 X 8
} DOT_PIXEL;

#define DOT_PIXEL_DFT  DOT_PIXEL_1X1	// Default dot pixel

/*******************************************************************************
function:	dot Fill style
*******************************************************************************/
typedef enum {
	DOT_FILL_AROUND  = 1,		// dot pixel 1 x 1
	DOT_FILL_RIGHTUP, 		// dot pixel 2 X 2
} DOT_STYLE;

#define DOT_STYLE_DFT  DOT_FILL_AROUND  // Default dot pixel

/*******************************************************************************
function:	solid line and dotted line
*******************************************************************************/
typedef enum {
	LINE_SOLID = 0,
	LINE_DOTTED,
} LINE_STYLE;

/*******************************************************************************
function:	DRAW Internal fill
*******************************************************************************/
typedef enum {
	DRAW_EMPTY = 0,
	DRAW_FULL,
} DRAW_FILL;

/*******************************************************************************
function:	time
*******************************************************************************/
typedef struct {
	uint16_t Year;  // 0000
	uint8_t  Month; // 1 - 12
	uint8_t  Day;   // 1 - 30
	uint8_t  Hour;  // 0 - 23
	uint8_t  Min;   // 0 - 59
	uint8_t  Sec;   // 0 - 59
} DEV_TIME;

extern DEV_TIME sDev_time;

/*******************************************************************************
function:	Defines commonly used colors for the display
*******************************************************************************/
#define LCD_BACKGROUND	BLACK   // Default background color
#define FONT_BACKGROUND	BLACK   // Default font background color
#define FONT_FOREGROUND	GRED    // Default font foreground color

#define WHITE		0xFFFF
#define BLACK		0x0000
#define BLUE		0x001F
#define BRED		0xF81F
#define GRED		0xFFE0
#define GBLUE		0x07FF
#define RED		0xF800
#define MAGENTA		0xF81F
#define GREEN		0x07E0
#define CYAN		0x7FFF
#define YELLOW		0xFFE0
#define BROWN		0xBC40
#define BRRED		0xFC07
#define GRAY		0x8430

/******************************************************************************
function:	Coordinate conversion
******************************************************************************/
static inline void GUI_Swap(POINT Point1, POINT Point2)
{
	register POINT Temp;

	Temp = Point1;
	Point1 = Point2;
	Point2 = Temp;
}

/*
 * inline function to draw ASCII characters faster
 */
static inline void GUI_FastChar(POINT Xpoint, POINT Ypoint,
				const char Acsii_Char, sFONT* Font,
				COLOR Color_Background, COLOR Color_Foreground)
{
	POINT Page, Column;

	if (Xpoint > sLCD_DIS.LCD_Dis_Column ||
	    Ypoint > sLCD_DIS.LCD_Dis_Page) {
		//DEBUG("GUI_DisChar Input exceeds the normal display range\r\n");
		return;
	}

	uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height *
				(Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
	const unsigned char *ptr = &Font->table[Char_Offset];

	for (Page = 0; Page < Font->Height; Page ++) {
		for (Column = 0; Column < Font->Width; Column ++) {
		// To determine whether the font background color and
		// screen background color is consistent
		  if (FONT_BACKGROUND == Color_Background) {
			// this process is to speed up the scan
			if (*ptr & (0x80 >> (Column % 8)))
				LCD_SetPointColor(Xpoint + Column,
						  Ypoint + Page,
					          Color_Foreground);
		  } else {
			if (*ptr & (0x80 >> (Column % 8))) {
				LCD_SetPointColor(Xpoint + Column,
						  Ypoint + Page,
					          Color_Foreground);
			} else {
				LCD_SetPointColor(Xpoint + Column,
						  Ypoint + Page,
					          Color_Background);
			}
		  }
		  // One pixel is 8 bits
		  if (Column % 8 == 7)
			ptr++;
		} /* Write a line */
		if (Font->Width % 8 != 0)
			ptr++;
	} /* Write all */
}

/*******************************************************************************
function:	Macro definition variable name
*******************************************************************************/
// Clear
void GUI_Clear(COLOR Color);

// Drawing
void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color,
		   DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_FillWay);
void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
		  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel);
void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
		       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel);
void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius,
		    COLOR Color, DRAW_FILL Draw_Fill, DOT_PIXEL Dot_Pixel);

// pic
void GUI_Disbitmap(POINT Xpoint, POINT Ypoint, const unsigned char *pMap,
		   POINT Width, POINT Height);
void GUI_DisGrayMap(POINT Xpoint, POINT Ypoint, const unsigned char *pBmp);

// Display string
void GUI_DisChar(POINT Xstart, POINT Ystart, const char Acsii_Char,
		 sFONT* Font, COLOR Color_Background, COLOR Color_Foreground);
void GUI_DisString(POINT Xstart, POINT Ystart, const char * pString,
		   sFONT* Font, COLOR Color_Background,
		      COLOR Color_Foreground);
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int32_t Nummber, sFONT* Font,
		COLOR Color_Background, COLOR Color_Foreground);
void GUI_Showtime(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
		  DEV_TIME *pTime, COLOR Color);

#endif
