// ToBitmapDlg.h : ヘッダー ファイル
//

#if !defined(AFX_TOBITMAPDLG_H__0C76B55A_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
#define AFX_TOBITMAPDLG_H__0C76B55A_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	WM_USER_APP_PROGRESS	(WM_USER + 0x100)

/////////////////////////////////////////////////////////////////////////////
// CToBitmapDlg ダイアログ
class CCheckListBox2 : public CCheckListBox {
public:
	CCheckListBox2();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
//	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
protected:
//	RECT m_max_rect;
	CSize m_size;
};


class CToBitmapDlg : public CDialog
{
// 構築
public:
	CToBitmapDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ
	virtual ~CToBitmapDlg();

// ダイアログ データ
	//{{AFX_DATA(CToBitmapDlg)
	enum { IDD = IDD_TOBITMAP_DIALOG };
	CStatic	m_progress;
	CEdit	m_output_dir;
	CCheckListBox2	m_file_list;
	CString	m_progress_str;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CToBitmapDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

public:
	susie_drop_target_t m_drop_target;
	convert_thread_t m_convert_thread;
	convert_thread_param_t *m_thread_param;
	int m_thread_param_num;
	HACCEL m_hAccel;

	DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

private:
	BOOL check_drop_point(CPoint point);

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CToBitmapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnConvertOk();
	afx_msg void OnDelete();
	afx_msg void OnConvertCancel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LONG OnUserAppProgress(UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TOBITMAPDLG_H__0C76B55A_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
