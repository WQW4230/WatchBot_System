//default
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) //
{
	 WriteComm(0x2a);
	 WriteData((Xstart)>>8);
	 WriteData((Xstart));
	 WriteData((Xend)>>8);
	 WriteData(Xend);
	
	 WriteComm(0x2b);
	 WriteData((Ystart)>>8);
	 WriteData((Ystart));
	 WriteData((Yend)>>8);
	 WriteData(Yend);

	 WriteComm(0x2c); 	
}
