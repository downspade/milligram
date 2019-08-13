

/*
	Susie�̃J�^���O�����D&D�����ꍇ�̃N���b�v�{�[�h�t�H�[�}�b�g�R�[�h

	�A�[�J�C�u�J�^���O����
		"FileGroupDescriptor"
		"FileContents"
		����ɑ΂���h�L�������g��MSDN��"Dragging and Dropping"�̍��ڂ��Q�Ƃ̎�
			FILEDESCRIPTOR�\���̂�cFileName�����o�͂Ȃ���UNICODE�ł��Ȃ������肷��̂Œ��ӂ��K�v�B
			������ShellAPI���o�O���Ă���񂾂낤�BUnicode�v���W�F�N�g�ō���Ă͂����Ȃ��̂ˁB
			FileContents�̓��e��Susie�ł�TYMED_HGLOBAL/DVASPECT_CONTENT���w�肵�Ȃ���΂����Ȃ�(�悤��)�B
	�ʏ�J�^���O����
		CF_HDROP (WM_DROPFILES�Ŏ󂯎���̂�OLE�ł��K�v�Ȃ�������

	���\�߂�ǂ��̂ˁB�܂�Explorer�����ɓ��t�@�C���������Ȃ��̂Ŏd���Ȃ��񂾂낤���ǂ��B
	���ʂɍl����Ə��ɓ��t�@�C����ShellNameSpace�ŉ������ׂ��ȋC�����邯�ǁE�E�E��Ȃ�����˂��B

	 �����ł�pWnd��CToBitmapDlg�̔h���N���X�Ɖ��肵�܂�

	�܂��{����CDropAcceptDlg�̔h���N���X�����肵��class CToBitmapDlg:public CDropAcceptDlg�Ƃ����\���ɂ��ׂ�
	�Ȃ�ł��傤�ȁB

	CToBitmapDlg�ł͎��̊֐�����������Ă��鎖�����҂���Ă��܂��B����͒ʏ�CView�Ŏ�������Ă�����̂�
	�������܂��B

		OnDragEnter / OnDragLeave / OnDragOver / OnDrop


	�������̂��Ǝ󂯓���\�ȃf�[�^�t�H�[�}�b�g�̈ꗗ���w�肵�Ă�����ĂƂ肠�����󂯎���Ă��܂���
	�����āA��̂��Ƃ�CWnd�ɂ��܂������Ă̂��ĊO���C�y�Ɋy�ȋC�����܂��ȁB

*/

#include "stdafx.h"
#include <vector>

#include "../spi_api.h"
#include "../spi_misc.h"
#include "../spiif.h"
//#include "spiif_derived.h"

#include "susie_drop.h"
#include "convert_thread.h"	/* ����ςȂɂ��Ԉ���Ă��� */

#include "ToBitmap.h"
#include "ToBitmapDlg.h"

susie_drop_target_t::
susie_drop_target_t()
{
	/* COleDataObject->GetGlobalData()����Ƃ��Ɏg��ID���擾���Ă��� */
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
	/* OnDrop��FALSE��ԋp�������ɂ͌��݂̂Ƃ��낷�ׂĂ�Drop Source�̎Q�Ƃ͂Ȃ�(��) */
}


DROPEFFECT susie_drop_target_t::
OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	DROPEFFECT status= DROPEFFECT_NONE;
	/*
	 * point��drop�\���ǂ�����₢���킹��
	 *   ���������̖₢���킹��pWnd�Ɉˑ����邪�ACWnd�͂���Ȃ��Ƃ͒m��Ȃ��E�E�E�B
	 *   �ėp�I�ɍ��ɂ͂ǂ������炢���񂾂낤?�B
	 *   CView��������OnDragOver�����邩��܂��Ȃ񂾂��ǂ˂��B
	 *   ����ω����Ԉ���Ă����ȁB
	 */

	/* �󂯓���\�ȃf�[�^���ǂ������m�F���� */
	if(!pDataObject->IsDataAvailable(m_file_group_id) &&
	   !pDataObject->IsDataAvailable(m_file_content_id) &&
	   !pDataObject->IsDataAvailable(m_file_drop_id) ) {
		return DROPEFFECT_NONE;
	}

	/* �󂯓���\��Window���ǂ������m�F����
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
	case 0:	/* �ꏊ�ɖ��Ȃ� -> ���X�g�{�b�N�X�ɑ}��(�\��) */
	case 1:	/* �ꏊ�ɖ��Ȃ� -> �G�f�B�b�g�{�b�N�X�ɑ}��(�\��) */
		status= DROPEFFECT_COPY;
		break;

	default:	/* �����ɂ̓h���b�v�o���Ȃ� */
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

	/* �󂯓���\�ȃf�[�^���ǂ������m�F���� */
	if(!pDataObject->IsDataAvailable(m_file_group_id) &&
	   !pDataObject->IsDataAvailable(m_file_content_id) &&
	   !pDataObject->IsDataAvailable(m_file_drop_id) ) {
		return FALSE;
	}

	/* �󂯓���\��Window���ǂ������m�F���� */
	CToBitmapDlg *p= dynamic_cast<CToBitmapDlg *>(pWnd);
	if (p == NULL) return FALSE;

	/* �������s�Ȃ� */
#if	1
	return p->OnDrop(pDataObject, dropEffect, point);
#else
	switch (p->query_drag_point(pDataObject, point)) {
	case 0:	/* �ꏊ�ɖ��Ȃ� -> ���X�g�{�b�N�X�ɑ}��(�\��) */
		/* �f�[�^��������� */
		
		
		break;

	case 1:	/* �ꏊ�ɖ��Ȃ� -> �G�f�B�b�g�{�b�N�X�ɑ}��(�\��) */

		break;

	default:	/* �����ɂ̓h���b�v�o���Ȃ� */
		status= FALSE;
		break;
		/* */
	}
#endif
	return status;
}

