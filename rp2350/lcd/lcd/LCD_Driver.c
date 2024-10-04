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
function:	Hardware reset
*******************************************************************************/
static void LCD_Reset(void)
{
	DEV_Digital_Write(LCD_RST_PIN, 1);
	sleep_ms(100);
	DEV_Digital_Write(LCD_RST_PIN, 0);
	sleep_ms(100);
	DEV_Digital_Write(LCD_RST_PIN, 1);
	sleep_ms(100);
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
function:	Common register initialization
*******************************************************************************/
static void LCD_InitReg(void)
{
	id = LCD_Read_Id();  // not really needed, we support the 3.5" LCD only

	LCD_WriteReg(0x21);  // Display Inversion On

	LCD_WriteReg(0xc2);  // Normal mode
	LCD_WriteData(0x33); // increase can change the display quality,
			     // while increasing power consumption

	LCD_WriteReg(0xc5);  // VCOM Control
	LCD_WriteData(0x00); // 0: NV memory is not programmed
	LCD_WriteData(0x1e); // VCM_REG [7:0] = -1.53125
	LCD_WriteData(0x80); // 1: VCOM value from VCM_REG [7:0]
	LCD_WriteData(0x40); // VCM_OUT [7:0]: NV memory programmed value

	LCD_WriteReg(0xb1);  // Sets the frame frequency of full color normal
			     // mode
	LCD_WriteData(0xb0); // 0xb0 =70HZ, <=0xb0.0xa0=62HZ
	LCD_WriteData(0x11); // 17 clocks per line

	LCD_WriteReg(0x36);  // Memory Access Control
	LCD_WriteData(0x28);

	LCD_WriteReg(0xe0);  // Positive Gamma Control
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

	LCD_WriteReg(0xe1);  // Negative Gamma Control
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

	LCD_WriteReg(0x3A);  // Set Interface Pixel Format
	LCD_WriteData(0x55); // 16 bits/pixel

	LCD_WriteReg(0x11);  // sleep out
	sleep_ms(120);

	LCD_WriteReg(0x29);  // Turn on the LCD display
}

/*******************************************************************************
function:	Set the display scan and color transfer modes
parameter:
		Scan_dir   :   Scan direction
		Colorchose :   RGB or GBR color format
*******************************************************************************/
void LCD_SetGramScanWay(LCD_SCAN_DIR Scan_dir)
{
	uint16_t MemoryAccessReg_Data = 0;
	uint16_t DisFunReg_Data = 0;

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
	case U2D_L2R:
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0
		   X-Y Exchange */
		/* Display Function control: NN = 0, GS = 0, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x22;
		break;
	case U2D_R2L:
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0
		   X-Y Exchange */
		/* Display Function control: NN = 0, GS = 0, SS = 0, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x02;
		break;
	case D2U_L2R:
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0
		   X-Y Exchange */
		/* Display Function control: NN = 0, GS = 1, SS = 1, SM = 0 */
		MemoryAccessReg_Data = 0x28;
		DisFunReg_Data = 0x62;
		break;
	case D2U_R2L:
		/* Memory access control: MY = 0, MX = 0, MV = 1, ML = 0
		   X-Y Exchange */
		/* Display Function control: NN = 0, GS = 1, SS = 0, SM = 0 */
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
		sLCD_DIS.LCD_Dis_Column	= LCD_3_5_HEIGHT;
		sLCD_DIS.LCD_Dis_Page = LCD_3_5_WIDTH;
	} else {
		sLCD_DIS.LCD_Dis_Column	= LCD_3_5_WIDTH;
		sLCD_DIS.LCD_Dis_Page = LCD_3_5_HEIGHT;
	}

	// Set the read / write scan direction of the frame memory
	LCD_WriteReg(0xb6);	// Display Function Control
	LCD_WriteData(0x00);
	LCD_WriteData(DisFunReg_Data);

	LCD_WriteReg(0x36);	// Memory Access Control
	LCD_WriteData(MemoryAccessReg_Data);
}

/*******************************************************************************
function:	initialization
*******************************************************************************/
void LCD_Init(LCD_SCAN_DIR LCD_ScanDir, uint16_t LCD_BLval)
{
    
	LCD_Reset();	// Hardware reset
	LCD_InitReg();	// Set the initialization register
	
	if (LCD_BLval > 1000)
		LCD_BLval = 1000;
	LCD_SetBackLight(LCD_BLval);
	
	LCD_SetGramScanWay(LCD_ScanDir); // Set the display scan and color
					 // transfer modes
}

/*******************************************************************************
function:	Clear screen
*******************************************************************************/
void LCD_Clear(COLOR Color)
{
	LCD_SetArealColor(0, 0, sLCD_DIS.LCD_Dis_Column, sLCD_DIS.LCD_Dis_Page,
			  Color);
}

uint8_t LCD_Read_Id(void)
{
	uint8_t reg = 0xdc;
	uint8_t tx_val = 0x00;
	uint8_t rx_val;

	DEV_Digital_Write(LCD_CS_PIN, 0);
	DEV_Digital_Write(LCD_DC_PIN, 0);
	SPI4W_Write_Byte(reg);
	spi_write_read_blocking(SPI_PORT, &tx_val, &rx_val, 1);
	DEV_Digital_Write(LCD_CS_PIN, 1);

	return rx_val;
}
