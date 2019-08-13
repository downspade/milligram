/*
 *	Susie Plug-in Interface
 *		SPI_API.TXTから仕様に従って作成(一部に違いがあるのでもとのドキュメントも見ておいてね)
 */
/*
 * 注意:
 *  オリジナルと同じ名前の構造体を利用するにはSPI_COMPATIBLEをdefineする
 *
 *  VC++用です。BCは持っていないのでテストしていません。
 *	以下の標準インクルードファイルが必要です
 *		WINDOWS.H
 *		time.h
 */
#ifndef	SUSIE_SPI_API_H
#define	SUSIE_SPI_API_H

#ifdef	_WIN32
#undef	_export
#define	_export		/* __declspec(dllexport) */
#endif

#if	defined(__BORLANDC__)
#undef	_export
#define	_export
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(__BORLANDC__)
#pragma option -a-
#else
#pragma	pack(1)
#endif
typedef struct SpiPictureInfo
{
	long left,top;
	long width;
	long height;
	WORD x_density;
	WORD y_density;
	short colorDepth;
	HLOCAL hInfo;		/* コメント情報[呼び出し側が解放] */
} SpiPictureInfo;

#ifdef	SPI_COMPATIBLE
typedef SpiPictureInfo PictureInfo;
#endif

typedef struct SpiArcFileInfo
{
	unsigned char method[8];	/* 圧縮法の種類 */
	unsigned long position;		/* ファイル上での位置 */
	unsigned long compsize;		/* 圧縮されたサイズ */
	unsigned long filesize;		/* 元のファイルサイズ */
	time_t timestamp;			/* ファイルの更新日時 */
	char path[200];				/* 相対パス */
	char filename[200];			/* ファイルネーム */
	unsigned long crc;			/* CRC */
} SpiArcFileInfo;
#ifdef	SPI_COMPATIBLE
typedef SpiArcFileInfo fileInfo;
#endif

#if !defined(__BORLANDC__)
#pragma pack()
#else
#pragma option -a.
#endif


/* Common Function */
int _export PASCAL GetPluginInfo (int infono, LPSTR buf,int buflen);
int _export PASCAL IsSupported (LPSTR filename,DWORD dw);


/* '00IN'の関数 */
int _export PASCAL GetPictureInfo (LPSTR buf,long len,unsigned int flag, SpiPictureInfo *lpInfo);
int _export PASCAL GetPicture (LPSTR buf,long len,unsigned int flag, HANDLE *pHBInfo,HANDLE *pHBm,
								FARPROC lpPrgressCallback,long lData);
int _export PASCAL GetPreview (LPSTR buf,long len,unsigned int flag, HANDLE *pHBInfo,HANDLE *pHBm,
								FARPROC lpPrgressCallback,long lData);

/* '00AM'の関数 */
int _export PASCAL GetArchiveInfo (LPSTR buf, long len, unsigned int flag, HLOCAL *lphInf);
int _export PASCAL GetFileInfo (LPSTR buf,long len, LPSTR filename, unsigned int flag, SpiArcFileInfo *lpInfo);
int _export PASCAL GetFile (LPSTR src,long len, LPSTR dest,unsigned int flag, FARPROC prgressCallback,long lData);

/* Plug-inの設定ダイアログ
 *  Susie32 v0.40より追加されたSPI
 */
enum {
	SUSIE_CONFIGDLG_ABOUT= 0,
	SUSIE_CONFIGDLG_SETTING,
	SUSIE_CONFIGDLG_RESERVED
};
int _export FAR PASCAL ConfigurationDlg(HWND parent,int fnc);

#ifdef __cplusplus
}
#endif

#endif	/* SUSIE_SPI_API_H */
