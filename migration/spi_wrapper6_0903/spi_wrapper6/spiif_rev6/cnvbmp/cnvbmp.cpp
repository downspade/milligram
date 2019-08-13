
/*
  テストプログラム
*/

#include <stdio.h>
#include <time.h>
#include <windows.h>

#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"
//#include "spiif_derived.h"

/* BMPファイルを出力する関数 */
int WriteBmpFile(char *bmpName, BITMAPINFO *info, LPVOID mem);


int
main(int argc, char **argv)
{
	char *file_name= argv[1];
	char *bmp_file;
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	DWORD len;

   	if (file_name == NULL) {
		fprintf(stderr, "Usage: tp file\n");
		return 0;
	}


	GetFullPathName(file_name, _MAX_PATH, path_buffer, NULL);
	_splitpath(path_buffer, drive, dir, fname, ext);

	len= strlen(drive) + strlen(dir) + strlen(fname) + strlen(ext) + 16;
	bmp_file= new char [len];


	if (strcmpi(ext, ".bmp") == 0) {
		sprintf(bmp_file, "%s%s%s_.bmp", drive, dir, fname);
	} else {
		sprintf(bmp_file, "%s%s%s.bmp", drive, dir, fname);
	}

	SpiFilesManage manager;
	SpiInputStream in;
	SpiOutputStream out;
	manager.SetPluginDirectory(NULL);
	manager.LoadPluginFile();

	in.set_stream_data(file_name, 0);
	if (manager.OpenFile(&in, file_name) < 0) {
		fprintf(stderr, "Open Failed[%d]\n", file_name);
	} else {
		SpiFile *p; 
		SpiDllFile *spi;
		printf("Open OK[%s]\n", file_name);
		if (in.query_input_type() != SpiInputFileStream) {
			/* 書庫内ファイルだからファイル名をかえるの
			 *   カレントディレクトリに出力するの
			*/
			if (strcmpi(ext, ".bmp") == 0) {
				sprintf(bmp_file, ".\\%s_.bmp", fname);
			} else {
				sprintf(bmp_file, ".\\%s.bmp", fname);
			}
		}
		p= manager.GetBitmapImage(&in, &out, 0, NULL, 0);
		if (p == 0) {
			fprintf(stderr, "GetBitmapImage Failed\n");
		} else {
			spi= dynamic_cast <SpiDllFile *>(p);
			printf("GetBitmapImage OK[Plug-in is `%s'/`%s']\n", spi->GetAboutName(), spi->GetPluginFileName());
			/* ConvertToBitmapFile (from wrnd) */
			printf("Output To BMP file[%s]\n", bmp_file);
			WriteBmpFile(bmp_file, out.query_bitmap_info(), out.query_bitmap_memory());
		}
	}
	manager.UnloadPluginFile();

	delete bmp_file;

	return 0;
}


int
WriteBmpFile(char *bmpName, BITMAPINFO *info, LPVOID mem)
{
	int status= 0;
	BITMAPFILEHEADER header;
	DWORD pal_size;
	HANDLE hFile;
	DWORD imageSize, t_width;

	memset(&header, 0, sizeof(header));
	header.bfType= 0x4d42;	/* 'BM' */
	header.bfSize= 0;
	header.bfOffBits= sizeof(BITMAPFILEHEADER) + info->bmiHeader.biSize;

	/* bmiHeader.biSizeImageが0でも大丈夫なようにする */
	imageSize= info->bmiHeader.biWidth;

	/* カラーパレット／画像のサイズを計算する */
	switch (info->bmiHeader.biBitCount) {
	case 1:
		pal_size= sizeof(RGBQUAD) * 2;
		if (imageSize % 8) {
			imageSize = imageSize / 8 + 1;
		} else {
			imageSize = imageSize / 8;
		}
		break;
	case 4:
		pal_size= sizeof(RGBQUAD) * 16;
		// 横幅が奇数のときに不正なBitmapを出力するのを修正
		if (imageSize % 2) {
			imageSize = imageSize / 2 + 1;
		} else {
			imageSize = imageSize / 2;
		}
		break;
	case 8:
		pal_size= sizeof(RGBQUAD) * 256;
		imageSize= imageSize;
		break;
	case 16:
		pal_size= sizeof(DWORD) * 3;
		if (info->bmiHeader.biCompression == BI_RGB)
			pal_size= 0;
		imageSize *= 2;
		break;
	case 24:
		pal_size= 0L;
		imageSize *= 3;
		break;
	case 32:
		pal_size= sizeof(DWORD) * 3;
		if (info->bmiHeader.biCompression == BI_RGB)
			pal_size= 0;
		imageSize *= 4;
		break;
	default:
		break;
	}
	header.bfOffBits += pal_size;

	t_width= imageSize;
	if ((t_width % 4) != 0) {			/* 4で割れないときのため */
		t_width= ((t_width / 4) + 1) * 4;
	}
	imageSize= t_width * info->bmiHeader.biHeight;

	header.bfSize += header.bfOffBits + imageSize;
	

	/* ファイルへ書き込む */
	hFile= CreateFile(bmpName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		status= 0;
	} else {
		DWORD written;

		WriteFile(hFile, &header, sizeof(header), &written, NULL);
		WriteFile(hFile, info, info->bmiHeader.biSize + pal_size, &written, NULL);
		WriteFile(hFile, mem, imageSize, &written, NULL);
		CloseHandle(hFile);
		status= 1;
	}
	return status;
}
