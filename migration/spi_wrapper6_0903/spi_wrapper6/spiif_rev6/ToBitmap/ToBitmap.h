// ToBitmap.h : TOBITMAP アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
#define AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CToBitmapApp:
// このクラスの動作の定義に関しては ToBitmap.cpp ファイルを参照してください。
//

class CToBitmapApp : public CWinApp
{
public:
	CToBitmapApp();

	SpiFilesManage m_manager;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CToBitmapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CToBitmapApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
