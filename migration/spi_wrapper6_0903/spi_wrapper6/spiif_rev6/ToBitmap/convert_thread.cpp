
/*
	�ϊ������X���b�h

	�Ăяo��I/F
		int set_parameter(hWnd, msg, convert_files)
			hWnd:
				�󋵐i�s�ʒm�̂��߂�Window ID
			msg:
				�󋵐i�s�ʒm�̂��߂�Message ID
			convert_files:
				�ϊ��O�̃t�@�C���� / �ϊ���̃t�@�C�����̑g�ݍ��킹
			return value:

		int start()

		int cancel()
				�\�ł���΃L�����Z�����s�Ȃ�
				���ʂƂ��ăL�����Z���ł��Ȃ����Ƃ�����(Plug-in�̎����ɂ��)

	�Ăяo�����X���b�h�Ƃ̃v���g�R��
		PostMessage(hWnd, msg, wParam, lParam)
			wParam: �i�s��(%)
					0�`100: �i����
					1000: �ϊ���������I��
					1001: �ϊ������ُ�I��
			lParam: convert_files�̂����̂ǂ����������(index:0�`)
			        ~0(0xffffffff)�̎��̓L�����Z�����ꂽ�Ƃ������ƁB
					~0 - 1�őS�Ă̏����I��

*/

#include <windows.h>
#include <process.h>
#include <time.h>
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"

#include "convert_thread.h"



int
WriteBmpFile(const char *bmpName, BITMAPINFO *info, LPVOID mem)
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

	/* bmiHeader.biSizeImage��0�ł����v�Ȃ悤�ɂ��� */
	imageSize= info->bmiHeader.biWidth;

	/* �J���[�p���b�g�^�摜�̃T�C�Y���v�Z���� */
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
		// ��������̂Ƃ��ɕs����Bitmap���o�͂���̂��C��
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
	if ((t_width % 4) != 0) {			/* 4�Ŋ���Ȃ��Ƃ��̂��� */
		t_width= ((t_width / 4) + 1) * 4;
	}
	imageSize= t_width * info->bmiHeader.biHeight;

	header.bfSize += header.bfOffBits + imageSize;
	

	/* �t�@�C���֏������� */
	hFile= CreateFile(bmpName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		status= 0;
	} else {
		DWORD written;

		written= 0;
		if (WriteFile(hFile, &header, sizeof(header), &written, NULL) == FALSE ||
			written != sizeof(header)) {
			status= -1;
		} else if (WriteFile(hFile, info, info->bmiHeader.biSize + pal_size, &written, NULL) == FALSE ||
				   written != info->bmiHeader.biSize + pal_size) {
			status= -1;
		} else if (WriteFile(hFile, mem, imageSize, &written, NULL) == FALSE ||
				   written != imageSize) {
			status= -1;
		} else {
			status= 1;
		}
		CloseHandle(hFile);
	}
	return status;
}


convert_thread_t::
convert_thread_t()
{
	m_manager= NULL;

	m_target_dir= NULL;

	m_notify_hWnd= 0;
	m_notify_wMsg= WM_USER;
	
	m_convert_files= NULL;
	m_convert_file_num= 0;
	m_convert_progress= 0;

	m_do_cancel= 0;

	m_hThread= NULL;
}

convert_thread_t::
~convert_thread_t()
{
	if (m_target_dir)
		delete [] m_target_dir;
	if (m_hThread != NULL) {
		/* �X���b�h�����ς݂ł���ꍇ�ɂ͊��ɏI�����Ă��邩�ǂ������m�F���� */
		if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
			/* �܂����쒆(�I������?�A�ǂ�����?) */
			TerminateThread(m_hThread, 1024);
		} else {
			/* �I�����Ă��� */
		}
		m_hThread= NULL;
	}
}


/*
 * Plug-in����Ăяo�����R�[���o�b�N�֐�
 */
int PASCAL convert_thread_t::
callback_func(int num, int denom, long l_data)
{
	convert_thread_t *p= (convert_thread_t *)l_data;
	int percent= (num * 100) / (denom==0?1:denom);
	int return_value= p->m_do_cancel;

	/* �R�[���o�b�N�ʒm */
	if (return_value) {
		PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, percent, ~0);
		p->m_do_cancel= 2;	/* Notifyed flag */
	} else {
		PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, percent, p->m_convert_progress);
	}
	return return_value;
}




#if 0
/*
 *	Win32��O�����ϊ��֐�
 */
static void
trans_func(unsigned int u, EXCEPTION_POINTER *pExp)
{
	throw u;
}
#endif


int convert_thread_t::
set_parameter(const char *targetdir, HWND hWnd, UINT wMsg, convert_thread_param_t *convert_files, int num)
{
	if (m_target_dir) delete [] m_target_dir;
	m_target_dir= new char [strlen(targetdir) + 1];
	strcpy(m_target_dir, targetdir);

	m_notify_hWnd= hWnd;
	m_notify_wMsg= wMsg;

	m_convert_files= convert_files;
	m_convert_file_num= num;
	m_convert_progress= 0;

	return 0;
}

void convert_thread_t::
set_plugin_manager(SpiFilesManage *manager)
{
	if (manager)
		m_manager= manager;
}

int convert_thread_t::
start(void)
{
	DWORD thread_id;

	/* �O��̃X���b�h�������Ă��邩�ǂ������m�F */
	if (m_hThread && WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
		return FALSE;
	}

	m_do_cancel= 0;

	/* �X���b�h���� */
	m_hThread= (HANDLE)	_beginthreadex(NULL, 0, threadex_func, (void *)this, 0, (unsigned int *)&thread_id);
	if (m_hThread != NULL)  return TRUE;

	return FALSE;
}

int convert_thread_t::
cancel(void)
{
	/* �P�ɃR�[���o�b�N�֐��̖߂�l��ύX���邾�� */
	if (m_hThread && WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
		m_do_cancel= 1;
	}
	return TRUE;
}

int convert_thread_t::
is_running(void)
{
	if (m_hThread && WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*
 *  ���ł�������ϊ��̏������s�Ȃ��X���b�h�֐�
 */

unsigned int __stdcall convert_thread_t::
threadex_func(void *param)
{
	convert_thread_t *p= (convert_thread_t *)param;
	int i, num;
	convert_thread_param_t *files;

	num= p->m_convert_file_num;
	files= p->m_convert_files;

	p->m_convert_progress= 0;

	for (i= 0; i < num; ++i) {
		SpiFile *spi;
		int result;
		int status;
		SpiInputStream input;
		SpiOutputStream out;

		p->m_convert_progress= i;
		result= FALSE;
		/* �t�@�C����ϊ����� xxx -> Bitmap */
	
		/* �t�@�C����ϊ����� Bitmap -> Bitmap File or JPEG File */
		input= files[i].m_in;
		status= p->m_manager->OpenFile(&input, input.query_file_name());
		if (status >= 0) {
			spi= p->m_manager->GetBitmapImage(&input, &out, 0, (FARPROC)p->callback_func, (long)p);
			if (spi == 0) {
				result= FALSE;
			} else {
				char fname[_MAX_FNAME], ext[_MAX_EXT];
				char *bmp_file;
				spi= dynamic_cast <SpiDllFile *>(p);
				/* ConvertToBitmapFile (from wrnd) */

				/* �o�͗p�̃t�@�C�����𐶐����� */
				_splitpath(input.query_file_name(), NULL, NULL, fname, ext);
				bmp_file= new char [_MAX_FNAME + _MAX_EXT + strlen(p->m_target_dir) + 1];
				sprintf(bmp_file, "%s\\%s.bmp", p->m_target_dir, fname);
				if (WriteBmpFile(bmp_file, out.query_bitmap_info(), out.query_bitmap_memory()) <= 0) {
					result= FALSE;
				} else {
					result= TRUE;
				}
				delete bmp_file;
			}
			files[i].m_result= result;
			if (result)
				PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, 1000, p->m_convert_progress);
			else
				PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, 1001, p->m_convert_progress);
		}
		files[i].m_result= result;

		if (p->m_do_cancel) {
			/* �L�����Z���̃��b�Z�[�W�ʒm�͂��ł�calback���ōs���Ă��� */
			for (; i < num; ++i)
				files[i].m_result= FALSE;
			if (p->m_do_cancel == 1)
				PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, 100, ~0);
			break;
		}
	}
	PostMessage(p->m_notify_hWnd, p->m_notify_wMsg, 0, ~0 - 1);

	return 0;
}
