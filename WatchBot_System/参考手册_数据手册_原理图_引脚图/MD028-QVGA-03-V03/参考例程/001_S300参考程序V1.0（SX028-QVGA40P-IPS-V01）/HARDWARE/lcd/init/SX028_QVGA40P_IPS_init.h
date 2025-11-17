void LCD_IC_Init()
{
      WriteComm(0x36); 							
      WriteData(0x00);

      WriteComm(0x3a); 
      WriteData(0x55); 
      //--ST7789V Frame rate setting-
      WriteComm(0xb2); 
      WriteData(0x0c); 
      WriteData(0x0c);			
      WriteData(0x00); 
      WriteData(0x33); 
      WriteData(0x33); 

      WriteComm(0xb7); 
      WriteData(0x74); 
      //-ST7789V Power setting-------// 
      WriteComm(0xbb);  //VCOM
      WriteData(0x20); //0x2b 

      WriteComm(0xc0); 
      WriteData(0x2c); 

      WriteComm(0xc2); 
      WriteData(0x01); 


      WriteComm(0xc3); 
      WriteData(0x10);  //VRH


      WriteComm(0xc4); 
      WriteData(0x15); //VDV

      WriteComm(0xc6); 
      WriteData(0x0f);  //FRAME RATE

      WriteComm(0xd0); 
      WriteData(0xa4); 
      WriteData(0xa1); 
      //--ST7789V gamma setting----// 
      WriteComm(0xe0); 
      WriteData(0xd0); 
      WriteData(0x00); 
      WriteData(0x05); 
      WriteData(0x0e); 
      WriteData(0x15); 
      WriteData(0x0d); 
      WriteData(0x37); 
      WriteData(0x43); 
      WriteData(0x47); 
      WriteData(0x09); 
      WriteData(0x15); 
      WriteData(0x12); 
      WriteData(0x16); 
      WriteData(0x19); 

      WriteComm(0xe1); 
      WriteData(0xd0); 
      WriteData(0x00); 
      WriteData(0x05); 
      WriteData(0x0d); 
      WriteData(0x0c); 
      WriteData(0x06); 
      WriteData(0x2d); 
      WriteData(0x44); 
      WriteData(0x40); 
      WriteData(0x0e); 
      WriteData(0x1c); 
      WriteData(0x18); 
      WriteData(0x16); 
      WriteData(0x19); 

      WriteComm(0x21);  // IPS

      WriteComm(0x11); 
      Delay(120);        //Delay 120ms 
      WriteComm(0x29);
}
