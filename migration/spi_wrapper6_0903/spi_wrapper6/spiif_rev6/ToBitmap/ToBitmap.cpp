// ToBitmap.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"

#include "susie_drop.h"
#include "convert_thread.h"

#include "ToBitmap.h"
#include "ToBitmapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToBitmapApp

BEGIN_MESSAGE_MAP(CToBitmapApp, CWinApp)
	//{{AFX_MSG_MAP(CToBitmapApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToBitmapApp クラスの構築

CToBitmapApp::CToBitmapApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CToBitmapApp オブジェクト

CToBitmapApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CToBitmapApp クラスの初期化

BOOL CToBitmapApp::InitInstance()
{
	AfxOleInit();
	AfxEnableControlContainer();

	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

	m_manager.SetPluginDirectory(NULL);
	m_manager.LoadPluginFile();

	CToBitmapDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ダイアログが <OK> で消された時のコードを
		//       記述してください。
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ダイアログが <ｷｬﾝｾﾙ> で消された時のコードを
		//       記述してください。
	}

	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

