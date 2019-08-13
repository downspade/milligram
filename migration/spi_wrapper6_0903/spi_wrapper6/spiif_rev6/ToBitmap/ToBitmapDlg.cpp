// ToBitmapDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"
//#include "spiif_derived.h"

#include "susie_drop.h"
#include "convert_thread.h"

#include "ToBitmap.h"
#include "ToBitmapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	regTreeKey		"Software\\kana\\ToBitmap"
#define	regOutputVal	"OutputDir"

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


/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われている CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// メッセージ ハンドラがありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/* ******************************* CCheckListBox2 ****** */
CCheckListBox2::
CCheckListBox2()
{
//	memset(&m_max_rect, 0, sizeof(m_max_rect));
}

void CCheckListBox2::
DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
//	CCheckListBox::DrawItem(lpDrawItemStruct);

	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) ==
		(LBS_OWNERDRAWFIXED | LBS_HASSTRINGS));

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (((LONG)(lpDrawItemStruct->itemID) >= 0) &&
		(lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
	{
		int cyItem = GetItemHeight(lpDrawItemStruct->itemID);
		BOOL fDisabled = !IsWindowEnabled() || !IsEnabled(lpDrawItemStruct->itemID);

		COLORREF newTextColor = fDisabled ?
			RGB(0x80, 0x80, 0x80) : GetSysColor(COLOR_WINDOWTEXT);  // light gray
		COLORREF oldTextColor = pDC->SetTextColor(newTextColor);

		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

		if (!fDisabled && ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0))
		{
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		}

		if (m_cyText == 0)
			VERIFY(cyItem >= CalcMinimumItemHeight());

		CString strText;
		GetText(lpDrawItemStruct->itemID, strText);

		pDC->ExtTextOut(lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top + max(0, (cyItem - m_cyText) / 2),
			ETO_OPAQUE, &(lpDrawItemStruct->rcItem), strText, strText.GetLength(), NULL);

		/* スクロールバーのサイズを再設定する
		 * ただし大きくなる方にしか対応していないのでDeleteString()しても
		 * スクロール範囲は小さくならない
		 */
		CSize size= pDC->GetOutputTextExtent(strText);
		if (size.cx > m_size.cx) {
			m_size= size;
			SetHorizontalExtent(lpDrawItemStruct->rcItem.left + m_size.cx);
		}

		pDC->SetTextColor(oldTextColor);
		pDC->SetBkColor(oldBkColor);
	}

	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) != 0)
		pDC->DrawFocusRect(&(lpDrawItemStruct->rcItem));
}


/////////////////////////////////////////////////////////////////////////////
// CToBitmapDlg ダイアログ

CToBitmapDlg::CToBitmapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToBitmapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CToBitmapDlg)
	m_progress_str = _T("");
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_thread_param= NULL;
}

CToBitmapDlg::~CToBitmapDlg()
{
	delete [] m_thread_param;
}

void CToBitmapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToBitmapDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_OUTPUT_DIR, m_output_dir);
	DDX_Control(pDX, IDC_BITMAP_LIST, m_file_list);
	DDX_Text(pDX, IDC_PROGRESS, m_progress_str);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CToBitmapDlg, CDialog)
	//{{AFX_MSG_MAP(CToBitmapDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(ID_CONVERT_OK, OnConvertOk)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_CONVERT_CANCEL, OnConvertCancel)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER_APP_PROGRESS, OnUserAppProgress)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToBitmapDlg メッセージ ハンドラ

BOOL CToBitmapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニュー項目をシステム メニューへ追加します。

	// IDM_ABOUTBOX はコマンド メニューの範囲でなければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	m_drop_target.Register(this);

//	::DragAcceptFiles(GetSafeHwnd(), TRUE);
	m_file_list.SetCheckStyle(BS_AUTOCHECKBOX);

	char buffer[2048];
	buffer[0]= 0;
	knRegTinyGetEx(HKEY_CURRENT_USER, regTreeKey, regOutputVal, buffer, sizeof(buffer));
	m_output_dir.SetWindowText(buffer);

	m_hAccel= ::LoadAccelerators(AfxFindResourceHandle(MAKEINTRESOURCE(IDR_MAINFRAME), RT_ACCELERATOR),MAKEINTRESOURCE(IDR_MAINFRAME));

	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void CToBitmapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CToBitmapDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR CToBitmapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



/* Drop位置を確認する
 *   TRUE:ディレクトリ欄の場合
 *   FALSE:その他
 */
BOOL CToBitmapDlg::
check_drop_point(CPoint point)
{
	RECT output_rect;

	/* Drop先が出力先ディレクトリ欄かどうかを確認 */
	m_output_dir.GetWindowRect(&output_rect);
	ScreenToClient(&output_rect);

	if ((output_rect.top <= point.y && point.y <= output_rect.bottom) &&
		(output_rect.left <= point.x && point.x <= output_rect.right)) {
		return TRUE;
	} else {
		return FALSE;
	}
}


void CToBitmapDlg::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
#if 0
	CDialog::OnDropFiles(hDropInfo);
#else

	/* FileManager D&Dの実装(WM_DROPFILES) 
	 *   CF_HDROPもWM_DROPFILESのエミュレーションで取り扱う
	 */

	/* */
	int nFiles = (int) ::DragQueryFile(hDropInfo, (UINT)(~0), NULL, 0);
	int i;
	POINT drop_point;

	/* Dropされた場所を確認 */
	if (::DragQueryPoint(hDropInfo, &drop_point) == FALSE) {
		/* クライアント領域以外であれば何もしないで終り */
		::DragFinish(hDropInfo);
		return ;
	}


	if (check_drop_point(drop_point) != FALSE) {
		int size;

		/* Drop先は出力先ディレクトリ */
		if (nFiles != 1) {
			::DragFinish(hDropInfo);
			return ;
		}
		/* 出力先を変更 */
		size= (int)DragQueryFile(hDropInfo, 0, NULL, 0) + 128;
		char *temp_string= new char [size];
		if (DragQueryFile(hDropInfo, 0, temp_string, size) > 0) {
			/* 指定されたものがディレクトリかどうかを確認 */
			DWORD atr= GetFileAttributes(temp_string);

			if ((atr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				/* 出力先ディレクトリを変更 */
				m_output_dir.SetWindowText(temp_string);
				knRegTinySet(HKEY_CURRENT_USER, regTreeKey, regOutputVal, REG_SZ, temp_string, strlen(temp_string));
			}
		}

		/* おわり */
		::DragFinish(hDropInfo);
		delete [] temp_string;
	} else {
		int size= 0;
		/* Drop先は変換ファイルリスト */

		/* 最大のファイル長を算出 */
		for (i= 0; i < nFiles; ++i) {
			int temp_size;
			temp_size= (int)DragQueryFile(hDropInfo, i, NULL, 0) + 128;
			if (temp_size > size)
				size= temp_size;
		}
		char *temp_string= new char [size];

		for (i= 0; i < nFiles; ++i) {
			int index;
			if (DragQueryFile(hDropInfo, i, temp_string, size) > 0 &&
				(GetFileAttributes(temp_string) & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				/* ファイル名が取得できた(ただしディレクトリは除外) */
				index= m_file_list.AddString(temp_string);
				if (index != LB_ERR && index != LB_ERRSPACE) {
					convert_thread_param_t *p= new convert_thread_param_t;
					p->m_in.set_stream_data(temp_string, 0);
					p->m_result= 0;

					m_file_list.SetCheck(index, 1);
					m_file_list.SetItemDataPtr(index, p);

				}
			}
		}

		::DragFinish(hDropInfo);
		delete [] temp_string;
	}

#endif
}


/* DROPEFFECT_NONEを返却しても効果が無いような気がするんだけど・・・ ?_? */
DROPEFFECT CToBitmapDlg::
OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if (check_drop_point(point) != FALSE) {
		HGLOBAL hgData;
		DROPFILES *hDropInfo= NULL;
		int size;
		char *temp_string= NULL;
		if (pDataObject->IsDataAvailable(m_drop_target.m_file_drop_id) &&
			(hgData = pDataObject->GetGlobalData(m_drop_target.m_file_drop_id)) != NULL) {

//			hDropInfo= (DROPFILES *)GlobalLock(hgData);

			size= (int)DragQueryFile((HDROP)hgData, 0, NULL, 0) + 128;
			temp_string= new char [size];

			if (DragQueryFile((HDROP)hgData, 0, temp_string, size) > 0 &&
				(GetFileAttributes(temp_string) & FILE_ATTRIBUTE_DIRECTORY)) {
				delete temp_string;
				return DROPEFFECT_COPY | DROPEFFECT_MOVE;
			}
			delete [] temp_string;
//			return DROPEFFECT_COPY | DROPEFFECT_MOVE;
		}
	} else {
		if (pDataObject->IsDataAvailable(m_drop_target.m_file_content_id) &&
			pDataObject->IsDataAvailable(m_drop_target.m_file_group_id)) {
			return DROPEFFECT_COPY;
		}
	}
	return DROPEFFECT_NONE;
}


BOOL CToBitmapDlg::
OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	BOOL status= FALSE;
	/* drop処理 */
	if (check_drop_point(point) != FALSE) {
		/* 出力ディレクトリの変更 */
		HGLOBAL hgData;
//		DROPFILES *hDropInfo;
		int size;
		char *temp_string= NULL;
		if (pDataObject->IsDataAvailable(m_drop_target.m_file_drop_id) &&
			(hgData = pDataObject->GetGlobalData(m_drop_target.m_file_drop_id)) != NULL) {

//			hDropInfo= (DROPFILES *)GlobalLock(hgData);

			size= (int)DragQueryFile((HDROP)hgData, 0, NULL, 0) + 128;
			temp_string= new char [size];

			if (DragQueryFile((HDROP)hgData, 0, temp_string, size) > 0 &&
				(GetFileAttributes(temp_string) & FILE_ATTRIBUTE_DIRECTORY)) {

				/* 出力先ディレクトリを変更 */
				m_output_dir.SetWindowText(temp_string);
				knRegTinySet(HKEY_CURRENT_USER, regTreeKey, regOutputVal, REG_SZ, temp_string, strlen(temp_string));

			}
			::DragFinish((HDROP)hgData);
			delete [] temp_string;
			status= TRUE;

		} else {
			status= FALSE;

		}
		/* おわり */

	} else {
		/* 出力先リストへの登録 */
		HGLOBAL hGroup= NULL;
		HGLOBAL hContent= NULL;
//		LPVOID group_image= NULL, content_image= NULL;
		FILEGROUPDESCRIPTOR *file_group;

		if (pDataObject->IsDataAvailable(m_drop_target.m_file_group_id)) {
			hGroup= pDataObject->GetGlobalData(m_drop_target.m_file_group_id);
		}
#if	0
		if (pDataObject->IsDataAvailable(m_drop_target.m_file_content_id)) {
			hContent= pDataObject->GetGlobalData(m_drop_target.m_file_content_id);
		}
#endif

		if (hGroup == NULL /* || hContent == NULL */) {
			status= FALSE;
		} else {
			int i;
			FORMATETC fmt_etc;
			STGMEDIUM stg;

			file_group= (FILEGROUPDESCRIPTOR *)GlobalLock(hGroup);
			for (i= 0; i < file_group->cItems; ++i) {
				/* イメージを取り出す */

				memset(&fmt_etc, 0, sizeof(fmt_etc));
				fmt_etc.cfFormat= m_drop_target.m_file_content_id;
				fmt_etc.lindex= i;
				fmt_etc.dwAspect= DVASPECT_CONTENT;
				fmt_etc.tymed= TYMED_HGLOBAL;

				if (pDataObject->GetData(m_drop_target.m_file_content_id, &stg, &fmt_etc) != 0) {
					int index;
					hContent= stg.hGlobal;
					index= m_file_list.AddString(file_group->fgd[i].cFileName);
					if (index != LB_ERR && index != LB_ERRSPACE) {
						convert_thread_param_t *p= new convert_thread_param_t;
						p->m_in.set_stream_data(file_group->fgd[i].cFileName,
											   stg.hGlobal, file_group->fgd[i].nFileSizeLow);
						p->m_result= 0;

						m_file_list.SetCheck(index, 1);
						m_file_list.SetItemDataPtr(index, p);

					}
				}
				hContent= NULL;
			}
			status= TRUE;
		}

	}

	return status;
}


void CToBitmapDlg::OnConvertOk() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	if (m_convert_thread.is_running()) {
		MessageBox("変換実行中です");
		return ;
	}

	/* 変換リストを生成 */
	int max_num, num, i;
	CString target;

	max_num= m_file_list.GetCount();
	if (m_thread_param)
		delete [] m_thread_param;
	m_thread_param= new convert_thread_param_t [max_num];
	for (i= num= 0; i < max_num; ++i) {
		convert_thread_param_t *p;
		if (m_file_list.GetCheck(i) == 1) {
			p= (convert_thread_param_t *)m_file_list.GetItemDataPtr(i);
			m_thread_param[num].m_in= p->m_in;
			m_thread_param[num].m_result= p->m_result;
			m_thread_param[num].m_data32= i;
			++num;
		}
	}
	m_thread_param_num= num;
	m_convert_thread.set_plugin_manager(&((CToBitmapApp *)AfxGetApp())->m_manager);
	m_output_dir.GetWindowText(target);
	m_convert_thread.set_parameter(target, GetSafeHwnd(), WM_USER_APP_PROGRESS,
									m_thread_param, m_thread_param_num);
	m_convert_thread.start();
}

void CToBitmapDlg::OnDelete() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	int max_num, i;
	int cur= m_file_list.GetCaretIndex();
	int anchor= m_file_list.GetAnchorIndex();
	max_num= m_file_list.GetCount();
	for (i= max_num - 1; i >= 0; --i) {	/* 下から削除しないとindexがずれちゃうよね */
		if (m_file_list.GetSel(i) > 0) {
			/* 選択しているリストを削除 */
			convert_thread_param_t *p;
			p= (convert_thread_param_t *)m_file_list.GetItemDataPtr(i);
			delete p;
			m_file_list.DeleteString(i);
		}
	}
	if (cur != LB_ERR) {
		int new_cur= cur > anchor ? anchor : cur;
		m_file_list.SetCaretIndex(new_cur);
		m_file_list.SetSel(new_cur);
	}
}

void CToBitmapDlg::OnConvertCancel() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	m_convert_thread.cancel();
}

LONG CToBitmapDlg::OnUserAppProgress(UINT wParam, LONG lParam)
{
	if (lParam == ~0) {
		m_progress_str= "Canceled";
	} else if (lParam == ~0 - 1) {
		m_progress_str= "Done";
	} else {
		if (wParam <= 100)
			m_progress_str.Format("[%d/%d Files] [%.3d%%] %s", lParam+1, m_thread_param_num, wParam, m_thread_param[lParam].m_in.query_file_name());
		if (wParam == 1000) {
			m_file_list.SetCheck(m_thread_param[lParam].m_data32, 0);
		}
	}
	UpdateData(FALSE);
	return TRUE;
}

void CToBitmapDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	int max_num, i;
	max_num= m_file_list.GetCount();
	for (i= max_num - 1; i >= 0; --i) {	/* 下から削除しないとindexがずれちゃうよね */
		convert_thread_param_t *p;
		p= (convert_thread_param_t *)m_file_list.GetItemDataPtr(i);
		delete p;
	}
//	m_convert_thread.cancel();
//	delete [] m_thread_param;
}


BOOL CToBitmapDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
#if	1
	// for modeless processing (or modal)
	ASSERT(m_hWnd != NULL);

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// fix around for VK_ESCAPE in a multiline Edit that is on a Dialog
	// that doesn't have a cancel or the cancel is disabled.
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL) &&
		(::GetWindowLong(pMsg->hwnd, GWL_STYLE) & ES_MULTILINE) /*&&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))*/)
	{
		HWND hItem = ::GetDlgItem(m_hWnd, IDCANCEL);
		if (hItem == NULL || ::IsWindowEnabled(hItem))
		{
			SendMessage(WM_COMMAND, IDCANCEL, 0);
			return TRUE;
		}
	}
	// filter both messages to dialog and from children
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// finally, translate the message
		int trans;
		if (m_hAccel != NULL) {
			trans= ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg);
			if (trans != 0) {
				return trans;
			}
		}
//		return hAccel != NULL &&  ::TranslateAccelerator(m_hWnd, hAccel, pMsg);
	}
	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
#else
	return CDialog::PreTranslateMessage(pMsg);
#endif
}
