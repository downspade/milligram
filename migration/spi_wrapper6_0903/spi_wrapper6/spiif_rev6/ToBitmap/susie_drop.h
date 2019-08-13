
#ifndef	TO_BITMAP_SUSIE_DROP_H
#define	TO_BITMAP_SUSIE_DROP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class susie_drop_target_t: public COleDropTarget {
public:
	susie_drop_target_t();
	virtual ~susie_drop_target_t();
	
public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	void OnDragLeave(CWnd* pWnd);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

public:
	UINT m_file_group_id;
	UINT m_file_content_id;
	UINT m_file_drop_id;
};

#endif	/* TO_BITMAP_SUSIE_DROP_H */