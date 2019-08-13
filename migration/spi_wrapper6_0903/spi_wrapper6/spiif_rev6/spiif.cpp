/*
 *	Susi32 Plugin Interface
 *			Copyright kana(mailto:inokuchi@mvg.biglobe.ne.jp)
 *
 */
/*
 *	History:
 *		2000/02/20(Sun) �` 2000/09/02(Sat): �����ƁA(��)�͍���̍�Ɨ\�荀�ڂł��B
 *			Plug-in�̊g���q�p�^�[����LoadPlugin���ɕێ�����悤�ύX
 *			(�܂����̊g���q��SpiFilesManage�ł͗��p���Ă��Ȃ��̂ł��܂�Ӗ����Ȃ�)
 *			���ɓ��t�@�C���̃I�[�v�����Ƀt�@�C�����R�s�[�p�o�b�t�@�����Ă���
 *			SpiInputStream�ŃR�s�[�R���X�g���N�^���g���ƃR�s�[����delete���ꂽ�^�C�~���O��
 *			������������Ă��܂��̂��C��(<-�Ƃ肠����map�Ń��t�@�����X�J�E���^���ǂ��ŊǗ�)
 *			ConfigurationDlg��Y��Ă����̂Œǉ�
 *			���W�X�g���̃��[�h���ɏ]���悤�ɕύX(��)
 *			���W�X�g����Plug-in�̗��p�p�^�[��(A, B, C)�ɏ]���悤�ɕύX(��)
 *		2000/01/16(Sun) �`:
 *			���W�X�g���̐ݒ�̉��߂��ς�����̂Ŏ�荇�����C���B�������g���E�g��Ȃ�
 *			�������f���Ă��Ȃ�
 *		1999/12/30(Thu) �` 2000/1/4(Tue):
 *			spi_derived.{h,cpp}�ƃ}�[�W
 *			���W�X�g���̒l��������擾����buffer overflow�ɑ΂��ė\�h
 *		1999/11/30(Tue) �`:
 *			�ꕔ��Plug-in��GetArchiveInfo/GetPictureInfo�̕s���SpiDllFile�ŋz��
 *			SpiDllFile::SmartLoad��AM Plug-in���l�����Ă��Ȃ��̂��C��
 *			SpiDllFile�ł�AM Plug-in�̌Ăяo�������ׂăG���[�ƂȂ��Ă����̂��C��
 *			���C�u�����o�[�W�������}�N���Œ�`����悤�ɒǉ�
 *			def_query_load_SpiFilesManage_callback�̃��W�X�g���L�[���Ԉ���Ă����̂��C��
 *		1999/05/09(Sun) �` :
 *			�R�[���o�b�N�֐��̎���(�h���N���X�Ȃ��ł�����ύX�\�Ƃ��邽��)
 *		1999/03/27(Sat) �` : 
 *			QueryPluginLoad/EvalLoadType�̎d�l�ύX�B
 *			backward compatibility�֐�FindFirstPlugin���T�|�[�g
 *		1999�N2��2��(��) �` :
 *			�o�����B���s�̓��I���[�h�ւ̑Ή�
 *		1997�N10��25��(�y):
 *			  SpiFile::LoadPlugin��00IN Plug-in�̎���GetPreview���Ȃ��Ă�
 *			��������悤�ɕύX�B
 *			���{�I�ɂ͎擾�ł��Ȃ������֐���GetPluginInfo/IsSupported����
 *			������Ԃ���  NOT_IMPLEMENTED��Ԃ��悤�ɂ����ق��������̂���
 *			����Ȃ����ǁB
 *		1997�N3��23��(��):
 *			  SpiFileManage::AddPlugin��ǉ����ABuilt-in Plugin
 *			��ǉ��ł���悤�ɕύX�B����ɒǉ����ꂽ�I�u�W�F�N�g
 *			��SpiFileManager:UnLoadPluginFile�ō폜�����̂�new���邱��
 *		1996�N10��05��(�y):
 *			  SpiFileManage::SearchPlugin�Ń`�F�b�N����offset��
 *			�w��ł���悤�ɂ����B�����offset=128�ŃR�[������
 *			���Ƃ�Mac�o�C�i�����ǂݍ��߂�͂�
 *			  SpiFileManage::SearchPlugin�Ō������s���Ă����炩��
 *			SpiFile��Ԃ��Ă����̂��C��
 *		1996�N9��2��(��):
 *			  SpiFileManage��SPI�t�@�C���������[�`���̃o�O
 *			malloc/free��new/delete�ɕύX
 *			  SearchPlugin�ŕԋp����SpiFile�|�C���^��chain��
 *			�O�̃|�C���^�ł���̂��C��
 *			  �t�@�C���o�[�W����������𖄂ߍ���
 *		1996�N8��21��(��):
 *			SpiFile�����o�[�W�����쐬�I��/SpiFileManage�쐬�J�n
 *			[Spi�֐��ďo�����̊֐��|�C���^��__stdcall��ǉ�]
 *			[���ꂪ�Ȃ��ƃX�^�b�N�t���[�������������Ȃ�݂���]
 *		1996�N8��18��(��):
 *			�����o�[�W�����쐬�J�n
 */

/* ����ϖ{�����g��Ȃ��Ƃ˂�;_;�A���� */
static char g_id_file[]="$File ID: SpiIf.cpp by kana $";
static char g_id_ver[]="$File Ver: 0.06.4 $";
static char g_id_class[]= "$Class ID: SpiFile/SpiDllFile/SpiFileManage $";

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mbstring.h>
#include <windows.h>
#include <winreg.h>

#include <assert.h>

/* for STL */
#include <vector>
#include <string>
#include <map>
#include <set>

#include "spi_api.h"
#include "spi_misc.h"

#include "spiif.h"

/* �萔 */

/* */
#define	strtok(__x, __y)		(char *)_mbstok((unsigned char *)__x, (unsigned char *)__y)
#define	strrchr(__x, __y)		(char *)_mbsrchr((unsigned char *)__x, __y)

/* Plugin�̊g���q */
/* Win32��Win16�ł̈Ⴂ�͑��ɂ����邩��Win16(PLG)�łł��g����Ȃ��
   ���҂��Ă͂����Ȃ��BWinNT�����g���ĂȂ��̂ł悭�킩��񂵁B */
#ifdef	_WIN32
/* 32bit Version */
#define	SUSIE_PLUGIN_EXT	"SPI"
#else
/* 16bit Version */
#define	SUSIE_PLUGIN_EXT	"PLG"
#endif


/* �^�錾 */
#if	0
typedef struct plugin_info_chain {
	struct plugin_info_chain *m_next;
	SpiFile *m_spiFile;
} plugin_info_chain;
#endif


/* �֐��錾 */
extern "C" {
	
int knRegTinyGet(HKEY tree, const char *key_name, 
					const char *val_name,LPVOID buffer);
int knRegTinyGetEx(HKEY tree, const char *key_name, 
					const char *val_name,LPVOID buffer, DWORD length);
int knRegTinySet(HKEY tree, const char *key_name,
					const char *val_name, DWORD type,
					LPVOID buffer, DWORD length);
}

/* �ϐ��錾 */

/* �R�[���o�b�N�֐��̃f�t�H���g�̎���
  `typedef' of functions  are:
	typedef DWORD (WINAPI *SpiDllFileLoadFileCallback_t)(SpiDllFile *plugin);
	typedef SpiFileLoadType (WINAPI *SpiDllFileEvalCallback_t)(SpiDllFile *plugin, DWORD pre, DWORD post);

�e�R�[���o�b�N�֐����i�[���Ă��郁���o�ϐ���public�ɂȂ��Ă���̂Œ��ڕύX���邱�Ƃ��\�ł���B
�������ύX����̂̓R���X�g���N�^���Ăяo�������オ�]�܂����B�܂��͔h���N���X������ď���������悤��
����̂𐄏�����B

�ESpiDllFile�̃��[�h�^�C�v:
	�f�t�H���g�̎����ł͉��z�A�h���X��Ԃ̌������`�F�b�N����B���ꂪ�������Ă���Ƃ������Ƃ�
	DLL���������[�h�o���Ȃ��Ȃ邱�Ƃ��Ӗ����Ă���B
	�܂����̎����ł�TLS�̌��E���`�F�b�N���邱�Ƃ͂��Ă��Ȃ��B����TLS�̌��E�Ɖ��z�A�h���X��Ԃ�
	�����͓Ɨ��������ۂł���̂ŗ����o���Ȃ����Ƃ����邱�Ƃ𗝉�����ׂ��ł���B
	�܂��Ȃ�Ƃ��Ȃ�Ȃ���ł����ǂ˂��E�E�E�E�B

	pre_load callback�̓v���O�C�������ۂ�LoadLibrary����O�ɌĂяo�����
	post_load_callback�̓v���O�C�������ۂ�LoadLibrary������ɌĂяo�����
	eval_load_type_callback�͂��ꂼ��̊֐����ԋp�����l�������ɌĂяo���B����ɂ�胍�[�h��ʂ�
	���肷�邱�Ƃ����҂���Ă���B�ڂ�����EvalLoadType�̐������Q�Ƃ̂��ƁB


�ESpiFilesManage�̓ǂݍ��ݖ₢���킹
	�f�t�H���g�̎����ł�Susie�̃��W�X�g���ݒ�ɏ]���悤�ɂȂ��Ă���
	TRUE��ԋp����Ƃ��̃t�@�C���ɑΉ�����SpiDllFile�𐶐�����BFALSE��ԋp����ƃ��[�h���Ȃ��B

*/
 
/*
   pre-load callback
*/
static DWORD WINAPI
def_pre_SpiDllFile_callback(SpiDllFile *plugin)
{
	MEMORYSTATUS mem;
	mem.dwLength= sizeof(mem);
	mem.dwAvailVirtual= 0;
	GlobalMemoryStatus(&mem);
	return mem.dwAvailVirtual;
}


/*
   post-laod callback
*/
static DWORD WINAPI
def_post_SpiDllFile_callback(SpiDllFile *plugin)
{
	MEMORYSTATUS mem;
	mem.dwLength= sizeof(mem);
	mem.dwAvailVirtual= 0;
	GlobalMemoryStatus(&mem);
	return mem.dwAvailVirtual;
}



/*
   eval load-type callback
*/
static SpiDllFile::SpiFileLoadType WINAPI 
def_eval_SpiDllFile_callback(SpiDllFile *plugin, DWORD pre, DWORD post)
{
	SpiDllFile::SpiFileLoadType type;

	if ((pre - post) != 0) {
		/* ����͓��I���[�h����̂͂܂����E�E�E */
		type= SpiDllFile::SpiFile_LoadStatic;
	} else {
		/* �{���͂�����`�F�b�N���Ȃ��Ƃ����Ȃ����Ƃ����邯�ǂ�(ex. TLS) */
		type= SpiDllFile::SpiFile_LoadDynamic;
	}
	return type;
}


/*
	SpiFilesManage��LoadPlugin�����s��������dir\name�ɂ���t�@�C�����g�p���邩
	�ǂ��������肷��֐�

	query load callback
*/
static int WINAPI 
def_query_load_SpiFilesManage_callback(SpiFilesManage *own, LPCSTR dir, LPCSTR name)
{
	/* Susie�ł̃��W�X�g���ݒ�ɏ]�� */
	DWORD on_off;

	on_off= 1;	/* �f�t�H���g�l��'���[�h����'�ł���E�E�E */
	knRegTinyGetEx(HKEY_CURRENT_USER, "Software\\Takechin\\Susie\\Plug-in\\Activate", name, &on_off, sizeof(on_off));
	return (on_off&0xffff) ? TRUE : FALSE;
}


static int PASCAL 
null_callback(int nNum, int nDenom, long lData)
{
	return 0;
}



/* ========================================================== SpiOuputStream */

SpiOutputStream::SpiOutputStream()
{
	m_hBitmapInfo= NULL;
	m_hMemory= NULL;
	m_bitmap_info= NULL;
	m_bitmap_memory= NULL;
}

SpiOutputStream::~SpiOutputStream()
{
	/* �f�[�^��������� */
	if (m_bitmap_info) LocalUnlock(m_hBitmapInfo);
	if (m_hBitmapInfo) LocalFree(m_hBitmapInfo);
	if (m_bitmap_memory) LocalUnlock(m_hMemory);
	if (m_hMemory) LocalFree(m_hMemory);
}

void SpiOutputStream::
set_stream_data(HANDLE hBitmapInfo, HANDLE hMemory)
{
	/* �ȑO�̃f�[�^����� */
	abandon_data(ABANDON_AND_FREE_DATA);

	/* �V�����f�[�^��ݒ� */
	m_hBitmapInfo= hBitmapInfo;
	m_hMemory= hMemory;
}

#if	0
void SpiOutputStream::
set_stream_data(HLOCAL hBitmapInfo, HLOCAL hMemory)
{
	set_stream_data((HANDLE)hBitmapInfo, (HANDLE)hMemory);
}
#endif


void SpiOutputStream::
unlock_data()
{
	if (m_bitmap_info && m_hBitmapInfo) LocalUnlock(m_hBitmapInfo);
	if (m_bitmap_memory && m_hMemory) LocalUnlock(m_hMemory);
	m_bitmap_info= NULL;
	m_bitmap_memory= NULL;
}

void SpiOutputStream::
abandon_data(SpiOutputStream_abandon_flag_t flag)
{
	if (flag == SpiOutputStream::ABANDON_AND_FREE_DATA) {
		/* ��������������� -> unlock����free */
		unlock_data();
		if (m_hBitmapInfo != NULL) LocalFree(m_hBitmapInfo);
		if (m_hMemory != NULL) LocalFree(m_hMemory);
	}

	/* �f�[�^����� */
	m_hBitmapInfo= NULL;
	m_hMemory= NULL;
	m_bitmap_info= NULL;
	m_bitmap_memory= NULL;
}


BITMAPINFO *SpiOutputStream::
query_bitmap_info()
{
	if (m_bitmap_info == NULL) m_bitmap_info= (BITMAPINFO *)LocalLock(m_hBitmapInfo);
	return m_bitmap_info;
}

LPVOID SpiOutputStream::
query_bitmap_memory()
{
	if (m_bitmap_memory == NULL) m_bitmap_memory= LocalLock(m_hMemory);
	return m_bitmap_memory;
}


/* ========================================================== SpiInputStream */

/*
 SpiInputStream�ŊǗ����Ă��郁����I�u�W�F�N�g�����t�@�����X�J�E���^�ɂ��
 �Ǘ�����N���X
// HLOCAL�n���h����SpiInputStream�ւ̃|�C���^�̑g�݂ŊǗ�����
*/
/*
typedef class std::multiset<SpiInputStream * > SpiInputStreamSet;
typedef class std::multimap<HLOCAL, SpiInputStreamSet, std::less<HLOCAL> > SpiInputStreamRefrencer;
typedef class std::multimap<HLOCAL, SpiInputStream *, std::less<HLOCAL> > SpiInputStreamRefrencer;
*/
typedef class std::map<HLOCAL, int, std::less<HLOCAL> > SpiInputStreamRefrencer;
SpiInputStreamRefrencer s_spi_input_stream_manage;
typedef SpiInputStreamRefrencer::value_type SpiInputStreamRefrencer_VT;

SpiInputStream::SpiInputStream()
{
	m_hFile= INVALID_HANDLE_VALUE;
	m_hImage= NULL;
	m_file_name= NULL;
	m_image_memory= NULL;
}

SpiInputStream::~SpiInputStream()
{
/*
	if (m_file_name)
		delete [] m_file_name;

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if (m_hImage != NULL)
		LocalFree(m_hImage);
*/
	close_stream_data();
}


/* �R�s�[������ɃR�s�[���̃I�u�W�F�N�g��delete�����ƌ��ʓI��
 * m_image_memory, m_hImage�������ɂȂ�̂ł���͂܂���
 * Ver 0.06.4�ȍ~�ł�m_hImage��ێ����Ă���SpiInputStream�I�u�W�F�N�g�̐���
 * �ێ����Ă����A�ŏI�I�ɕێ����Ă���I�u�W�F�N�g��0�ɂȂ����i�K�ŉ������
 * �悤�ɕύX
 */
/* ������Z�q */
void SpiInputStream::operator=(const SpiInputStream &x)
{
	/* �����I�u�W�F�N�g�ł���Ή������Ȃ� */
	if (this == &x) return ;
	
	m_data_type= x.m_data_type;

	/* �t�@�C�����̃R�s�[ */
	if (m_file_name) {
		delete [] m_file_name;
	}
	if (x.m_file_name) {
		m_file_name= new char [strlen(x.m_file_name) + 1];
		strcpy(m_file_name, x.m_file_name);
	} else {
		m_file_name= NULL;
	}

	/* �f�[�^�̃R�s�[ */
	/* disk */
	m_offset= x.m_offset;

	/* memory */
	m_hImage= x.m_hImage;
	m_size= x.m_size;
	if (m_hImage != NULL) {
		SpiInputStreamRefrencer::iterator it;
		it= s_spi_input_stream_manage.find(m_hImage);
		assert(it != s_spi_input_stream_manage.end());
		if (it == s_spi_input_stream_manage.end()) {
			/* �����ɗ��邱�Ƃ���������o�O���Ă܂���... */
			SpiInputStreamRefrencer_VT p(m_hImage, 2);
			s_spi_input_stream_manage.insert(p);
		} else {
			++(*it).second;
		}
		m_image_memory= x.m_image_memory;
	} else {
		m_image_memory= NULL;
	}
}

/*
 *  �f�[�^���������
 */
void SpiInputStream::
abandon_data()
{
	if (m_file_name) {
		delete [] m_file_name;
		m_file_name= NULL;
	}
	m_hFile= INVALID_HANDLE_VALUE;

	if (m_hImage != NULL) {
		SpiInputStreamRefrencer::iterator it;
		it= s_spi_input_stream_manage.find(m_hImage);
		assert(it != s_spi_input_stream_manage.end());
		if (it != s_spi_input_stream_manage.end()) {
			--(*it).second;
		}
	}
	m_hImage= NULL;
	m_image_memory= NULL;
}

void SpiInputStream::
close_stream_data()
{
	int do_free= FALSE;
	if (m_file_name) {
		delete [] m_file_name;
		m_file_name= NULL;
	}

	if (m_hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hFile);
		m_hFile= INVALID_HANDLE_VALUE;
	}

	if (m_hImage != NULL) {
		SpiInputStreamRefrencer::iterator it;
		it= s_spi_input_stream_manage.find(m_hImage);
		assert(it != s_spi_input_stream_manage.end());
		if (it != s_spi_input_stream_manage.end()) {
			if (--(*it).second <= 0) {
			/* �ێ��J�E���^��0�ɂȂ�����HLOCAL�ƃJ�E���^����2����� */
				s_spi_input_stream_manage.erase(it);
				do_free= TRUE;
			}
		}
	}
	if (m_hImage != NULL && do_free) {
		if (m_image_memory) {
			LocalUnlock(m_image_memory);
			m_image_memory= NULL;
		}
		LocalFree(m_hImage);
		m_hImage= NULL;
	}

}

int SpiInputStream::
set_stream_data(LPCSTR filename, long offset)
{
	close_stream_data();

	if (filename) {
		m_file_name= new char[strlen(filename) + 1];
		strcpy(m_file_name, filename);
	}

	m_offset= offset;

	m_data_type= SpiInputFileStream;

	return 0;
}

int SpiInputStream::
set_stream_data(LPCSTR filename, HLOCAL memory, long size)
{
	close_stream_data();

	if (filename) {
		m_file_name= new char[strlen(filename) + 1];
		strcpy(m_file_name, filename);
	}

	m_hImage= memory;
	m_size= size;
	m_image_memory= (void *)LocalLock(m_hImage);

	m_data_type= SpiInputMemoryStream;

	if (m_hImage != NULL) {
		SpiInputStreamRefrencer::iterator it;
		it= s_spi_input_stream_manage.find(m_hImage);
//		ASSERT(it != s_spi_intpu_stream_manage.end());
		if (it != s_spi_input_stream_manage.end()) {
			++(*it).second;
		} else {
			SpiInputStreamRefrencer_VT p(m_hImage, 1);
			s_spi_input_stream_manage.insert(p);
		}
	}

	return 0;
}

LPSTR SpiInputStream::
query_file_name()
{
	return m_file_name;
}


LPSTR SpiInputStream::
query_buffer(SpiArcFileInfo *info)
{
	if (m_data_type == SpiInputFileStream) {
		return m_file_name;
	} else {
		/* �������n���h�������b�N����K�v������ */
		LPSTR mem= (LPSTR)m_image_memory;
		return mem + (info ? info->position: 0);
	}
}

long SpiInputStream::
query_len(SpiArcFileInfo *info)
{
	if (m_data_type == SpiInputFileStream) {
		return m_offset + (info ? info->position : 0);
	} else {
		return (int)m_size - (info ? info->position : 0);
	}
}

/*
 * Plug-in I/F�Ŏg�p����flag�̒l
 * ���̓t���O�̒l�Ƃ��Ă͂�����g�����Ƃ��ł���
 */
unsigned int SpiInputStream::
query_flag()
{
	if (m_data_type == SpiInputFileStream)
		return 0;	/* Disk */
	else
		return 1;	/* Memory */
}




/* ============================================================== SpiDllFile */

/* Plugin�t�@�C�����(SpiDllFile) */
SpiDllFile::
SpiDllFile()
{
	m_GetPluginInfo= m_IsSupported= NULL;
	m_GetPictureInfo= m_GetPicture= m_GetPreview= NULL;
	m_GetArchiveInfo= m_GetFileInfo= m_GetFile= NULL;

	m_about_name[0]= 0;
	m_file_name[0]= 0;
	m_plugin_ext_list= NULL;

	m_load_type= SpiFile_LoadUnknown;
	m_hModule= NULL;

	/* �f�t�H���g�̃R�[���o�b�N */
	m_pre_load_callback_func= def_pre_SpiDllFile_callback;
	m_post_load_callback_func= def_post_SpiDllFile_callback;
	m_eval_load_type_callback_func= def_eval_SpiDllFile_callback;
//	m_default_callback= (FARPROC)null_callback;
//	m_callback_data32= 0;
}

SpiDllFile::
SpiDllFile(SpiFileLoadType type)
{
	m_GetPluginInfo= m_IsSupported= m_ConfigurationDlg= NULL;
	m_GetPictureInfo= m_GetPicture= m_GetPreview= NULL;
	m_GetArchiveInfo= m_GetFileInfo= m_GetFile= NULL;

	m_about_name[0]= 0;
	m_file_name[0]= 0;
	m_plugin_ext_list= NULL;

	m_load_type= type;
	m_hModule= NULL;

	/* �f�t�H���g�̃R�[���o�b�N */
	m_pre_load_callback_func= def_pre_SpiDllFile_callback;
	m_post_load_callback_func= def_post_SpiDllFile_callback;
	m_eval_load_type_callback_func= def_eval_SpiDllFile_callback;
//	m_default_callback= (FARPROC)null_callback;
//	m_callback_data32= 0;
}

SpiDllFile::
~SpiDllFile()
{
	/* Dll�̃A�����[�h */
	if (m_hModule != NULL) {
		FreeLibrary(m_hModule);
		m_hModule= NULL;
	}
	if (m_plugin_ext_list) {
		delete [] m_plugin_ext_list;
	}
}


/* �w�肳�ꂽPlug-in��ǂݍ���ł݂� */
int SpiDllFile::
LoadFile(LPCTSTR dirname, LPCTSTR filename)
{
	char plugin_file[MAX_PATH];
	HINSTANCE h;
	char version[10];
	DWORD pre, post;
	int count, i;
	std::string item_ext;

	if (m_hModule) {
		/* ���Ƀ��[�h���Ă���E�E�E(�Ⴄ�t�@�C����������Ȃ�����) */
		return TRUE;
	}

	if (dirname != NULL && dirname[0] != 0)
		sprintf(plugin_file, "%s\\%s", dirname, filename);
	else
		strcpy(plugin_file, filename);

	pre= (m_pre_load_callback_func) ? (*m_pre_load_callback_func)(this) : 0;

	h= LoadLibrary(plugin_file);
	if (h == NULL) {
		/* ���[�h�ł��Ȃ� */
		return FALSE;
	}
	post= (m_post_load_callback_func) ? (*m_post_load_callback_func)(this) : 0;

	/* �t�@�C�������R�s�[ */
	strcpy(m_file_name, plugin_file);

	m_GetPluginInfo= GetProcAddress(h, "GetPluginInfo");
	if (m_GetPluginInfo == NULL) {
		/* �擾���s(Spi�t�@�C���ł͂Ȃ�?) */
		goto error_out;
	}

	/* �b��I��m_type��ݒ肷�� */
//	type= m_type;
//	m_type= SpiFile_ImportFilter;
//	m_type= type;
	
	if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(0, version, sizeof(version)) == 0) {
		/* �����Susie Plug-in�ł͂Ȃ�? */
		goto error_out;
	}
	m_about_name[0]= 0;
	if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(1, m_about_name, sizeof(m_about_name)) == 0) {
		/* �����Susie Plug-in�ł͂Ȃ�? */
		goto error_out;
	}
//	GetPluginInfo(0, m_version, sizeof(m_version));

#define	GETPROC(x, y)			do { \
									x= GetProcAddress(h, y); \
									if (x == NULL) goto error_out; \
								} while (0)
#undef	GETPROC

    /* Plug-in�̎�ʂƊ֐��A�h���X���擾 */
	switch (version[2]) {
	case 'I':
		m_type= SpiFile_ImportFilter;
		/* IN */
		m_IsSupported= GetProcAddress(h, "IsSupported");
 		m_GetPictureInfo= GetProcAddress(h, "GetPictureInfo");
		m_GetPicture= GetProcAddress(h, "GetPicture");
		m_GetPreview= GetProcAddress(h, "GetPreview");
		m_ConfigurationDlg= GetProcAddress(h, "ConfigurationDlg");
		break;

	case 'X':
		/* Not Supported */
		m_type= SpiFile_ExportFilter;
		goto error_out;
		break;

	case 'A':
		m_type= SpiFile_Archive;
		/* AM */
		m_IsSupported= GetProcAddress(h, "IsSupported");
		m_GetArchiveInfo= GetProcAddress(h, "GetArchiveInfo");
		m_GetFileInfo= GetProcAddress(h, "GetFileInfo");
		m_GetFile= GetProcAddress(h, "GetFile");
		m_ConfigurationDlg= GetProcAddress(h, "ConfigurationDlg");
		break;

	default:
		/* Unknown Version */
		m_type= SpiFile_Unknown;
		goto error_out;

	}
	/* �擾���� */

	/* ��\�I�Ȋg���q���擾���܂� */
	for (count= i= 0; ; ++i) {
		/* 2n+2 */
		char buffer[1024];	/* �����蒷�������ďo�Ă��邩��? */
		char *s, *t;

		if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(2*i+2, buffer, sizeof(buffer)) == 0) {
			break;
		}

		/* �g���q�̃p�^�[�����R�s�[(*.JPG��JPG�ɕϊ����Ă���) */
		for (s= strtok(buffer, ";"); s != NULL; s= strtok(NULL, ";")) {
			/* ".xxx"�̃p�^�[�����R�s�[����񂾂� */
			t= strrchr(s, '.');
			if (t == NULL) {
				/* "."���Ȃ��̂�"JPG"�Ƃ��ɂȂ��Ă���p�^�[��(���Ă���̂��Ȃ��A�����?) */
				item_ext += ".";
			}
			item_ext += t;
			item_ext += ";";
		}
	}
	if (m_plugin_ext_list) delete [] m_plugin_ext_list;
	m_plugin_ext_list= new char [item_ext.length() + 1];
	strcpy(m_plugin_ext_list, item_ext.c_str());

	/* ���[�h��ʂ�]������ */
	m_load_type= EvalLoadType(pre, post);
	if (m_load_type == SpiFile_LoadDynamic) {
		FreeLibrary(h);
		m_hModule= NULL;
	} else {
		/* ���W���[���n���h����ۑ����Ă��� */
		m_hModule= h;
	}

	return TRUE;

 error_out:
	EvalLoadType(0, 0);	/* free private data */

	FreeLibrary(h);
	return FALSE;
}



int SpiDllFile::
UnloadFile(void)
{
	HINSTANCE h= m_hModule;
	if (h) {
		m_hModule= NULL;
		m_GetPluginInfo= m_IsSupported= m_ConfigurationDlg= NULL;
		m_GetPictureInfo= m_GetPicture= m_GetPreview= NULL;
		m_GetArchiveInfo= m_GetFileInfo= m_GetFile= NULL;
		FreeLibrary(h);
	}
	return TRUE;
}



/* Spi�t�@�C���̊֐����ďo�����[�`�� */

#define	TYPECHECK(x)		if (m_type != x) {return -1;}

int SpiDllFile::
GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	int status;
	typedef int (WINAPI *GetPluginInfo_t)(int, LPSTR, int);

#if	0
	if (m_type != SpiFile_ImportFilter &&
		m_type != SpiFile_Archive)
		return -1;
#endif

	SmartLoad();
	if (m_GetPluginInfo == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetPluginInfo_t proc= (GetPluginInfo_t)m_GetPluginInfo;
	try {
		status= (*proc)(infono, buf, buflen);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}

/* Plug-in�ŗL�̐ݒ�_�C�A���O���Ăяo��
 * �g���d�l�Ƃ���
 *   fno= -1�̎��ɂ�������ConfigurationDlg���������Ă��邩�ǂ�����ԋp����
 */
int SpiDllFile::
ConfigurationDlg(HWND hWnd, int fno)
{
	int status;
	typedef int (WINAPI *ConfigurationDlg_t)(HWND hWnd, int fno);

	SmartLoad();
	if (m_ConfigurationDlg == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	if (fno == -1) {
		if (m_ConfigurationDlg == NULL)
			return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
		else
			return SUSIE_IF_ERROR_SUCCESS;
	} else {
		ConfigurationDlg_t proc= (ConfigurationDlg_t)m_ConfigurationDlg;
		try {
			status= (*proc)(hWnd, fno);
			SmartUnload();
		} catch (...) {
			status= SUSIE_IF_ERROR_INTERNAL;
			SmartUnload();
		}
	}
	return status;
}

int SpiDllFile::
IsSupported(LPSTR filename, DWORD dw)
{
	int status;
	typedef int (WINAPI *IsSupported_t)(LPSTR, DWORD);

	if (m_type != SpiFile_ImportFilter &&
		m_type != SpiFile_Archive)
		return -1;

	SmartLoad();
	if (m_IsSupported == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	IsSupported_t proc= (IsSupported_t)m_IsSupported;
	try {
		status= (*proc)(filename, dw);
		SmartUnload();
	} catch (...) {
		status= 0;
		SmartUnload();
	}
	return status;
}

int SpiDllFile::
GetPictureInfo(LPSTR buf, long len, unsigned int flag, 
			   SpiPictureInfo *lpInfo)
{
	int status;
	typedef int  (WINAPI *GetPictureInfo_t)(LPSTR, long, unsigned int,
									SpiPictureInfo *);

	TYPECHECK(SpiFile_ImportFilter);

	SmartLoad();
	if (m_GetPictureInfo == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetPictureInfo_t proc= (GetPictureInfo_t)m_GetPictureInfo;
	try {
		memset(&lpInfo, 0, sizeof(SpiPictureInfo));	/* for some plug-in's bug */
		status= (*proc)(buf, len, flag, lpInfo);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}

int SpiDllFile::
GetPicture(LPSTR buf, long len, unsigned int flag,
		   HANDLE *pHBInfo, HANDLE *pHBm,
		   FARPROC lpPrgressCallback, long lData)
{
	int status;
	typedef int (WINAPI *GetPicture_t)(LPSTR, long, unsigned int,
								HANDLE *, HANDLE *,
								FARPROC, long);

	TYPECHECK(SpiFile_ImportFilter);

	SmartLoad();
	if (m_GetPicture == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetPicture_t proc= (GetPicture_t)m_GetPicture;
	try {
		status= (proc)(buf, len, flag, pHBInfo, pHBm, lpPrgressCallback, lData);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}

int SpiDllFile::
GetPreview(LPSTR buf, long len, unsigned int flag,
		   HANDLE *pHBInfo, HANDLE *pHBm,
		   FARPROC lpPrgressCallback, long lData)
{
	int status;
	typedef int (WINAPI *GetPreview_t)(LPSTR, long, unsigned int,
								HANDLE *, HANDLE *,
								FARPROC, long);

	TYPECHECK(SpiFile_ImportFilter);

	SmartLoad();
	if (m_GetPreview == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetPreview_t proc= (GetPreview_t)m_GetPreview;
	try {
		status= (*proc)(buf, len, flag, pHBInfo, pHBm, lpPrgressCallback, lData);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}

int SpiDllFile::
GetArchiveInfo(LPSTR buf, long len,
			   unsigned int flag, HLOCAL *lphInf)
{
	int status;
	typedef int (WINAPI *GetArchiveInfo_t)(LPSTR, long,
									unsigned int, HLOCAL *);

	*lphInf= NULL;
	TYPECHECK(SpiFile_Archive);

	SmartLoad();
	if (m_GetArchiveInfo == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetArchiveInfo_t proc= (GetArchiveInfo_t)m_GetArchiveInfo;
	try {
		status= (*proc)(buf, len, flag, lphInf);
		SmartUnload();
		if (*lphInf != NULL)	/* for some plug-in's bug */
			status= SUSIE_IF_ERROR_SUCCESS;
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}

	return status;
}

int SpiDllFile::
GetFileInfo(LPSTR buf, long len,
			LPSTR filename, unsigned int flag, 
			SpiArcFileInfo *lpInfo)
{
	int status;
	typedef int (WINAPI *GetFileInfo_t)(LPSTR, long,
								 LPSTR, unsigned int,
								 SpiArcFileInfo *);

	TYPECHECK(SpiFile_Archive);

	SmartLoad();
	if (m_GetFileInfo == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetFileInfo_t proc= (GetFileInfo_t)m_GetFileInfo;
	try {
		status= (*proc)(buf, len, filename, flag, lpInfo);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}


int SpiDllFile::
GetFile(LPSTR src, long len,
		LPSTR dest, unsigned int flag,
		FARPROC prgressCallback, long lData)
{
	int status;
	typedef int (WINAPI *GetFile_t)(LPSTR, long,
							 LPSTR, unsigned int,
							 FARPROC, long);

	TYPECHECK(SpiFile_Archive);

	SmartLoad();
	if (m_GetFile == NULL)
		return SUSIE_IF_ERROR_NOT_IMPLEMENTED;
	GetFile_t proc= (GetFile_t)m_GetFile;
	try {
		status= (*proc)(src, len, dest, flag, prgressCallback, lData);
		SmartUnload();
	} catch (...) {
		status= SUSIE_IF_ERROR_INTERNAL;
		SmartUnload();
	}
	return status;
}




/* Note: SpiFileManage::QueryPluginLoad()�Ƃ̊֌W
 *  QueryLoadPlugin�Ƃ͒��ڂ̊֌W�͂Ȃ��B������QueryLoadPlugin�̉����ɂ�
 *  SpiDllFile::LoadFile���ďo�т����܂��B�����QueryLoadPlugin�ɋ��߂�
 *  ��Ă���̂��w���Plug-in��I��I�Ƀ��[�h���邱�Ƃ�����ł��B
 *
 */
/*
 * ���̊֐��͎w�肳�ꂽPlug-in�𓮓I�Ƀ��[�h���邩�ǂ�����]�����邱�Ƃ��ړI�ł���B
 * ���I���[�h���K�v�łȂ�Plug-in�܂��͓��I���[�h���Ă͕s�s�����N����Plug-in��]����
 * ��̂����̊֐��ɋ��߂��邱�Ƃł���B
 *
 * EvalLoadType��LoadFile�����ۂɌĂяo�����B�܂�LoadFile�͈�x�����Ă΂�Ȃ��B
 *
 * pre/post�͂��ꂼ��LoadLibrary���钼�O�ƒ���̏�Ԃ�ێ����Ă���悤��32bit�̕ϐ�
 * �ł���A���ꂼ��m_pre_load_callback_func/m_post_load_callback_func���ԋp����l
 * �ł���Bm_eval_load_type_callback_func��pre/post�ł̒l���g���ă��[�h��ʂ����ۂ�
 * �]�����邱�Ƃ����߂���Bm_eval_load_type_callback_func�����݂��Ȃ��ꍇ�ɂ�
 * ���I�ɓǂݍ��ނƔ��肳���B
 * �܂�pre/post�͂Ȃ�炩�̃|�C���^�ł����Ă��悢���A���ꂪ�̎w���̈�͓��I�Ɋm��
 * �����K�v������A������������̂�m_eval_load_type_callback_func�̎d���ł���B
 * �܂�LoadLibrary�Ɏ��s������Load���Plug-in�łȂ��Ɣ��f���ꂽ�ꍇ�ɂ�post==0�Ƃ�
 * �ČĂяo�����B
 */
SpiDllFile::SpiFileLoadType SpiDllFile::
EvalLoadType(DWORD pre, DWORD post)
{
#if	1
	/* SpiDllFile�ł̕W���I�ȓ���͂���ł��� */
	if (m_load_type == SpiFile_LoadUnknown) {
		if (m_eval_load_type_callback_func) {
			return (*m_eval_load_type_callback_func)(this, pre, post);
		} else {
			return SpiFile_LoadDynamic;
		}
	} else {
		return m_load_type;
	}
#else
	/* �������͔h���N���X�Ŏ������悤�ˁB */
	SpiFileLoadType return_type= SpiFile_LoadStatic;
	/* ������Plug-in���g�̃��[�h��Dynamic�ɂ��邩Static�ɂ��邩�̕]��������
	 *   �Ǝv�������ǁE�E�E�A���ꂶ�႟�]���ł��Ȃ���˂�(;_;)
	 * ��͂肱���ň�xLoadLibrary/FreeLibrary���邵���E�E�E<-���Ȃ�Ԕ������ۂ� �B
	 */

	if (m_file_name[0]) {
#define	TLS_EXPECTED_MAX_AVAILABLE			(TLS_MINIMUM_AVAILABLE + 1024)
		DWORD tlss[TLS_EXPECTED_MAX_AVAILABLE];
		int i;
		
		HANDLE h;
		int pre_tls, post_tls;
		MEMORYSTATUS pre_mem, post_mem;

		/*  */
		memset(&pre_mem, 0, sizeof(pre_mem));
		pre_mem.dwLength= sizeof(pre_mem);
		post_mem= pre_mem;
		GlobalMemoryStatus(&pre_mem);

		/* Plug-in��TLS�𗘗p���Ă��邩�ǂ�����]������
		 *  �g���Ă��Ȃ��悤��������LoadStatic�̉\��������
		 */
		for (pre_tls= 0; pre_tls < TLS_EXPECTED_MAX_AVAILABLE; ++pre_tls) {
			if ((tlss[pre_tls]= TlsAlloc()) == 0xffffffff) {
				break;
			}
		}
		for (i= 0; i < pre_tls; ++i) {
			TlsFree(tlss[pre_tls]);
		}

		h= LoadLibrary(m_file_name);
		if (h == NULL)
			return SpiFile_LoadStatic;

		for (post_tls= 0; post_tls < TLS_EXPECTED_MAX_AVAILABLE; ++post_tls) {
			if ((tlss[post_tls]= TlsAlloc()) == 0xffffffff) {
				break;
			}
		}
		for (i= 0; i < post_tls; ++i) {
			TlsFree(tlss[post_tls]);
		}
		FreeLibrary(h);

		/* Something check... */
		GlobalMemoryStatus(&post_mem);

		if ((pre_tls - post_tls) > 0) {
			/* TLS���������Ă���(�����Dynamic����) */
			return_type= SpiFile_LoadDynamic;
		} else {

			/* ���z�A�h���X��Ԃ�H���Ԃ��Ȃ����̃`�F�b�N
			 *   ����ifjpegv6.spi���Ȃ����H�ׂ��Ⴄ�̂�˂�(;_;)�B
			 *   ���ɏǏ󂪔������Ȃ����������Ă��̃`�F�b�N���Ӗ������邩�ǂ����͂��Ȃ�䂾
			 */
			if ((pre_mem.dwAvailVirtual - post_mem.dwAvailVirtual) == 0) {
				return_type= SpiFile_LoadStatic;
			} else {
				return_type= SpiFile_LoadDynamic;
			}

		}

	}
	return return_type;
#endif
}


int SpiDllFile::
SmartLoad()
{
	int status= FALSE;
	if (m_hModule == NULL) {
		/* ���`�ށA���ǂ̓��[�h����K�v������񂾂�Ȃ� */
		if (m_load_type == SpiFile_LoadDynamic) {
			/*  */
#if 1
			m_hModule= LoadLibrary(m_file_name);
			if (m_hModule) {
				status= TRUE;
				/* Common */
				m_GetPluginInfo= GetProcAddress(m_hModule, "GetPluginInfo");
				m_IsSupported= GetProcAddress(m_hModule, "IsSupported");
				m_ConfigurationDlg= GetProcAddress(m_hModule, "ConfigurationDlg");
				/* IN Plug-in */
				m_GetPictureInfo= GetProcAddress(m_hModule, "GetPictureInfo");
				m_GetPicture= GetProcAddress(m_hModule, "GetPicture");
				m_GetPreview= GetProcAddress(m_hModule, "GetPreview");
				/* AM Plug-in */
				m_GetArchiveInfo= GetProcAddress(m_hModule, "GetArchiveInfo");
				m_GetFileInfo= GetProcAddress(m_hModule, "GetFileInfo");
				m_GetFile= GetProcAddress(m_hModule, "GetFile");
			}
			status= (m_hModule != NULL) ? TRUE : FALSE;
#else
			status= LoadFile(NULL, m_file_name);		/* ���`��A�R�s�[���p������Ȃ� */
#endif
		} else if (m_load_type == SpiFile_LoadStatic) {
			/* What Happen? */
			status= LoadFile(NULL, m_file_name);		/* ���`��A�R�s�[���p������Ȃ� */
		} else {
			/* Unknown? */
		}
	} else {
		status= TRUE;
	}
	return status;
}

int SpiDllFile::
SmartUnload()
{
	if (m_hModule) {
		if (m_load_type == SpiFile_LoadDynamic) {
			HINSTANCE h= m_hModule;
			m_hModule= NULL;
			m_GetPluginInfo= m_IsSupported= m_ConfigurationDlg= NULL;
			m_GetPictureInfo= m_GetPicture= m_GetPreview= NULL;
			m_GetArchiveInfo= m_GetFileInfo= m_GetFile= NULL;
			FreeLibrary(h);
		}
	}
	return TRUE;
}


char *
SpiDllFile::GetAboutName()
{
	return m_about_name;
}

char *
SpiDllFile::GetPluginFileName()
{
	return m_file_name;
}


int SpiDllFile::
IsSupported(SpiInputStream *in)
{
	int status= 0;
	HANDLE hFile;
	char buffer[2048];
	DWORD dw;

	if (in->query_input_type() == SpiInputFileStream) {
		hFile= CreateFile(in->query_file_name(), GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return 0;
		} else {
			DWORD readed;
			memset(buffer, 0, sizeof(buffer));
			dw= (DWORD)buffer;
			ReadFile(hFile, buffer, 2048, &readed, NULL);
			CloseHandle(hFile);
		}
	} else {
		int len= in->query_len();
		if (len < 2048) {
			memcpy(buffer, in->query_buffer(), len);
			memset(buffer+len, 0, sizeof(buffer)-len);
			dw= (DWORD)buffer;
		} else {
			dw= (DWORD)in->query_buffer();
		}
	}

	status= IsSupported(in->query_file_name(), dw);

	return status;
}

/* ========-- IN Plug-in --============= */

int SpiDllFile::
GetPicture(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm, FARPROC lpPrgressCallback, long lData)
{
	int status;
	status= GetPicture(in->query_buffer(), in->query_len(), in->query_flag(),
					   phInfo, phm, lpPrgressCallback, lData);
	return status;
}

#if	0
/* �P�Ɉ�����HANDLE�����HLOCAL�ɂȂ������� */
int SpiDllFile::
GetPicture(SpiInputStream *in, HLOCAL *phInfo, HLOCAL *phm)
{
	return GetPicture(in, (HANDLE *)phInfo, (HANDLE *)phm);
}
#endif

int SpiDllFile::
GetPreview(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm, FARPROC lpPrgressCallback, long lData)
{
	int status;
	status= GetPreview(in->query_buffer(), in->query_len(), in->query_flag(),
					   phInfo, phm, lpPrgressCallback, lData);
	return status;
}

#if	0
/* �P�Ɉ�����HANDLE�����HLOCAL�ɂȂ������� */
int SpiDllFile::
GetPreview(SpiInputFileStream *in, HLOCAL *phInfo, HLOCAL *phm)
{
	return GetPreview(in, (HANDLE *)phInfo, (HANDLE *)phm);
}
#endif


/* ========-- AM Plug-in --============= */

/* -- GetArchiveInfo -- */

int SpiDllFile::
GetArchiveInfo(SpiInputStream *in, SpiArcFileInfo **lpInf)
{
	int status;
	HLOCAL hMem;

	*lpInf= NULL;
	status= GetArchiveInfo(in, &hMem);
	if (status == SUSIE_IF_ERROR_SUCCESS) {
		int count= 0;
		SpiArcFileInfo *info;
		info= (SpiArcFileInfo *)LocalLock(hMem);
		if (info == NULL) {
			LocalFree(hMem);
			status= SUSIE_IF_ERROR_MEMORY;
		} else {
			/* �i�[�t�@�C�������擾 */
			while (info[count].method[0] != '\0')
				++count;

			/* �̈���R�s�[�A���̗̈�͌Ăяo�����ŉ�����邱��
			 *   (new���malloc�̕����ǂ���������?)
			 *   ex) delete [] lpInf;
			 *  +1����͔̂O�̂��߂������ꂪ�L���łȂ�Plug-in�����݂���̂�
			 *  �������ă������̈��ݒ肷��B
			 */
			*lpInf= new SpiArcFileInfo[count+1];
			memcpy(*lpInf, info, sizeof(SpiArcFileInfo) * count);
			memset(&((*lpInf)[count]), 0, sizeof(SpiArcFileInfo));

			LocalUnlock(hMem);
			LocalFree(hMem);
		}

	} else {
		/* nothing to do */
	}
	return status;
}

int SpiDllFile::
GetArchiveInfo(SpiInputStream *in, HLOCAL *lphInf)
{
	int status;

	*lphInf= NULL;

	status= GetArchiveInfo(in->query_buffer(), in->query_len(), in->query_flag(), lphInf);
	return status;
}


/* -- GetFileInfo -- */
/*
 *	flag�̓t�@�C������case���ǂ����邩�����w��ł��܂���B
 *    flag==1 -> ��ʂ��Ȃ�
 *    flag==0 -> ��ʂ���
 */
int SpiDllFile::
GetFileInfo(SpiInputStream *in, LPCSTR filename, unsigned int flag, SpiArcFileInfo *info)
{
	int status;

	flag &= 1;	/* omit other bit flags */

	/* call for GetFileInfo */
	status= GetFileInfo(in->query_buffer(), in->query_len(),		/* input */
					(char *)filename,								/* query name */
					in->query_flag() | (flag<<8),					/* flag */
					info);											/* info */
	return status;
}


/* -- GetFile -- */

int SpiDllFile::
GetFile(SpiInputStream *in, SpiArcFileInfo *info, LPCSTR output_file_name,
		FARPROC lpPrgressCallback, long lData)	/* File */
{
	int status;

	/* call for GetFile */
	status= GetFile(in->query_buffer(info), in->query_len(info),	/* input */
					(char *)output_file_name,						/* output */
					in->query_flag() | (0<<8),						/* flag */
					lpPrgressCallback, lData);
	return status;
}


int SpiDllFile::
GetFile(SpiInputStream *in, SpiArcFileInfo *info, HLOCAL *lphMem,
		FARPROC lpPrgressCallback, long lData)			/* Memory */
{
	int status;

	/* for some plug-in's bug */
	*lphMem= NULL;

	/* call for GetFile */
	status= GetFile(in->query_buffer(info), in->query_len(info),	/* input */
					(char *)lphMem,									/* output */
					in->query_flag() | (1<<8),						/* flag */
					lpPrgressCallback, lData);
	return status;
}


/*
 * �w��̓���(in)�Ɏw��̃t�@�C��(query_name)�����݂��邩�ǂ������`�F�b�N���܂��B
 * �w��̃t�@�C�������ɓ��t�@�C���ł������ꍇ�ɂ�next_target�Ɏ��̃p�X���ԋp����܂��B
 *
 * ��)�S�Ă�Plug-in�ɑ΂��ď�����K�p����̂�SpiFilesManage�n�N���X�̎d��
 *    �Ȃ��Ȃ珑�ɓ��̏��Ƀt�@�C���͌`�����قȂ�\�������邩��ł��B
 *      ex) c:/a.lzh/b.zip/c.tar/d.cpp �Ƃ��E�E�E�B�܂�����Ȏ���͏��Ȃ��ł��傤���B
 */
int SpiDllFile::
CheckExistingFile(SpiInputStream *in, LPCSTR query_name, LPSTR *next_target)
{
	int i, loop_end;
	HLOCAL hInfo= NULL;
	SpiArcFileInfo *info= NULL;
	int status= -1;

	if (GetArchiveInfo(in, &info) != SUSIE_IF_ERROR_SUCCESS) {
		return status;
	}

	/* ���X�g����ړI�̃t�@�C����T���܂��B
	 * GetFileInfo���g��Ȃ��̂͏��ɓ����ɂ̃t�@�C���̉\�������邩��ł��B
	 */
	for (loop_end= FALSE, i= 0; info[i].method[0] != 0 && loop_end == FALSE; ++i) {
		char temp[1024];	/* sizeof(temp) >= sizeof(path) + sizeof(filename) */
		int len;

		/* �t�@�C�����̑啶���������ɂ��Ă͋�ʂ��܂��� */
		strcpy(temp, info[i].path);
		strcat(temp, info[i].filename);
		len= strlen(temp);
		if (strnicmp(temp, query_name, len) == 0) {
			HLOCAL mem;

			/* ��v���� */
			if (temp[len] == '\\' || temp[len] == '/' ) {	/* ���ɓ��p�X */
				*next_target= (char *)&query_name[len];	/* ���̖₢���킹�� */

				/* ���̓X�g���[�����č쐬 */
				if (GetFile(in, &info[i], &mem, (FARPROC)null_callback, 0) == SUSIE_IF_ERROR_SUCCESS) {
					/* �擾OK */

					temp[len]= 0;
					in->set_stream_data(temp, mem, info[i].filesize);

					status= 0;
				} else {
					/* �G���[�E�E�E����Plug-in�� */
					status= -1;
				}
				loop_end= TRUE;

			} else if (temp[len] == '\0') {					/* ��v */

				/* ���̓X�g���[�����č쐬 */
				if (GetFile(in, &info[i], &mem, (FARPROC)null_callback, 0) == SUSIE_IF_ERROR_SUCCESS) {
					/* �擾OK */
					*next_target= NULL;				/* ���͂Ȃ� */
					temp[len]= 0;
					in->set_stream_data(temp, mem, info[i].filesize);
					status= 1;
				} else {
					status= -1;
				}

				loop_end= TRUE;

			} else {										/* ������v -> ����̓X�L�b�v */
				/* skip */
			}
		} /* if (strnicmp() == 0) */
	} /* for (loop_end= FALSE, i= 0; ...) */

	/* �̈����� */
	if (info) delete [] info;

	return status;
}



/* =========================================================== SpiFilesManage */
/* Plugin�t�@�C�����Ǘ�����(SpiFilesManage) */
//int SpiFilesManage::m_reference_count= 0;

SpiFilesManage::
SpiFilesManage()
{
	m_plugin_dir[0]= 0;
//	m_plugin_offset= NULL;
//	m_plugin_info= NULL;
	m_query_plugin_load_callback_func= def_query_load_SpiFilesManage_callback;

	SetPluginDirectory(NULL);
}


SpiFilesManage::
~SpiFilesManage()
{
	UnloadPluginFile();
}


/*
 *	Plugin�̂���f�B���N�g����ݒ肷��
 *		Plugin���J�����g���ASusie Plugin�f�B���N�g���ɂȂ��ꍇ��
 *		�ݒ肷��K�v������B
 *  �Ǝ���Plug-in�̂���f�B���N�g�����Ǘ�����ꍇ�ɂ̓p�����[�^�Ƃ���
 *  �n�����ƁB�܂�Plug-in�f�B���N�g���ȊO�ɒǉ��������ꍇ�ɂ�AddPlugin
 *  �ŌX�ɓo�^���邱�ƁB
 */
int SpiFilesManage::
SetPluginDirectory(LPCSTR directory)
{

	if (directory == NULL) {
		/* NULL�̏ꍇ�ɂ�Susie��Plugin�f�B���N�g�����Q�Ƃ��� 
		 *   �������A�����Susie 0.32(?)�ȏオ�K�v
		 */
		m_plugin_dir[0]= 0;
		knRegTinyGetEx(HKEY_CURRENT_USER, 
					 "Software\\Takechin\\Susie\\Plug-in",
					 "Path",
					 m_plugin_dir, sizeof(m_plugin_dir));
		if (m_plugin_dir[0] == 0) {
			/* �擾���s */
			;
		}
	} else {
		strcpy(m_plugin_dir, directory);
	}
	return TRUE;
}


/* 
 *	SetPluginDirectory�Ŏw�肳�ꂽ�f�B���N�g������Plugin
 *  �t�@�C�������[�h���܂�
 */
void SpiFilesManage::
LoadPluginFile()
{
	char plugin_file[sizeof(m_plugin_dir) + 10];
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	BOOL status;

	/* ����SPI�t�@�C���� -> [Dir]\*.SPI */
	if (m_plugin_dir[0] != 0) {
		sprintf(plugin_file, "%s\\*.%s", m_plugin_dir, SUSIE_PLUGIN_EXT);
	} else {
		sprintf(plugin_file, "*.%s", SUSIE_PLUGIN_EXT);
	}

	/* SPI�t�@�C���ꗗ���擾 */
	hFind= FindFirstFile(plugin_file, &find_data);
	if (hFind == INVALID_HANDLE_VALUE) {
		/* ��v����t�@�C���Ȃ� */
		return ;
	}

	/* ���Ƀ`�F�[��������΂���͔j������ */
	if (m_plugins.size() > 0)
		UnloadPluginFile();

	status= TRUE;
	for (; status == TRUE; status= FindNextFile(hFind, &find_data)) {
		SpiFile *spi;

#define	IGNORE_ATTR	(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)
		if ((find_data.dwFileAttributes & IGNORE_ATTR) != 0) {
			/* �f�B���N�g���������B -> �������� */
			continue;
		}

		/* Plug-in�t�@�C�����擾���� */
		spi= QueryPluginLoad(find_data.cFileName);
		if (spi) {
			/* ����͊Ǘ����֒ǉ����� */
			m_plugins.push_back(spi);
		}

	}
	FindClose(hFind);

	/* ����Ń`�F�[���������܂����B */

	return;
}


/* LoadPluginFile()�Ń��[�h����Plugin�t�@�C�����A�����[�h���܂� */
void SpiFilesManage::
UnloadPluginFile()
{
	/* QueryPluginLoad��AddPlugin�Ŏ擾����SpiFile��delete����
	 *  �{����AddPlugin�Œǉ��������̂ɂ��Ă�delete���Ȃ����������̂���
	 */
#if 1
	/* Create/FindNext/Close���Ăяo�����ق����悢�̂��낤��? */
	std::vector<SpiFile *>::iterator it;
	it= m_plugins.begin();
	for (it= m_plugins.begin(); it != m_plugins.end(); ++it) {
		delete *it;
	}
#else
	SpiFindData *find= CreateFindData();
	while (find) {
		SpiFile *spi;
		spi= FindNextPlugin(find);
		delete spi;
	}
	CloseFindData(find);
#endif

	m_plugins.erase(m_plugins.begin(), m_plugins.end());
}


/*
  �w��̃f�B���N�g�����ɂ���Plug-in�̂����ǂ�𗘗p���邩���`���܂��B
  �f�t�H���g�̎����ł͂��ׂĂ�Plug-in���u���[�h����v�Ƃ��܂�
  �ύX����ɂ͔h���N���X�������QueryPluginLoad�����������邩�R�[���o�b
  �N�֐����`���܂��B
  �Ⴆ�΁u�g�p���Ȃ��v�Ƃ����ݒ�ɂȂ��Ă���Plug-in���ǂ����`�F�b�N����
  �肵�܂��B
 */
SpiFile *SpiFilesManage::
QueryPluginLoad(LPCSTR name)
{
	int status= TRUE;		/* TRUE: load / FALSE: do not load */
	SpiDllFile *spi= NULL;

	/* ������SpiFile�x�[�X�ō쐬�����ق��������̂��Ȃ�?�B */

	/* �R�[���o�b�N�֐����Ăяo���Ė₢���킹�� */
	if (m_query_plugin_load_callback_func) {
		status= (*m_query_plugin_load_callback_func)(this, m_plugin_dir, name);
	}

	if (status) {
		spi= new SpiDllFile;
		if (spi->LoadFile(m_plugin_dir, name) == FALSE) {
			/* NG */
			delete spi;
			spi= NULL;
		}
	}
	return spi;
}


/* Plug-in��ǉ����� */
int SpiFilesManage::
AddPlugin(SpiFile *spi)
{
	m_plugins.push_back(spi);
	return TRUE;
}


/* fileName�œW�J�ł���Plugin��Ԃ��܂��B���݂��Ȃ����NULL��ԋp���܂�
   SpiFindData��ԋp������������̂����E�E�E�B
 */
SpiFile *SpiFilesManage::
SearchPlugin(LPCSTR fileName, int offset)
{
	HANDLE hFile;
	SpiFile *spi, *status;
	char head_buff[1024 * 2 + 128];
	DWORD readed= 0;
	std::vector<SpiFile *>::iterator it;

	/* �t�@�C�����I�[�v�� */
	hFile= CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ,
					  NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	/* �w��̈ʒu�܂Ńt�@�C���|�C���^���ړ�(�����܂�seek����K�v�͂Ȃ��ł��傤) */
	if (SetFilePointer(hFile, offset, NULL, FILE_BEGIN) < offset) {
		/* offset�̈ʒu�܂ňړ��ł��Ȃ� -> �t�@�C�����Z������ */
		CloseHandle(hFile);
		return NULL;
	}

	memset(head_buff, 0, sizeof(head_buff));
	if (ReadFile(hFile, head_buff, sizeof(head_buff), &readed, NULL) == FALSE) {
		/* �t�@�C���ǂݍ��݂Ɏ��s */
		CloseHandle(hFile);
		return NULL;
	}

	spi= NULL;
	for (it= m_plugins.begin(); it != m_plugins.end(); ++it) {
		int support;

		status= *it;		/* it->second()? */

		/* ���̃t�@�C�����T�|�[�g���Ă��邩? */
		support= status->IsSupported((LPSTR)fileName, (DWORD)&head_buff[0]);
		if (support != 0 /* && status->GetSpiFileType() == SpiFile::SpiFile_ImportFilter */) {
			spi= status;
//			m_current_plugin= spi; /* �J�����g�v���O�C����ݒ� */
			break;
		}
	}

	/* �t�@�C���N���[�Y */
	CloseHandle(hFile);

	return spi;
}


/* 
 *	�`�F�[���ɂȂ��ł���Plugin�t�@�C�����������܂� 
 *	(�O���C���^�t�F�[�X)
 */
SpiFindData *SpiFilesManage::
CreateFindData(void)
{
	SpiFindData *find= new SpiFindData;
	find->m_pos= m_plugins.begin();
	return find;
}

/* Backward Compatibility function */
SpiFile *SpiFilesManage::
FindFirstPlugin(SpiFindData *find)
{
	return FindNextPlugin(find);
}


SpiFile *SpiFilesManage::
FindNextPlugin(SpiFindData *find)
{
#if	1
	SpiFile *spi;

	if (find == NULL) {
		spi= NULL;
	} else {
		if (find->m_pos == m_plugins.end()) {
			spi= NULL;
		} else {
			spi= *find->m_pos;		/* second()? */
			++find->m_pos;
		}
	}
	return spi;
#else
	SpiFile *spi;
	spi= m_plugins[data->GetPos()];
	if (spi) {
		data->Add();
		return spi;
	}
	return NULL;
#endif
}

void SpiFilesManage::
CloseFindData(SpiFindData *data)
{
	/* do close a data for SpiFindData */
	/* nothing to do */
	if (data)
		delete data;
}



/*

	int OpenFile(SpiInputStream *in, LPCSTR filename, int offset= 0)

Description:
	�t�@�C�����I�[�v������B
	����͏��ɓ��̃t�@�C���Ȃǂւ̃A�N�Z�X���T�|�[�g����B

Parameter:
	in:	���̓t�@�C���܂��̓������C���[�W�B����̓t�@�C���̂ݑΉ�
	filename: ���̓t�@�C�����̃I�[�v���������t�@�C���Bin��filename������̏ꍇ�����肤��
	offset: in�̃f�[�^�̐擪����ǂݔ�΂��o�C�g��(���T�|�[�g)

Return Value:
	=>0: ����ɏI���Bin�̓I�[�v���������Ƃɂ���Ď擾�����t�@�C�����܂���
	     �t�@�C���C���[�W�ɍX�V�����B
	< 0: filename�����݂��Ȃ��Ȃǂ̗��R�Ŏ��s�B
         ���ɓ��t�@�C���ł���΂����������Plug-in���Ȃ����Ƃ��l������B

Remark:
	�R�[���o�b�N�֐��͌Ăяo����܂���B�ł��̂ň�U�Ăяo���ƃL�����Z��
	���邱�Ƃ͂ł��܂���B

*/
#define	DIR_SEPARATOR		"\\/"
int SpiFilesManage::
OpenFile(SpiInputStream *in, LPCSTR filename/*, FARPROC lpPrgressCallback, long lData*/)
{
	SpiFindData *find;
	int status= 0, open_flag;
	LPSTR file, p, e;
	LPSTR edit_file;
	HANDLE hFile;
	SpiInputStream new_in;
	FARPROC callback= /*lpPrgressCallback ? lpPrgressCallback :*/ (FARPROC) null_callback;

	/* ���͎�ʂ��`�F�b�N */
	switch (in->query_input_type()) {
	case SpiInputMemoryStream:
		return 1;
	case SpiInputFileStream:
		break;
	default:
		return -1;
	}

	/* ���̂����肪in->query_input_type == SpiInputFileStream�łȂ��ƑΉ����Ă��Ȃ�
	 * ���Ă��Ƃł��B
	 */
	file= in->query_buffer();

	/* (1)Preprocess opening file */
	/* �w�肳�ꂽ�t�@�C�����ʏ�̃t�@�C�������ǂ������`�F�b�N 
	 *   �����ŃI�[�v���ɐ�������Ώ����͂���ŏI���B
	 */
	hFile= CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		/* in�̏C���͕K�v�Ȃ� */
		return 1;
	}

	/* (2)Preprocess splitting filename */
	/* copy filename */
	p= new char [strlen(file) + 1+1+10];
	edit_file= new char [strlen(file)+1+1+10];	/* +10�͗\�� */
	strcpy(p, file);
	file= p;

	/* ���t�@�C����T�� */
	edit_file[0]= 0;
	open_flag= FALSE;
	for (e= strtok(file, DIR_SEPARATOR); e != NULL; e= strtok(NULL, DIR_SEPARATOR)) {
		strcat(edit_file, e);

		/* Try to open file[edit_file] */
		hFile= CreateFile(edit_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			open_flag= TRUE;

			/* New InputStream */
			new_in.set_stream_data(edit_file, 0);

			break;
		}

		strcat(edit_file, "\\");
	}

	/* ���`����A�I�[�v���ł��Ȃ�����
	 *     -> ���s�ŏI���
	 */
	if (open_flag == FALSE) {
		delete edit_file;
		delete p;
		return -1;
	}

	/* �c��̃t�@�C�������쐬���� */
/* �ǂ����̕��@���������͓���ˁB */
#if	1
	strcpy(edit_file, e + strlen(e) + 1);
#else
	edit_file[0]= 0;
	while ((e= strtok(NULL, DIR_SEPARATOR))) {
		strcat(edit_file, e);
		strcat(edit_file, "\\");	/* ���`��A`/'�̕�����������? */
	}
#endif

	e= edit_file;

	/* �����܂ł�
	 * in�������C���[�W�ɂ���filename�͏��ɓ��t�@�C���ł���B
	 *   �ƁA���肷��B
	 */
	find= CreateFindData();
	if (find) {
		SpiFile *s;
		int check_status;
		LPSTR next;

		for (check_status= 0; check_status == 0; ) {
			/* �ŏ���1���߂�offset==0 */
			while ((s= FindNextPlugin(find)) != NULL) {
				SpiDllFile *spi= dynamic_cast <SpiDllFile *>(s);
				int status;

				/* �L���X�g�Ɏ��s���Ă���̂Ȃ瑼�̔h���N���X�̃I�u�W�F�N�g */
				if (spi == NULL) continue;

				/* AM Plug-in�łȂ���΃X�L�b�v */
				if (spi->GetSpiFileType() != SpiFile::SpiFile_Archive)
					continue;

				/* IsSupported -> GetArchiveInfo -> GetFileInfo -> GetFile */
				status= spi->IsSupported(&new_in);
				if (status == 0) {
					/* ���T�|�[�g */
					continue;
				}

				/* �T�|�[�g�\? */
				check_status= spi->CheckExistingFile(&new_in, e, &next);
				if (check_status == 0) {		/* Let's next try[down file name tree] */
					e= next;
				} else if (check_status < 0) {	/* error */
					/* IsSupported�͐�����Ԃ������ۂɓW�J���悤�Ƃ���ƃG���[�ɂȂ��� */
					continue;
				} else { /*check_status > 0 */	/* found it */
					break;
				}

			} /* while ((s= FindNextPlugin(find)) != NULL) */
			CloseFindData(find);

			/* next try offset == 128 
			 * Not supported!!. How the caller determine this offset parameter?
			 *  -> In next implementation, i will probably resolve this problem.
			 */

			/* ... implementation code here ... */

		} /* for (;check_status ...;) */

		if (check_status > 0) {		/* �擾���� */
			/* Stream���R�s�[ */
			*in= new_in;

			/* new_in�̓��e��j�� */
			new_in.abandon_data();

			status= 1;
		} else {					/* �G���[ */
			status= -1;
		}

	} else {
		/* �G���[ */
		status= -1;
	}

	/* �ꎞ�̈�̍폜 */
	delete edit_file;
	delete p;

	return status;
}


/*
	in����Bitmap�C���[�W���擾����
  Parameters:
	mode == 0: GetPicture���s�Ȃ�
	mode != 0: GetPreview���s�Ȃ�

  Return Value:
	!= NULL: �W�J�ɐ����B�W�J�ɗp����SpiFile(SpiDllFile)��ԋp
	== NULL: �W�J�Ɏ��s
*/
SpiFile *SpiFilesManage::
GetBitmapImage(SpiInputStream *in, SpiOutputStream *out, int mode, FARPROC lpPrgressCallback, long lData)
{
	HANDLE hBitmap, hMem;
	SpiFindData *find;
	int status= FALSE;
	SpiFile *s= NULL;
	FARPROC callback= lpPrgressCallback ? lpPrgressCallback : (FARPROC) null_callback;

	find= CreateFindData();
	if (find) {

		while ((s= FindNextPlugin(find)) != NULL) {
			SpiDllFile *spi= dynamic_cast <SpiDllFile *>(s);
			int status;

			/* �L���X�g�Ɏ��s���Ă���̂Ȃ瑼�̔h���N���X�̃I�u�W�F�N�g */
			if (spi == NULL) continue;

			/* IN Plug-in�łȂ���΃X�L�b�v(FindNextPlugin�Ƀt�B���^�[�t�����ق����������) */
			if (spi->GetSpiFileType() != SpiFile::SpiFile_ImportFilter)
				continue;

			status= spi->IsSupported(in);
			if (status == 0) {
				/* ���T�|�[�g */
				continue;
			}

			if (mode == 0) {
				status= spi->GetPicture(in, &hBitmap, &hMem, callback, lData);
			} else {
				status= spi->GetPreview(in, &hBitmap, &hMem, callback, lData);
				if (status == SUSIE_IF_ERROR_NOT_IMPLEMENTED) {
					/* GetPreview���������Ă��Ȃ��̂Ȃ�GetPicture�����ɌĂ� */
					status= spi->GetPicture(in, &hBitmap, &hMem, callback, lData);
				}
			}

			if (status == 0) {
				/* �擾���� */
				out->set_stream_data(hBitmap, hMem);
				break;
			} else if (status == SUSIE_IF_ERROR_CANCEL_EXPAND) {
				/* �L�����Z�����ꂽ */
				s= NULL;
				break;
			} else {
				/* ���̂������Ă݂� -> next plug-in */
			}

		} /* while ((s= FindNextPlugin(find)) != NULL) */
		CloseFindData(find);

	}

	return s;
}

