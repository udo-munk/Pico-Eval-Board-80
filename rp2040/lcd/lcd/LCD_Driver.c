/*****************************************************************************
* | File      	: LCD_Driver.c
* | Author      : Waveshare team
* | Function    : LCD Drive function
* | Info        :
*----------------
* | This version: V1.0
* | Date        : 2018-01-11
* | Info        : Basic version
******************************************************************************/

/**************************Intermediate driver layer**************************/
#include "LCD_Driver.h"

LCD_DIS sLCD_DIS;
uint8_t id;

/*******************************************************************************
function:
	Hardware reset
*******************************************************************************/
static void LCD_Reset(void)
{
	DEV_Digital_Write(LCD_RST_PIN, 1);
	sleep_ms(500);
	DEV_Digital_Write(LCD_RST_PIN, 0);
	sleep_ms(500);
	DEV_Digital_Write(LCD_RST_PIN, 1);
	sleep_ms(500);
}

static void PWM_SetValue(uint16_t duty)
{	
	uint slice_num = pwm_gpio_to_slice_num(LCD_BKL_PIN);

	pwm_set_wrap(slice_num, 10000);
	pwm_set_chan_level(slice_num, PWM_CHAN_B, duty * 10);
	pwm_set_enabled(slice_num, true);
}

void LCD_SetBackLight(uint16_t value)
{
	PWM_SetValue(value);
}

/*******************************************************************************
function:
		Write register address and data
*******************************************************************************/
void LCD_WriteReg(uint8_t Reg)
{
	DEV_Digital_Write(LCD_DC_PIN, 0);
	DEV_Digital_Write(LCD_CS_PIN, 0);
	SPI4W_Write_Byte(Reg);
	DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_WriteData(uint16_t Data)
{
	DEV_Digital_Write(LCD_DC_PIN, 1);
	DEV_Digital_Write(LCD_CS_PIN, 0);
	SPI4W_Write_Byte(Data >> 8);
	SPI4W_Write_Byte(Data & 0XFF);
	DEV_Digital_Write(LCD_CS_PIN, 1);
}

/*******************************************************************************
function:
		Write register data
*******************************************************************************/
static void LCD_Write_AllData(uint16_t Data, uint32_t DataLen)
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
function:
		Common register initialization
*******************************************************************************/
static void LCD_InitReg(void)
{
	id = LCD_Read_Id();

	LCD_WriteReg(0x21);
	LCD_WriteReg(0xC2);  // Normal mode, increase can change the display quality, while increasing power consumption
	LCD_WriteData(0x33);
	LCD_WriteReg(0XC5);
	LCD_WriteData(0x00);
	LCD_WriteData(0x1e); // VCM_REG[7:0]. <=0X80.
	LCD_WriteData(0x80);
	LCD_WriteReg(0xB1);  // Sets the frame frequency of full color normal mode
	LCD_WriteData(0xB0); // 0XB0 =70HZ, <=0XB0.0xA0=62HZ
	LCD_WriteReg(0x36);
	LCD_WriteData(0x28); // 2 DOT FRAME MODE,F<=70HZ.
	LCD_WriteReg(0XE0);
	LCD_WriteData(0x0);
	LCD_WriteData(0x13);
	LCD_WriteData(0x18);
	LCD_WriteData(0x04);
	LCD_WriteData(0x0F);
	LCD_WriteData(0x06);
	LCD_WriteData(0x3a);
	LCD_WriteData(0x56);
	LCD_WriteData(0x4d);
	LCD_WriteData(0x03);
	LCD_WriteData(0x0a);
	LCD_WriteData(0x06);
	LCD_WriteData(0x30);
	LCD_WriteData(0x3e);
	LCD_WriteData(0x0f);		
	LCD_WriteReg(0XE1);
	LCD_WriteData(0x0);
	LCD_WriteData(0x13);
	LCD_WriteData(0x18);
	LCD_WriteData(0x01);
	LCD_WriteData(0x11);
	LCD_WriteData(0x06);
	LCD_WriteData(0x38);
	LCD_WriteData(0x34);
	LCD_WriteData(0x4d);
	LCD_WriteData(0x06);
	LCD_WriteData(0x0d);
	LCD_WriteData(0x0b);
	LCD_WriteData(0x31);
	LCD_WriteData(0x37);
	LCD_WriteData(0x0f);
	LCD_WriteReg(0X3A);  // Set Interface Pixel Format
	LCD_WriteData(0x55);
	LCD_WriteReg(0x11);  // sleep out
	sleep_ms(120);
	LCD_WriteReg(0x29);  // Turn on the LCD display
}

/********************************************************************************
function:	Set the display scan and color transfer modes
parameter:
		Scan_dir   :   Scan direction
		Colorchose :   RGB or GBR color format
********************************************************************************/
void LCD_SetGramScanWay(LCD_SCAN_DIR Scan_dir)
{
	uint16_t MemoryAccessReg_Data = 0;	// addr:0x36
	uint16_t DisFunReg_Data = 0;		// addr:0xB6

	// Pico-ResTouch-LCD-3.5
	// Gets the scan direction of GRAM
	switch (Scan_dir) {
	case L2R_U2D:
		/* Memory access control: MY = 0, MX = 0, MV = 0, ML = 0 */
		/* Display Function control: NN = 0, GS = 0, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x08;
		DisFunReg_Data = 0x22;
		break;
	case L2R_D2U:
		/* Memory access control: MY = 0, MX = 0, MV = 0, ML = 0 */
		/* Display Function control: NN = 0, GS = 1, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x08;
		DisFunReg_Data = 0x62;
		break;
	case R2L_U2D: 
		/* Memory access control: MY = 0, MX = 0, MV = 0, ML = 0 */
		/* Display Function control: NN = 0, GS = 0, SS = 0, SM = 0 */
		MemoryAccessReg_Data = 0x08;
		DisFunReg_Data = 0x02;
		break;
	case R2L_D2U: 
		/* Memory access control: MY = 0, MX = 0, MV = 0, ML = 0 */
		/* Display Function control: NN = 0, GS = 1, SS = 0, SM = 0 */
		MemoryAccessReg_Data = 0x08;
		DisFunReg_Data = 0x42;
		break;
	case U2D_L2R: // 0X2
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0 X-Y Exchange */
		/* Display Function control: NN = 0, GS = 0, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x22;
		break;
	case U2D_R2L: // 0X6
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0 X-Y Exchange */
		/* Display Function control: NN = 0, GS = 0, SS = 0, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x02;
		break;
	case D2U_L2R: // 0XA
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0 X-Y Exchange*/
		/* Display Function control: NN = 0, GS = 1, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x62;
		break;
	case D2U_R2L: // 0XE
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0 X-Y Exchange*/
		/* Display Function control: NN = 0, GS = 1, SS = 0, SM = 0	*/
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x42;
		break;
	}

	// Get the screen scan direction
	sLCD_DIS.LCD_Scan_Dir = Scan_dir;

	// Get GRAM and LCD width and height
	// 480*320, horizontal default
	if (Scan_dir == L2R_U2D || Scan_dir == L2R_D2U ||
	    Scan_dir == R2L_U2D || Scan_dir == R2L_D2U) {
		sLCD_DIS.LCD_Dis_Column	= LCD_3_5_HEIGHT ;
		sLCD_DIS.LCD_Dis_Page = LCD_3_5_WIDTH ;
	} else {
		sLCD_DIS.LCD_Dis_Column	= LCD_3_5_WIDTH ;
		sLCD_DIS.LCD_Dis_Page = LCD_3_5_HEIGHT ;
	}

	// Set the read / write scan direction of the frame memory
	LCD_WriteReg(0xB6);
	LCD_WriteData(0X00);
	LCD_WriteData(DisFunReg_Data);

	LCD_WriteReg(0x36);
	LCD_WriteData(MemoryAccessReg_Data);
}

/********************************************************************************
function:
	initialization
********************************************************************************/
void LCD_Init(LCD_SCAN_DIR LCD_ScanDir, uint16_t LCD_BLval)
{
    
	LCD_Reset();	// Hardware reset
	LCD_InitReg();	// Set the initialization register
	
	if (LCD_BLval > 1000)
		LCD_BLval = 1000;
	LCD_SetBackLight(LCD_BLval);
	
	LCD_SetGramScanWay(LCD_ScanDir); // Set the display scan and color transfer modes
	sleep_ms(200);
}

/********************************************************************************
function:	Sets the start position and size of the display area
parameter:
	Xstart 	:   X direction Start coordinates
	Ystart  :   Y direction Start coordinates
	Xend    :   X direction end coordinates
	Yend    :   Y direction end coordinates
********************************************************************************/
void LCD_SetWindow(POINT Xstart, POINT Ystart,	POINT Xend, POINT Yend)
{	

	// set the X coordinates
	LCD_WriteReg(0x2A);
	LCD_WriteData(Xstart >> 8);       // Set the horizontal starting point to the high octet
	LCD_WriteData(Xstart & 0xff);	  // Set the horizontal starting point to the low octet
	LCD_WriteData((Xend - 1) >> 8);   // Set the horizontal end to the high octet
	LCD_WriteData((Xend - 1) & 0xff); // Set the horizontal end to the low octet

	// set the Y coordinates
	LCD_WriteReg(0x2B);
	LCD_WriteData(Ystart >> 8);
	LCD_WriteData(Ystart & 0xff );
	LCD_WriteData((Yend - 1) >> 8);
	LCD_WriteData((Yend - 1) & 0xff);

	LCD_WriteReg(0x2C);
}

/********************************************************************************
function:	Set the display point (Xpoint, Ypoint)
parameter:
	xStart :   X direction Start coordinates
	xEnd   :   X direction end coordinates
********************************************************************************/
void LCD_SetCursor(POINT Xpoint, POINT Ypoint)
{
	LCD_SetWindow(Xpoint, Ypoint, Xpoint, Ypoint);
}

/********************************************************************************
function:	Set show color
parameter:
		Color  :   Set show color, 16-bit depth
********************************************************************************/
void LCD_SetColor(COLOR Color, POINT Xpoint, POINT Ypoint)
{
	LCD_Write_AllData(Color, (uint32_t) Xpoint * (uint32_t) Ypoint);
}

/********************************************************************************
function:	Point (Xpoint, Ypoint) Fill the color
parameter:
	Xpoint :   The x coordinate of the point
	Ypoint :   The y coordinate of the point
	Color  :   Set the color
********************************************************************************/
void LCD_SetPointlColor(POINT Xpoint, POINT Ypoint, COLOR Color)
{
	if ((Xpoint <= sLCD_DIS.LCD_Dis_Column) &&
	    (Ypoint <= sLCD_DIS.LCD_Dis_Page)) {
		LCD_SetCursor (Xpoint, Ypoint);
		LCD_SetColor(Color, 1, 1);
	}
}

/********************************************************************************
function:	Fill the area with the color
parameter:
	Xstart :   Start point x coordinate
	Ystart :   Start point y coordinate
	Xend   :   End point coordinates
	Yend   :   End point coordinates
	Color  :   Set the color
********************************************************************************/
void LCD_SetArealColor(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
		       COLOR Color)
{
	if ((Xend > Xstart) && (Yend > Ystart)) {
		LCD_SetWindow(Xstart, Ystart, Xend, Yend);
		LCD_SetColor (Color, Xend - Xstart, Yend - Ystart);
	}
}

/********************************************************************************
function:
			Clear screen
********************************************************************************/
void LCD_Clear(COLOR Color)
{
	LCD_SetArealColor(0, 0, sLCD_DIS.LCD_Dis_Column, sLCD_DIS.LCD_Dis_Page,
			  Color);
}

uint8_t LCD_Read_Id(void)
{
	uint8_t reg = 0xDC;
	uint8_t tx_val = 0x00;
	uint8_t rx_val;

	DEV_Digital_Write(LCD_CS_PIN, 0);
	DEV_Digital_Write(LCD_DC_PIN, 0);
	SPI4W_Write_Byte(reg);
	spi_write_read_blocking(SPI_PORT, &tx_val, &rx_val, 1);
	DEV_Digital_Write(LCD_CS_PIN, 1);

	return rx_val;
}
