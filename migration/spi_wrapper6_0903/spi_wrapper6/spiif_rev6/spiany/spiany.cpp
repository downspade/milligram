
/*

  ����̗\��:
	Susie Plug-in��I/F�����̂��߂�
		�eSPI�̒P�Ǝ��s�Ȃǂ̏�����ǉ�
		SPI�̃�����/�t�@�C���C���^�t�F�[�X�̑I��
		�ԋp�����f�[�^�̏o�͋@�\
	��ǉ�����B�܂��A���̊��ł̍ăR���p�C�����l������B



Usage: spiany [options] 'Plug-in File' [FileName...]
Options:
	-m		00IN Plug-in�ɑ΂��ă�����I/F�Ńf�[�^��n��
	-c		�R�[���o�b�N�֐����w�肷��
	-f		00IN Plug-in�ɑ΂��ăt�@�C��I/F�Ńf�[�^��n��
	-p		GetPicture�̂�����GetPreview���Ăяo��
	-i		Plug-in��about����T�|�[�g���Ă���g���q�ɂ��Ă̏����o�͂���
	-o[n]	Plug-in�̐ݒ�E�B���h�E��\������
			n��ConfigurationDlg�ɑ΂���@�\�R�[�h���w�肷��(�f�t�H���g=1)

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


/* Susie���ۂ�SPI�̎g���������邩�ǂ�����
 * -m�I�v�V�����ɕύX�ɂȂ�܂����B
 * -c�I�v�V�����ŃR�[���o�b�N�֐����g���܂��B
 * -p�I�v�V������GetPicture�̂�����GetPreview���g���܂��B
 */

int use_extention_info= 0;
int use_callback= 0;
int use_memory_if= 0;
int use_preview= 0;
int use_configuration_dlg= 0;
int config_func= 1;


/* ----------------------------------------------------------------- get_opt
 * �I�v�V�����̉��
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
			/* �I�v�V�������폜 */
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
 *	GetPictureXXX�̃v���O���X�p�̃R�[���o�b�N���[�`��
 */
int PASCAL
callback_func(int nNum,int nDenom,long lData)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hStdout= (HANDLE)lData;

	/* ��Ə󋵂�\������B�܂��̓L�����Z���̋@���^���� */
	if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
		/* �ǂ���nDenom==0�ŌĂяo��Plug-in�����݂���炵�� 
		 *   ��������Plug-in�ւ̑΍�
		 */
		nDenom= (nDenom == 0) ? 1 : nDenom;

		/* �i�s���̕\�� */
		printf("(%3d/%3d)\t%4d%%    ", nNum, nDenom, (int)(nNum * 100 / nDenom));

		/* �J�[�\���ʒu���ړ�(���炾��\�����Ă����傤���Ȃ��̂�) */
		csbi.dwCursorPosition.X= 0;
		SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);

		/* ���̂�����ŃL�[���͌��o������Ē��f�\�ɂ���Ƃ�
		 * GUI�Ȃ璆�f�{�^���Ńt���O�𗧂ĂĂ����Ō��o����Ƃ��B
		 *   ���������@�͂��邩�ȁB
		 */

	}

	/* 0�ȊO��Ԃ���Plug-in�͓W�J�𒆎~���� */
	return 0;	
}

/* �t���[���|�C���^�̍폜�������Ȃ�
 *   �o�O���Ă�̂�����? -> VC++6.0SP3
 */
#pragma optimize("y", off)

/* -------------------------------------------------------------- main
 * ���C�����[�`��
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
		fprintf(stderr, "\t-o[n]\tCall ConfigurationDlg(n:0�`)\n");
		return 1;
	}

	if (spi.LoadFile(NULL, argv[1]) == FALSE) {
		printf("SPI�t�@�C��(%s)�̃��[�h�Ɏ��s���܂���(GetLastError==%d)�B\n", argv[1], GetLastError());
		return 1;
	}

	if (use_extention_info) {
		/* �g���q����\�� */
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

		/* spiif rev0.06.4�ȍ~�̂�(���C�u�����̃o�[�W������ς��Ė����Ȃ�;_;�A���߂�) */
		printf("Config Dialog\t[%s]\n",
				(spi.ConfigurationDlg(NULL, -1) != SUSIE_IF_ERROR_NOT_IMPLEMENTED) ? "Supported" : "Not Supported");
		fflush(stdout);
	}

	if (use_configuration_dlg) {
		int status;
		status= spi.ConfigurationDlg(NULL, config_func);
		if (status != 0) {
			printf("ConfigurationDlg(NULL, %d)�����s���܂���(Ret==%d)\n", config_func, status);
		}
	}
	fflush(stdout);


	for (i= 2; i < argc /*argv[i] != NULL*/; ++i) {

		hFile= CreateFile(argv[i], GENERIC_READ, FILE_SHARE_READ,
						  NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("�t�@�C��[%s]�̃I�[�v���Ɏ��s���܂����B\n", argv[i]);
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
				/* �T�|�[�g���Ă��Ȃ��t�H�[�}�b�g */
				printf("%s�͖��T�|�[�g�̃t�H�[�}�b�g�ł�\n", argv[i]);
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
		case SpiFile::SpiFile_ImportFilter:	/* �摜�p��Plug-in�̏ꍇ */

			if ((ret= spi.GetPictureInfo(argv[i], 0, 0, &picInfo)) != 0) {
				printf("%s�̉摜���擾�Ɏ��s���܂���(Ret==%d)�B\n", argv[i], ret);
				printf("Continue...\n");
				/*continue;*/
			} else {

				printf("PictureInfomation[%s]\n", argv[i]);
				printf("\tPoint\t[%d,%d]\n", picInfo.left, picInfo.top);
				printf("\tSize\t[%d,%d]\n", picInfo.width, picInfo.height);
				printf("\t�������x\t[%d]\n", picInfo.x_density);
				printf("\t�������x\t[%d]\n", picInfo.y_density);
				printf("\tDepth\t%dbits\n", picInfo.colorDepth);
				/* �R�����g�̕\�� */
				if (picInfo.hInfo != NULL) {
					comment= (char *)LocalLock(picInfo.hInfo);
					printf("\tComment\t%s\n", comment);
					LocalUnlock(picInfo.hInfo);
					
					/* �R�����g�̈�̉�� */
					LocalFree(picInfo.hInfo);
				}
			}

			if (use_callback) {
				/* �R�[���o�b�N�֐����g���Ă݂� */
				lpPrgressCallback= (FARPROC)callback_func;
				
				/* �R�[���o�b�N�֐��ɓn���f�[�^
				 * long�ɔ[�܂�Ή��ł�OK�Ȃ̂ŋɒ[�Ɍ�����
				 * �N���X��this��n�������OK�ł��B
				 * (���`��sizeof(long) == sizeof(�|�C���^)�Ƃ����O��ł�)
				 */
				lData= (long)GetStdHandle(STD_OUTPUT_HANDLE);
			}


			fflush(stdout);
			/* GetPicutre��GetPreview��I�����Ď��s */
			if (!use_preview) {
				ret= spi.GetPicture(argv[i], 0, 0, &hBInfo, &hBm,
									lpPrgressCallback, lData);
			} else {
				ret= spi.GetPreview(argv[i], 0, 0, &hBInfo, &hBm,
									lpPrgressCallback, lData);
			}
			
			if (ret == 0) {
				/* �W�J���� */
			} else {
				printf("\n%s�̓W�J�Ɏ��s���܂���(Ret==%d)�B\n", argv[i], ret);
				continue;
			}


			/* �摜���W�J�ł����B*/
			
			/* �������A�h���X�̎擾 */
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
			
			/* �������J�� */
			LocalUnlock(hBm);
			LocalUnlock(hBInfo);
			LocalFree(hBm);
			LocalFree(hBInfo);

			break;

		case SpiFile::SpiFile_Archive:		/* �A�[�J�C�uPlug-in�̏ꍇ */

			/* lhasad.spi�Ȃǂ͏��GetArchiveInfo��2��ԋp����̂�
			 * �擾�ł���hInfo��NULL���ǂ����Ŋm�F����
			 */
			hInfo= NULL;
			spi.GetArchiveInfo(argv[i], 0/*Offset*/, 0/*DISK*/, &hInfo);
			if (hInfo == NULL) {
				printf("�t�@�C�����̎擾�Ɏ��s\n");
			} else {
				printf("�t�@�C����� -- \n");
				arcInfo= (SpiArcFileInfo *)LocalLock(hInfo);
				for (p= arcInfo; p->method[0] != '\0'; ++p) {
					HLOCAL dest;
					printf("Method: %s\tPosition: %d\n", p->method, p->position);
					printf("Size: %d -> %d\n", p->compsize, p->filesize);
					printf("Date: %s", asctime(localtime(&p->timestamp)));
					printf("Name: (%s)\\(%s)\n", p->path, p->filename);
					printf(" CRC: %ud\n", p->crc);

					/* �o�͐�: ������(�t�@�C���ɑΉ����Ă��Ȃ����̂�����)
					 * 
					 */
					if (use_callback) {
						/* �R�[���o�b�N�֐����g���Ă݂� */
						lpPrgressCallback= (FARPROC)callback_func;
						
						/* �R�[���o�b�N�֐��ɓn���f�[�^
						 * long�ɔ[�܂�Ή��ł�OK�Ȃ̂ŋɒ[�Ɍ�����
						 * �N���X��this��n�������OK�ł��B
						 * (���`��sizeof(long) == sizeof(�|�C���^)�Ƃ����O��ł�)
						 */
						lData= (long)GetStdHandle(STD_OUTPUT_HANDLE);
					}
					/* 
					 * src:  �t�@�C����
					 * len:  fileInfo��position�����o
					 * Flag: ����(DISK)/�o��(������) = 0x0100
					 */
					ret= spi.GetFile(argv[i], p->position, 
									 (LPSTR)&dest, 0x0100,
									 lpPrgressCallback, lData);
					if (ret != 0) {
						/* NG */
						char *msg[]= {
							"�W�J����",
							"�L�����Z��", "���Ή��̃t�H�[�}�b�g",
							"�j���f�[�^", "���������蓖�Ď��s",
							"���������쎸�s", "�t�@�C���ǂݍ��݃G���[",
							"�\��", "�����G���[",
						};
						if (1 <= ret && ret <= 8) {
							printf("%s\n", msg[ret]);
						} else {
							printf("�t�@�C�����擾�ł��܂���ł���(Error=%d)\n", ret);
						}
					} else {
						/* OK */
						/* �������삵�� */
						printf("GetFile����\n");
						
						/* �Ō�Ƀ�������� */
						LocalFree(dest);
					}
					
					printf("\n");

				} /* for () */

				
				/* �t�@�C�����̃���������� */
				LocalLock(hInfo);
				LocalFree(hInfo);
			}
			break;

		default: /* ���̑���Plug-in */
			printf("���Ή��̌`����Plug-in�ł��B\n");
			
		} /* switch */

	} /* for (argv[]) */

	return 0;
}
