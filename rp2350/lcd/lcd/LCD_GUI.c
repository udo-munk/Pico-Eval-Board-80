/*****************************************************************************
* | File      	: LCD_GUI.c
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
#include "LCD_GUI.h"

extern LCD_DIS sLCD_DIS;
extern uint8_t id;

/******************************************************************************
function:	Clear LCD
******************************************************************************/
void GUI_Clear(COLOR Color)
{
	LCD_Clear(Color);
}

/******************************************************************************
function:	Draw Point(Xpoint, Ypoint) Fill the color
parameter:
	Xpoint		: The x coordinate of the point
	Ypoint		: The y coordinate of the point
	Color		: Set color
	Dot_Pixel	: point size
******************************************************************************/
void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color,
                   DOT_PIXEL Dot_Pixel, DOT_STYLE DOT_STYLE)
{
	if (Xpoint > sLCD_DIS.LCD_Dis_Column ||
	    Ypoint > sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DrawPoint Input exceeds the normal display range\r\n");
		return;
	}

	uint16_t XDir_Num, YDir_Num;

	if (DOT_STYLE == DOT_STYLE_DFT) {
		for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
			for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1;
			     YDir_Num++) {
				LCD_SetPointColor(Xpoint + XDir_Num - Dot_Pixel,
						  Ypoint + YDir_Num - Dot_Pixel,
						  Color);
			}
		}
	} else {
		for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++) {
			for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) {
				LCD_SetPointColor(Xpoint + XDir_Num - 1,
						  Ypoint + YDir_Num - 1, Color);
			}
		}
	}
}

/******************************************************************************
function:	Draw a line of arbitrary slope
parameter:
	Xstart ：Starting x point coordinates
	Ystart ：Starting x point coordinates
	Xend   ：End point x coordinate
	Yend   ：End point y coordinate
	Color  ：The color of the line segment
******************************************************************************/
void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
	if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page
	    || Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DrawLine Input exceeds the normal display range\r\n");
		return;
	}

	if (Xstart > Xend)
		GUI_Swap(Xstart, Xend);
	if (Ystart > Yend)
		GUI_Swap(Ystart, Yend);

	POINT Xpoint = Xstart;
	POINT Ypoint = Ystart;
	int32_t dx = (int32_t) Xend - (int32_t) Xstart >= 0 ? Xend - Xstart :
		     Xstart - Xend;
	int32_t dy = (int32_t) Yend - (int32_t) Ystart <= 0 ? Yend - Ystart :
		     Ystart - Yend;

	// Increment direction, 1 is positive, -1 is counter;
	int32_t XAddway = Xstart < Xend ? 1 : -1;
	int32_t YAddway = Ystart < Yend ? 1 : -1;

	// Cumulative error
	int32_t Esp = dx + dy;
	int8_t Line_Style_Temp = 0;

	for(;;) {
		Line_Style_Temp++;
		// Painted dotted line, 2 point is really virtual
		if (Line_Style == LINE_DOTTED && Line_Style_Temp % 3 == 0) {
			Debug("LINE_DOTTED\r\n");
			GUI_DrawPoint(Xpoint, Ypoint, LCD_BACKGROUND, Dot_Pixel,
				      DOT_STYLE_DFT);
			Line_Style_Temp = 0;
		} else {
			GUI_DrawPoint(Xpoint, Ypoint, Color, Dot_Pixel,
				      DOT_STYLE_DFT);
		}
		if (2 * Esp >= dy) {
			if (Xpoint == Xend)
				break;
			Esp += dy;
			Xpoint += XAddway;
		}
		if (2 * Esp <= dx) {
			if (Ypoint == Yend)
				break;
			Esp += dx;
			Ypoint += YAddway;
		}
	}
}

/******************************************************************************
function:	Draw a rectangle
parameter:
	Xstart ：Rectangular Starting x point coordinates
	Ystart ：Rectangular Starting y point coordinates
	Xend   ：Rectangular End point x coordinate
	Yend   ：Rectangular End point y coordinate
	Color  ：The color of the Rectangular segment
	Filled : Whether it is filled, 1: solid 0：empty
******************************************************************************/
void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel)
{
	if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page
	    || Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page) {
		Debug("Input exceeds the normal display range\r\n");
		return;
	}

	if (Xstart > Xend)
		GUI_Swap(Xstart, Xend);
	if (Ystart > Yend)
		GUI_Swap(Ystart, Yend);
    
	if (Filled) {
#if LOW_Speed_Show
		POINT Ypoint;
		for (Ypoint = Ystart; Ypoint < Yend; Ypoint++) {
			GUI_DrawLine(Xstart, Ypoint, Xend, Ypoint, Color,
				     LINE_SOLID, Dot_Pixel);
		}
#elif HIGH_Speed_Show
		LCD_SetArealColor(Xstart, Ystart, Xend, Yend, Color);
#endif
	} else {
		GUI_DrawLine(Xstart, Ystart, Xend, Ystart, Color,
			     LINE_SOLID, Dot_Pixel);
		GUI_DrawLine(Xstart, Ystart, Xstart, Yend, Color,
			     LINE_SOLID, Dot_Pixel);
		GUI_DrawLine(Xend, Yend, Xend, Ystart, Color,
			     LINE_SOLID, Dot_Pixel);
		GUI_DrawLine(Xend, Yend, Xstart, Yend, Color,
			     LINE_SOLID, Dot_Pixel);
	}
}

/******************************************************************************
function:	Use the 8-point method to draw a circle of the
		specified size at the specified position.
parameter:
	X_Center  ：Center X coordinate
	Y_Center  ：Center Y coordinate
	Radius    ：circle Radius
	Color     ：The color of the ：circle segment
	Filled    : Whether it is filled: 1 filling 0：Do not
******************************************************************************/
void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius,
                    COLOR Color, DRAW_FILL  Draw_Fill , DOT_PIXEL Dot_Pixel)
{
	if (X_Center > sLCD_DIS.LCD_Dis_Column ||
	Y_Center >= sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DrawCircle Input exceeds the normal display range\r\n");
		return;
	}

	// Draw a circle from(0, R) as a starting point
	int16_t XCurrent, YCurrent;
	XCurrent = 0;
	YCurrent = Radius;

	// Cumulative error, judge the next point of the logo
	int16_t Esp = 3 - (Radius << 1);

	int16_t sCountY;
	if (Draw_Fill == DRAW_FULL) { // Draw a filled circle
		while(XCurrent <= YCurrent) { // Realistic circles
			for (sCountY = XCurrent; sCountY <= YCurrent;
			     sCountY++) {
			  GUI_DrawPoint(X_Center + XCurrent, Y_Center + sCountY,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//1
			  GUI_DrawPoint(X_Center - XCurrent, Y_Center + sCountY,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//2
			  GUI_DrawPoint(X_Center - sCountY, Y_Center + XCurrent,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//3
			  GUI_DrawPoint(X_Center - sCountY, Y_Center - XCurrent,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//4
			  GUI_DrawPoint(X_Center - XCurrent, Y_Center - sCountY,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//5
			  GUI_DrawPoint(X_Center + XCurrent, Y_Center - sCountY,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//6
			  GUI_DrawPoint(X_Center + sCountY, Y_Center - XCurrent,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//7
			  GUI_DrawPoint(X_Center + sCountY, Y_Center + XCurrent,
					Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//0
			}
			if (Esp < 0)
				Esp += 4 * XCurrent + 6;
			else {
				Esp += 10 + 4 * (XCurrent - YCurrent);
				YCurrent--;
			}
			XCurrent++;
		}
	} else { // Draw a hollow circle
		while (XCurrent <= YCurrent) {
			GUI_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//1
			GUI_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//2
			GUI_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//3
			GUI_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//4
			GUI_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//5
			GUI_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//6
			GUI_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//7
			GUI_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent,
				      Color, Dot_Pixel, DOT_STYLE_DFT);//0

			if (Esp < 0)
				Esp += 4 * XCurrent + 6;
			else {
				Esp += 10 + 4 * (XCurrent - YCurrent);
				YCurrent--;
			}
			XCurrent++;
		}
	}
}

/******************************************************************************
function:	Show ASCII characters
parameter:
	Xpoint           ：X coordinate
	Ypoint           ：Y coordinate
	Acsii_Char       ：To display the ASCII characters
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the ASCII character
	Color_Foreground : Select the foreground color of the ASCII character
******************************************************************************/
void GUI_DisChar(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
	POINT Page, Column;

	if (Xpoint > sLCD_DIS.LCD_Dis_Column ||
	    Ypoint > sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DisChar Input exceeds the normal display range\r\n");
		return;
	}

	uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height *
			       (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
	const unsigned char *ptr = &Font->table[Char_Offset];

	for (Page = 0; Page < Font->Height; Page++) {
		for (Column = 0; Column < Font->Width; Column++) {
		// To determine whether the font background color and
		// screen background color is consistent
		  if (FONT_BACKGROUND == Color_Background) {
			// this process is to speed up the scan
			if (*ptr & (0x80 >> (Column % 8)))
				GUI_DrawPoint(Xpoint + Column, Ypoint + Page,
					      Color_Foreground, DOT_PIXEL_DFT,
					      DOT_STYLE_DFT);
		  } else {
			if (*ptr & (0x80 >> (Column % 8))) {
				GUI_DrawPoint(Xpoint + Column, Ypoint + Page,
					      Color_Foreground, DOT_PIXEL_DFT,
					      DOT_STYLE_DFT);
			} else {
				GUI_DrawPoint(Xpoint + Column, Ypoint + Page,
					      Color_Background, DOT_PIXEL_DFT,
					      DOT_STYLE_DFT);
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

/******************************************************************************
function:	Display the string
parameter:
	Xstart           ：X coordinate
	Ystart           ：Y coordinate
	pString          ：The first address of the ASCII string to be displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the ASCII character
	Color_Foreground : Select the foreground color of the ASCII character
******************************************************************************/
void GUI_DisString(POINT Xstart, POINT Ystart, const char *pString,
                   sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
	POINT Xpoint = Xstart;
	POINT Ypoint = Ystart;

	if (Xstart > sLCD_DIS.LCD_Dis_Column ||
	    Ystart > sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DisString Input exceeds the normal display range\r\n");
		return;
	}

	while (*pString != '\0') {
		// if X direction filled, reposition to(Xstart,Ypoint), Ypoint
		// is Y direction plus the height of the character
		if ((Xpoint + Font->Width) > sLCD_DIS.LCD_Dis_Column) {
			Xpoint = Xstart;
			Ypoint += Font->Height;
		}

		// If the Y direction is full, reposition to(Xstart, Ystart)
		if ((Ypoint + Font->Height) > sLCD_DIS.LCD_Dis_Page) {
			Xpoint = Xstart;
			Ypoint = Ystart;
		}
		GUI_DisChar(Xpoint, Ypoint, *pString, Font, Color_Background,
			    Color_Foreground);

		// The next character of the address
		pString++;

		// The next word of the abscissa increases the font of the
		// broadband
		Xpoint += Font->Width;
	}
}

/******************************************************************************
function:	Display the string
parameter:
	Xstart           ：X coordinate
	Ystart           : Y coordinate
	Nummber          : The number displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the ASCII character
	Color_Foreground : Select the foreground color of the ASCII character
******************************************************************************/
#define  ARRAY_LEN 255
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int32_t Nummber,
                sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
	int16_t Num_Bit = 0, Str_Bit = 0;
	uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
	uint8_t *pStr = Str_Array;

	if (Xpoint > sLCD_DIS.LCD_Dis_Column ||
	    Ypoint > sLCD_DIS.LCD_Dis_Page) {
		Debug("GUI_DisNum Input exceeds the normal display range\r\n");
		return;
	}

	// Converts a number to a string
	while (Nummber) {
		Num_Array[Num_Bit] = Nummber % 10 + '0';
		Num_Bit++;
		Nummber /= 10;
	}

	// The string is inverted
	while (Num_Bit > 0) {
		Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
		Str_Bit++;
		Num_Bit--;
	}

	// show
	GUI_DisString(Xpoint, Ypoint, (const char *) pStr, Font,
		      Color_Background, Color_Foreground);
}

/******************************************************************************
function:	Display the bit map, 1 byte = 8bit = 8 points
parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
note:
	This function is suitable for bitmap, because a 16-bit data accounted
	for 16 points
******************************************************************************/
void GUI_DisBitMap(POINT Xpoint, POINT Ypoint, const unsigned char *pMap,
                   POINT Width, POINT Height)
{
	POINT i, j, byteWidth = (Width + 7) / 8;

	for (j = 0; j < Height; j++) {
		for (i = 0; i < Width; i++) {
			if (*(pMap + j * byteWidth + i / 8) &
			    (128 >> (i & 7))) {
				GUI_DrawPoint(Xpoint + i, Ypoint + j, WHITE,
					      DOT_PIXEL_DFT, DOT_STYLE_DFT);
			}
		}
	}
}

/******************************************************************************
function:	Display the Gray map, 1 byte = 8bit = 2 points
parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
note:
	This function is suitable for bitmap, because a 4-bit data accounted
	for 1 points
	Please use the Image2lcd generated array
******************************************************************************/
void GUI_DisGrayMap(POINT Xpoint, POINT Ypoint, const unsigned char *pBmp)
{
	// Get the Map header Gray, width, height
	char Gray;
	Gray = *(pBmp + 1);
	POINT Height, Width;
	Width = (*(pBmp + 3) << 8) | (*(pBmp + 2));
	Height = (*(pBmp + 5) << 8) | (*(pBmp + 4));
	POINT i, j;

	if (Gray == 0x04) { //Sixteen gray levels
		pBmp = pBmp + 6;
		for (j = 0; j < Height; j++)
			for (i = 0; i < Width / 2; i++) {
				GUI_DrawPoint(Xpoint + i * 2, Ypoint + j,
					      ~(*pBmp >> 4), DOT_PIXEL_DFT,
					      DOT_STYLE_DFT);
				GUI_DrawPoint(Xpoint + i * 2 + 1, Ypoint + j,
					      ~*pBmp , DOT_PIXEL_DFT,
					      DOT_STYLE_DFT);
				pBmp++;
			}
	} else {
		Debug("Does not support type\r\n");
		return;
	}
}

sFONT *GUI_GetFontSize(POINT Dx, POINT Dy)
{
	sFONT *Font;

	if (Dx > Font24.Width && Dy > Font24.Height) {
		Font = &Font24;
	} else if ((Dx > Font20.Width && Dx < Font24.Width) &&
		   (Dy > Font20.Height && Dy < Font24.Height)) {
		Font = &Font20;
	} else if ((Dx > Font16.Width && Dx < Font20.Width) &&
		   (Dy > Font16.Height && Dy < Font20.Height)) {
		Font = &Font16;
	} else if ((Dx > Font12.Width && Dx < Font16.Width) &&
		   (Dy > Font12.Height && Dy < Font16.Height)) {
		Font = &Font12;
	} else if ((Dx > Font8.Width && Dx < Font12.Width) &&
		   (Dy > Font8.Height && Dy < Font12.Height)) {
		Font = &Font8;
	} else {
		Debug("Please change the display area size, or add a larger font to modify\r\n");
	}
	return Font;
}

/******************************************************************************
  function:	According to the display area adaptive display time
  parameter:
		xStart : X direction Start coordinates
		Ystart : Y direction Start coordinates
		Xend   : X direction end coordinates
		Yend   : Y direction end coordinates
		pTime  : Pointer to the definition of the structure
		Color  : Set show color
  note:
******************************************************************************/
void GUI_Showtime(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  DEV_TIME *pTime, COLOR Color)
{
	uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	sFONT *Font;

	// According to the display area adaptive font size
	POINT Dx = (Xend - Xstart) / 7; // determine the spacing between
					// characters
	POINT Dy = Yend - Ystart;	// determine the font size
	Font = GUI_GetFontSize(Dx, Dy);

	if ((pTime->Min == 0) && (pTime->Sec <= 1)) {
		// 00:00:00
		GUI_DrawRectangle(Xstart, Ystart, Xend, Yend,
				  FONT_BACKGROUND, DRAW_FULL, DOT_PIXEL_1X1);
		goto done;
	}

	if (((pTime->Min % 10) == 0) && (pTime->Sec <= 1)) {
		// xx:00:00
		GUI_DrawRectangle(Xstart + Dx * 2, Ystart, Xend, Yend,
				  FONT_BACKGROUND, DRAW_FULL, DOT_PIXEL_1X1);
		goto done;
	}

	if (pTime->Sec <= 1) {
		// xx:x0:00
		GUI_DrawRectangle(Xstart + Dx * 3, Ystart, Xend, Yend,
				  FONT_BACKGROUND, DRAW_FULL, DOT_PIXEL_1X1);
		goto done;
	}

	if ((pTime->Sec % 10) <= 1) {
		// xx:xx:00
		GUI_DrawRectangle(Xstart + Dx * 5, Ystart, Xend, Yend,
				  FONT_BACKGROUND, DRAW_FULL, DOT_PIXEL_1X1);
		goto done;
	}

	// xx:xx:x0
	GUI_DrawRectangle(Xstart + Dx * 6, Ystart, Xend, Yend, FONT_BACKGROUND,
			  DRAW_FULL, DOT_PIXEL_1X1);

done:
	// Write data into the cache
	GUI_DisChar(Xstart, Ystart, value[pTime->Hour / 10], Font,
		    FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx, Ystart, value[pTime->Hour % 10], Font,
		    FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx  + Dx / 4 + Dx / 2, Ystart, ':', Font,
		    FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx * 2 + Dx / 2, Ystart, value[pTime->Min / 10],
		    Font, FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx * 3 + Dx / 2, Ystart, value[pTime->Min % 10],
		    Font, FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx * 4 + Dx / 2 - Dx / 4, Ystart, ':', Font,
		    FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx * 5, Ystart, value[pTime->Sec / 10], Font,
		    FONT_BACKGROUND, Color);
	GUI_DisChar(Xstart + Dx * 6, Ystart, value[pTime->Sec % 10], Font,
		    FONT_BACKGROUND, Color);
}
