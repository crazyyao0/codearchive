// DCT needs 28 adds and 22 mul
// IDCT needs 32 adds and 32 mul

#define  C1 0.98078528f			//sin(PI*7/16) or cos(PI*1/16)
#define  C2 0.92387953f			//sin(PI*6/16) or cos(PI*2/16)
#define  C3 0.83146961f			//sin(PI*5/16) or cos(PI*3/16)
#define  C4 0.70710678f			//sin(PI*4/16) or cos(PI*4/16)
#define  C5 0.55557023f			//sin(PI*3/16) or cos(PI*5/16)
#define  C6 0.38268343f			//sin(PI*2/16) or cos(PI*6/16)
#define  C7 0.19509032f			//sin(PI/16)   or cos(PI*7/16)

void fdctrow(float *blk)
{ 
	float S07,S16,S25,S34,S0734,S1625;
	float D07,D16,D25,D34,D0734,D1625;

	S07=blk[0]+blk[7];
	S16=blk[1]+blk[6];
	S25=blk[2]+blk[5];
	S34=blk[3]+blk[4];
	S0734=S07+S34;
	S1625=S16+S25;

	D07=blk[0]-blk[7]; 
	D16=blk[1]-blk[6];
	D25=blk[2]-blk[5];
	D34=blk[3]-blk[4];
	D0734=S07-S34;
	D1625=S16-S25;

	blk[0]=0.5f*(C4*(S0734+S1625));
	blk[1]=0.5f*(C1*D07+C3*D16+C5*D25+C7*D34);
	blk[2]=0.5f*(C2*D0734+C6*D1625);
	blk[3]=0.5f*(C3*D07-C7*D16-C1*D25-C5*D34);
	blk[4]=0.5f*(C4*(S0734-S1625));
	blk[5]=0.5f*(C5*D07-C1*D16+C7*D25+C3*D34);
	blk[6]=0.5f*(C6*D0734-C2*D1625);
	blk[7]=0.5f*(C7*D07-C5*D16+C3*D25-C1*D34);
}

void fdctcol(float *blk)
{
	float S07,S16,S25,S34,S0734,S1625;
	float D07,D16,D25,D34,D0734,D1625;

	S07=blk[0*8]+blk[7*8];
	S16=blk[1*8]+blk[6*8];
	S25=blk[2*8]+blk[5*8];
	S34=blk[3*8]+blk[4*8];
	S0734=S07+S34;
	S1625=S16+S25;

	D07=blk[0*8]-blk[7*8]; 
	D16=blk[1*8]-blk[6*8];
	D25=blk[2*8]-blk[5*8];
	D34=blk[3*8]-blk[4*8];
	D0734=S07-S34;
	D1625=S16-S25;

	blk[0*8]=0.5f*(C4*(S0734+S1625));
	blk[1*8]=0.5f*(C1*D07+C3*D16+C5*D25+C7*D34);
	blk[2*8]=0.5f*(C2*D0734+C6*D1625);
	blk[3*8]=0.5f*(C3*D07-C7*D16-C1*D25-C5*D34);
	blk[4*8]=0.5f*(C4*(S0734-S1625));
	blk[5*8]=0.5f*(C5*D07-C1*D16+C7*D25+C3*D34);
	blk[6*8]=0.5f*(C6*D0734-C2*D1625);
	blk[7*8]=0.5f*(C7*D07-C5*D16+C3*D25-C1*D34);
}

void dct(float *block)
{
	int i;
	for (i=0; i<8; i++)
		fdctrow(block+8*i);
	for (i=0; i<8; i++)
		fdctcol(block+i);
}

void idctrow(float *blk)
{
	float tmp[16];

	tmp[0]=blk[0]*C4+blk[2]*C2;
	tmp[1]=blk[4]*C4+blk[6]*C6;
	tmp[2]=blk[0]*C4+blk[2]*C6;
	tmp[3]=-blk[4]*C4-blk[6]*C2;
	tmp[4]=blk[0]*C4-blk[2]*C6;
	tmp[5]=-blk[4]*C4+blk[6]*C2;
	tmp[6]=blk[0]*C4-blk[2]*C2;
	tmp[7]=blk[4]*C4-blk[6]*C6;

	tmp[8]=blk[1]*C7-blk[3]*C5;
	tmp[9]=blk[5]*C3-blk[7]*C1;
	tmp[10]=blk[1]*C5-blk[3]*C1;
	tmp[11]=blk[5]*C7+blk[7]*C3;
	tmp[12]=blk[1]*C3-blk[3]*C7;
	tmp[13]=-blk[5]*C1-blk[7]*C5;
	tmp[14]=blk[1]*C1+blk[3]*C3;
	tmp[15]=blk[5]*C5+blk[7]*C7;

	tmp[0]=0.5f*(tmp[0]+tmp[1]);
	tmp[1]=0.5f*(tmp[2]+tmp[3]);
	tmp[2]=0.5f*(tmp[4]+tmp[5]);
	tmp[3]=0.5f*(tmp[6]+tmp[7]);
	tmp[4]=0.5f*(tmp[8]+tmp[9]);
	tmp[5]=0.5f*(tmp[10]+tmp[11]);
	tmp[6]=0.5f*(tmp[12]+tmp[13]);
	tmp[7]=0.5f*(tmp[14]+tmp[15]);

	blk[0]=tmp[0]+tmp[7];
	blk[1]=tmp[1]+tmp[6];
	blk[2]=tmp[2]+tmp[5];
	blk[3]=tmp[3]+tmp[4];
	blk[4]=tmp[3]-tmp[4];
	blk[5]=tmp[2]-tmp[5];
	blk[6]=tmp[1]-tmp[6];
	blk[7]=tmp[0]-tmp[7];
}



void idctcol(float *blk)
{
	float tmp[16];

	/*first step*/
	tmp[0]=blk[0*8]*C4+blk[2*8]*C2;
	tmp[1]=blk[4*8]*C4+blk[6*8]*C6;
	tmp[2]=blk[0*8]*C4+blk[2*8]*C6;
	tmp[3]=-blk[4*8]*C4-blk[6*8]*C2;
	tmp[4]=blk[0*8]*C4-blk[2*8]*C6;
	tmp[5]=-blk[4*8]*C4+blk[6*8]*C2;
	tmp[6]=blk[0*8]*C4-blk[2*8]*C2;
	tmp[7]=blk[4*8]*C4-blk[6*8]*C6;

	tmp[8]=blk[1*8]*C7-blk[3*8]*C5;
	tmp[9]=blk[5*8]*C3-blk[7*8]*C1;
	tmp[10]=blk[1*8]*C5-blk[3*8]*C1;
	tmp[11]=blk[5*8]*C7+blk[7*8]*C3;
	tmp[12]=blk[1*8]*C3-blk[3*8]*C7;
	tmp[13]=-blk[5*8]*C1-blk[7*8]*C5;
	tmp[14]=blk[1*8]*C1+blk[3*8]*C3;
	tmp[15]=blk[5*8]*C5+blk[7*8]*C7;


	/*second step*/
	tmp[0]=0.5f*(tmp[0]+tmp[1]);
	tmp[1]=0.5f*(tmp[2]+tmp[3]);
	tmp[2]=0.5f*(tmp[4]+tmp[5]);
	tmp[3]=0.5f*(tmp[6]+tmp[7]);
	tmp[4]=0.5f*(tmp[8]+tmp[9]);
	tmp[5]=0.5f*(tmp[10]+tmp[11]);
	tmp[6]=0.5f*(tmp[12]+tmp[13]);
	tmp[7]=0.5f*(tmp[14]+tmp[15]);

	/*third step*/
	blk[0*8]=tmp[0]+tmp[7];
	blk[1*8]=tmp[1]+tmp[6];
	blk[2*8]=tmp[2]+tmp[5];
	blk[3*8]=tmp[3]+tmp[4];
	blk[4*8]=tmp[3]-tmp[4];
	blk[5*8]=tmp[2]-tmp[5];
	blk[6*8]=tmp[1]-tmp[6];
	blk[7*8]=tmp[0]-tmp[7];
}


void idct(float *block)
{
	int i;

	for (i=0; i<8; i++)
		idctrow(block+8*i);

	for (i=0; i<8; i++)
		idctcol(block+i);
}
