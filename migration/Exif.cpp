#include "noigol_function.h"
#include <vcl.h>
#pragma hdrstop

#include "Exif.h"
#include <sys\stat.h>
#include <math.h>
#include <windows.h>

__fastcall CExif::CExif(void)
{
	ExposureProgramSL = new TStringList();
	MeteringModeSL = new TStringList();
	LightSourceSL = new TStringList();
	FlashSL = new TStringList();

	ExposureProgramSL->Add(L"Not defineded");
	ExposureProgramSL->Add(L"Manual");
	ExposureProgramSL->Add(L"Normal program");
	ExposureProgramSL->Add(L"Aperture priority");
	ExposureProgramSL->Add(L"Shutter priority");
	ExposureProgramSL->Add(L"Creative program");
	ExposureProgramSL->Add(L"Action program");
	ExposureProgramSL->Add(L"Portrait mode");
	ExposureProgramSL->Add(L"Landscape mode");
	ExposureProgramSL->Add(L"other");

	MeteringModeSL->Add(L"Unknown");
	MeteringModeSL->Add(L"Average");
	MeteringModeSL->Add(L"Center-weighted Average");
	MeteringModeSL->Add(L"Spot");
	MeteringModeSL->Add(L"Multi-spot");
	MeteringModeSL->Add(L"Multi-segment");
	MeteringModeSL->Add(L"Partial");
	MeteringModeSL->Add(L"other");

	LightSourceSL->Add(L"None");
	LightSourceSL->Add(L"Daylight");
	LightSourceSL->Add(L"Fluorescent");
	LightSourceSL->Add(L"Tungsten");
	LightSourceSL->Add(L"Flash");
	LightSourceSL->Add(L"undefinded");
	LightSourceSL->Add(L"undefinded");
	LightSourceSL->Add(L"undefinded");
	LightSourceSL->Add(L"undefinded");
	LightSourceSL->Add(L"Fine Weather");
	LightSourceSL->Add(L"Cloudy");
	LightSourceSL->Add(L"Shade");
	LightSourceSL->Add(L"Daylight Fluorescent");
	LightSourceSL->Add(L"Day White Fluorescent");
	LightSourceSL->Add(L"Cool White Fluorescent");
	LightSourceSL->Add(L"White Fluorescent");
	LightSourceSL->Add(L"Standard Light A");
	LightSourceSL->Add(L"Standard Light B");
	LightSourceSL->Add(L"Standard Light C");
	LightSourceSL->Add(L"D55");
	LightSourceSL->Add(L"D65");
	LightSourceSL->Add(L"D75");
	LightSourceSL->Add(L"D50");
	LightSourceSL->Add(L"ISO Studio Tungsten L");
	LightSourceSL->Add(L"other");

	FlashSL->Add(L" : L");
	FlashSL->Add(L"Not Fired");
	FlashSL->Add(L"Fired");
	FlashSL->Add(L"Detect available");
	FlashSL->Add(L"Return detected");
	FlashSL->Add(L"Undefinded");
	FlashSL->Add(L"Compulsory flash firing");
	FlashSL->Add(L"Compulsory flash suppression");
	FlashSL->Add(L"Auto mode");
	FlashSL->Add(L"No flash function");
	FlashSL->Add(L"Red-eye reduction");

	Clear();
}

__fastcall CExif::~CExif(void)
{
	delete ExposureProgramSL;
	delete MeteringModeSL;
	delete LightSourceSL;
	delete FlashSL;
}

UnicodeString __fastcall CExif::GetStringsByValue(TStringList *Src, int Value)
{
	if(Src->Count < Value)Value = Src->Count - 1;
	return(Src->Strings[Value]);
}

UnicodeString __fastcall CExif::GetStringsByBit(TStringList *Src, unsigned int Value)
{
	int i = 0;
	UnicodeString Result = L"";;
	while(Value > 0)
	{
		if((Value % 2) > 0)
			Result = Result + GetStringsByValue(Src, i);
		i++;
		Value /= 2;
	}
	return(Result);
}

UnicodeString __fastcall CExif::GetFlashStrings(TStringList *Src, unsigned int Value)
{
	UnicodeString Result = L"";
	int i;
	if(FlashSL->Count < 2)return(L"none");

	for(i = 0;i < 3;i++)
	{
		if(FlashSL->Count < i + 3)goto getflashstrings_breakout;
		if((Value % 2) > 0)
		{
			if(Result != L"")Result += FlashSL->Strings[0];
			Result += FlashSL->Strings[i + 2];
		}
		Value /= 2;
	}

	if((Value & 3) > 0)
	{
		if(Result != L"")Result += FlashSL->Strings[0];
		Result += FlashSL->Strings[(Value & 3) + 5];
	}
	Value /= 4;

	i = 0;
	while(Value > 0)
	{
		if(FlashSL->Count < i + 10)goto getflashstrings_breakout;
		if((Value % 2) > 0)
		{
			if(Result != L"")Result += FlashSL->Strings[0];
			Result += FlashSL->Strings[i + 9];
		}
		Value /= 2;
		i++;
	}
getflashstrings_breakout:
	if(Result == L"")Result += FlashSL->Strings[1];
	return(Result);
}

void __fastcall CExif::Clear(void)
{
	MakerName = L"none";						// !xN
	CameraName = L"none";					// !xC

	ShutterSpeed = L"none";    		// !xS
	FNumber = L"none";         		// !xF

	ShutterSpeedSub = L"none";    	// !xS
	FNumberSub = L"none";         	// !xF

	FoculLength = L"none"; 				// !xL
	ExposureBiasValue = L"none";   // !xB
	ISOSpeedRatings = L"none";     // !xI
	Distance = L"none";            // !xR

	Date = L"none";              	// !xD
	ExposureProgram = L"none";			// !xP
	MeteringMode = L"none";				// !xM

	LightSource = L"none";					// !x
	Flash = L"none";								// !x
	BrightnessValue = L"none";			// !x

	Width = L"none";               // !xW
	Height = L"none";              // !xH

	Orientation = -1;
	Flip = false;
}

bool __fastcall CExif::SetValue(unsigned short int Type, unsigned short int DataType, unsigned int DataLength, BYTE *Data, int Mode)
{
	unsigned int UBunshi;
	unsigned int UBunbo;
	int Bunshi;
	int Bunbo;
	unsigned short int USInt;
	unsigned int SInt;
	float a, b;
	int i;
	int j;
	bool Minus;
	bool Ended;

	switch(Type)
	{
		case 0x0112:
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			switch(USInt)
			{
				default: Flip = false; Orientation = 0; break;
				case 1: Flip = false;  Orientation = 0; break;
				case 2:	Flip = true ;  Orientation = 0; break;
				case 3:	Flip = false;  Orientation = 2; break;
				case 4:	Flip = true ;  Orientation = 2; break;
				case 5:	Flip = true ;  Orientation = 1; break;
				case 6:	Flip = false;  Orientation = 1; break;
				case 7: Flip = true ;  Orientation = 3; break;
				case 8: Flip = false;  Orientation = 3; break;
			}
			break;

		case 0x010f:
			{
				if(MakerName != L"none")break;
				wchar_t *tempData;
				tempData = new wchar_t[DataLength];
				UnicodeString Temp = (UnicodeString)((char *)Data);
				wcscpy(tempData, Temp.c_str());
				if(tempData[0] != ' ')MakerName = ((UnicodeString)tempData).Trim();
				delete[] tempData;
			}
			break;

		case 0x0110:
			{
				if(CameraName != L"none")break;
				wchar_t *tempData;
				tempData = new wchar_t[DataLength];
				UnicodeString Temp = (UnicodeString)((char *)Data);
				wcscpy(tempData, Temp.c_str());
				if(tempData[0] != ' ')CameraName = ((UnicodeString)tempData).Trim();
				delete[] tempData;
			}
			break;

		case 0x829a: // ExposureTime
			if(ShutterSpeed != L"none")break;
			UBunshi = *(unsigned int *)(Data);
			Endian(&UBunshi, 4, Mode);
			UBunbo = *(unsigned int *)(Data + 4);
			Endian(&UBunbo, 4, Mode);
			if(UBunshi == 0 || UBunbo == 0)return(false);
			a = (float)UBunshi / UBunbo;
			if(a < 0.3)
			{
				do
				{
					Ended = true;
					for(j = 2;j <= UBunshi && j <= UBunbo / 2;j++)
					{
						if(UBunshi % j == 0 && UBunbo % j == 0)
						{
							UBunshi /= j;
							UBunbo /= j;
							Ended = false;
							break;
						}
					}
				}
				while(Ended == false);
				ShutterSpeed = IntToStr((int)UBunshi) + L"/" + IntToStr((int)UBunbo) + L"s";
			}
			else if(a < 1)
			{
				ShutterSpeed = FloatToStrF(a, ffFixed, 10, 1) + L"s";
			}
			else
			{
				i = a;
				ShutterSpeed = IntToStr(i) + L"s";
			}
			break;

		case 0x829d: // FNumber
			if(FNumber != L"none")break;
			UBunshi = *(unsigned int *)(Data);
			Endian(&UBunshi, 4, Mode);
			UBunbo = *(unsigned int *)(Data + 4);
			Endian(&UBunbo, 4, Mode);
			if(UBunshi == 0 || UBunbo == 0)return(false);
			a = (float)UBunshi / UBunbo;
			i = a;
			b = i;
			if(b == a)
				FNumber = IntToStr(i);
			else
				FNumber = FloatToStrF(a, ffFixed, 10, 1);
			break;

		case 0x920a: // Focul Length
			if(FoculLength != L"none")break;
			UBunshi = *(unsigned int *)(Data);
			Endian(&UBunshi, 4, Mode);
			UBunbo = *(unsigned int *)(Data + 4);
			Endian(&UBunbo, 4, Mode);
			if(UBunshi == 0 || UBunbo == 0)return(false);
			i = UBunshi / UBunbo;
			if(i == 0)return(false);
			FoculLength = IntToStr(i) + L"mm";
			break;

		case 0x9204: // Bias
			if(ExposureBiasValue != L"none")break;
			ExposureBiasValue = L"+";
			Minus = false;
			Bunshi = *(int *)(Data);
			Endian(&Bunshi, 4, Mode);
			Bunbo = *(int *)(Data + 4);
			Endian(&Bunbo, 4, Mode);

      if(Bunshi == 0){ExposureBiasValue = L"0"; return(true);}
			if(Bunbo == 0){ExposureBiasValue = L"none"; return(false);}
			if(Bunshi < 0){Minus = !Minus; Bunshi = -Bunshi;}
			if(Bunbo < 0){Minus = !Minus; Bunbo = -Bunbo;}
			if(Minus == true)ExposureBiasValue = L"-";
			if((Bunshi % Bunbo) > 0)
			{
				do
				{
					Ended = true;
					for(j = 2;j <= Bunshi && j <= Bunbo / 2;j++)
					{
						if(Bunshi % j == 0 && Bunbo % j == 0)
						{
							Bunshi /= j;
							Bunbo /= j;
							Ended = false;
							break;
						}
					}
				}
				while(Ended == false);
				ExposureBiasValue = ExposureBiasValue + IntToStr(Bunshi) + L"/" + IntToStr(Bunbo);
			}
			else
			{
				ExposureBiasValue = ExposureBiasValue + IntToStr(Bunshi / Bunbo);
			}
			break;

		case 0x8827: // ISO
			if(ISOSpeedRatings != L"none")break;
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			ISOSpeedRatings = IntToStr(USInt);
			break;

		case 0xa215: // ISO
			if(ISOSpeedRatings != L"none")break;
			UBunshi = *(unsigned int *)(Data);
			Endian(&UBunshi, 4, Mode);
			UBunbo = *(unsigned int *)(Data + 4);
			Endian(&UBunbo, 4, Mode);
			if(UBunshi == 0 || UBunbo == 0)return(false);
			i = UBunshi / UBunbo;
			ISOSpeedRatings = IntToStr(i);
			break;

		case 0x9206: // Distance
			if(Distance != L"none")break;
			UBunshi = *(unsigned int *)(Data);
			Endian(&UBunshi, 4, Mode);
			UBunbo = *(unsigned int *)(Data + 4);
			Endian(&UBunbo, 4, Mode);
			if(UBunshi == 0 || UBunbo == 0)return(false);
			i = UBunshi / UBunbo;
			Distance = L"";
			j = i / 1000; i -= j * 1000;
			if(j > 0)Distance = IntToStr(j) + L"m";
			j = i / 100; i -= j * 100;
			if(j > 0)Distance = IntToStr(j) + L"cm";
			Distance = IntToStr(i) + L"mm";
			break;

		case 0x8822: // ExposureProgram
			if(ExposureProgram != L"none")break;
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			ExposureProgram = GetStringsByValue(ExposureProgramSL, USInt);
			break;

		case 0x9207: // MeteringMode
			if(MeteringMode != L"none")break;
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			MeteringMode = GetStringsByValue(MeteringModeSL, USInt);
			break;

		case 0x9003: // Date
			if(Date != L"none")break;
			{
				wchar_t *tempData;
				tempData = new wchar_t[DataLength];
				UnicodeString Temp = (UnicodeString)((char *)Data);
				wcscpy(tempData, Temp.c_str());
				if(tempData[0] != ' ')Date = ((UnicodeString)tempData).Trim();
				delete[] tempData;
			}
			break;

		case 0x9208: // LightSource
			if(LightSource != L"none")break;
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			LightSource = GetStringsByValue(LightSourceSL, USInt);
			break;

		case 0x9209: // Flash
			if(Flash != L"none")break;
			USInt = *(unsigned short int *)(Data);
			Endian(&USInt, 2, Mode);
			Flash = GetFlashStrings(FlashSL, USInt);
			break;

		case 0x9201: // ShutterSpeed
			if(ShutterSpeedSub != L"none")break;
			Bunshi = *(int *)(Data);
			Endian(&Bunshi, 4, Mode);
			Bunbo = *(int *)(Data + 4);
			Endian(&Bunbo, 4, Mode);
			if(Bunbo == 0)return(false);
			a = (float)Bunshi / Bunbo;
			ShutterSpeedSub = CreateShutterSpeed(a);
			break;

		case 0x9202: // FNumber
			if(FNumberSub != L"none")break;
			Bunshi = *(int *)(Data);
			Endian(&Bunshi, 4, Mode);
			Bunbo = *(int *)(Data + 4);
			Endian(&Bunbo, 4, Mode);
			if(Bunbo == 0)return(false);
			a = (float)Bunshi / Bunbo;
			FNumberSub = CreateFNumber(a);
			break;

		case 0x9203: // BrightnessValue
			if(BrightnessValue != L"none")break;
			Bunshi = *(int *)(Data);
			Endian(&Bunshi, 4, Mode);
			Bunbo = *(int *)(Data + 4);
			Endian(&Bunbo, 4, Mode);
			if(Bunbo == 0)return(false);
			a = (float)Bunshi / Bunbo;
			BrightnessValue = CreateBrightnessValue(a);
			break;

		case 0xa002:
			if(Width != L"none")break;
			if(DataLength == 2)
			{
				USInt = *(unsigned short int *)(Data);
				Endian(&USInt, 2, Mode);
				Width = IntToStr(USInt);
			}
			else if(DataLength == 4)
			{
				SInt = *(unsigned int *)(Data);
				Endian(&SInt, 4, Mode);
				Width = IntToStr((int)SInt);
			}
			break;

		case 0xa003:
			if(Height != L"none")break;
			if(DataLength == 2)
			{
				USInt = *(unsigned short int *)(Data);
				Endian(&USInt, 2, Mode);
				Height = IntToStr(USInt);
			}
			else if(DataLength == 4)
			{
				SInt = *(unsigned int *)(Data);
				Endian(&SInt, 4, Mode);
				Height = IntToStr((int)SInt);
			}
			break;

		default:
			break;
	}
	return(true);
}


UnicodeString __fastcall CExif::CreateShutterSpeed(float Value)
{
	UnicodeString Result = L"";
	float a, b;
	bool Minus;
	int i;

	if(Value < 0)
	{
		Value = -Value;
		Minus = true;
	}
	a = pow(2.0f, Value);
	i = a;
	b = i;
	if(b == a)
		Result = IntToStr(i);
	else
		Result = FloatToStrF(a, ffFixed, 10, 1);

	if(Minus == false)
	{
		Result = L"1/" + Result;
	}

	Result += L"s";
	return(Result);
}

UnicodeString __fastcall CExif::CreateFNumber(float Value)
{
	UnicodeString Result = L"";
	float a, b;
	int i;

	a = pow(2.0f, Value / 2.0f);
	i = a;
	b = i;
	if(b == a)
		Result = IntToStr(i);
	else
		Result = FloatToStrF(a, ffFixed, 10, 1);

	return(Result);
}

UnicodeString __fastcall CExif::CreateBrightnessValue(float Value)
{
	UnicodeString Result = L"";
	float a, b;
	int i;

	a = pow(2.0f, Value);
	i = a;
	b = i;
	if(b == a)
		Result = IntToStr(i);
	else
		Result = FloatToStrF(a, ffFixed, 10, 1);

	return(Result);
}


