#include "acfc_SubFunction.h"

#define SAFE_FREE(p)				 { if(p != NULL) { free (p);       (p) = NULL; } }
#define SAFE_DELETE(p)       { if(p != NULL) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p != NULL) { delete[] (p);   (p) = NULL; } }
#define SAFE_RELEASE(p)      { if(p != NULL) { (p)->Release(); (p) = NULL; } }

bool EncodeStock(CStock *DStock, CStock *SStock, int Encode, int CRType)
{
	unsigned char BOM[][3] = {
													{0xFF, 0xFE},
													{0xFE, 0xFF},
													{0xEF, 0xBB, 0xBF}
												 };
	unsigned char *CRCode[] = {"\r\n", "\r", "\n"};
	unsigned char *CRCode16[] = {"\0\r\0\n\0", "\0\r\0", "\0\n\0"};
	int CRLength[] = {2, 1, 1};

	BYTE *TempBuffer = SStock->DataBuffer;
	bool Result;
	int Size = SStock->Size / sizeof(wchar_t);

	SStock->SetMode(CSTOCK_TEXT);
	DStock->SetMode(CSTOCK_BINARY);

	// BOM を書き込む
	switch(Encode)
	{
		default:
			break;
		case ACFC_ENCODE_UTF16_LE:
			DStock->Write(BOM[0], 2);
			break;
		case ACFC_ENCODE_UTF16_BE:
			DStock->Write(BOM[1], 2);
			break;
		case ACFC_ENCODE_UTF8:
			DStock->Write(BOM[2], 3);
			break;
	}

	UnicodeString Line;
	int q = 0, p = 0, l;
	wchar_t *Buf = NULL, *P = (wchar_t *)SStock->DataBuffer;

	// ここからループ
	while(p < Size)
	{
		// 改行もしくは終端までコピー
		l = 0;
		while(p < Size && P[p] != '\n')
		{
			p++;
			l++;
		}

		Buf = new wchar_t[l + 1];
		wcsncpy(Buf, P + q, l);
		Buf[l] = 0;

		Line = Buf;

		switch(Encode)
		{
			case ACFC_ENCODE_SHIFTJIS:
				{
					AnsiString Temp = Line;
					DStock->Write(Temp.c_str(), Temp.Length());
					if(p < Size)DStock->Write(CRCode[CRType], CRLength[CRType]);
				}
				break;
			case ACFC_ENCODE_UTF16LE:
			case ACFC_ENCODE_UTF16_LE:
				{
					DStock->Write(Line.c_str(), Line.Length() * 2);
					if(p < Size)DStock->Write(CRCode16[CRType] + 1, CRLength[CRType] * 2);
				}
				break;
			case ACFC_ENCODE_UTF16BE:
			case ACFC_ENCODE_UTF16_BE:
				{
					int j;
					unsigned char Temp[3];
					for(j = 1;j <= Line.Length();j++)
					{
						*(wchar_t *)Temp = Line[j];
						Temp[2] = Temp[0], Temp[0] = Temp[1], Temp[1] = Temp[2];
						DStock->Write(Temp, 2);
					}
					if(p < Size)DStock->Write(CRCode16[CRType], CRLength[CRType] * 2);
				}
				break;
			case ACFC_ENCODE_UTF8:
			case ACFC_ENCODE_UTF8N:
				{
					UTF8String Temp = Line;
					DStock->Write(Temp.c_str(), Temp.Length());
					if(p < Size)DStock->Write(CRCode[CRType], CRLength[CRType]);
				}
				break;
		}
		SAFE_DELETE_ARRAY(Buf);
		p++;
		q = p;
	}

	return(Result);
}


bool DecodeStock(CStock *DStock, CStock *SStock)
{
	int Encode = ACFC_ENCODE_SHIFTJIS, CRType = ACFC_ENCODE_CRLF;
	return(DecodeStock(DStock, SStock, Encode, CRType));
}

bool CodeCheck(CStock *SStock, int &Encode, int &CRType)
{
	unsigned char BOM[][3] = {
													{0xFF, 0xFE},
													{0xFE, 0xFF},
													{0xEF, 0xBB, 0xBF}
												 };
	unsigned char *CRCode[] = {"\r\n", "\r", "\n"};
	unsigned char *CRCode16[] = {"\0\r\0\n\0", "\0\r\0", "\0\n\0"};
	int CRLength[] = {2, 1, 1};

	int Result = false;
	int Size;
	BYTE *Buf;
	Size = SStock->Size;

	if(Size < 3)return(false);

	// 改行コードを変換する
	int i, p, l;

	Buf = SStock->DataBuffer;

	// BOM をまずチェックしてみる
	if(memcmp(Buf, BOM[0], 2) == 0){Encode = ACFC_ENCODE_UTF16_LE; Buf += 2; Size -= 2; Result = true;}
	if(memcmp(Buf, BOM[1], 2) == 0){Encode = ACFC_ENCODE_UTF16_BE; Buf += 2; Size -= 2; Result = true;}
	if(memcmp(Buf, BOM[2], 3) == 0){Encode = ACFC_ENCODE_UTF8; Buf += 3; Size -= 3; Result = true;}


	if(Result == false)
	{
		bool ShiftJis = true;
		// UTF-8 か Shift-JIS である可能性が高い
		// Shift-JIS であるかどうかをチェック
		i = 0;
		while(i < Size)
		{
			if(Buf[i] == 0){ShiftJis = false; break;}
			if(Buf[i] >= 0xa1 && Buf[i] <= 0xdf){ShiftJis = false; break;}
			if((Buf[i] >= 0x81 && Buf[i] <= 0x9f) || (Buf[i] >= 0xe0 && Buf[i] <= 0xfc))
			{
				i++;
				if(i == Size){ShiftJis = false; break;}
				if(Buf[i] < 0x40 || (Buf[i] > 0x7e && Buf[i] < 0x80) || Buf[i] > 0xfc){ShiftJis = false; break;}
			}
			i++;
		}

		// UTF-8 であるかどうかをチェック
		bool UTF8 = true;
		i = 0;
		while(i < Size)
		{
			if(Buf[i] == 0){UTF8 = false; break;}

			if(Buf[i] >= 0x80 && Buf[i] <= 0xbf){UTF8 = false; break;}

			if(Buf[i] >= 0xc2 && Buf[i] <= 0xdf)
			{
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
			}

			if(Buf[i] >= 0xe0 && Buf[i] <= 0xef)
			{
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
			}

			if(Buf[i] >= 0xf0 && Buf[i] <= 0xf7)
			{
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
				i++;
				if(i == Size){UTF8 = false; break;}
				if(Buf[i] < 0x80 || Buf[i] > 0xbf){UTF8 = false; break;}
			}

			if(Buf[i] >= 0xf8 && Buf[i] <= 0xff){UTF8 = false; break;}
			i++;
		}

		if(UTF8 == true && ShiftJis == false)
		{
			Encode = ACFC_ENCODE_UTF8N;
			Result = true;
		}

		if(UTF8 == false && ShiftJis == true)
		{
			Encode = ACFC_ENCODE_SHIFTJIS;
			Result = true;
		}
	}

	// 改行をひとつ見つけるまで進めてみる
	for(i = 0;i < Size;i++)
	{
		if(Buf[i] == '\n')
		{
			// Shift-JIS か UTF-8
			CRType = ACFC_ENCODE_LF;
			break;
		}

		if(Buf[i] == '\r')
		{
			if(i < Size - 1 && Buf[i + 1] == '\n')
			{
				// Shift-JIS か UTF-8
				CRType = ACFC_ENCODE_CRLF;
				break;
			}
			else if(i < Size - 2 && Buf[i + 1] == 0 && Buf[i + 2] == '\n')
			{
				// UTF-16 であることはほぼ確定
				CRType = ACFC_ENCODE_CRLF;
				if(Result == false)
				{
					if((i % 2) == 0)
					{
						if(i < Size - 3 && Buf[i + 3] == 0)
							Encode = ACFC_ENCODE_UTF16LE;
					}
					else
					{
						if(i > 0 && Buf[i - 1] == 0)
							Encode = ACFC_ENCODE_UTF16BE;
					}
				}
				break;
			}
			else
			{
				CRType = ACFC_ENCODE_CR;
				if(Result == false)
				{
					if((i % 2) == 0)
					{
						if(i < Size - 1 && Buf[i + 1] == 0)
							Encode = ACFC_ENCODE_UTF16LE;
					}
					else
					{
						if(i > 0 && Buf[i - 1] == 0)
							Encode = ACFC_ENCODE_UTF16BE;
					}
				}
				break;
			}
		}
	}
	return(Result);
}

bool DecodeStock(CStock *DStock, CStock *SStock, int &Encode, int &CRType)
{
	unsigned char BOM[][3] = {
													{0xFF, 0xFE},
													{0xFE, 0xFF},
													{0xEF, 0xBB, 0xBF}
												 };
	unsigned char *CRCode[] = {"\r\n", "\r", "\n"};
	unsigned char *CRCode16[] = {"\0\r\0\n\0", "\0\r\0", "\0\n\0"};
	int CRLength[] = {2, 1, 1};

	int Result = true;
	int Size = SStock->Size;
	BYTE *Buf = SStock->DataBuffer;

	SStock->SetMode(CSTOCK_BINARY);
	DStock->SetMode(CSTOCK_TEXT);
	CodeCheck(SStock, Encode ,CRType);

	// 改行コードを変換する
	int i, p, l;

	// BOM をまずチェックしてみる
	switch(Encode)
	{
		case ACFC_ENCODE_UTF16_LE:
			Buf += 2; Size -= 2;
			break;
		case ACFC_ENCODE_UTF16_BE:
			Buf += 2; Size -= 2;
			break;
		case ACFC_ENCODE_UTF8:
			Buf += 3; Size -= 3;
			break;
	}

	switch(Encode)
	{
		case ACFC_ENCODE_SHIFTJIS:
			{
				char *Temp;
				AnsiString asTemp;
				p = l = 0;
				for(i = 0;i < Size - (CRLength[CRType] - 1);i++)
				{
					// 改行コードが来たかどうかを調べる
					if(memcmp(Buf + i, CRCode[CRType], CRLength[CRType]) == 0)
					{
						if(l > 0)
						{
							// 改行コードまでを AnsiString にコピーする
							Temp = new BYTE[l + 1];
							memcpy(Temp, Buf + p, l);
							Temp[l] = 0;
							asTemp = Temp;
							SAFE_DELETE_ARRAY(Temp);
							// Unicode に変換してデータに保存する
							UnicodeString usTemp = asTemp;
							DStock->WriteStr(usTemp.w_str());
						}
						DStock->WriteStr(L"\n");
						i += (CRLength[CRType] - 1);
						l = 0;
						p = i + 1;
					}
					else
					{
						l++;
					}
				}

				if(l > 0)
				{
					l++;
					// 最後までを AnsiString にコピーする
					Temp = new BYTE[l + 1];
					memcpy(Temp, Buf + p, l);
					Temp[l] = 0;
					asTemp = Temp;
					SAFE_DELETE_ARRAY(Temp);
					// Unicode に変換してデータに保存する
					UnicodeString usTemp = asTemp;
					DStock->WriteStr(usTemp.w_str());
				}
			}
			break;

		case ACFC_ENCODE_UTF16LE:
		case ACFC_ENCODE_UTF16_LE:
			{
				wchar_t *Temp;
				UnicodeString usTemp;
				p = l = 0;
				for(i = 0;i < Size;i += sizeof(wchar_t))
				{
					// 改行コードが来たかどうかを調べる
					if(memcmp(Buf + i, CRCode16[CRType] + 1, CRLength[CRType] * sizeof(wchar_t)) == 0)
					{
						if(l > 0)
						{
							// 改行コードまでを UnicodeString にコピーする
							Temp = new wchar_t[l + 1];
							memcpy(Temp, Buf + p, l * sizeof(wchar_t));
							Temp[l] = 0;
							usTemp = Temp;
							SAFE_DELETE_ARRAY(Temp);
							// データに保存する
							DStock->WriteStr(usTemp.w_str());
						}
						DStock->WriteStr(L"\n");
						i += (CRLength[CRType] - 1) * sizeof(wchar_t);
						l = 0;
						p = i + sizeof(wchar_t);
					}
					else
					{
						l++;
					}
				}

				if(l > 0)
				{
					l++;
					// 改行コードまでを UnicodeString にコピーする
					Temp = new wchar_t[l + 1];
					memcpy(Temp, Buf + p, l);
					Temp[l] = 0;
					usTemp = Temp;
					SAFE_DELETE_ARRAY(Temp);
					// データに保存する
					DStock->WriteStr(usTemp.w_str());
				}
			}
			break;

		case ACFC_ENCODE_UTF16BE:
		case ACFC_ENCODE_UTF16_BE:
			{
				wchar_t *Temp;
				BYTE *a, b;
				int j;
				UnicodeString usTemp;
				p = l = 0;
				for(i = 0;i < Size;i += sizeof(wchar_t))
				{
					// 改行コードが来たかどうかを調べる
					if(memcmp(Buf + i, CRCode16[CRType], CRLength[CRType] * sizeof(wchar_t)) == 0)
					{
						if(l > 0)
						{
							// 改行コードまでを UnicodeString にコピーする
							Temp = new wchar_t[l + 1];
							memcpy(Temp, Buf + p,  l * sizeof(wchar_t));
							// LittleEndian にする
							for(j = 0;j < l;j++)
							{
								a = (BYTE *)&Temp[j];
								b = a[0]; a[0] = a[1]; a[1] = b;
							}
							Temp[l] = 0;

							usTemp = Temp;
							SAFE_DELETE_ARRAY(Temp);
							// データに保存する
							DStock->WriteStr(usTemp.w_str());
						}
						DStock->WriteStr(L"\n");
						i += (CRLength[CRType] - 1) * sizeof(wchar_t);
						l = 0;
						p = i + sizeof(wchar_t);
					}
					else
					{
						l++;
					}
				}

				if(l > 0)
				{
					l++;
					// 改行コードまでを UnicodeString にコピーする
					Temp = new wchar_t[l];
					memcpy(Temp, Buf + p, l);
					Temp[l] = 0;
					usTemp = Temp;
					SAFE_DELETE_ARRAY(Temp);
					// データに保存する
					DStock->WriteStr(usTemp.w_str());
				}
			}
			break;

		case ACFC_ENCODE_UTF8:
		case ACFC_ENCODE_UTF8N:
			{
				char *Temp;
				UTF8String ut8Temp;
				p = l = 0;
				for(i = 0;i < Size;i++)
				{
					// 改行コードが来たかどうかを調べる
					if(memcmp(Buf + i, CRCode[CRType], CRLength[CRType]) == 0)
					{
						if(l > 0)
						{
							// 改行コードまでを AnsiString にコピーする
							Temp = new BYTE[l + 1];
							memcpy(Temp, Buf + p, l);
							Temp[l] = 0;
							ut8Temp = Temp;
							SAFE_DELETE_ARRAY(Temp);
							// Unicode に変換してデータに保存する
							UnicodeString usTemp = ut8Temp;
							DStock->WriteStr(usTemp.w_str());
						}
						DStock->WriteStr(L"\n");
						i += (CRLength[CRType] - 1);
						l = 0;
						p = i + 1;
					}
					else
					{
						l++;
					}
				}

				if(l > 0)
				{
					l++;
					// 最後までを AnsiString にコピーする
					Temp = new BYTE[l + 1];
					memcpy(Temp, Buf + p, l);
					Temp[l] = 0;
					ut8Temp = Temp;
					SAFE_DELETE_ARRAY(Temp);
					// Unicode に変換してデータに保存する
					UnicodeString usTemp = ut8Temp;
					DStock->WriteStr(usTemp.w_str());
				}
			}
			break;
	}

	return(Result);
}


bool StockToStringList(TStringList *StringList, CStock *Stock)
{
	BYTE *TempBuffer = Stock->DataBuffer;
	bool Result = true;
	int Size = Stock->Size / sizeof(wchar_t);

	UnicodeString Line;
	int q = 0, p = 0, l;
	wchar_t *Buf = NULL, *P = (wchar_t *)Stock->DataBuffer;

	// ここからループ
	while(p < Size)
	{
		// 改行もしくは終端までコピー
		l = 0;
		while(p < Size && P[p] != '\n')
		{
			p++;
			l++;
		}

		Buf = new wchar_t[l + 1];
		wcsncpy(Buf, P + q, l);
		Buf[l] = 0;

		Line = Buf;
		SAFE_DELETE_ARRAY(Buf);
		Line = Line.Trim();
		StringList->Add(Line);

		p++;
		q = p;
	}

	return(Result);
}


bool StringListToStock(CStock *Stock, TStringList *StringList)
{
	bool Result = true;
	int i;
	for(i = 0;i < StringList->Count;i++)
	{
		Stock->WriteStr((StringList->Strings[i] + L"\n").w_str());
	}

	return(Result);
}

bool SaveStringList(TStringList *SrcSL, UnicodeString FileName)
{
	bool Result;
	CStock *SStock = new CStock();
	CStock *DStock = new CStock();

	SStock->SetMode(CSTOCK_TEXT);
	StringListToStock(SStock, SrcSL);
	EncodeStock(DStock, SStock, ACFC_ENCODE_UTF16LE, ACFC_ENCODE_CRLF);
	DStock->SetMode(CSTOCK_BINARY);

	Result = DStock->Save(FileName.w_str());

	delete SStock;
	delete DStock;
	return(Result);
}

bool LoadStringList(TStringList *DestSL, UnicodeString FileName)
{
	bool Result;
	int Enc, Cr;
	CStock *SStock = new CStock();
	CStock *DStock = new CStock();

	SStock->SetMode(CSTOCK_BINARY);
	SStock->Load(FileName.w_str());

	DecodeStock(DStock, SStock);
	StockToStringList(DestSL, DStock);

	delete SStock;
	delete DStock;
	return(Result);
}

