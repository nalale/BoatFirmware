#include <math.h>
#include <stdint.h>
#include <string.h>

/*
 * GY-GPS6MV2 GPS receiver module example program using
 * the STM32F0 Discovery board available from STMicroelectronics
 *
 * Author: Harris Shallcross
 * Year: ~13/9/2014
 *
 *An example of using the GY-GPS6MV2 GPS receiver with the STM32F0
 *Discovery board. Sentences are received and separated into
 *different buffers with flags controlling whether these sentences
 *can be overwritten. Flags are also present to tell when a sentence
 *has been received.
 *
 *Code and example descriptions can be found on my blog at:
 *www.hsel.co.uk
 *
 *The MIT License (MIT)
 *Copyright (c) 2014 Harris Shallcross
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in all
 *copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *SOFTWARE.
 */

#define GMT

enum
{
    commaPos_CurrentTime = 0,
    commaPos_Status,
    commaPos_Latitude,
    commaPos_LatSide,
    commaPos_Longtitude,
    commaPos_LongSide,
    commaPos_Speed,
    commaPos_TrackAngle,
    commaPos_CurrentDate,
    commaPos_MagneticVariation,
} GPRMC_CommaPos_e;

typedef struct
{
    int32_t CurrentTime;
    char Status;
    int32_t Latitude;
    char LatSide;
    int32_t Longitude;
    char LongtitudeSide;
    int32_t Speed_0p1Knots;
    int32_t TrackAngle_0p1Deg;
    int32_t CurrentDate;
    int32_t MagneticVariation;

} GPRMC_RawStruct_t;

enum
{
    commaPos_VTG_CourseOverGround_T = 0,
    commaPos_VTG_Unit_T,
    commaPos_VTG_CourseOverGround_M,
    commaPos_VTG_Unit_M,
    commaPos_VTG_Speed_knots,
    commaPos_VTG_Unit_kn,
    commaPos_VTG_Speed_kmph,
    commaPos_VTG_Unit_kmph,
    commaPos_VTG_ModeIndicator,    
} GPVTG_CommaPos_e;

typedef struct
{
    int32_t CourseOverGround_T;
    char Unit_T;
    int32_t CourseOverGround_M;
    char Unit_M;
    int32_t Speed_knots;
    char Unit_K;
    int32_t Speed_kmph;
    char Unit_Kmph;
    char ModeIndicator;    

} GPVTG_RawStruct_t;

GPRMC_RawStruct_t gprmc_raw_data;
GPVTG_RawStruct_t gpvtg_raw_data;

static void GPRMC_FillStruct(void);
static void GPVTG_FillStruct(void);

//Simple string compare function. This function compares strings
//up to "Amnt" places. It will also terminate the compare if the end
//of a string is met before Amnt has been met, returning 0 to say the
//strings aren't equal to Amnt places.
uint8_t StrCmp(char *StrA, char *StrB, uint8_t Amnt){
	uint8_t Cnt;

	//Check through string and compare. If Amnt > either string length
	//Ensure for loop quits at terminator character
	for(Cnt = 0; Cnt<Amnt && StrA[Cnt] != 0 && StrB[Cnt] != 0; Cnt++){
		//for(Cnt = 0; Cnt<Amnt; Cnt++){
		if(StrA[Cnt] != StrB[Cnt]){
			break;
		}
	}

	//If Cnt and Amnt are equal, return 1, otherwise return 0
	return(Cnt == Amnt);
}

//Copy a string from source to destination to Amnt places
void StrCpy(char *StrSrc, char *StrDest, uint8_t Amnt){
	uint8_t Cnt;

	for(Cnt = 0; Cnt<Amnt; Cnt++){
		StrDest[Cnt] = StrSrc[Cnt];
	}
}

//Copy a string from source to destination until a certain
//character is met in the source string. Really useful for
//copying variable length strings terminated by a specific
//character!
void StrCpyCh(char *StrSrc, char *StrDest, char Char){
	uint8_t Index = 0;

	while(StrSrc[Index] != Char && Index < 100){
		StrDest[Index] = StrSrc[Index];
		Index++;
	}
}

//Search through a string looking for the positions of all commas within
//that string.
void CommaPositions(char *Sentence, int16_t *Pos, int16_t *Lengths){
	uint16_t ChPos = 0, CPos = 0, CChar = 1, Cnt;

	while(CChar != '\0'){
		CChar = Sentence[ChPos];

		if(CChar == ','){
			Pos[CPos] = ChPos;
			CPos++;
		}

		ChPos++;
	}

	for(Cnt = 0; Cnt<CPos-1; Cnt++){
		Lengths[Cnt] = Pos[Cnt+1] - Pos[Cnt] - 1;
	}
}

//USART RX Interrupt variables
uint8_t GSent = 0, GGAWrite = 1, GLLWrite = 1, GSAWrite = 1, SentenceWrite = 1;
uint8_t GSVWrite = 1, RMCWrite = 1, VTGWrite = 1;
uint8_t GGAGot = 0, GLLGot = 0, GSAGot = 0, GSVGot = 0;
uint8_t RMCGot = 0, VTGGot = 0;

//Variables to store the GPS Sentences
char GGASnt[200], GLLSnt[200], GSASnt[200];
char GSVSnt[200], RMCSnt[200], VTGSnt[200];

//The main part where all the magic happens, the USART RX interrupt!
void neo6m_ProcessCharacter(char CChar){
	//char CChar = 0;
	uint8_t Cnt;
	static uint8_t SentenceBegin = 0, SentenceCnt = 0;//, Sentences = 0;
	static char TmpBuf[200];

    //Beginning of sentence character as defined by the NMEA
    //standard.
    if(CChar == '$'){
        //If the beginning character has been received, set the
        //SentenceBegin flag, reset the sentence position counter
        //and tell the main loop that a GPS sentence hasn't been
        //received (GSent = 0).
        SentenceBegin = 1;
        SentenceCnt = 0;
        GSent = 0;
    }

    //End of sentence character
    if(CChar == '\n' && SentenceBegin){
        SentenceBegin = 0;

        //Sentence has been received
        //Write each sentence to sentence buffer
        if(StrCmp(&TmpBuf[3], "GGA", 3)){
            //If sentence is alright to be written to, i.e.
            //the sentence isn't being written or read outside
            //of the interrupt.
            if(GGAWrite) StrCpyCh(&TmpBuf[3], GGASnt, '\0');

            //Current sentence has successfully written.
            GGAGot = 1;
        }
        else if(StrCmp(&TmpBuf[3], "GLL", 3)){
            if(GLLWrite) StrCpyCh(&TmpBuf[3], GLLSnt, '\0');
            GLLGot = 1;
        }
        else if(StrCmp(&TmpBuf[3], "GSA", 3)){
            if(GSAWrite) StrCpyCh(&TmpBuf[3], GSASnt, '\0');
            GSAGot = 1;
        }
        else if(StrCmp(&TmpBuf[3], "GSV", 3)){
            if(GSVWrite) StrCpyCh(&TmpBuf[3], GSVSnt, '\0');
            GSVGot = 1;
        }
        else if(StrCmp(&TmpBuf[3], "RMC", 3)){
            if(RMCWrite) StrCpyCh(&TmpBuf[3], RMCSnt, '\0');
            RMCGot = 1;
        }
        else if(StrCmp(&TmpBuf[3], "VTG", 3)){
            if(VTGWrite) StrCpyCh(&TmpBuf[3], VTGSnt, '\0');
            VTGGot = 1;
        }

        //Clear temporary buffer
        for(Cnt = 0; Cnt<200; Cnt++) TmpBuf[Cnt] = 0;

        //Set sentence received flag
        GSent = 1;
    }

    //If the sentence has just started, store whole sentence
    //in the temporary buffer
    if(SentenceBegin){
        TmpBuf[SentenceCnt] = CChar;
        SentenceCnt++;
    }	
}

//A integer pow function, not really required as math.h is now included
//but I like as little dependencies as possible! Returns a^x if x>1, if
//x=1, returns a, if x = 0, returns 1, on the non existent off chance,
//0 is returned though this will never happen as Pow is unsigned!
int32_t FPow(int32_t Num, uint32_t Pow){
	int32_t NumO = Num;
	uint32_t Cnt;
	if(Pow>1){
		for(Cnt = 0; Cnt<Pow-1; Cnt++){
			Num*=NumO;
		}
		return Num;
	}

	if(Pow==1) return Num;
	if(Pow==0) return 1;
	else return 0;
}

//Check the length of a number in base10 digits
uint8_t CheckNumLength(int32_t Num){
	uint8_t Len = 0, Cnt;

	for(Cnt = 0; Cnt<10; Cnt++){
		if(Num>=FPow(10, Cnt)){
			Len = Cnt;
		}
		else{
			Len = Cnt;
			break;
		}
	}

	return Len;
}

//Integer absolute value function
int32_t Abs(int32_t Num){
	if(Num<0) return -Num;
	else return Num;
}

//Floating point absolute value function
float FAbs(float Num){
	if(Num<0) return -Num;
	else return Num;
}

//Parse an integer number for length amount of characters, while
//also checking if the number is negative or not. "Sentence" is a
//pointer to a the first character of a string e.g. Str[] = "hi, 12345"
//the value of &Str[4] should be passed to the function, otherwise the
//text will be parsed and the number will be erroneous!
int32_t ParseInt(char *Sentence, uint8_t Len){
	int32_t Num = 0;
	int8_t Cnt;

	if(Sentence[0] == '-'){
		for(Cnt = 0; Cnt<Len; Cnt++){
			Num += (Sentence[Cnt+1]-'0')*FPow(10, (Len-1)-Cnt);
		}

		return -Num;
	}
	else{
		for(Cnt = 0; Cnt<Len; Cnt++){
			Num += (Sentence[Cnt]-'0')*FPow(10, (Len-1)-Cnt);
		}

		return Num;
	}
}

//Parsing a floating point number! Remember here that floating point
//numbers don't have infinite precision so this function only works
//up to a prec value of ~7.
float ParseFloat(char *Sentence, uint8_t Prec){
	uint8_t Cnt, CChar = 0, CCnt = 0;
    char NumBuf[10] = {0,0,0,0,0,0,0,0,0,0};
	int32_t INumPre, INumPost;

	while(CChar != '.'){
		CChar = Sentence[CCnt];
		NumBuf[CCnt] = CChar;
		CCnt++;
	}

	INumPre = ParseInt(NumBuf, CCnt-1);

	for(Cnt = CCnt; Cnt<(CCnt+Prec); Cnt++){
		NumBuf[Cnt-CCnt] = Sentence[Cnt];
	}

	INumPost = ParseInt(NumBuf, Prec);

	return (float)INumPre + (float)INumPost/(float)FPow(10, Prec);
}

//Parse any decimal number returning two integers. One of these integers will be
//the digits before the decimal point, the other will be the digits after the
//decimal point. This function has a special addition because if EndPoint is <10,
//the parser will parse to a number amount of places (32bit integers only store
//up to 10 digits anyway). If the EndPoint is more than 10, it will search
//through the string looking for the EndPoint character. This is really useful for
//parsing the GGA string as Lat and Long are sent as decimal values and all text
//is delimited by commas.
void ParseDec(char *Sentence, int32_t *PreDec, int32_t *PostDec, uint8_t EndPoint){	
	uint8_t Cnt, CChar = 0, CCnt = 0, CCVal;
    char NumBuf[10] = {0,0,0,0,0,0,0,0,0,0};
	int32_t INumPre, INumPost;

	while(CChar != '.' && CChar != ','){
		CChar = Sentence[CCnt];
		NumBuf[CCnt] = CChar;
		CCnt++;
	}

	INumPre = ParseInt(NumBuf, CCnt-1);
	*PreDec = INumPre;



    if(CChar == ',')
    {
        return;
    }

    memset(NumBuf, 0, sizeof(NumBuf));

	if(EndPoint<10){
		for(Cnt = CCnt; Cnt<(CCnt+EndPoint); Cnt++){
			NumBuf[Cnt-CCnt] = Sentence[Cnt];
		}

		INumPost = ParseInt(NumBuf, Cnt-CCnt);
	}
	else{
		CCVal = CCnt;
		while(CChar != EndPoint){
			CChar = Sentence[CCnt];
			NumBuf[CCnt-CCVal] = CChar;
			CCnt++;
		}

		INumPost = ParseInt(NumBuf, CCnt-CCVal-1);

//		if(Sentence[CCnt] == 'S' || Sentence[CCnt] == 'W'){
//			INumPre = -INumPre;
//		}
	}

	*PostDec = INumPost;
}

//Parse the current time from any of the sentences containing
//time information! Time is normally presented in the form
//hhmmss.sss. This only parses hhmmss. Make sure the string
//sent to this function starts from the numbers! E.g. if you
//want to parse the string Str[] = "hi 123456.00", make sure you
//give the function &Str[3]. Time is returned to the array sent
//to the function as Time. This array will contain 3 values,
//Hours, Minutes and Seconds. As time is UTC and I live in UK,
//I've added a GMT define which shifts the time by 1 hour. Time
//is returned in 12 hour format.
void ParseTime(char *Sntce, uint8_t *Time){
#ifdef GMT
	Time[0] = (1+(Sntce[1]-'0')+(Sntce[0]-'0')*10)%12;
#else
	Time[0] = (Sntce[1]-'0')+(Sntce[0]-'0')*10;
#endif
	Time[1] = (Sntce[3]-'0')+(Sntce[2]-'0')*10;
	Time[2] = (Sntce[5]-'0')+(Sntce[4]-'0')*10;
}

void ParseVelocity(char *Sntce, uint16_t *Velocity)
{
    //Sntce[1]
}

//A parser to parse the latitude and longitude sent to the function
//from NMEA style (ddmm.mmmmm) into degrees, minutes and seconds. To
//make sure that floating point errors don't occur in data being
//being received by the function, ValPre and ValPost are the two numbers
//before and after the decimal point, in decimal lat/long format.
void ParseLatLong(int32_t ValPre, int32_t ValPost, int32_t *Deg, int32_t *Min, float *Sec){
	int32_t DegT, MinT;
	float SecT;

	DegT = ValPre/100;
	MinT = ValPre-DegT*100;
	SecT = (float)(ValPost*60)/(float)FPow(10, CheckNumLength(ValPost));

	*Deg = DegT;
	*Min = Abs(MinT);
	*Sec = FAbs(SecT);
}

//A function that converts the deg/min/sec lat and long into their
//decimal equivalent. This is the opposite of the above function!
float DecLatLong(int32_t Deg, int32_t Min, float Sec){
	float Num;

	Num = (float)Abs(Deg) + (float)Min/60 + Sec/3600;

	if(Deg<0) return -Num;
	else return Num;
}

//Calculate the distance and bearing between two points in km. The
//equations for this were from:
//
//http://www.ig.utexas.edu/outreach/googleearth/latlong.html
//
//These equations can be widely found everywhere so the above website
//was just one of the first clicks on google! The source lat/long,
//and destination lat/long are taken by the function and the distance
//and bearing between the two points is returned. This function
//normally uses the Haversine method though commented out is the
//spherical law of cosines method - deemed less accurate at times.
//The bearing value is returned in degrees, relative to North.
void DistanceBetweenPoints(float LatSrc, float LongSrc, float LatDst, float LongDst, float *Distance, float *Bearing){
	float DeltaLong, DeltaLat, A, C, TBearing;
	//Radius in km
	const float R = 6371.0f;

	LatSrc = LatSrc*M_PI/180.0f;
	LongSrc = LongSrc*M_PI/180.0f;

	LatDst = LatDst*M_PI/180.0f;
	LongDst = LongDst*M_PI/180.0f;

	DeltaLong = LongDst - LongSrc;
	DeltaLat = LatDst - LatSrc;

	A = sinf(DeltaLat*0.5f)*sinf(DeltaLat*0.5f) + cosf(LatSrc)*cosf(LatDst)*sinf(DeltaLong*0.5f)*sinf(DeltaLong*0.5f);
	C = 2*atan2f(sqrtf(A), sqrtf(1-A));

	*Distance = R*C;

	//*Distance = acosf(sinf(LatSrc)*sinf(LatDst)+cosf(LatSrc)*cosf(LatDst)*cosf(DeltaLong))*R;

	//Relative to north!
	TBearing = 360.0f + 180.0f*atan2f(sinf(DeltaLong)*cosf(LatDst), cosf(LatSrc)*sinf(LatDst)-sinf(LatSrc)*cosf(LatDst)*cosf(DeltaLong))/M_PI;

	//The floating point modulo equivalent! Successively subtract
	//360 until the value is less than 360.
	while(TBearing>360.0f){
		TBearing -= 360.0f;
	}
	*Bearing = TBearing;
}

uint16_t GetVelocityKmph()
{
    return gpvtg_raw_data.Speed_kmph;
}

uint32_t GetTime()
{
    return gprmc_raw_data.CurrentTime;
}

void GPVTG_FillStruct()
{
    int32_t dot;
    int16_t commas_pos[11];
    int16_t length[11];
    CommaPositions(VTGSnt, commas_pos, length);       
    
    ParseDec(&VTGSnt[commas_pos[commaPos_VTG_Speed_kmph] + 1], &gpvtg_raw_data.Speed_kmph, &dot, 3);
}

void GPRMC_FillStruct()
{
    int32_t dot;
    int16_t commas_pos[11];
    int16_t length[11];
    CommaPositions(RMCSnt, commas_pos, length);   
    
    ParseDec(&RMCSnt[commas_pos[commaPos_CurrentTime] + 1], &gprmc_raw_data.CurrentTime, &dot, 2);
    ParseDec(&RMCSnt[commas_pos[commaPos_Speed] + 1], &gprmc_raw_data.Speed_0p1Knots, &dot, 1);
}

void neo6m_Thread(void)
{	
	//Variables to hold various data, a lot!
	uint8_t Cnt;  
	
    //Wait until all 6 sentences have been acquired
    if(!GLLGot || !GSAGot || !GGAGot || !GSVGot || !RMCGot || !VTGGot)
        return;

    //Tell the interrupt that it can't write to the sentence buffers
    GSAWrite = GLLWrite = GGAWrite = GSVWrite = RMCWrite = VTGWrite = 0;

    //Parse the current time of fix from the GLL Sentence
    //ParseTime(&GLLSnt[31], Time);
    
    GPRMC_FillStruct();
    GPVTG_FillStruct();
    
    //Debugging breakpoint - Read the data here!

    //Clear the sentence buffers, ready to receive new sentences
    for(Cnt = 0; Cnt<200; Cnt++){
        GSASnt[Cnt] = GLLSnt[Cnt] = GGASnt[Cnt] = GSVSnt[Cnt] = RMCSnt[Cnt] = VTGSnt[Cnt] = 0;
    }

    //Tell the interrupt that it can now write to the sentence buffers
    GSAWrite = GLLWrite = GGAWrite = GSVWrite = RMCWrite = VTGWrite = 1;

    //Reset all the sentence received variables.
    GSAGot = GLLGot = GGAGot = GSVGot = RMCGot = VTGGot = 0;
	
}