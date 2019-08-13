/*
 *	Susi32 Plugin Interface
 *			Copyright kana(mailto:inokuchi@mvg.biglobe.ne.jp)
 *
 */
/*
 *	History:
 *		2000/02/20(Sun) ～ 2000/09/02(Sat): えっと、(未)は今後の作業予定項目です。
 *			Plug-inの拡張子パターンをLoadPlugin時に保持するよう変更
 *			(まだこの拡張子をSpiFilesManageでは利用していないのであまり意味がない)
 *			書庫内ファイルのオープン時にファイル名コピー用バッファが溢れていた
 *			SpiInputStreamでコピーコンストラクタを使うとコピー元がdeleteされたタイミングで
 *			メモリ解放してしまうのを修正(<-とりあえずmapでリファレンスカウンタもどきで管理)
 *			ConfigurationDlgを忘れていたので追加
 *			レジストリのロード順に従うように変更(未)
 *			レジストリのPlug-inの利用パターン(A, B, C)に従うように変更(未)
 *		2000/01/16(Sun) ～:
 *			レジストリの設定の解釈が変わったので取り合えず修正。ただし使う・使わない
 *			しか反映していない
 *		1999/12/30(Thu) ～ 2000/1/4(Tue):
 *			spi_derived.{h,cpp}とマージ
 *			レジストリの値文字列を取得時のbuffer overflowに対して予防
 *		1999/11/30(Tue) ～:
 *			一部のPlug-inのGetArchiveInfo/GetPictureInfoの不具合をSpiDllFileで吸収
 *			SpiDllFile::SmartLoadがAM Plug-inを考慮していないのを修正
 *			SpiDllFileでのAM Plug-inの呼び出しがすべてエラーとなっていたのを修正
 *			ライブラリバージョンをマクロで定義するように追加
 *			def_query_load_SpiFilesManage_callbackのレジストリキーが間違っていたのを修正
 *		1999/05/09(Sun) ～ :
 *			コールバック関数の実装(派生クラスなしでも動作変更可能とするため)
 *		1999/03/27(Sat) ～ : 
 *			QueryPluginLoad/EvalLoadTypeの仕様変更。
 *			backward compatibility関数FindFirstPluginをサポート
 *		1999年2月2日(火) ～ :
 *			出直し。流行の動的ロードへの対応
 *		1997年10月25日(土):
 *			  SpiFile::LoadPluginで00IN Plug-inの時にGetPreviewがなくても
 *			成功するように変更。
 *			根本的には取得できなかった関数はGetPluginInfo/IsSupportedを除
 *			成功を返して  NOT_IMPLEMENTEDを返すようにしたほうがいいのかも
 *			しれないけど。
 *		1997年3月23日(日):
 *			  SpiFileManage::AddPluginを追加し、Built-in Plugin
 *			を追加できるように変更。これに追加されたオブジェクト
 *			はSpiFileManager:UnLoadPluginFileで削除されるのでnewすること
 *		1996年10月05日(土):
 *			  SpiFileManage::SearchPluginでチェックするoffsetを
 *			指定できるようにした。これでoffset=128でコールする
 *			ことでMacバイナリも読み込めるはず
 *			  SpiFileManage::SearchPluginで検索失敗しても何らかの
 *			SpiFileを返していたのを修正
 *		1996年9月2日(月):
 *			  SpiFileManageのSPIファイル検索ルーチンのバグ
 *			malloc/freeをnew/deleteに変更
 *			  SearchPluginで返却するSpiFileポインタがchainの
 *			前のポインタであるのを修正
 *			  ファイルバージョン文字列を埋め込み
 *		1996年8月21日(水):
 *			SpiFile初期バージョン作成終了/SpiFileManage作成開始
 *			[Spi関数呼出し時の関数ポインタに__stdcallを追加]
 *			[これがないとスタックフレームがおかしくなるみたい]
 *		1996年8月18日(日):
 *			初期バージョン作成開始
 */

/* やっぱ本物を使わないとねぇ;_;、えぇ */
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

/* 定数 */

/* */
#define	strtok(__x, __y)		(char *)_mbstok((unsigned char *)__x, (unsigned char *)__y)
#define	strrchr(__x, __y)		(char *)_mbsrchr((unsigned char *)__x, __y)

/* Pluginの拡張子 */
/* Win32とWin16での違いは他にもあるからWin16(PLG)版でも使えるなんて
   期待してはいけない。WinNTしか使ってないのでよくわからんし。 */
#ifdef	_WIN32
/* 32bit Version */
#define	SUSIE_PLUGIN_EXT	"SPI"
#else
/* 16bit Version */
#define	SUSIE_PLUGIN_EXT	"PLG"
#endif


/* 型宣言 */
#if	0
typedef struct plugin_info_chain {
	struct plugin_info_chain *m_next;
	SpiFile *m_spiFile;
} plugin_info_chain;
#endif


/* 関数宣言 */
extern "C" {
	
int knRegTinyGet(HKEY tree, const char *key_name, 
					const char *val_name,LPVOID buffer);
int knRegTinyGetEx(HKEY tree, const char *key_name, 
					const char *val_name,LPVOID buffer, DWORD length);
int knRegTinySet(HKEY tree, const char *key_name,
					const char *val_name, DWORD type,
					LPVOID buffer, DWORD length);
}

/* 変数宣言 */

/* コールバック関数のデフォルトの実装
  `typedef' of functions  are:
	typedef DWORD (WINAPI *SpiDllFileLoadFileCallback_t)(SpiDllFile *plugin);
	typedef SpiFileLoadType (WINAPI *SpiDllFileEvalCallback_t)(SpiDllFile *plugin, DWORD pre, DWORD post);

各コールバック関数を格納しているメンバ変数はpublicになっているので直接変更することが可能である。
ただし変更するのはコンストラクタを呼び出した直後が望ましい。または派生クラスを作って書き換えるように
するのを推奨する。

・SpiDllFileのロードタイプ:
	デフォルトの実装では仮想アドレス空間の減少をチェックする。これが減少しているということは
	DLLをいつかロード出来なくなることを意味している。
	またこの実装ではTLSの限界をチェックすることはしていない。ただTLSの限界と仮想アドレス空間の
	減少は独立した現象であるので両立出来ないことがあることを理解するべきである。
	まぁなんともならないんですけどねぇ・・・・。

	pre_load callbackはプラグインを実際にLoadLibraryする前に呼び出される
	post_load_callbackはプラグインを実際にLoadLibraryした後に呼び出される
	eval_load_type_callbackはそれぞれの関数が返却した値を引数に呼び出す。これによりロード種別を
	決定することが期待されている。詳しくはEvalLoadTypeの説明を参照のこと。


・SpiFilesManageの読み込み問い合わせ
	デフォルトの実装ではSusieのレジストリ設定に従うようになっている
	TRUEを返却するとそのファイルに対応するSpiDllFileを生成する。FALSEを返却するとロードしない。

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
		/* これは動的ロードするのはまずい・・・ */
		type= SpiDllFile::SpiFile_LoadStatic;
	} else {
		/* 本当はもう一つチェックしないといけないことがあるけどね(ex. TLS) */
		type= SpiDllFile::SpiFile_LoadDynamic;
	}
	return type;
}


/*
	SpiFilesManageがLoadPluginを実行した時にdir\nameにあるファイルを使用するか
	どうかを決定する関数

	query load callback
*/
static int WINAPI 
def_query_load_SpiFilesManage_callback(SpiFilesManage *own, LPCSTR dir, LPCSTR name)
{
	/* Susieでのレジストリ設定に従う */
	DWORD on_off;

	on_off= 1;	/* デフォルト値は'ロードする'である・・・ */
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
	/* データを解放する */
	if (m_bitmap_info) LocalUnlock(m_hBitmapInfo);
	if (m_hBitmapInfo) LocalFree(m_hBitmapInfo);
	if (m_bitmap_memory) LocalUnlock(m_hMemory);
	if (m_hMemory) LocalFree(m_hMemory);
}

void SpiOutputStream::
set_stream_data(HANDLE hBitmapInfo, HANDLE hMemory)
{
	/* 以前のデータを解放 */
	abandon_data(ABANDON_AND_FREE_DATA);

	/* 新しいデータを設定 */
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
		/* メモリも解放する -> unlockしてfree */
		unlock_data();
		if (m_hBitmapInfo != NULL) LocalFree(m_hBitmapInfo);
		if (m_hMemory != NULL) LocalFree(m_hMemory);
	}

	/* データを放棄 */
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
 SpiInputStreamで管理しているメモりオブジェクトをリファレンスカウンタにより
 管理するクラス
// HLOCALハンドルとSpiInputStreamへのポインタの組みで管理する
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


/* コピーした後にコピー元のオブジェクトがdeleteされると結果的に
 * m_image_memory, m_hImageが無効になるのでこれはまずい
 * Ver 0.06.4以降ではm_hImageを保持しているSpiInputStreamオブジェクトの数を
 * 保持しておき、最終的に保持しているオブジェクトが0になった段階で解放する
 * ように変更
 */
/* 代入演算子 */
void SpiInputStream::operator=(const SpiInputStream &x)
{
	/* 同じオブジェクトであれば何もしない */
	if (this == &x) return ;
	
	m_data_type= x.m_data_type;

	/* ファイル名のコピー */
	if (m_file_name) {
		delete [] m_file_name;
	}
	if (x.m_file_name) {
		m_file_name= new char [strlen(x.m_file_name) + 1];
		strcpy(m_file_name, x.m_file_name);
	} else {
		m_file_name= NULL;
	}

	/* データのコピー */
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
			/* ここに来ることがあったらバグってますね... */
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
 *  データを放棄する
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
			/* 保持カウンタが0になったらHLOCALとカウンタ情報の2つを解放 */
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
		/* メモリハンドルをロックする必要がある */
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
 * Plug-in I/Fで使用するflagの値
 * 入力フラグの値としてはこれを使うことができる
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

/* Pluginファイル情報(SpiDllFile) */
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

	/* デフォルトのコールバック */
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

	/* デフォルトのコールバック */
	m_pre_load_callback_func= def_pre_SpiDllFile_callback;
	m_post_load_callback_func= def_post_SpiDllFile_callback;
	m_eval_load_type_callback_func= def_eval_SpiDllFile_callback;
//	m_default_callback= (FARPROC)null_callback;
//	m_callback_data32= 0;
}

SpiDllFile::
~SpiDllFile()
{
	/* Dllのアンロード */
	if (m_hModule != NULL) {
		FreeLibrary(m_hModule);
		m_hModule= NULL;
	}
	if (m_plugin_ext_list) {
		delete [] m_plugin_ext_list;
	}
}


/* 指定されたPlug-inを読み込んでみる */
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
		/* 既にロードしている・・・(違うファイルかもしれないけど) */
		return TRUE;
	}

	if (dirname != NULL && dirname[0] != 0)
		sprintf(plugin_file, "%s\\%s", dirname, filename);
	else
		strcpy(plugin_file, filename);

	pre= (m_pre_load_callback_func) ? (*m_pre_load_callback_func)(this) : 0;

	h= LoadLibrary(plugin_file);
	if (h == NULL) {
		/* ロードできない */
		return FALSE;
	}
	post= (m_post_load_callback_func) ? (*m_post_load_callback_func)(this) : 0;

	/* ファイル名をコピー */
	strcpy(m_file_name, plugin_file);

	m_GetPluginInfo= GetProcAddress(h, "GetPluginInfo");
	if (m_GetPluginInfo == NULL) {
		/* 取得失敗(Spiファイルではない?) */
		goto error_out;
	}

	/* 暫定的にm_typeを設定する */
//	type= m_type;
//	m_type= SpiFile_ImportFilter;
//	m_type= type;
	
	if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(0, version, sizeof(version)) == 0) {
		/* これはSusie Plug-inではない? */
		goto error_out;
	}
	m_about_name[0]= 0;
	if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(1, m_about_name, sizeof(m_about_name)) == 0) {
		/* これはSusie Plug-inではない? */
		goto error_out;
	}
//	GetPluginInfo(0, m_version, sizeof(m_version));

#define	GETPROC(x, y)			do { \
									x= GetProcAddress(h, y); \
									if (x == NULL) goto error_out; \
								} while (0)
#undef	GETPROC

    /* Plug-inの種別と関数アドレスを取得 */
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
	/* 取得成功 */

	/* 代表的な拡張子を取得します */
	for (count= i= 0; ; ++i) {
		/* 2n+2 */
		char buffer[1024];	/* これより長い物って出てくるかな? */
		char *s, *t;

		if ((*(int (WINAPI *)(int, LPSTR, int))m_GetPluginInfo)(2*i+2, buffer, sizeof(buffer)) == 0) {
			break;
		}

		/* 拡張子のパターンをコピー(*.JPGはJPGに変換しておく) */
		for (s= strtok(buffer, ";"); s != NULL; s= strtok(NULL, ";")) {
			/* ".xxx"のパターンをコピーするんだよ */
			t= strrchr(s, '.');
			if (t == NULL) {
				/* "."がないので"JPG"とかになっているパターン(ってあるのかなぁ、手入力?) */
				item_ext += ".";
			}
			item_ext += t;
			item_ext += ";";
		}
	}
	if (m_plugin_ext_list) delete [] m_plugin_ext_list;
	m_plugin_ext_list= new char [item_ext.length() + 1];
	strcpy(m_plugin_ext_list, item_ext.c_str());

	/* ロード種別を評価する */
	m_load_type= EvalLoadType(pre, post);
	if (m_load_type == SpiFile_LoadDynamic) {
		FreeLibrary(h);
		m_hModule= NULL;
	} else {
		/* モジュールハンドルを保存しておく */
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



/* Spiファイルの関数を呼出すルーチン */

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

/* Plug-in固有の設定ダイアログを呼び出す
 * 拡張仕様として
 *   fno= -1の時にそもそもConfigurationDlgを実装しているかどうかを返却する
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




/* Note: SpiFileManage::QueryPluginLoad()との関係
 *  QueryLoadPluginとは直接の関係はない。ただしQueryLoadPluginの延長にて
 *  SpiDllFile::LoadFileが呼出びされるます。これはQueryLoadPluginに求めら
 *  れているのが指定のPlug-inを選択的にロードすることだからです。
 *
 */
/*
 * この関数は指定されたPlug-inを動的にロードするかどうかを評価することが目的である。
 * 動的ロードが必要でないPlug-inまたは動的ロードしては不都合が起きるPlug-inを評価す
 * るのがこの関数に求められることである。
 *
 * EvalLoadTypeはLoadFileした際に呼び出される。またLoadFileは一度しか呼ばれない。
 *
 * pre/postはそれぞれLoadLibraryする直前と直後の状態を保持しているような32bitの変数
 * であり、それぞれm_pre_load_callback_func/m_post_load_callback_funcが返却する値
 * である。m_eval_load_type_callback_funcはpre/postでの値を使ってロード種別を実際に
 * 評価することが求められる。m_eval_load_type_callback_funcが存在しない場合には
 * 動的に読み込むと判定される。
 * またpre/postはなんらかのポインタであってもよいが、これがの指す領域は動的に確保
 * される必要があり、それを解放するのはm_eval_load_type_callback_funcの仕事である。
 * またLoadLibraryに失敗したりLoad後にPlug-inでないと判断された場合にはpost==0とし
 * て呼び出される。
 */
SpiDllFile::SpiFileLoadType SpiDllFile::
EvalLoadType(DWORD pre, DWORD post)
{
#if	1
	/* SpiDllFileでの標準的な動作はこれである */
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
	/* こっちは派生クラスで実現しようね。 */
	SpiFileLoadType return_type= SpiFile_LoadStatic;
	/* ここでPlug-in自身のロードをDynamicにするかStaticにするかの評価をする
	 *   と思ったけど・・・、これじゃぁ評価できないよねぇ(;_;)
	 * やはりここで一度LoadLibrary/FreeLibraryするしか・・・<-かなり間抜けっぽい 。
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

		/* Plug-inがTLSを利用しているかどうかを評価する
		 *  使っていないようだったらLoadStaticの可能性がある
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
			/* TLSが減少している(こりはDynamic決定) */
			return_type= SpiFile_LoadDynamic;
		} else {

			/* 仮想アドレス空間を食いつぶさないかのチェック
			 *   あぁifjpegv6.spiもなぜか食べちゃうのよねぇ(;_;)。
			 *   わりに症状が発現しない時もあってこのチェックが意味があるかどうかはかなり謎だ
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
		/* う～む、結局はロードする必要があるんだよなぁ */
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
			status= LoadFile(NULL, m_file_name);		/* う～ん、コピーが頻発するなぁ */
#endif
		} else if (m_load_type == SpiFile_LoadStatic) {
			/* What Happen? */
			status= LoadFile(NULL, m_file_name);		/* う～ん、コピーが頻発するなぁ */
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
/* 単に引数がHANDLEからにHLOCALになっただけ */
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
/* 単に引数がHANDLEからにHLOCALになっただけ */
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
			/* 格納ファイル数を取得 */
			while (info[count].method[0] != '\0')
				++count;

			/* 領域をコピー、この領域は呼び出し元で解放すること
			 *   (newよりmallocの方が良かったかな?)
			 *   ex) delete [] lpInf;
			 *  +1するのは念のためだがそれが有効でないPlug-inも存在するので
			 *  分割してメモリ領域を設定する。
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
 *	flagはファイル名のcaseをどうするかしか指定できません。
 *    flag==1 -> 区別しない
 *    flag==0 -> 区別する
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
 * 指定の入力(in)に指定のファイル(query_name)が存在するかどうかをチェックします。
 * 指定のファイルが書庫内ファイルであった場合にはnext_targetに次のパスが返却されます。
 *
 * 注)全てのPlug-inに対して処理を適用するのはSpiFilesManage系クラスの仕事
 *    なぜなら書庫内の書庫ファイルは形式が異なる可能性があるからです。
 *      ex) c:/a.lzh/b.zip/c.tar/d.cpp とか・・・。まぁこんな実例は少ないでしょうが。
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

	/* リストから目的のファイルを探します。
	 * GetFileInfoを使わないのは書庫内書庫のファイルの可能性があるからです。
	 */
	for (loop_end= FALSE, i= 0; info[i].method[0] != 0 && loop_end == FALSE; ++i) {
		char temp[1024];	/* sizeof(temp) >= sizeof(path) + sizeof(filename) */
		int len;

		/* ファイル名の大文字小文字については区別しません */
		strcpy(temp, info[i].path);
		strcat(temp, info[i].filename);
		len= strlen(temp);
		if (strnicmp(temp, query_name, len) == 0) {
			HLOCAL mem;

			/* 一致する */
			if (temp[len] == '\\' || temp[len] == '/' ) {	/* 書庫内パス */
				*next_target= (char *)&query_name[len];	/* 次の問い合わせ名 */

				/* 入力ストリームを再作成 */
				if (GetFile(in, &info[i], &mem, (FARPROC)null_callback, 0) == SUSIE_IF_ERROR_SUCCESS) {
					/* 取得OK */

					temp[len]= 0;
					in->set_stream_data(temp, mem, info[i].filesize);

					status= 0;
				} else {
					/* エラー・・・次のPlug-inへ */
					status= -1;
				}
				loop_end= TRUE;

			} else if (temp[len] == '\0') {					/* 一致 */

				/* 入力ストリームを再作成 */
				if (GetFile(in, &info[i], &mem, (FARPROC)null_callback, 0) == SUSIE_IF_ERROR_SUCCESS) {
					/* 取得OK */
					*next_target= NULL;				/* 次はない */
					temp[len]= 0;
					in->set_stream_data(temp, mem, info[i].filesize);
					status= 1;
				} else {
					status= -1;
				}

				loop_end= TRUE;

			} else {										/* 部分一致 -> これはスキップ */
				/* skip */
			}
		} /* if (strnicmp() == 0) */
	} /* for (loop_end= FALSE, i= 0; ...) */

	/* 領域を解放 */
	if (info) delete [] info;

	return status;
}



/* =========================================================== SpiFilesManage */
/* Pluginファイルを管理する(SpiFilesManage) */
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
 *	Pluginのあるディレクトリを設定する
 *		Pluginがカレントか、Susie Pluginディレクトリにない場合は
 *		設定する必要がある。
 *  独自にPlug-inのあるディレクトリを管理する場合にはパラメータとして
 *  渡すこと。またPlug-inディレクトリ以外に追加したい場合にはAddPlugin
 *  で個々に登録すること。
 */
int SpiFilesManage::
SetPluginDirectory(LPCSTR directory)
{

	if (directory == NULL) {
		/* NULLの場合にはSusieのPluginディレクトリを参照する 
		 *   ただし、これはSusie 0.32(?)以上が必要
		 */
		m_plugin_dir[0]= 0;
		knRegTinyGetEx(HKEY_CURRENT_USER, 
					 "Software\\Takechin\\Susie\\Plug-in",
					 "Path",
					 m_plugin_dir, sizeof(m_plugin_dir));
		if (m_plugin_dir[0] == 0) {
			/* 取得失敗 */
			;
		}
	} else {
		strcpy(m_plugin_dir, directory);
	}
	return TRUE;
}


/* 
 *	SetPluginDirectoryで指定されたディレクトリからPlugin
 *  ファイルをロードします
 */
void SpiFilesManage::
LoadPluginFile()
{
	char plugin_file[sizeof(m_plugin_dir) + 10];
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	BOOL status;

	/* 検索SPIファイル名 -> [Dir]\*.SPI */
	if (m_plugin_dir[0] != 0) {
		sprintf(plugin_file, "%s\\*.%s", m_plugin_dir, SUSIE_PLUGIN_EXT);
	} else {
		sprintf(plugin_file, "*.%s", SUSIE_PLUGIN_EXT);
	}

	/* SPIファイル一覧を取得 */
	hFind= FindFirstFile(plugin_file, &find_data);
	if (hFind == INVALID_HANDLE_VALUE) {
		/* 一致するファイルなし */
		return ;
	}

	/* 既にチェーンがあればそれは破棄する */
	if (m_plugins.size() > 0)
		UnloadPluginFile();

	status= TRUE;
	for (; status == TRUE; status= FindNextFile(hFind, &find_data)) {
		SpiFile *spi;

#define	IGNORE_ATTR	(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)
		if ((find_data.dwFileAttributes & IGNORE_ATTR) != 0) {
			/* ディレクトリだった。 -> 無視する */
			continue;
		}

		/* Plug-inファイルを取得した */
		spi= QueryPluginLoad(find_data.cFileName);
		if (spi) {
			/* これは管理情報へ追加する */
			m_plugins.push_back(spi);
		}

	}
	FindClose(hFind);

	/* これでチェーンが得られました。 */

	return;
}


/* LoadPluginFile()でロードしたPluginファイルをアンロードします */
void SpiFilesManage::
UnloadPluginFile()
{
	/* QueryPluginLoadやAddPluginで取得したSpiFileをdeleteする
	 *  本当はAddPluginで追加したものについてはdeleteしない方がいいのかも
	 */
#if 1
	/* Create/FindNext/Closeを呼び出したほうがよいのだろうか? */
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
  指定のディレクトリ中にあるPlug-inのうちどれを利用するかを定義します。
  デフォルトの実装ではすべてのPlug-inを「ロードする」とします
  変更するには派生クラスを作ってQueryPluginLoadを書き換えるかコールバッ
  ク関数を定義します。
  例えば「使用しない」という設定になっているPlug-inかどうかチェックした
  りします。
 */
SpiFile *SpiFilesManage::
QueryPluginLoad(LPCSTR name)
{
	int status= TRUE;		/* TRUE: load / FALSE: do not load */
	SpiDllFile *spi= NULL;

	/* ここはSpiFileベースで作成したほうがいいのかなぁ?。 */

	/* コールバック関数を呼び出して問い合わせる */
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


/* Plug-inを追加する */
int SpiFilesManage::
AddPlugin(SpiFile *spi)
{
	m_plugins.push_back(spi);
	return TRUE;
}


/* fileNameで展開できるPluginを返します。存在しなければNULLを返却します
   SpiFindDataを返却する方がいいのかも・・・。
 */
SpiFile *SpiFilesManage::
SearchPlugin(LPCSTR fileName, int offset)
{
	HANDLE hFile;
	SpiFile *spi, *status;
	char head_buff[1024 * 2 + 128];
	DWORD readed= 0;
	std::vector<SpiFile *>::iterator it;

	/* ファイルをオープン */
	hFile= CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ,
					  NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	/* 指定の位置までファイルポインタを移動(遠くまでseekする必要はないでしょう) */
	if (SetFilePointer(hFile, offset, NULL, FILE_BEGIN) < offset) {
		/* offsetの位置まで移動できない -> ファイルが短かすぎ */
		CloseHandle(hFile);
		return NULL;
	}

	memset(head_buff, 0, sizeof(head_buff));
	if (ReadFile(hFile, head_buff, sizeof(head_buff), &readed, NULL) == FALSE) {
		/* ファイル読み込みに失敗 */
		CloseHandle(hFile);
		return NULL;
	}

	spi= NULL;
	for (it= m_plugins.begin(); it != m_plugins.end(); ++it) {
		int support;

		status= *it;		/* it->second()? */

		/* このファイルをサポートしているか? */
		support= status->IsSupported((LPSTR)fileName, (DWORD)&head_buff[0]);
		if (support != 0 /* && status->GetSpiFileType() == SpiFile::SpiFile_ImportFilter */) {
			spi= status;
//			m_current_plugin= spi; /* カレントプラグインを設定 */
			break;
		}
	}

	/* ファイルクローズ */
	CloseHandle(hFile);

	return spi;
}


/* 
 *	チェーンにつないでいるPluginファイルを検索します 
 *	(外部インタフェース)
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
	ファイルをオープンする。
	これは書庫内のファイルなどへのアクセスをサポートする。

Parameter:
	in:	入力ファイルまたはメモリイメージ。現状はファイルのみ対応
	filename: 入力ファイル中のオープンしたいファイル。inとfilenameが同一の場合もありうる
	offset: inのデータの先頭から読み飛ばすバイト数(未サポート)

Return Value:
	=>0: 正常に終了。inはオープンしたことによって取得したファイル名または
	     ファイルイメージに更新される。
	< 0: filenameが存在しないなどの理由で失敗。
         書庫内ファイルであればそれを扱えるPlug-inがないことも考えられる。

Remark:
	コールバック関数は呼び出されません。ですので一旦呼び出すとキャンセル
	することはできません。

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

	/* 入力種別をチェック */
	switch (in->query_input_type()) {
	case SpiInputMemoryStream:
		return 1;
	case SpiInputFileStream:
		break;
	default:
		return -1;
	}

	/* このあたりがin->query_input_type == SpiInputFileStreamでないと対応していない
	 * ってことです。
	 */
	file= in->query_buffer();

	/* (1)Preprocess opening file */
	/* 指定されたファイルが通常のファイル名かどうかをチェック 
	 *   ここでオープンに成功すれば処理はこれで終了。
	 */
	hFile= CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		/* inの修正は必要ない */
		return 1;
	}

	/* (2)Preprocess splitting filename */
	/* copy filename */
	p= new char [strlen(file) + 1+1+10];
	edit_file= new char [strlen(file)+1+1+10];	/* +10は予備 */
	strcpy(p, file);
	file= p;

	/* 実ファイルを探す */
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

	/* ぜ～ぜん、オープンできなかった
	 *     -> 失敗で終わり
	 */
	if (open_flag == FALSE) {
		delete edit_file;
		delete p;
		return -1;
	}

	/* 残りのファイル名を作成する */
/* どっちの方法がいいかは難しいね。 */
#if	1
	strcpy(edit_file, e + strlen(e) + 1);
#else
	edit_file[0]= 0;
	while ((e= strtok(NULL, DIR_SEPARATOR))) {
		strcat(edit_file, e);
		strcat(edit_file, "\\");	/* う～ん、`/'の方がいいかな? */
	}
#endif

	e= edit_file;

	/* ここまでで
	 * inが示すイメージにあるfilenameは書庫内ファイルである。
	 *   と、仮定する。
	 */
	find= CreateFindData();
	if (find) {
		SpiFile *s;
		int check_status;
		LPSTR next;

		for (check_status= 0; check_status == 0; ) {
			/* 最初の1周めはoffset==0 */
			while ((s= FindNextPlugin(find)) != NULL) {
				SpiDllFile *spi= dynamic_cast <SpiDllFile *>(s);
				int status;

				/* キャストに失敗しているのなら他の派生クラスのオブジェクト */
				if (spi == NULL) continue;

				/* AM Plug-inでなければスキップ */
				if (spi->GetSpiFileType() != SpiFile::SpiFile_Archive)
					continue;

				/* IsSupported -> GetArchiveInfo -> GetFileInfo -> GetFile */
				status= spi->IsSupported(&new_in);
				if (status == 0) {
					/* 未サポート */
					continue;
				}

				/* サポート可能? */
				check_status= spi->CheckExistingFile(&new_in, e, &next);
				if (check_status == 0) {		/* Let's next try[down file name tree] */
					e= next;
				} else if (check_status < 0) {	/* error */
					/* IsSupportedは成功を返すが実際に展開しようとするとエラーになった */
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

		if (check_status > 0) {		/* 取得成功 */
			/* Streamをコピー */
			*in= new_in;

			/* new_inの内容を破棄 */
			new_in.abandon_data();

			status= 1;
		} else {					/* エラー */
			status= -1;
		}

	} else {
		/* エラー */
		status= -1;
	}

	/* 一時領域の削除 */
	delete edit_file;
	delete p;

	return status;
}


/*
	inからBitmapイメージを取得する
  Parameters:
	mode == 0: GetPictureを行なう
	mode != 0: GetPreviewを行なう

  Return Value:
	!= NULL: 展開に成功。展開に用いたSpiFile(SpiDllFile)を返却
	== NULL: 展開に失敗
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

			/* キャストに失敗しているのなら他の派生クラスのオブジェクト */
			if (spi == NULL) continue;

			/* IN Plug-inでなければスキップ(FindNextPluginにフィルター付けたほうがいいよな) */
			if (spi->GetSpiFileType() != SpiFile::SpiFile_ImportFilter)
				continue;

			status= spi->IsSupported(in);
			if (status == 0) {
				/* 未サポート */
				continue;
			}

			if (mode == 0) {
				status= spi->GetPicture(in, &hBitmap, &hMem, callback, lData);
			} else {
				status= spi->GetPreview(in, &hBitmap, &hMem, callback, lData);
				if (status == SUSIE_IF_ERROR_NOT_IMPLEMENTED) {
					/* GetPreviewを実装していないのならGetPictureを代わりに呼ぶ */
					status= spi->GetPicture(in, &hBitmap, &hMem, callback, lData);
				}
			}

			if (status == 0) {
				/* 取得成功 */
				out->set_stream_data(hBitmap, hMem);
				break;
			} else if (status == SUSIE_IF_ERROR_CANCEL_EXPAND) {
				/* キャンセルされた */
				s= NULL;
				break;
			} else {
				/* 他のも試してみる -> next plug-in */
			}

		} /* while ((s= FindNextPlugin(find)) != NULL) */
		CloseFindData(find);

	}

	return s;
}

