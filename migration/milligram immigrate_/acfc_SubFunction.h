#ifndef acfcSubFunctionH
#define acfcSubFunctionH


#include "vcl.h"
#include "acfc_Function.h"


#define ACFC_ENCODE_SHIFTJIS 0
#define ACFC_ENCODE_UTF16LE 1
#define ACFC_ENCODE_UTF16BE 2
#define ACFC_ENCODE_UTF16_LE 3
#define ACFC_ENCODE_UTF16_BE 4
#define ACFC_ENCODE_UTF8 5
#define ACFC_ENCODE_UTF8N 6

#define ACFC_ENCODE_CRLF 0
#define ACFC_ENCODE_CR 1
#define ACFC_ENCODE_LF 2


// エンコードを変換しつつ保存と読み込み
bool EncodeStock(CStock *DStock, CStock *SStock, int Encode, int CRType);
bool DecodeStock(CStock *DStock, CStock *SStock, int &Encode, int &CRType);
bool DecodeStock(CStock *DStock, CStock *SStock);
bool CodeCheck(CStock *SStock, int &Encode, int &CRType);

// CStock を TStringList に変換
bool StockToStringList(TStringList *StringList, CStock *Stock);
bool StringListToStock(CStock *Stock, TStringList *StringList);

// 保存読み込み
bool SaveStringList(TStringList *SrcSL, UnicodeString FileName);
bool LoadStringList(TStringList *DestSL, UnicodeString FileName);



#endif

