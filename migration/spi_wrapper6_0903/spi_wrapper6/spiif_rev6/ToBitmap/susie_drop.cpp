

/*
	SusieのカタログからのD&Dした場合のクリップボードフォーマットコード

	アーカイブカタログから
		"FileGroupDescriptor"
		"FileContents"
		これに対するドキュメントはMSDNの"Dragging and Dropping"の項目を参照の事
			FILEDESCRIPTOR構造体のcFileNameメンバはなぜかUNICODEでこなかったりするので注意が必要。
			きっとShellAPIがバグっているんだろう。Unicodeプロジェクトで作ってはいけないのね。
			FileContentsの内容はSusieではTYMED_HGLOBAL/DVASPECT_CONTENTを指定しなければいけない(ようだ)。
	通常カタログから
		CF_HDROP (WM_DROPFILESで受け取れるのでOLEでやる必要ないかもね

	結構めんどいのね。まぁExplorerが書庫内ファイルを扱えないので仕方ないんだろうけどさ。
	普通に考えると書庫内ファイルはShellNameSpaceで解決すべきな気もするけど・・・危ないからねぇ。

	 ここではpWndはCToBitmapDlgの派生クラスと仮定します

	まぁ本来はCDropAcceptDlgの派生クラスを仮定してclass CToBitmapDlg:public CDropAcceptDlgという構成にすべき
	なんでしょうな。

	CToBitmapDlgでは次の関数が実装されている事が期待されています。これは通常CViewで実装されているものに
	相当します。

		OnDragEnter / OnDragLeave / OnDragOver / OnDrop


	いっそのこと受け入れ可能なデータフォーマットの一覧を指定してもらってとりあえず受け取ってしまって
	おいて、後のことはCWndにおまかせってのも案外お気楽極楽な気がしますな。

*/

#include "stdafx.h"
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"
//#include "spiif_derived.h"

#include "susie_drop.h"
#include "convert_thread.h"	/* やっぱなにか間違っている */

#include "ToBitmap.h"
#include "ToBitmapDlg.h"

susie_drop_target_t::
susie_drop_target_t()
{
	/* COleDataObject->GetGlobalData()するときに使うIDを取得しておく */
	m_file_group_id= ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	m_file_content_id= ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
	m_file_drop_id= CF_HDROP;	/* OnDropFiles */
}

susie_drop_target_t::
~susie_drop_target_t()
{
}


DROPEFFECT susie_drop_target_t::
OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

void susie_drop_target_t::
OnDragLeave(CWnd* pWnd)
{
	// nothing to do?.
	/* OnDropがFALSEを返却した時には現在のところすべてのDrop Sourceの参照はない(筈) */
}


DROPEFFECT susie_drop_target_t::
OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	DROPEFFECT status= DROPEFFECT_NONE;
	/*
	 * pointにdrop可能かどうかを問い合わせる
	 *   ただしこの問い合わせはpWndに依存するが、CWndはそんなことは知らない・・・。
	 *   汎用的に作るにはどうしたらいいんだろう?。
	 *   CViewだったらOnDragOverがあるからましなんだけどねぇ。
	 *   やっぱ何か間違っているよな。
	 */

	/* 受け入れ可能なデータかどうかを確認する */
	if(!pDataObject->IsDataAvailable(m_file_group_id) &&
	   !pDataObject->IsDataAvailable(m_file_content_id) &&
	   !pDataObject->IsDataAvailable(m_file_drop_id) ) {
		return DROPEFFECT_NONE;
	}

	/* 受け入れ可能なWindowかどうかを確認する
	 *   
	 */
	CToBitmapDlg *p= dynamic_cast<CToBitmapDlg *>(pWnd);
	if (p == NULL) { 
		return DROPEFFECT_NONE;
	}

#if 1
	return p->OnDragOver(pDataObject, dwKeyState, point);
#else
	switch (p->query_drag_point(pDataObject, dwKeyState, point)) {
	case 0:	/* 場所に問題なし -> リストボックスに挿入(予定) */
	case 1:	/* 場所に問題なし -> エディットボックスに挿入(予定) */
		status= DROPEFFECT_COPY;
		break;

	default:	/* ここにはドロップ出来ない */
		break;
		/* */
	}
#endif
	return status;
}


BOOL susie_drop_target_t::
OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	BOOL status= FALSE;

	/* 受け入れ可能なデータかどうかを確認する */
	if(!pDataObject->IsDataAvailable(m_file_group_id) &&
	   !pDataObject->IsDataAvailable(m_file_content_id) &&
	   !pDataObject->IsDataAvailable(m_file_drop_id) ) {
		return FALSE;
	}

	/* 受け入れ可能なWindowかどうかを確認する */
	CToBitmapDlg *p= dynamic_cast<CToBitmapDlg *>(pWnd);
	if (p == NULL) return FALSE;

	/* 処理を行なう */
#if	1
	return p->OnDrop(pDataObject, dropEffect, point);
#else
	switch (p->query_drag_point(pDataObject, point)) {
	case 0:	/* 場所に問題なし -> リストボックスに挿入(予定) */
		/* データを引き取る */
		
		
		break;

	case 1:	/* 場所に問題なし -> エディットボックスに挿入(予定) */

		break;

	default:	/* ここにはドロップ出来ない */
		status= FALSE;
		break;
		/* */
	}
#endif
	return status;
}

