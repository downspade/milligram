class CExif
{
public:
	UnicodeString MakerName;						// !xN
	UnicodeString CameraName;					// !xC
	UnicodeString ShutterSpeed;    		// !xS
	UnicodeString ShutterSpeedSub;    	// !xS
	UnicodeString FNumber;         		// !xF
	UnicodeString FNumberSub;         	// !xf
	UnicodeString FoculLength; 				// !xL
	UnicodeString ExposureBiasValue;   // !xB
	UnicodeString ISOSpeedRatings;     // !xI
	UnicodeString Distance;            // !xR
	UnicodeString ExposureProgram;			// !xP
	UnicodeString MeteringMode;				// !xM
	UnicodeString LightSource;         // !xG
	UnicodeString Flash;               // !xO
	UnicodeString BrightnessValue;     // !xV

	UnicodeString Width;               // !xW
	UnicodeString Height;              // !xH
	UnicodeString Date;              	// !xD

	__fastcall CExif(void);
	__fastcall ~CExif(void);
	void __fastcall Clear(void);
	bool __fastcall SetValue(unsigned short int Type, unsigned short int DataType, unsigned int DataLength, BYTE *Data, int Mode);
	UnicodeString __fastcall GetStringsByValue(TStringList *Src, int Value);
	UnicodeString __fastcall GetStringsByBit(TStringList *Src, unsigned int Value);
	UnicodeString __fastcall GetFlashStrings(TStringList *Src, unsigned int Value);
	UnicodeString __fastcall CreateShutterSpeed(float Value);
	UnicodeString __fastcall CreateFNumber(float Value);
	UnicodeString __fastcall CreateBrightnessValue(float Value);

	TStringList *ExposureProgramSL;
	TStringList *MeteringModeSL;
	TStringList *LightSourceSL;
	TStringList *FlashSL;

	bool Flip;
	int Orientation;
};

