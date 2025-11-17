void LCD_IC_Init()
{
	WriteComm(0xCF);  
	WriteData(0x00); 
	WriteData(0xc1); 
	WriteData(0X30); 
	 
	WriteComm(0xED);  
	WriteData(0x64); 
	WriteData(0x03); 
	WriteData(0X12); 
	WriteData(0X81); 
	 
	WriteComm(0xE8);  
	WriteData(0x85); 
	WriteData(0x10); 
	WriteData(0x7a); 
	 
	WriteComm(0xCB);  
	WriteData(0x39); 
	WriteData(0x2C); 
	WriteData(0x00); 
	WriteData(0x34); 
	WriteData(0x02); 
	 
	WriteComm(0xF7);  
	WriteData(0x20); 
	 
	WriteComm(0xEA);  
	WriteData(0x00); 
	WriteData(0x00); 

WriteComm(0xC0);    //Power control 
WriteData(0x21);   //VRH[5:0] 
 
WriteComm(0xC1);    //Power control 
WriteData(0x10);   //SAP[2:0];BT[3:0] 
 
WriteComm(0xC5);    //VCM control 
WriteData(0x16); 
WriteData(0x3F); 
 
 
WriteComm(0x3A);    
WriteData(0x55); 

WriteComm(0x36);    // Memory Access Control 
//WriteData(0x88);//08 48
 //WriteData(0x80);//08 48///(RGB)（12点视角）
WriteData(0x08);//08 48///(RGB)       （6点视角）



WriteComm(0xb1); 
WriteData(0x00); 
WriteData(0x18); 
 

WriteComm(0xb6); 
WriteData(0x0A); 
WriteData(0xA2); 
 
 
WriteComm(0xF2);    // 3Gamma Function Disable 
WriteData(0x00); 
 
WriteComm(0x26);    //Gamma curve selected 
WriteData(0x01); 

WriteComm(0x20); 

WriteComm(0xE0);    //Set Gamma 
WriteData(0xD0); 
WriteData(0x00); 
WriteData(0x02); 
WriteData(0x07); 
WriteData(0x0B); 
WriteData(0x1A); 
WriteData(0x31); 
WriteData(0x54); 
WriteData(0x40); 
WriteData(0x29); 
WriteData(0x12); 
WriteData(0x12);
WriteData(0x12);
WriteData(0x17); 

WriteComm(0XE1);    //Set Gamma 
WriteData(0xD0); 
WriteData(0x00); 
WriteData(0x02); 
WriteData(0x07); 
WriteData(0x05); 
WriteData(0x25); 
WriteData(0x2D); 
WriteData(0x44); 
WriteData(0x45); 
WriteData(0x1C); 
WriteData(0x18); 
WriteData(0x16); 
WriteData(0x1C); 
WriteData(0x1D);


	//ClearColor(15);
	 
	WriteComm(0x11);    //Exit Sleep 
	
	WriteComm(0x29);    //Display on 

}
