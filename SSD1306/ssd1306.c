#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
 
#define I2C_BUS_AVAILABLE                 1               
#define SLAVE_DEVICE_NAME        "ETX_OLED" 
#define SSD1306_SLAVE_ADDR             0x3C               
#define SSD1306_MAX_SEG                 128             
#define SSD1306_MAX_LINE                  7          
#define SSD1306_DEF_FONT_SIZE             5           

static struct i2c_adapter *etx_i2c_adapter     = NULL;  
static struct i2c_client  *etx_i2c_client_oled = NULL;  
 
static uint8_t SSD1306_LineNum   = 0;
static uint8_t SSD1306_CursorPos = 0;
static uint8_t SSD1306_FontSize  = SSD1306_DEF_FONT_SIZE;


static const unsigned char SSD1306_font[][SSD1306_DEF_FONT_SIZE]= 
{
    {0x00, 0x00, 0x00, 0x00, 0x00},   // space
    {0x00, 0x00, 0x2f, 0x00, 0x00},   // !
    {0x00, 0x07, 0x00, 0x07, 0x00},   // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14},   // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},   // $
    {0x23, 0x13, 0x08, 0x64, 0x62},   // %
    {0x36, 0x49, 0x55, 0x22, 0x50},   // &
    {0x00, 0x05, 0x03, 0x00, 0x00},   // '
    {0x00, 0x1c, 0x22, 0x41, 0x00},   // (
    {0x00, 0x41, 0x22, 0x1c, 0x00},   // )
    {0x14, 0x08, 0x3E, 0x08, 0x14},   // *
    {0x08, 0x08, 0x3E, 0x08, 0x08},   // +
    {0x00, 0x00, 0xA0, 0x60, 0x00},   // ,
    {0x08, 0x08, 0x08, 0x08, 0x08},   // -
    {0x00, 0x60, 0x60, 0x00, 0x00},   // .
    {0x20, 0x10, 0x08, 0x04, 0x02},   // /

    {0x3E, 0x51, 0x49, 0x45, 0x3E},   // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},   // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},   // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},   // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},   // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},   // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},   // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},   // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},   // 9

    {0x00, 0x36, 0x36, 0x00, 0x00},   // :
    {0x00, 0x56, 0x36, 0x00, 0x00},   // ;
    {0x08, 0x14, 0x22, 0x41, 0x00},   // <
    {0x14, 0x14, 0x14, 0x14, 0x14},   // =
    {0x00, 0x41, 0x22, 0x14, 0x08},   // >
    {0x02, 0x01, 0x51, 0x09, 0x06},   // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E},   // @

    {0x7C, 0x12, 0x11, 0x12, 0x7C},   // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},   // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},   // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},   // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},   // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A},   // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   // H
    {0x00, 0x41, 0x7F, 0x41, 0x00},   // I
    {0x20, 0x40, 0x41, 0x3F, 0x01},   // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},   // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},   // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},   // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},   // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},   // R
    {0x46, 0x49, 0x49, 0x49, 0x31},   // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},   // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},   // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F},   // W
    {0x63, 0x14, 0x08, 0x14, 0x63},   // X
    {0x07, 0x08, 0x70, 0x08, 0x07},   // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},   // Z

    {0x00, 0x7F, 0x41, 0x41, 0x00},   // [
    {0x55, 0xAA, 0x55, 0xAA, 0x55},   // Backslash (Checker pattern)
    {0x00, 0x41, 0x41, 0x7F, 0x00},   // ]
    {0x04, 0x02, 0x01, 0x02, 0x04},   // ^
    {0x40, 0x40, 0x40, 0x40, 0x40},   // _
    {0x00, 0x03, 0x05, 0x00, 0x00},   // `

    {0x20, 0x54, 0x54, 0x54, 0x78},   // a
    {0x7F, 0x48, 0x44, 0x44, 0x38},   // b
    {0x38, 0x44, 0x44, 0x44, 0x20},   // c
    {0x38, 0x44, 0x44, 0x48, 0x7F},   // d
    {0x38, 0x54, 0x54, 0x54, 0x18},   // e
    {0x08, 0x7E, 0x09, 0x01, 0x02},   // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},   // g
    {0x7F, 0x08, 0x04, 0x04, 0x78},   // h
    {0x00, 0x44, 0x7D, 0x40, 0x00},   // i
    {0x40, 0x80, 0x84, 0x7D, 0x00},   // j
    {0x7F, 0x10, 0x28, 0x44, 0x00},   // k
    {0x00, 0x41, 0x7F, 0x40, 0x00},   // l
    {0x7C, 0x04, 0x18, 0x04, 0x78},   // m
    {0x7C, 0x08, 0x04, 0x04, 0x78},   // n
    {0x38, 0x44, 0x44, 0x44, 0x38},   // o
    {0xFC, 0x24, 0x24, 0x24, 0x18},   // p
    {0x18, 0x24, 0x24, 0x18, 0xFC},   // q
    {0x7C, 0x08, 0x04, 0x04, 0x08},   // r
    {0x48, 0x54, 0x54, 0x54, 0x20},   // s
    {0x04, 0x3F, 0x44, 0x40, 0x20},   // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C},   // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C},   // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C},   // w
    {0x44, 0x28, 0x10, 0x28, 0x44},   // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},   // y
    {0x44, 0x64, 0x54, 0x4C, 0x44},   // z

    {0x00, 0x10, 0x7C, 0x82, 0x00},   // {
    {0x00, 0x00, 0xFF, 0x00, 0x00},   // |
    {0x00, 0x82, 0x7C, 0x10, 0x00},   // }
    {0x00, 0x06, 0x09, 0x09, 0x06}    // ~ (Degrees)
};

 

static int I2C_Write(unsigned char *buf, unsigned int len)
{

  int ret = i2c_master_send(etx_i2c_client_oled, buf, len);
  
  return ret;
}
 
/*static int I2C_Read(unsigned char *out_buf, unsigned int len)
{

  int ret = i2c_master_recv(etx_i2c_client_oled, out_buf, len);
  
  return ret;
}*/
 

static void SSD1306_Write(bool is_cmd, unsigned char data)
{
  unsigned char buf[2] = {0};
  int ret;
  
  if( is_cmd == true )
  {
      buf[0] = 0x00;
  }
  else
  {
      buf[0] = 0x40;
  }
  
  buf[1] = data;
  
  ret = I2C_Write(buf, 2);
}
static void SSD1306_Fill(unsigned char data)
{
  unsigned int total  = 128 * 8;  
  unsigned int i      = 0;
  
  for(i = 0; i < total; i++)
  {
      SSD1306_Write(false, data);
  }
}

static void SSD1306_SetCursor( uint8_t lineNo, uint8_t cursorPos )
{
  if((lineNo <= SSD1306_MAX_LINE) && (cursorPos < SSD1306_MAX_SEG))
  {
    SSD1306_LineNum   = lineNo;            
    SSD1306_CursorPos = cursorPos;          

    SSD1306_Write(true, 0x21);              
    SSD1306_Write(true, cursorPos);         
    SSD1306_Write(true, SSD1306_MAX_SEG-1); 

    SSD1306_Write(true, 0x22);              
    SSD1306_Write(true, lineNo);            
    SSD1306_Write(true, SSD1306_MAX_LINE);  
  }
}

static void  SSD1306_GoToNextLine( void )
{

  SSD1306_LineNum++;
  SSD1306_LineNum = (SSD1306_LineNum & SSD1306_MAX_LINE);

  SSD1306_SetCursor(SSD1306_LineNum,0); 
}

static void SSD1306_PrintChar(unsigned char c)
{
  uint8_t data_byte;
  uint8_t temp = 0;
 
  if( (( SSD1306_CursorPos + SSD1306_FontSize ) >= SSD1306_MAX_SEG ) ||
      ( c == '\n' )
  )
  {
    SSD1306_GoToNextLine();
  }

  if( c != '\n' )
  {
  
    c -= 0x20; 

    do
    {
      data_byte= SSD1306_font[c][temp]; 

      SSD1306_Write(false, data_byte);  
      SSD1306_CursorPos++;
      
      temp++;
      
    } while ( temp < SSD1306_FontSize);
    SSD1306_Write(false, 0x00);         
    SSD1306_CursorPos++;
  }
}


static void SSD1306_String(unsigned char *str)
{
  while(*str)
  {
    SSD1306_PrintChar(*str++);
  }
}


static void SSD1306_InvertDisplay(bool need_to_invert)
{
  if(need_to_invert)
  {
    SSD1306_Write(true, 0xA7); 
  }
  else
  {
    SSD1306_Write(true, 0xA6); 
  }
}


static void SSD1306_SetBrightness(uint8_t brightnessValue)
{
    SSD1306_Write(true, 0x81); 
    SSD1306_Write(true, brightnessValue); 
}


static void SSD1306_StartScrollHorizontal( bool is_left_scroll,uint8_t start_line_no,uint8_t end_line_no)
{
  if(is_left_scroll)
  {
    SSD1306_Write(true, 0x27);
  }
  else
  { 
    SSD1306_Write(true, 0x26);
  }
  
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, start_line_no);   
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, end_line_no);     
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, 0xFF);            
  SSD1306_Write(true, 0x2F);            
}

static void SSD1306_StartScrollVerticalHorizontal( bool is_vertical_left_scroll,uint8_t start_line_no,uint8_t end_line_no,uint8_t vertical_area,uint8_t rows)
{
  
  SSD1306_Write(true, 0xA3);            
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, vertical_area);   
  
  if(is_vertical_left_scroll)
  {
    SSD1306_Write(true, 0x2A);
  }
  else
  { 
    SSD1306_Write(true, 0x29);
  }
  
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, start_line_no);   
  SSD1306_Write(true, 0x00);            
  SSD1306_Write(true, end_line_no);     
  SSD1306_Write(true, rows);            
  SSD1306_Write(true, 0x2F);            
}
 
 
static int SSD1306_DisplayInit(void)
{
  msleep(100);              


  SSD1306_Write(true, 0xAE); 
  SSD1306_Write(true, 0xD5); 
  SSD1306_Write(true, 0x80); 
  SSD1306_Write(true, 0xA8); 
  SSD1306_Write(true, 0x3F); 
  SSD1306_Write(true, 0xD3); 
  SSD1306_Write(true, 0x00); 
  SSD1306_Write(true, 0x40); 
  SSD1306_Write(true, 0x8D); 
  SSD1306_Write(true, 0x14); 
  SSD1306_Write(true, 0x20); 
  SSD1306_Write(true, 0x00); 
  SSD1306_Write(true, 0xA1); 
  SSD1306_Write(true, 0xC8); 
  SSD1306_Write(true, 0xDA); 
  SSD1306_Write(true, 0x12); 
  SSD1306_Write(true, 0x81); 
  SSD1306_Write(true, 0x80); 
  SSD1306_Write(true, 0xD9); 
  SSD1306_Write(true, 0xF1); 
  SSD1306_Write(true, 0xDB); 
  SSD1306_Write(true, 0x20); 
  SSD1306_Write(true, 0xA4); 
  SSD1306_Write(true, 0xA6); 
  SSD1306_Write(true, 0x2E); 
  SSD1306_Write(true, 0xAF); 
  
  SSD1306_Fill(0x00);
  return 0;
}
 

static int etx_oled_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
  SSD1306_DisplayInit();
  

  SSD1306_SetCursor(0,0);  
  SSD1306_StartScrollHorizontal( true, 0, 2);


  SSD1306_String("Welcome\nTo\nEmbeTronicX\n\n");
  
  pr_info("OLED Probed!!!\n");
  
  return 0;
}
 

static int etx_oled_remove(struct i2c_client *client)
{
  SSD1306_String("ThanK YoU!!!");
  
  msleep(1000);
  
  SSD1306_SetCursor(0,0);  
  SSD1306_Fill(0x00);
  
  SSD1306_Write(true, 0xAE); 
  
  pr_info("OLED Removed!!!\n");
  return 0;
}
 

static const struct i2c_device_id etx_oled_id[] = {
  { SLAVE_DEVICE_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, etx_oled_id);
 

static struct i2c_driver etx_oled_driver = {
  .driver = {
    .name   = SLAVE_DEVICE_NAME,
    .owner  = THIS_MODULE,
  },
  .probe          = etx_oled_probe,
  .remove         = etx_oled_remove,
  .id_table       = etx_oled_id,
};
 

static struct i2c_board_info oled_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)
};
 

static int __init etx_driver_init(void)
{
  int ret = -1;
  etx_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
  
  if( etx_i2c_adapter != NULL )
  {
    etx_i2c_client_oled = i2c_new_device(etx_i2c_adapter, &oled_i2c_board_info);
    
    if( etx_i2c_client_oled != NULL )
    {
      i2c_add_driver(&etx_oled_driver);
      ret = 0;
    }
      
      i2c_put_adapter(etx_i2c_adapter);
  }
  
  pr_info("Driver Added!!!\n");
  return ret;
}
 

static void __exit etx_driver_exit(void)
{
  i2c_unregister_device(etx_i2c_client_oled);
  i2c_del_driver(&etx_oled_driver);
  pr_info("Driver Removed!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("SSD1306 I2C Driver");
MODULE_VERSION("1.40");
