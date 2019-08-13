#ifndef	SUSIE_PLUGIN_SPI_IF_H
#define	SUSIE_PLUGIN_SPI_IF_H
/*
 *	Susie Plugin Interface
 *		Copyright 1996-2000 by kana
 *
 *
 */
/*
 * Require: <vector> / (STL:vector.h)
 *			"spi_api.h"
 *			"spi_misc.h"
 *  using:std;
 *
 * �Â�Visual C++�ɕt����STL��vector�Ƃ��o�O���Ă���̂ŏC�����K�v�ł��B
 * �������̂���vector����߂�CPtrArray�Ƃ��g�����ق����C�����ȒP�ł��B
 * CPtrArray�ł��r���h�\�Ȕł��ꎞ�I�ɑ��݂��Ă��܂������ێ炳��Ă��Ȃ�
 * ���߂ɔj������܂����B�ȒP�ȕύX�Ȃ̂ŕK�v�ȕ��͂������ł���Ă��������B
 */

#define	SPIIF_WRAPPER_LIBRARY_VERSION	6


/*
	SpiDllFile�ւ̓��o��I/F
*/
/* 
 * Plug-in�̒��ړI��I/F�ł���SpiDllFile��I/F�ɂ������ǉ���������
 *
 */
typedef enum {
	SpiInputFileStream= 0,
	SpiInputMemoryStream
} SpiInputDataType_t;

class SpiInputStream {
public:
	SpiInputStream();
	virtual ~SpiInputStream();
	
public:
	int set_stream_data(LPCSTR file_name, long offset= 0);
	int set_stream_data(LPCSTR file_name, HLOCAL memory, long size);

	void abandon_data();

	SpiInputDataType_t query_input_type() { return m_data_type; }

	LPSTR query_file_name();
	LPSTR query_buffer(SpiArcFileInfo *info= NULL);
	long query_len(SpiArcFileInfo *info= NULL);
	unsigned int query_flag();

	/* �R�s�[�R���X�g���N�^���K�v����? */
	/* name(const name &x); */

	/* ������Z�q
	 * abandon_data�͂��Ȃ��Ă�OK�ɂȂ����Ǝv���܂���...�B
	 * ���t�@�����X�J�E���^�ŊǗ�����悤�ɕύX���܂����B
	 */
	void operator =(const SpiInputStream &x);

private:
	void close_stream_data();

private:
	LPSTR	m_file_name;	/* �t�@�C�����ւ̃|�C���^ */
	long	m_offset;		/* �J�n�I�t�Z�b�g */

	HANDLE	m_hFile;		/* not used */

	HLOCAL	m_hImage;
	void	*m_image_memory;/* m_hImage�����b�N������̃������̈� */
	size_t	m_size;

	SpiInputDataType_t m_data_type;
};


class SpiOutputStream {
 public:
	SpiOutputStream();
	virtual ~SpiOutputStream();

 public:
	typedef enum {
		ABANDON_DATA= 0,		/* �f�[�^��������邾�� */
		ABANDON_AND_FREE_DATA,	/* �f�[�^���������ۂɃf�[�^��������� */
	} SpiOutputStream_abandon_flag_t;

	/* interface */
	/* ����2�̃o���G�[�V�����͑S���������� */
	void set_stream_data(HANDLE hBitmapInfo, HANDLE hMemory);
//	void set_stream_data(HLOCAL hBitmapInfo, HLOCAL hMemory);

	/* �����ŕێ����Ă���f�[�^��������� */
	void abandon_data(SpiOutputStream_abandon_flag_t flag= SpiOutputStream::ABANDON_DATA);

	/* ���b�N������������unlock����(����������͂��܂���) */
	void unlock_data();

	/* query interface, query_xxxxx function get xxxxx value in the object. */
	BITMAPINFO *query_bitmap_info();
	LPVOID query_bitmap_memory();

	/* ������Z�q */
/* 	void operator =(SpiOutputStream &x); */

 private:
	HLOCAL m_hBitmapInfo;
	HLOCAL m_hMemory;

	BITMAPINFO *m_bitmap_info;
	LPVOID m_bitmap_memory;
};






/* ================================================================= SpiFile */

/* Susie Plug-in�ւ̃C���^�t�F�[�X�N���X
 *  �e�֐��̐����͂���Ȃ��ł����(^^)�B
 *
 *   ��������Plug-in���쐬����ꍇ�ɂ�SpiFile����h��������SpiFileManage�ɓo�^����
 */
class SpiFile {
 public:
	typedef enum {
		SpiFile_Unknown= -1,
		SpiFile_ImportFilter= 0,
		SpiFile_ExportFilter= 1,
		SpiFile_Archive= 2,
	} SpiFileType;

	SpiFile() { m_type= SpiFile_Unknown; };
	virtual ~SpiFile() {};

 public:
	/* Load or Unload specified Plugin-File  */
	virtual int LoadFile(LPCTSTR direname, LPCTSTR filename)= NULL;
	virtual int UnloadFile(void)= NULL;

	/* misc(Plug-in type) */
	virtual SpiFileType GetSpiFileType(void) { return m_type; };


	/* ---- Plugin-Function ---- */
	/* Common */
	virtual int GetPluginInfo(int infono, LPSTR buf, int buflen) { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int IsSupported(LPSTR filename, DWORD dw) { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int ConfigurationDlg(HWND hWnd, int func) { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};

	/* 00IN */
	virtual int GetPictureInfo(LPSTR buf, long len, unsigned int flag, 
					   SpiPictureInfo *lpInfo)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int GetPicture(LPSTR buf, long len, unsigned int flag,
				   HANDLE *pHBInfo, HANDLE *pHBm,
				   FARPROC lpPrgressCallback, long lData)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int GetPreview(LPSTR buf, long len, unsigned int flag,
				   HANDLE *pHBInfo, HANDLE *pHBm,
				   FARPROC lpPrgressCallback, long lData)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};

	/* 00AM */
	virtual int GetArchiveInfo(LPSTR buf, long len,
							   unsigned int flag, HLOCAL *lphInf)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int GetFileInfo(LPSTR buf, long len,
							LPSTR filename, unsigned int flag, 
							SpiArcFileInfo *lpInfo)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};
	virtual int GetFile(LPSTR src, long len,
						LPSTR dest, unsigned int flag,
						FARPROC prgressCallback, long lData)  { return SUSIE_IF_ERROR_NOT_IMPLEMENTED;};

#if	0
	/* �����̊֐��|�C���^�Ȃǂ����N���X�͕ʓr��`���� */
 protected:
	/* SPI�t�@�C���ւ̊֐��|�C���^ */
	FARPROC m_GetPluginInfo;
	FARPROC m_IsSupported;
	FARPROC m_GetPictureInfo;
	FARPROC m_GetPicture;
	FARPROC m_GetPreview;
	FARPROC m_GetArchiveInfo;
	FARPROC m_GetFileInfo;
	FARPROC m_GetFile;

	char m_version[10];	/* GetPluginInfo�̏��ԍ�0�̏����i�[���܂� */
	char m_name[64];		/* Plugin�t�@�C���� */
	HANDLE m_hModule;
#endif
 protected:
	SpiFileType m_type;		/* Plug-in�̎�� */
};




/* ============================================================== SpiDllFile */

/* ���ۂ�DLL�t�@�C���ɑ�������N���X */
class SpiDllFile : public SpiFile {
public:
	typedef enum {
		SpiFile_LoadUnknown= 0,			/* ���[�h��ʂ��s�� */
		SpiFile_LoadStatic,				/* ��x���[�h������A�����[�h���Ȃ� */
		SpiFile_LoadDynamic,			/* �g�p����Ƃ��������[�h����
										 * ���������\�[�X�R��Ȃǂ̗��R��Dynamic��
										 * ���p����̂ɖ�肪����ꍇ�ɂ�Static�ɂȂ�B
										 * ����͍ŏ���LoadLibrary�̎��ɕ]�������B
										 */
	} SpiFileLoadType;

	SpiDllFile();
	SpiDllFile(SpiFileLoadType type);
	virtual ~SpiDllFile();

public:
	/* -- */
	/* Plugin-File Loading... */
	virtual int LoadFile(LPCTSTR direname, LPCTSTR filename);
	virtual int UnloadFile(void);

	/* convinience function */
	char *GetAboutName();
	char *GetPluginFileName();


	/* ---- Plugin-Function ---- */
	/* Common */
	virtual int GetPluginInfo(int infono, LPSTR buf, int buflen);
	virtual int IsSupported(LPSTR filename, DWORD dw);
	virtual int ConfigurationDlg(HWND hWnd, int func);

	/* 00IN */
	virtual int GetPictureInfo(LPSTR buf, long len, unsigned int flag, 
					   SpiPictureInfo *lpInfo);
	virtual int GetPicture(LPSTR buf, long len, unsigned int flag,
				   HANDLE *pHBInfo, HANDLE *pHBm,
				   FARPROC lpPrgressCallback, long lData);
	virtual int GetPreview(LPSTR buf, long len, unsigned int flag,
				   HANDLE *pHBInfo, HANDLE *pHBm,
				   FARPROC lpPrgressCallback, long lData);

	/* 00AM */
	virtual int GetArchiveInfo(LPSTR buf, long len,
							   unsigned int flag, HLOCAL *lphInf);
	virtual int GetFileInfo(LPSTR buf, long len,
							LPSTR filename, unsigned int flag, 
							SpiArcFileInfo *lpInfo);
	virtual int GetFile(LPSTR src, long len,
						LPSTR dest, unsigned int flag,
						FARPROC prgressCallback, long lData);

	/* �ȉ���SpiInputStream/SpiOutputStream���g��I/F */
	/* common */
	virtual int IsSupported(SpiInputStream *in);

	/* ---------- 00IN -------------- */
	/* HANDLE�x�[�X�͎~�߂��ق������������� */
	virtual int GetPicture(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm,
						   FARPROC lpPrgressCallback, long lData);
	virtual int GetPreview(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm,
						   FARPROC lpPrgressCallback, long lData);

	/* ---------- 00AM -------------- */
	/*
	 * �` GetArchiveInfo �`
	 * [Description]
	 *   in: ���͂��Ӗ�����
	 *   lpInf: �w��̓��͂Ɋ܂܂��t�@�C���̈ꗗ�BSpiArcFileInfo[]�ւ̃|�C���^���ԋp�����
	 *   lphInf: �w��̓��͂Ɋ܂܂��t�@�C���̈ꗗ���i�[���Ă��郁�����̈�̃��[�J���n���h��
	 * [Return Value]
	 *   SpiDllFile::GetArchiveInfo�Ɠ��l�B���������������ꍇ�ɂ�lpInf�Ŏ������̈��
	 *   �������K�v������B
	 *   SpiDllFile::GetArchiveInfo�͐������ɂ͕K��SUSIE_IF_ERROR_SUCCESS��ԋp���邱�Ƃ�
	 *   �v�������B�ꕔ��Plug-in�ł̕s��ɑΏ�����ӔC������B
	 *   (Require: SpiIf.cpp Ver 0.06 and above)
	 */
	virtual int GetArchiveInfo(SpiInputStream *in, SpiArcFileInfo **lpInf);
	virtual int GetArchiveInfo(SpiInputStream *in, HLOCAL *lphInf);

	virtual int GetFileInfo(SpiInputStream *in, LPCSTR filename,
							unsigned int flag, SpiArcFileInfo *info);

	/*
	 * in: GetArchiveInfo�ɑ΂��ēn��������
	 * info: �擾���悤�Ƃ���fileinfo�\���̂ւ̃|�C���^
	 *       NULL��n�����ꍇ�ɂ�in��position�ɂ��ď����ςƉ��肷��
	 *       info��GetArchiveInfo�܂���GetFileInfo�Ŏ擾�������̂�n��
	 * output_file_name: �o�͐悪�t�@�C���̏ꍇ�̏o�̓t�@�C����
	 * lphMem: �o�͐悪�������̏ꍇ�̃��[�J���n���h���ւ̃|�C���^
	 */
	virtual int GetFile(SpiInputStream *in, SpiArcFileInfo *info, LPCSTR output_file_name,
						FARPROC lpPrgressCallback, long lData);	/* �o�͂��t�@�C�� */
	virtual int GetFile(SpiInputStream *in, SpiArcFileInfo *info, HLOCAL *lphMem,
						FARPROC lpPrgressCallback, long lData);	/* �o�͂������� */


	/* File/Memory���͂������N���X�ŃA�^�b�`���������������悤�ȋC������ */
	/* �` CheckExistingFile �`
	 * �w��̃t�@�C�����A�[�J�C�u���ɑ��݂��邩�ǂ������`�F�b�N����
	 *   �t�@�C�����̓A�[�J�C�u�t�@�C���ɑ΂��鑊�΃p�X�Ŏw�肷��
	 *   �֐��̓A�[�J�C�u�t�@�C�����ɑO����v����t�@�C����ԋp����
	 *   A. ?:/xxxx.lzh/yyyy.lzh/z.jpg
	 *      ^^^^^^^^^^^���t�@�C��
	 *                  ^^^^^^^^���ɓ����Ƀt�@�C��
	 *                           ^^^^^���ɓ��t�@�C��
     *   �Ƃ����ꍇ������B
	 *    CheckExistingFile("?:/xxxx.lzh", "yyyy.lzh/z.jpg", &ptr);
	 *   �ł́Aptr��"z.jpg"���w���悤�ɏ�����������B�܂�"yyyy.lzh"��"z.jpg"�̊Ԃ̃Z�p���[�^��
	 *   "\0"�Œu�������B
	 *   ���̃`�F�b�N�̍ۂɂ͊���IsSupported���Ă�ŃT�|�[�g�����`���ł��邱�Ƃ��m�F���邱��
	 */
	/*
	 * [Description]
	 *  in�Ŏw�肳�����̓f�[�^����query_name�Ŏw�肳�ꂽ�t�@�C������
	 *  ���݂��邩�ǂ������`�F�b�N���܂��B
	 *
	 * [Return Value]
	 *
	 *   > 0  query_name�͑��݂��A���ɓ����Ƀt�@�C���ł��Ȃ��B
	 *  == 0  query_name�͑��݂��邪�Ain���ɂ��鏑�ɓ����Ƀt�@�C���ł���B
	 *        next_target�����Ƀ`�F�b�N���ׂ����ɓ��̃p�X�������B
	 *        in�͏��ɓ����Ƀt�@�C�������ۂɎ擾���A���̃C���[�W��ێ�����悤�ɕύX�����B
	 *   < 0  query_name�͑��݂��Ȃ��p�X�ł���B�܂���in�Ŏw�肳�ꂽ�X�g���[���͕s���ł���B
	 *
	 * Example: (���ۂɂ͎���CheckExistingFile���s�Ȃ����ɂ͑���Plug-in�������K�v������)
	 *  name= "C:/a.lzh/b.lzh/c.jpg";
	 *    .......  <= name= "C:/a.lzh", query= "b.lzh/c.jpg"
	 *  in= new SpiInputStream;
	 *  in->set_stream_data(name, 0);
	 *  while (1) {
	 *    status= CheckExistingFile(in, query, &next);
	 *    if (status == 0) {
	 *      query= next;	* Let's next try *
	 *    } if (status < 0) {
	 *      break;	* error *
	 *    } else if (status > 0) {
	 *      break;	* found it *
	 *    }
	 *  }
	 *  if (status > 0) {
	 *    * found it  -> GetPicture()? *
	 *  } else {
	 *    * error *
	 *    delete in;
	 *  }
	 */
	/* (Mac Binary �̂悤��)�C���[�W�̐擪offset�̈�����SpiInputStream�Ŏw�肳���
	 * ���R�͂Ȃ�ƂȂ��E�E�E�B���s������?
	 */
	virtual int CheckExistingFile(SpiInputStream *in, LPCSTR query_name, LPSTR *next_target);

protected:
	SpiFileLoadType EvalLoadType(DWORD pre, DWORD post);
	int SmartLoad();
	int SmartUnload();

public:
	/* �����͗��p�ґ�����`����(���܂�g�����Ƃ���܂���) */
	typedef DWORD (WINAPI *SpiDllFileLoadFileCallback_t)(SpiDllFile *plugin);
	typedef SpiFileLoadType (WINAPI *SpiDllFileEvalCallback_t)(SpiDllFile *plugin, DWORD pre, DWORD post);

	SpiDllFileLoadFileCallback_t  m_pre_load_callback_func;
	SpiDllFileLoadFileCallback_t m_post_load_callback_func;
	SpiDllFileEvalCallback_t m_eval_load_type_callback_func;

#if	0
	/* SpiDllFile�̃��x���ł͂���͔p�~���܂�
	 */
	/* 
	 * �����ňÖٓI�ɌĂ΂��R�[���o�b�N�֐��B
	 * �f�t�H���g�ł͉������Ȃ��֐����Ăяo����܂��B����͈ȉ���Plug-in I/F�œK�p����܂��B
	 * �܂����̊֐��������I�ɌĂяo�����ꍇ�ɂ��K�p����邱�Ƃɒ��ӂ��Ă��������B
	 * 00IN: GetPicture, GetPictureInfo
	 * 00AM: GetArchiveInfo, GetFile
	 */
	FARPROC m_default_callback;
	unsigned long m_callback_data32;
#endif

protected:
	/* SPI�t�@�C���ւ̊֐��|�C���^ */
	FARPROC m_GetPluginInfo;
	FARPROC m_IsSupported;
	FARPROC m_GetPictureInfo;
	FARPROC m_GetPicture;
	FARPROC m_GetPreview;
	FARPROC m_GetArchiveInfo;
	FARPROC m_GetFileInfo;
	FARPROC m_GetFile;
	FARPROC m_ConfigurationDlg;


	HINSTANCE m_hModule;				/* ���W���[���n���h��/Static�̎��ɗ��p */
	SpiFileLoadType m_load_type;		/* ���[�h�^�C�v */
	char m_about_name[128];					/* Plug-in�̖��O(GetPluginInfo(1, xx, xx)�̌���) */
	char m_file_name[MAX_PATH];			/* Plug-in�t�@�C���� */
	char *m_plugin_ext_list;			/* Plug-in �g���q���X�g */
};



/* SpiFilesManage�ɂ�����FindNextPlugin�����s����ۂ̓����f�[�^��ێ����܂��B
 *   ���`��A���܂����E�E�E�Btypedef int SpiFindData;�ŏ[�������(;_;)�B
 *   �܁A�������ɗ��������邳(<-����ȓ��������菑���������ق������������;_;)�B
 * ����I/F�̒�`�͌��\�Y�܂����ł��˂��B�f�U�C���p�^�[����ǂ�ł�����
 * �׋����ׂ���?�B
 */
class SpiFindData {
public:
	SpiFindData() {};
	virtual ~SpiFindData() {};

#if	0
	virtual void First(vector<SpiFile *> *plugins) { m_pos= plugins->m_begin();};
	virtual void Next() { ++m_pos;};
	virtual SpiFile *GetPos() { return m_pos->second();};
	virtual void Free() {};
#endif

public:
	std::vector<SpiFile * >::iterator m_pos;
};



/* SpiFile�N���X�̏W�����Ǘ����܂��B */

class SpiFilesManage {
 public:
	SpiFilesManage();
	virtual ~SpiFilesManage();

 public:
	/* Plugin�̂���f�B���N�g�����w�肷�� */
	virtual int SetPluginDirectory(LPCSTR directory);

	/* Plugin�t�@�C�������[�h���� */
	virtual void LoadPluginFile();

	/* Plugin�t�@�C�����A�����[�h���� */
	virtual void UnloadPluginFile();


	/* fileName��������Plug-in�t�@�C����T��(convenience function)
	 * ������fileName�������镡����Plug-in�����݂��Ă����̓��̍ŏ���SpiFile��
	 * �ԋp���邱�Ƃɒ��ӂ���B���ׂĂ�Plug-in�𗘗p���Ȃ���΂����Ȃ��ꍇ�ɂ�
	 * �񋓗p��I/F(FindXXX�n)���g�p���邱�ƁB
	 */
	/* SpiOpenFile�̎������I�������SearchPlugin�͉��z�����ꂽ�n���h���𗘗p����
	 * SpiFile��ԋp����悤�ɂȂ�B
	 */
	virtual SpiFile *SearchPlugin(LPCSTR fileName, int offset= 0);

#if	0
	/* New I/F
	 *  �܂������͂��̂����ˁE�E�E�B
	 *  �w�肳�ꂽ���ɓ�(��������Ȃ�)�t�@�C������舵��
	 *  �����ł̓t�@�C���ւ̉��z�����ꂽ�n���h�����擾����
	 */
	virtual SpiOpenHandle SpiOpenFile(LPCSTR fileName/* ??? */) { return FALSE; };
#endif


	/* �v���O�C���̎擾������
	 *   - �ȑO�̃o�[�W�����Ɗ֐����Ɍ݊���������܂��� -
	 * ex:
	 *   find= FindCreatePlugin();
     *   if (find) {
	 *     for (spi= FindNextPlugin(find); spi != NULL; spi= FindNextPlugin(find)) {
	 *       spi->IsSupported(); .....
	 *     }
	 *     FindClosePlugin(find)
	 *   }
	 */
	virtual SpiFindData *CreateFindData(void);
	virtual SpiFile *FindFirstPlugin(SpiFindData *find);	/* FindNextPlugin�Ƃ܂����������A�݊��֐� */
	virtual SpiFile *FindNextPlugin(SpiFindData *find);
	virtual void CloseFindData(SpiFindData *find);

	/* �v���O�C���t�@�C�����ォ��ǉ�����
	 *   ����͓���Plug-in�Ȃǂ𗘗p���邽�߂Ɏg�p�����
	 *   �ǉ����ꂽ�I�u�W�F�N�g��UnloadPluginFIle����delete�����̂ɒ��ӂ���
	 */
	virtual int AddPlugin(SpiFile *spi);

	/* �w��̃t�@�C�����I�[�v������(�K�v�ł����AM Plug-in���g����) */
	virtual int OpenFile(SpiInputStream *in, LPCSTR filename);

	/* OpenFile�ŃI�[�v�������C���[�W����Bitmap�𐶐�����(�K�v�ł����IN Plug-in���g����) */
	virtual SpiFile *GetBitmapImage(SpiInputStream *in, SpiOutputStream *out, int mode,
									FARPROC lpPrgressCallback, long lData);

public:
	/* �����͗��p�ґ�����`���� */
	typedef int (WINAPI *SpiFilesManageQueryCallback_t)(SpiFilesManage *own, LPCSTR dir, LPCSTR name);

	SpiFilesManageQueryCallback_t m_query_plugin_load_callback_func;

 protected:
	/* fileName��Plug-in�����[�h���邩?
	 *  ���[�h����ꍇ�ɂ͗L����(new�Ŋ��蓖�Ă�)SpiFile�ւ̃|�C���^��ԋp����
	 *  ���[�h���Ȃ��ꍇ�ɂ�NULL��ԋp���邱��
	 * ���̊֐���LoadPluginFile()���s���̃R�[���o�b�N�֐��̂悤�ɓ��삷��
	 * [�P���ɔh������LoadPluginFile��u��������Ηǂ��悤�ȋC�����邯�ǂ�]
	 *
	 * �Ⴆ��"ifjpegv6.spi"���w����z���ă��[�h���Ȃ��Ƃ������悤�Ȃ��Ƃ���������(;P)�B
	 *
	 * �h�������Ȃ��ꍇ�ɂ�m_query_plugin_load_callback_func�����p�ł���
	 */
	virtual SpiFile *QueryPluginLoad(LPCSTR fileName);


 protected:
	char m_plugin_dir[MAX_PATH+260];	/* Plug-in�����[�h����f�B���N�g�� */

#if	0
/* ����͂�߂܂��� */
	static int m_reference_count;		/* Plugin�̌�������񂵂����Ȃ����̂��� */
#endif

	std::vector<SpiFile *> m_plugins;	/* SpiFile��ێ����� */

};

#endif	/* SUSIE_PLUGIN_SPI_IF_H */
