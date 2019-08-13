
/*

  今後の予定:
	Susie Plug-inのI/F調査のために
		各SPIの単独実行などの処理を追加
		SPIのメモリ/ファイルインタフェースの選択
		返却されるデータの出力機能
	を追加する。また、他の環境での再コンパイルを考慮する。



Usage: spiany [options] 'Plug-in File' [FileName...]
Options:
	-m		00IN Plug-inに対してメモリI/Fでデータを渡す
	-c		コールバック関数を指定する
	-f		00IN Plug-inに対してファイルI/Fでデータを渡す
	-p		GetPictureのかわりにGetPreviewを呼び出す
	-i		Plug-inのabout情報やサポートしている拡張子についての情報を出力する
	-o[n]	Plug-inの設定ウィンドウを表示する
			nはConfigurationDlgに対する機能コードを指定する(デフォルト=1)

*/


#include <windows.h>
#include <stdio.h>
#include <mbstring.h>
#include <time.h>
#include <ctype.h>
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"


/* SusieっぽいSPIの使い方をするかどうかは
 * -mオプションに変更になりました。
 * -cオプションでコールバック関数を使います。
 * -pオプションでGetPictureのかわりにGetPreviewを使います。
 */

int use_extention_info= 0;
int use_callback= 0;
int use_memory_if= 0;
int use_preview= 0;
int use_configuration_dlg= 0;
int config_func= 1;


/* ----------------------------------------------------------------- get_opt
 * オプションの解析
 */
int
get_opt(int *argc, char **argv)
{
	int i;
	for (i= 1; i < *argc; ++i) {
		if (argv[i][0] == '-') {
			switch (toupper(argv[i][1])) {
			case 'C': /* use callback */
				use_callback= 1;
				break;

			case 'M': /* use memory i/f */
				use_memory_if= 1;
				break;

			case 'F': /* use file i/f */
				use_memory_if= 0;
				break;

			case 'P': /* use GetPreview */
				use_preview= 1;
				break;

			case 'I': /* Display Extention Information */
				use_extention_info= 1;
				break;

			case 'O': /* call ConfigurationDlg */
				use_configuration_dlg= 1;
				if (argv[i][2] != '\0') {
					config_func= atoi(&argv[i][2]);
				}
				break;

			default:
				fprintf(stderr, "No such option(%s)\n", argv[i]);
				break;
			}
			/* オプションを削除 */
			--(*argc);
			int j;
			for (j= i; argv[j] != NULL; ++j) {
				argv[j]= argv[j+1];
			}
			argv[j]= NULL;
			--i;
		}
	}
	return *argc;
}


/* --------------------------------------------------------- callback_func
 *	GetPictureXXXのプログレス用のコールバックルーチン
 */
int PASCAL
callback_func(int nNum,int nDenom,long lData)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hStdout= (HANDLE)lData;

	/* 作業状況を表示する。またはキャンセルの機会を与える */
	if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
		/* どうもnDenom==0で呼び出すPlug-inも存在するらしい 
		 *   そういうPlug-inへの対策
		 */
		nDenom= (nDenom == 0) ? 1 : nDenom;

		/* 進行率の表示 */
		printf("(%3d/%3d)\t%4d%%    ", nNum, nDenom, (int)(nNum * 100 / nDenom));

		/* カーソル位置を移動(だらだら表示してもしょうがないので) */
		csbi.dwCursorPosition.X= 0;
		SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);

		/* このあたりでキー入力検出をやって中断可能にするとか
		 * GUIなら中断ボタンでフラグを立ててここで検出するとか。
		 *   いくつか方法はあるかな。
		 */

	}

	/* 0以外を返せばPlug-inは展開を中止する */
	return 0;	
}

/* フレームポインタの削除をさせない
 *   バグってんのかしら? -> VC++6.0SP3
 */
#pragma optimize("y", off)

/* -------------------------------------------------------------- main
 * メインルーチン
 */

int
main(int argc, char **argv)
{
	int i, j;
	SpiDllFile spi;
	HANDLE hFile;

	SpiPictureInfo picInfo;
	HLOCAL hBm, hBInfo;
	LPVOID pBmp;
	BITMAPINFO *pBmpInfo;
//	BITMAPFILEHEADER bmpHeader;

	SpiArcFileInfo *arcInfo, *p;
	HLOCAL hInfo;

//	char new_name_buff[1024];
//	char *new_name= new_name_buff;
//	DWORD pal_size;
	FARPROC lpPrgressCallback= NULL;
	long lData;

	int ret;
	char spi_buffer[2048+128];
	DWORD args, readed;

	char info_buffer[2048];
	char *comment;

	get_opt(&argc, argv);

	if (argv[1] == NULL) {
		fprintf(stderr, "Usage: %s [-c] [-m] [-f] [-p] [-i] Plug-in [GraphicFile...]\n", argv[0]);
		fprintf(stderr, "Option:\n\t-c\tUse Callback\n\t-m\tUse Memory I/F\n");
		fprintf(stderr, "\t-f\tNot use Memory I/F\n\t-p\tUse GetPreview I/F\n");
		fprintf(stderr, "\t-i\tDisplay Plugin Information\n");
		fprintf(stderr, "\t-o[n]\tCall ConfigurationDlg(n:0～)\n");
		return 1;
	}

	if (spi.LoadFile(NULL, argv[1]) == FALSE) {
		printf("SPIファイル(%s)のロードに失敗しました(GetLastError==%d)。\n", argv[1], GetLastError());
		return 1;
	}

	if (use_extention_info) {
		/* 拡張子情報を表示 */
		printf("Plug-in Information\n");
		fflush(stdout);

		info_buffer[0]= 0;
		spi.GetPluginInfo(0, info_buffer, sizeof(info_buffer));
		printf("API Version\t[%s]\n", info_buffer);

		fflush(stdout);
		info_buffer[0]= 0;
		spi.GetPluginInfo(1, info_buffer, sizeof(info_buffer));
		printf("Plug-in about\t[%s]\n", info_buffer);

		for (j= 0; ; ++j) {
			fflush(stdout);

			info_buffer[0]= 0;
			ret= spi.GetPluginInfo(2 * j + 2, info_buffer, sizeof(info_buffer));
			if (ret == 0)
				break;
			printf("Extention\t[%s]\n", info_buffer);

			info_buffer[0]= 0;
			ret= spi.GetPluginInfo(2 * j + 3, info_buffer, sizeof(info_buffer));
			if (ret == 0)
				break;
			printf("Format Name\t[%s]\n", info_buffer);

		}

		fflush(stdout);

		/* spiif rev0.06.4以降のみ(ライブラリのバージョンを変えて無いなぁ;_;、ごめん) */
		printf("Config Dialog\t[%s]\n",
				(spi.ConfigurationDlg(NULL, -1) != SUSIE_IF_ERROR_NOT_IMPLEMENTED) ? "Supported" : "Not Supported");
		fflush(stdout);
	}

	if (use_configuration_dlg) {
		int status;
		status= spi.ConfigurationDlg(NULL, config_func);
		if (status != 0) {
			printf("ConfigurationDlg(NULL, %d)が失敗しました(Ret==%d)\n", config_func, status);
		}
	}
	fflush(stdout);


	for (i= 2; i < argc /*argv[i] != NULL*/; ++i) {

		hFile= CreateFile(argv[i], GENERIC_READ, FILE_SHARE_READ,
						  NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("ファイル[%s]のオープンに失敗しました。\n", argv[i]);
			continue;
		}
		fflush(stdout);

		if (use_memory_if) {
			memset(spi_buffer, 0, sizeof(spi_buffer));
			ReadFile(hFile, spi_buffer, sizeof(spi_buffer), &readed, NULL);
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			args= (DWORD)spi_buffer;
		} else {
			args= (DWORD)hFile;
		}

		if (spi.IsSupported(argv[i], args) == FALSE) {
			/* Mac Binary Retry */
			if (use_memory_if) {
				args= (DWORD)&spi_buffer[128];
			} else {
				SetFilePointer(hFile, 128, NULL, FILE_CURRENT);
				args= (DWORD)hFile;
			}
			if (spi.IsSupported(argv[i], args) == FALSE) {
				/* サポートしていないフォーマット */
				printf("%sは未サポートのフォーマットです\n", argv[i]);
				CloseHandle(hFile);
				continue;
			} else {
				CloseHandle(hFile);
			}
		} else {
			/* OK */
			CloseHandle(hFile);
		}

		
		fflush(stdout);

		switch (spi.GetSpiFileType()) {
		case SpiFile::SpiFile_ImportFilter:	/* 画像用のPlug-inの場合 */

			if ((ret= spi.GetPictureInfo(argv[i], 0, 0, &picInfo)) != 0) {
				printf("%sの画像情報取得に失敗しました(Ret==%d)。\n", argv[i], ret);
				printf("Continue...\n");
				/*continue;*/
			} else {

				printf("PictureInfomation[%s]\n", argv[i]);
				printf("\tPoint\t[%d,%d]\n", picInfo.left, picInfo.top);
				printf("\tSize\t[%d,%d]\n", picInfo.width, picInfo.height);
				printf("\t水平密度\t[%d]\n", picInfo.x_density);
				printf("\t垂直密度\t[%d]\n", picInfo.y_density);
				printf("\tDepth\t%dbits\n", picInfo.colorDepth);
				/* コメントの表示 */
				if (picInfo.hInfo != NULL) {
					comment= (char *)LocalLock(picInfo.hInfo);
					printf("\tComment\t%s\n", comment);
					LocalUnlock(picInfo.hInfo);
					
					/* コメント領域の解放 */
					LocalFree(picInfo.hInfo);
				}
			}

			if (use_callback) {
				/* コールバック関数を使ってみる */
				lpPrgressCallback= (FARPROC)callback_func;
				
				/* コールバック関数に渡すデータ
				 * longに納まれば何でもOKなので極端に言えば
				 * クラスのthisを渡したりもOKです。
				 * (え～とsizeof(long) == sizeof(ポインタ)という前提です)
				 */
				lData= (long)GetStdHandle(STD_OUTPUT_HANDLE);
			}


			fflush(stdout);
			/* GetPicutreとGetPreviewを選択して実行 */
			if (!use_preview) {
				ret= spi.GetPicture(argv[i], 0, 0, &hBInfo, &hBm,
									lpPrgressCallback, lData);
			} else {
				ret= spi.GetPreview(argv[i], 0, 0, &hBInfo, &hBm,
									lpPrgressCallback, lData);
			}
			
			if (ret == 0) {
				/* 展開成功 */
			} else {
				printf("\n%sの展開に失敗しました(Ret==%d)。\n", argv[i], ret);
				continue;
			}


			/* 画像が展開できた。*/
			
			/* メモリアドレスの取得 */
			pBmp= LocalLock(hBm);
			pBmpInfo= (BITMAPINFO *)LocalLock(hBInfo);
			
			
			printf("BITMAPINFOHEADER\n");
			printf("\tStructSize:\t%d\n", pBmpInfo->bmiHeader.biSize);
			printf("\tImageSize:\t%d\n", pBmpInfo->bmiHeader.biSizeImage);
			printf("\tBiClrSize:\t%d\n", pBmpInfo->bmiHeader.biClrUsed);
			printf("\tWidth:\t%d\n", pBmpInfo->bmiHeader.biWidth);
			printf("\tHeight:\t%d\n", pBmpInfo->bmiHeader.biHeight);
			printf("\tPlanes:\t%d\n", pBmpInfo->bmiHeader.biPlanes);
			printf("\tDepth:\t%d\n", pBmpInfo->bmiHeader.biBitCount);
			printf("\tXPelsPerMeter:\t%d\n", pBmpInfo->bmiHeader.biXPelsPerMeter);
			printf("\tYPelsPerMeter:\t%d\n", pBmpInfo->bmiHeader.biYPelsPerMeter);		
			
			/* メモリ開放 */
			LocalUnlock(hBm);
			LocalUnlock(hBInfo);
			LocalFree(hBm);
			LocalFree(hBInfo);

			break;

		case SpiFile::SpiFile_Archive:		/* アーカイブPlug-inの場合 */

			/* lhasad.spiなどは常にGetArchiveInfoで2を返却するので
			 * 取得できたhInfoがNULLかどうかで確認する
			 */
			hInfo= NULL;
			spi.GetArchiveInfo(argv[i], 0/*Offset*/, 0/*DISK*/, &hInfo);
			if (hInfo == NULL) {
				printf("ファイル情報の取得に失敗\n");
			} else {
				printf("ファイル情報 -- \n");
				arcInfo= (SpiArcFileInfo *)LocalLock(hInfo);
				for (p= arcInfo; p->method[0] != '\0'; ++p) {
					HLOCAL dest;
					printf("Method: %s\tPosition: %d\n", p->method, p->position);
					printf("Size: %d -> %d\n", p->compsize, p->filesize);
					printf("Date: %s", asctime(localtime(&p->timestamp)));
					printf("Name: (%s)\\(%s)\n", p->path, p->filename);
					printf(" CRC: %ud\n", p->crc);

					/* 出力先: メモリ(ファイルに対応していないものも多い)
					 * 
					 */
					if (use_callback) {
						/* コールバック関数を使ってみる */
						lpPrgressCallback= (FARPROC)callback_func;
						
						/* コールバック関数に渡すデータ
						 * longに納まれば何でもOKなので極端に言えば
						 * クラスのthisを渡したりもOKです。
						 * (え～とsizeof(long) == sizeof(ポインタ)という前提です)
						 */
						lData= (long)GetStdHandle(STD_OUTPUT_HANDLE);
					}
					/* 
					 * src:  ファイル名
					 * len:  fileInfoのpositionメンバ
					 * Flag: 入力(DISK)/出力(メモリ) = 0x0100
					 */
					ret= spi.GetFile(argv[i], p->position, 
									 (LPSTR)&dest, 0x0100,
									 lpPrgressCallback, lData);
					if (ret != 0) {
						/* NG */
						char *msg[]= {
							"展開成功",
							"キャンセル", "未対応のフォーマット",
							"破損データ", "メモリ割り当て失敗",
							"メモリ操作失敗", "ファイル読み込みエラー",
							"予約", "内部エラー",
						};
						if (1 <= ret && ret <= 8) {
							printf("%s\n", msg[ret]);
						} else {
							printf("ファイルを取得できませんでした(Error=%d)\n", ret);
						}
					} else {
						/* OK */
						/* 何か操作して */
						printf("GetFile成功\n");
						
						/* 最後にメモリ解放 */
						LocalFree(dest);
					}
					
					printf("\n");

				} /* for () */

				
				/* ファイル情報のメモリを解放 */
				LocalLock(hInfo);
				LocalFree(hInfo);
			}
			break;

		default: /* その他のPlug-in */
			printf("未対応の形式のPlug-inです。\n");
			
		} /* switch */

	} /* for (argv[]) */

	return 0;
}
