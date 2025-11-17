void LCD_IC_Init()
{
//************* Start Initial Sequence **********
	
WriteComm(0x3A);  
WriteData(0x55); 
 
WriteComm(0xF6);  
WriteData(0x01); 
WriteData(0x33); 

	
WriteComm(0xB5);  
WriteData(0x04); 
WriteData(0x04); 
WriteData(0x0A); 
WriteData(0x14); 
 
WriteComm(0x35);  
WriteData(0x00); 
 
WriteComm(0xCF);  
WriteData(0x00); 
WriteData(0xEA); 
WriteData(0XF0); 
 
WriteComm(0xED);  
WriteData(0x64); 
WriteData(0x03); 
WriteData(0X12); 
WriteData(0X81);

WriteComm(0xE8);  
WriteData(0x85); 
WriteData(0x00); 
WriteData(0x78); 
 
WriteComm(0xCB);  
WriteData(0x39); 
WriteData(0x2C); 
WriteData(0x00); 
WriteData(0x33); 
WriteData(0x06); 
 
WriteComm(0xF7);  
WriteData(0x20); 
 
WriteComm(0xEA);  
WriteData(0x00); 
WriteData(0x00); 
 
WriteComm(0xC0);    //Power control 
WriteData(0x21);   //VRH[5:0] 
 
WriteComm(0xC1);    //Power control 
WriteData(0x10);   //BT[3:0] 
 
WriteComm(0xC5);    //VCM control 
WriteData(0x4F); 
WriteData(0x38); 

WriteComm(0xC7);  
WriteData(0x98); 

 
WriteComm(0x36);    // Memory Access Control 
WriteData(0x08); 
 
WriteComm(0xB1);   
WriteData(0x00);   
WriteData(0x13); 
 
WriteComm(0xB6);    // Display Function Control 
WriteData(0x0A); 
WriteData(0xA2); 
 
WriteComm(0xF2);    // 3Gamma Function Disable
WriteData(0x02); 
 
WriteComm(0xE0);    //Set Gamma 
WriteData(0x0F); 
WriteData(0x27); 
WriteData(0x24); 
WriteData(0x0C); 
WriteData(0x10); 
WriteData(0x08); 
WriteData(0x55); 
WriteData(0X87); 
WriteData(0x45); 
WriteData(0x08); 
WriteData(0x14); 
WriteData(0x07); 
WriteData(0x13); 
WriteData(0x08); 
WriteData(0x00); 
 
WriteComm(0XE1);    //Set Gamma 
WriteData(0x00); 
WriteData(0x0F); 
WriteData(0x12); 
WriteData(0x05); 
WriteData(0x11); 
WriteData(0x06); 
WriteData(0x25); 
WriteData(0x34); 
WriteData(0x37); 
WriteData(0x01); 
WriteData(0x08); 
WriteData(0x07); 
WriteData(0x2B); 
WriteData(0x34); 
WriteData(0x0F); 

WriteComm(0x11);    //Exit Sleep 
Delay(120); 
WriteComm(0x29);    //Display on 


}
