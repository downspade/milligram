// ToBitmap.h : TOBITMAP �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#if !defined(AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
#define AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// ���C�� �V���{��

/////////////////////////////////////////////////////////////////////////////
// CToBitmapApp:
// ���̃N���X�̓���̒�`�Ɋւ��Ă� ToBitmap.cpp �t�@�C�����Q�Ƃ��Ă��������B
//

class CToBitmapApp : public CWinApp
{
public:
	CToBitmapApp();

	SpiFilesManage m_manager;

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CToBitmapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����

	//{{AFX_MSG(CToBitmapApp)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_TOBITMAP_H__0C76B558_AFF2_11D3_B449_00C0DF49BE7F__INCLUDED_)
