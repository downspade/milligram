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
 * 古いVisual C++に付属のSTLはvectorとかバグっているので修正が必要です。
 * いっそのことvectorをやめてCPtrArrayとか使ったほうが修正が簡単です。
 * CPtrArrayでもビルド可能な版が一時的に存在していましたが保守されていない
 * ために破棄されました。簡単な変更なので必要な方はご自分でやってください。
 */

#define	SPIIF_WRAPPER_LIBRARY_VERSION	6


/*
	SpiDllFileへの入出力I/F
*/
/* 
 * Plug-inの直接的なI/FであるSpiDllFileのI/Fにいくつか追加したもの
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

	/* コピーコンストラクタも必要かな? */
	/* name(const name &x); */

	/* 代入演算子
	 * abandon_dataはやらなくてもOKになったと思いますが...。
	 * リファレンスカウンタで管理するように変更しました。
	 */
	void operator =(const SpiInputStream &x);

private:
	void close_stream_data();

private:
	LPSTR	m_file_name;	/* ファイル名へのポインタ */
	long	m_offset;		/* 開始オフセット */

	HANDLE	m_hFile;		/* not used */

	HLOCAL	m_hImage;
	void	*m_image_memory;/* m_hImageをロックした後のメモリ領域 */
	size_t	m_size;

	SpiInputDataType_t m_data_type;
};


class SpiOutputStream {
 public:
	SpiOutputStream();
	virtual ~SpiOutputStream();

 public:
	typedef enum {
		ABANDON_DATA= 0,		/* データを放棄するだけ */
		ABANDON_AND_FREE_DATA,	/* データを放棄する際にデータを解放する */
	} SpiOutputStream_abandon_flag_t;

	/* interface */
	/* この2つのバリエーションは全く同じもの */
	void set_stream_data(HANDLE hBitmapInfo, HANDLE hMemory);
//	void set_stream_data(HLOCAL hBitmapInfo, HLOCAL hMemory);

	/* 内部で保持しているデータを放棄する */
	void abandon_data(SpiOutputStream_abandon_flag_t flag= SpiOutputStream::ABANDON_DATA);

	/* ロックしたメモリをunlockする(ただし解放はしません) */
	void unlock_data();

	/* query interface, query_xxxxx function get xxxxx value in the object. */
	BITMAPINFO *query_bitmap_info();
	LPVOID query_bitmap_memory();

	/* 代入演算子 */
/* 	void operator =(SpiOutputStream &x); */

 private:
	HLOCAL m_hBitmapInfo;
	HLOCAL m_hMemory;

	BITMAPINFO *m_bitmap_info;
	LPVOID m_bitmap_memory;
};






/* ================================================================= SpiFile */

/* Susie Plug-inへのインタフェースクラス
 *  各関数の説明はいらないですよね(^^)。
 *
 *   もし内蔵Plug-inを作成する場合にはSpiFileから派生させてSpiFileManageに登録する
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
	/* これらの関数ポインタなどを持つクラスは別途定義する */
 protected:
	/* SPIファイルへの関数ポインタ */
	FARPROC m_GetPluginInfo;
	FARPROC m_IsSupported;
	FARPROC m_GetPictureInfo;
	FARPROC m_GetPicture;
	FARPROC m_GetPreview;
	FARPROC m_GetArchiveInfo;
	FARPROC m_GetFileInfo;
	FARPROC m_GetFile;

	char m_version[10];	/* GetPluginInfoの情報番号0の情報を格納します */
	char m_name[64];		/* Pluginファイル名 */
	HANDLE m_hModule;
#endif
 protected:
	SpiFileType m_type;		/* Plug-inの種類 */
};




/* ============================================================== SpiDllFile */

/* 実際のDLLファイルに相当するクラス */
class SpiDllFile : public SpiFile {
public:
	typedef enum {
		SpiFile_LoadUnknown= 0,			/* ロード種別が不明 */
		SpiFile_LoadStatic,				/* 一度ロードしたらアンロードしない */
		SpiFile_LoadDynamic,			/* 使用するときだけロードする
										 * ただしリソース漏れなどの理由でDynamicに
										 * 利用するのに問題がある場合にはStaticになる。
										 * これは最初のLoadLibraryの時に評価される。
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

	/* 以下はSpiInputStream/SpiOutputStreamを使うI/F */
	/* common */
	virtual int IsSupported(SpiInputStream *in);

	/* ---------- 00IN -------------- */
	/* HANDLEベースは止めたほうがいいかもね */
	virtual int GetPicture(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm,
						   FARPROC lpPrgressCallback, long lData);
	virtual int GetPreview(SpiInputStream *in, HANDLE *phInfo, HANDLE *phm,
						   FARPROC lpPrgressCallback, long lData);

	/* ---------- 00AM -------------- */
	/*
	 * ～ GetArchiveInfo ～
	 * [Description]
	 *   in: 入力を意味する
	 *   lpInf: 指定の入力に含まれるファイルの一覧。SpiArcFileInfo[]へのポインタが返却される
	 *   lphInf: 指定の入力に含まれるファイルの一覧を格納しているメモリ領域のローカルハンドル
	 * [Return Value]
	 *   SpiDllFile::GetArchiveInfoと同様。ただし成功した場合にはlpInfで示される領域は
	 *   解放する必要がある。
	 *   SpiDllFile::GetArchiveInfoは成功時には必ずSUSIE_IF_ERROR_SUCCESSを返却することを
	 *   要求される。一部のPlug-inでの不具合に対処する責任がある。
	 *   (Require: SpiIf.cpp Ver 0.06 and above)
	 */
	virtual int GetArchiveInfo(SpiInputStream *in, SpiArcFileInfo **lpInf);
	virtual int GetArchiveInfo(SpiInputStream *in, HLOCAL *lphInf);

	virtual int GetFileInfo(SpiInputStream *in, LPCSTR filename,
							unsigned int flag, SpiArcFileInfo *info);

	/*
	 * in: GetArchiveInfoに対して渡した入力
	 * info: 取得しようとするfileinfo構造体へのポインタ
	 *       NULLを渡した場合にはinはpositionについて処理済と仮定する
	 *       infoはGetArchiveInfoまたはGetFileInfoで取得したものを渡す
	 * output_file_name: 出力先がファイルの場合の出力ファイル名
	 * lphMem: 出力先がメモリの場合のローカルハンドルへのポインタ
	 */
	virtual int GetFile(SpiInputStream *in, SpiArcFileInfo *info, LPCSTR output_file_name,
						FARPROC lpPrgressCallback, long lData);	/* 出力がファイル */
	virtual int GetFile(SpiInputStream *in, SpiArcFileInfo *info, HLOCAL *lphMem,
						FARPROC lpPrgressCallback, long lData);	/* 出力がメモり */


	/* File/Memory入力を扱うクラスでアタッチさせた方がいいような気がする */
	/* ～ CheckExistingFile ～
	 * 指定のファイルがアーカイブ内に存在するかどうかをチェックする
	 *   ファイル名はアーカイブファイルに対する相対パスで指定する
	 *   関数はアーカイブファイル中に前方一致するファイルを返却する
	 *   A. ?:/xxxx.lzh/yyyy.lzh/z.jpg
	 *      ^^^^^^^^^^^実ファイル
	 *                  ^^^^^^^^書庫内書庫ファイル
	 *                           ^^^^^書庫内ファイル
     *   という場合がある。
	 *    CheckExistingFile("?:/xxxx.lzh", "yyyy.lzh/z.jpg", &ptr);
	 *   では、ptrは"z.jpg"を指すように書き換えられる。また"yyyy.lzh"と"z.jpg"の間のセパレータは
	 *   "\0"で置換される。
	 *   このチェックの際には既にIsSupportedを呼んでサポートされる形式であることを確認すること
	 */
	/*
	 * [Description]
	 *  inで指定される入力データからquery_nameで指定されたファイル名が
	 *  存在するかどうかをチェックします。
	 *
	 * [Return Value]
	 *
	 *   > 0  query_nameは存在し、書庫内書庫ファイルでもない。
	 *  == 0  query_nameは存在するが、in内にある書庫内書庫ファイルである。
	 *        next_targetが次にチェックすべき書庫内のパスを示す。
	 *        inは書庫内書庫ファイルを実際に取得し、そのイメージを保持するように変更される。
	 *   < 0  query_nameは存在しないパスである。またはinで指定されたストリームは不正である。
	 *
	 * Example: (実際には次のCheckExistingFileを行なう時には他のPlug-inも試す必要がある)
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
	/* (Mac Binary のような)イメージの先頭offsetの扱いはSpiInputStreamで指定される
	 * 理由はなんとなく・・・。失敗かしら?
	 */
	virtual int CheckExistingFile(SpiInputStream *in, LPCSTR query_name, LPSTR *next_target);

protected:
	SpiFileLoadType EvalLoadType(DWORD pre, DWORD post);
	int SmartLoad();
	int SmartUnload();

public:
	/* これらは利用者側が定義する(あまり使うことありません) */
	typedef DWORD (WINAPI *SpiDllFileLoadFileCallback_t)(SpiDllFile *plugin);
	typedef SpiFileLoadType (WINAPI *SpiDllFileEvalCallback_t)(SpiDllFile *plugin, DWORD pre, DWORD post);

	SpiDllFileLoadFileCallback_t  m_pre_load_callback_func;
	SpiDllFileLoadFileCallback_t m_post_load_callback_func;
	SpiDllFileEvalCallback_t m_eval_load_type_callback_func;

#if	0
	/* SpiDllFileのレベルではこれは廃止します
	 */
	/* 
	 * 内部で暗黙的に呼ばれるコールバック関数。
	 * デフォルトでは何もしない関数が呼び出されます。これは以下のPlug-in I/Fで適用されます。
	 * また他の関数が内部的に呼び出した場合にも適用されることに注意してください。
	 * 00IN: GetPicture, GetPictureInfo
	 * 00AM: GetArchiveInfo, GetFile
	 */
	FARPROC m_default_callback;
	unsigned long m_callback_data32;
#endif

protected:
	/* SPIファイルへの関数ポインタ */
	FARPROC m_GetPluginInfo;
	FARPROC m_IsSupported;
	FARPROC m_GetPictureInfo;
	FARPROC m_GetPicture;
	FARPROC m_GetPreview;
	FARPROC m_GetArchiveInfo;
	FARPROC m_GetFileInfo;
	FARPROC m_GetFile;
	FARPROC m_ConfigurationDlg;


	HINSTANCE m_hModule;				/* モジュールハンドル/Staticの時に利用 */
	SpiFileLoadType m_load_type;		/* ロードタイプ */
	char m_about_name[128];					/* Plug-inの名前(GetPluginInfo(1, xx, xx)の結果) */
	char m_file_name[MAX_PATH];			/* Plug-inファイル名 */
	char *m_plugin_ext_list;			/* Plug-in 拡張子リスト */
};



/* SpiFilesManageにおいてFindNextPluginを実行する際の内部データを保持します。
 *   う～ん、いまいち・・・。typedef int SpiFindData;で充分じゃん(;_;)。
 *   ま、いつか役に立つ日も来るさ(<-そんな日が来るより書き直したほうが早いじゃん;_;)。
 * このI/Fの定義は結構悩ましいですねぇ。デザインパターンを読んでちゃんと
 * 勉強すべきか?。
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



/* SpiFileクラスの集合を管理します。 */

class SpiFilesManage {
 public:
	SpiFilesManage();
	virtual ~SpiFilesManage();

 public:
	/* Pluginのあるディレクトリを指定する */
	virtual int SetPluginDirectory(LPCSTR directory);

	/* Pluginファイルをロードする */
	virtual void LoadPluginFile();

	/* Pluginファイルをアンロードする */
	virtual void UnloadPluginFile();


	/* fileNameが扱えるPlug-inファイルを探す(convenience function)
	 * ただしfileNameが扱える複数のPlug-inが存在してもその内の最初のSpiFileを
	 * 返却することに注意せよ。すべてのPlug-inを利用しなければいけない場合には
	 * 列挙用のI/F(FindXXX系)を使用すること。
	 */
	/* SpiOpenFileの実装が終了すればSearchPluginは仮想化されたハンドルを利用して
	 * SpiFileを返却するようになる。
	 */
	virtual SpiFile *SearchPlugin(LPCSTR fileName, int offset= 0);

#if	0
	/* New I/F
	 *  まぁ実装はそのうちね・・・。
	 *  指定された書庫内(かもしれない)ファイルを取り扱う
	 *  ここではファイルへの仮想化されたハンドルを取得する
	 */
	virtual SpiOpenHandle SpiOpenFile(LPCSTR fileName/* ??? */) { return FALSE; };
#endif


	/* プラグインの取得をする
	 *   - 以前のバージョンと関数名に互換性がありません -
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
	virtual SpiFile *FindFirstPlugin(SpiFindData *find);	/* FindNextPluginとまったく同じ、互換関数 */
	virtual SpiFile *FindNextPlugin(SpiFindData *find);
	virtual void CloseFindData(SpiFindData *find);

	/* プラグインファイルを後から追加する
	 *   これは内蔵Plug-inなどを利用するために使用される
	 *   追加されたオブジェクトはUnloadPluginFIle時にdeleteされるのに注意せよ
	 */
	virtual int AddPlugin(SpiFile *spi);

	/* 指定のファイルをオープンする(必要であればAM Plug-inを使って) */
	virtual int OpenFile(SpiInputStream *in, LPCSTR filename);

	/* OpenFileでオープンしたイメージからBitmapを生成する(必要であればIN Plug-inを使って) */
	virtual SpiFile *GetBitmapImage(SpiInputStream *in, SpiOutputStream *out, int mode,
									FARPROC lpPrgressCallback, long lData);

public:
	/* これらは利用者側が定義する */
	typedef int (WINAPI *SpiFilesManageQueryCallback_t)(SpiFilesManage *own, LPCSTR dir, LPCSTR name);

	SpiFilesManageQueryCallback_t m_query_plugin_load_callback_func;

 protected:
	/* fileNameのPlug-inをロードするか?
	 *  ロードする場合には有効な(newで割り当てた)SpiFileへのポインタを返却する
	 *  ロードしない場合にはNULLを返却すること
	 * この関数はLoadPluginFile()実行中のコールバック関数のように動作する
	 * [単純に派生してLoadPluginFileを置き換えれば良いような気もするけどね]
	 *
	 * 例えば"ifjpegv6.spi"を指名手配してロードしないとかいうようなことを実装する(;P)。
	 *
	 * 派生させない場合にはm_query_plugin_load_callback_funcが利用できる
	 */
	virtual SpiFile *QueryPluginLoad(LPCSTR fileName);


 protected:
	char m_plugin_dir[MAX_PATH+260];	/* Plug-inをロードするディレクトリ */

#if	0
/* これはやめました */
	static int m_reference_count;		/* Pluginの検索を一回しかしない時のため */
#endif

	std::vector<SpiFile *> m_plugins;	/* SpiFileを保持する */

};

#endif	/* SUSIE_PLUGIN_SPI_IF_H */
