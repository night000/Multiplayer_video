#include "StdAfx.h"
#include "WndHeaderImagesView.h"
#include "WndRoom.h"
#include "WndChat.h"
#include "WndLobby.h"
#include <WinInet.h>

#pragma comment(lib, "Wininet.lib")

CWndHeaderImagesView::CWndHeaderImagesView()
: m_bTrackMouse(false)
, m_nCurSelUserId(0)
, m_pBkImage(NULL)
{
}

CWndHeaderImagesView::~CWndHeaderImagesView()
{
}

LRESULT CWndHeaderImagesView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_pBkImage = _GlobalSetting.m_wndResImgMgr.GetResFromId(20004);

	_InitCtrls();
	_InitIECtrl();

	SetWindowPos(NULL, 0, 0, 478, 310, SWP_NOMOVE|SWP_NOZORDER);
	_Layout();

	AtlAdviseSinkMap(this, true);

	return 0L;
}

LRESULT CWndHeaderImagesView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	AtlAdviseSinkMap(this, false);

	if (m_wndIE.m_hWnd)
	{
		m_wndIE.DestroyWindow();
		m_wndIE.m_hWnd = NULL;
	}

	return 0L;
}

LRESULT CWndHeaderImagesView::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	return 0L;
}

LRESULT CWndHeaderImagesView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_virBtns.OnMouseMove(uMsg, wParam, lParam, bHandled);

	if (!m_bTrackMouse)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 10;
		tme.hwndTrack = m_hWnd;
		_TrackMouseEvent(&tme);

		m_bTrackMouse = true;
	}

	return 0L;
}

LRESULT CWndHeaderImagesView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_bTrackMouse)
	{
		m_bTrackMouse = false;

		m_virBtns.OnMouseLeave(uMsg, wParam, lParam, bHandled);
	}

	return 0L;
}

LRESULT CWndHeaderImagesView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_virBtns.OnLButtonUp(uMsg, wParam, lParam, bHandled);

	return 0L;
}

LRESULT CWndHeaderImagesView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_virBtns.OnLButtonDown(uMsg, wParam, lParam, bHandled);

	CPoint pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	HWND hParent = ::GetParent(m_hWnd);
	SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pt.x, pt.y));

	return 0L;
}

LRESULT CWndHeaderImagesView::OnVBLBtnDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int nBtnId = (int)lParam;

	switch (nBtnId)
	{
	case ID_HEADERIMAGES_WND_BTN_TAB0:
		_OnClickedBtnTab0();
		break;
	case ID_HEADERIMAGES_WND_BTN_TAB1:
		_OnClickedBtnTab1();
		break;
	case ID_HEADERIMAGES_WND_BTN_TAB2:
		_OnClickedBtnTab2();
		break;
	case ID_HEADERIMAGES_WND_BTN_CLOSE:
		_OnClickedBtnClose();
		break;
	default:
		break;
	}

	return 0L;
}

void __stdcall CWndHeaderImagesView::OnBeforeNavigate2 ( IDispatch* pDisp, VARIANT* URL, VARIANT* Flags,
												VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel)
{
	//判断特征字符串是否存在
	CString strURL;
	strURL = (LPCTSTR)URL->pbstrVal;
	int nPos = strURL.Find(_T("headerimg://"), 0);

	if(nPos != -1)
	{
		*Cancel = TRUE;
		CString strHeaderImagesInfo = strURL.Right(strURL.GetLength()-nPos-15);
		int nHeaderImagesId = _wtoi((LPCTSTR)strHeaderImagesInfo);
		if(nHeaderImagesId >=0)
		{
			//设置头像 叶寅威注释
			m_nCurSelUserId=nHeaderImagesId;
			_GlobalSetting.m_wndMain.m_pWndLobby->m_pHeaderImg=_GlobalSetting.m_headerImgMgr.GetBigHeaderImageFromId(nHeaderImagesId);
			_GlobalSetting.m_wndMain.m_pWndLobby->InvalidateRect(_GlobalSetting.m_wndMain.m_pWndLobby->m_vbtnHeaderImgPos.GetrcPos(),FALSE);
		}
	}

	//这里不需要重置用户,只要用户愿意，可以对一个人盖很多章
	//m_nCurSelUserId = 0;
	return;
}

void CWndHeaderImagesView::DoPaint(CDCHandle dc)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	//dc.FillSolidRect(&rcClient, RGB(236, 233, 216));
	//绘制标准对话框
	_DrawBkgnd(dc, rcClient);
	m_virBtns.OnPaint(dc, rcClient);
}

void CWndHeaderImagesView::_DrawBkgnd(CDCHandle &dc, CRect &rcDraw)
{
#ifdef VER_9158KTV

	CRect rcTmp;
	rcTmp = rcDraw;
	dc.FillSolidRect(&rcTmp, RGB(0, 110, 178));
	rcTmp.DeflateRect(1,1,1,1);
	dc.FillSolidRect(&rcTmp, RGB(234, 246, 255));

	if(m_pBkImage)
	{
		int src_left = m_pBkImage->m_nLeft;
		int src_top = m_pBkImage->m_nTop;
		int src_right = m_pBkImage->m_nRight;
		int src_bottom = m_pBkImage->m_nBottom;

		int src_width = src_right - src_left +1;
		int src_height = src_bottom - src_top +1;

		//上半边
		m_pBkImage->Draw3splitter(dc.m_hDC, 
			rcDraw.left, 	rcDraw.top, 
			rcDraw.Width(), 38, 
			60,
			src_left, src_top, src_width, 38);

		////中间部分
		//m_pBkImage->Draw3splitter(dc.m_hDC, 
		//	rcDraw.left, 	rcDraw.top+38, 
		//	rcDraw.Width(), rcDraw.Height()-50, 
		//	60,
		//	src_left, src_top+38, src_width, 1);

		//下半部分
		m_pBkImage->Draw3splitter(dc.m_hDC, 
			rcDraw.left, 	rcDraw.bottom-12, 
			rcDraw.Width(), 12, 
			60,
			src_left, src_top+38, src_width, 12);
	}

	rcTmp.left = POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_LEFT(rcDraw);
	rcTmp.top = POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_TOP(rcDraw);
	rcTmp.right = POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_RIGHT(rcDraw);
	rcTmp.bottom = POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_BOTTOM(rcDraw);
	rcTmp.InflateRect(1, 1, 1, 1);
	dc.FillSolidRect(&rcTmp, RGB(98, 172, 235));
#else
	CBrush brush;
	brush.CreateSolidBrush(RGB(228,245,255));

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(42,163,216));

	HBRUSH hBrushOld = dc.SelectBrush(brush.m_hBrush);
	HPEN hPenOld = dc.SelectPen(pen.m_hPen);

	dc.Rectangle(rcDraw);

	//绘制标题栏
	if(m_pBkImage)
	{
		int src_left = m_pBkImage->m_nLeft;
		int src_top = m_pBkImage->m_nTop;
		int src_right = m_pBkImage->m_nRight;
		int src_bottom = m_pBkImage->m_nBottom;

		int src_width = src_right- src_left +1;
		int src_height = src_bottom - src_top +1;

		int cellheight= src_height/2;

		//上半边
		m_pBkImage->Draw3splitter(dc.m_hDC, 
			rcDraw.left, 	rcDraw.top, 
			rcDraw.Width(), cellheight, 
			3,
			src_left, src_top, src_width, cellheight);
	}

	dc.SelectBrush(hBrushOld);
	dc.SelectPen(hPenOld);
#endif

}

void CWndHeaderImagesView::ShowWnd(HWND hParentWnd, unsigned int nUserId)
{
	if (this->m_hWnd == NULL)
	{
		Create(hParentWnd, NULL, NULL, WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	}

	if (IsWindowVisible())
	{
		return;
	}

	CenterWindow(hParentWnd);
	ShowWindow(SW_SHOW);
}

void CWndHeaderImagesView::HideWnd()
{
	if (this->m_hWnd)
	{
		DestroyWindow();
		this->m_hWnd = NULL;
	}
}

int CWndHeaderImagesView::SetUrl(TCHAR *pszUrl)
{
	if (!m_wndIE.m_hWnd)
		return -1;

	CComPtr<IWebBrowser2> pWB2;
	HRESULT hr;

	hr = m_wndIE.QueryControl(&pWB2);

	if (pWB2)
	{
		CComVariant v;
		CComBSTR url(pszUrl);

		hr = pWB2->Navigate(url, &v, &v, &v, &v);

		if ( FAILED(hr))
		{
			MessageBox(_T("error"));
		}
	}

	return 0;
}

int CWndHeaderImagesView::_InitCtrls()
{
	CRect rcPos;
	memset(&rcPos, 0, sizeof(CRect));

#ifdef VER_9158KTV
	m_vbtnTab[0].Create(this->m_hWnd, rcPos, _T("男性"), VB_STYPE_RADIO, VB_ATTR_BMP|VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB0, 1);
	m_vbtnTab[0].SetBmp(20020);
	m_vbtnTab[0].SetTextColor(RGB(35, 67, 116));
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB0, &(m_vbtnTab[0]));
	m_vbtnTab[0].ModifyAttribute(0, VB_ATTR_CHECHED);

	m_vbtnTab[1].Create(this->m_hWnd, rcPos, _T("女性"), VB_STYPE_RADIO, VB_ATTR_BMP|VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB1, 1);
	m_vbtnTab[1].SetBmp(20020);
	m_vbtnTab[1].SetTextColor(RGB(35, 67, 116));
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB1, &(m_vbtnTab[1]));

	m_vbtnTab[2].Create(this->m_hWnd, rcPos, _T("生肖"), VB_STYPE_RADIO, VB_ATTR_BMP|VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB2, 1);
	m_vbtnTab[2].SetBmp(20020);
	m_vbtnTab[2].SetTextColor(RGB(35, 67, 116));
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB2, &(m_vbtnTab[2]));

	m_vbtnWndTitle.Create(this->m_hWnd, rcPos, _T("头像设置"), VB_STYPE_STATIC, VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_LEFT, ID_SEAL_WND_BTN_WND_TITLE);
	m_vbtnWndTitle.SetTextColor(RGB(0,58,108));
	m_vbtnWndTitle.SetFont(&_GlobalSetting.m_fontBig);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_WND_TITLE, &m_vbtnWndTitle);

	m_vbtnClose.Create(this->m_hWnd, rcPos, NULL/*_T("关闭")*/, VB_STYPE_BTN, VB_ATTR_NORMAL|VB_ATTR_BMP|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_CLOSE);
	m_vbtnClose.SetBmp(20015);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_CLOSE, &m_vbtnClose);
#else
	m_vbtnTab[0].Create(this->m_hWnd, rcPos, _T("男性"), VB_STYPE_CHECKBOX, VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB0);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB0, &(m_vbtnTab[0]));
	m_vbtnTab[0].ModifyAttribute(0, VB_ATTR_CHECHED);

	m_vbtnTab[1].Create(this->m_hWnd, rcPos, _T("女性"), VB_STYPE_CHECKBOX, VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB1);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB1, &(m_vbtnTab[1]));

	m_vbtnTab[2].Create(this->m_hWnd, rcPos, _T("生肖"), VB_STYPE_CHECKBOX, VB_ATTR_NORMAL|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_TAB2);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_TAB2, &(m_vbtnTab[2]));

	m_vbtnClose.Create(this->m_hWnd, rcPos, NULL/*_T("关闭")*/, VB_STYPE_BTN, VB_ATTR_NORMAL|VB_ATTR_BMP|VB_ATTR_VISIBLE|VB_ATTR_TEXT_CENTER, ID_HEADERIMAGES_WND_BTN_CLOSE);
	m_vbtnClose.SetBmp(20002);
	m_virBtns.AddVirBtn(ID_HEADERIMAGES_WND_BTN_CLOSE, &m_vbtnClose);
#endif
	return 0L;
}

int CWndHeaderImagesView::_InitIECtrl()
{
	m_strHeaderImagesUrl[0].Format(_T("%s\\imageheader\\0.html"), _GlobalSetting.m_szAppPathW);
	m_strHeaderImagesUrl[1].Format(_T("%s\\imageheader\\1.html"), _GlobalSetting.m_szAppPathW);
	m_strHeaderImagesUrl[2].Format(_T("%s\\imageheader\\2.html"), _GlobalSetting.m_szAppPathW);

	CRect rcPos;
	memset(&rcPos, 0, sizeof(CRect));

	HWND hWndWB = m_wndIE.Create(m_hWnd, rcPos, NULL, WS_CHILD|WS_VISIBLE, 0, ID_HEADERIMAGES_WND_IE_HEADERIMAGES_VIEW);

	ATLASSERT(hWndWB);

	CComPtr<IUnknown> punkCtrl;
	CComQIPtr<IWebBrowser2> pWB2;
	CComVariant v;

	HRESULT hr = m_wndIE.CreateControlEx(_T("Shell.Explorer"), NULL, NULL, &punkCtrl);

	if (FAILED(hr))
	{
		if (m_wndIE.m_hWnd)
			m_wndIE.DestroyWindow();
		return -1;
	}

	CComBSTR url(m_strHeaderImagesUrl[0].GetBuffer());
	pWB2 = punkCtrl;

	pWB2->Navigate(url, &v, &v, &v, &v);

	return 0L;
}

int CWndHeaderImagesView::_Layout()
{
	CRect rcClient;
	GetClientRect(&rcClient);
#ifdef VER_9158KTV
	m_vbtnTab[0].SetPos(POS_HEADERIMAGES_WND_BTN_TAB0_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_BOTTOM(rcClient));
	m_vbtnTab[1].SetPos(POS_HEADERIMAGES_WND_BTN_TAB1_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_BOTTOM(rcClient));
	m_vbtnTab[2].SetPos(POS_HEADERIMAGES_WND_BTN_TAB2_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_BOTTOM(rcClient));

	m_vbtnWndTitle.SetPos(POS_HEADERIMAGES_WND_BTN_WND_TITLE_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_BOTTOM(rcClient));
	m_vbtnClose.SetPos(POS_HEADERIMAGES_WND_BTN_CLOSE_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_BOTTOM(rcClient));

	SET_WND_POS(m_wndIE.m_hWnd, POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_LEFT(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_TOP(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_RIGHT(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_BOTTOM(rcClient))
#else
	m_vbtnTab[0].SetPos(POS_HEADERIMAGES_WND_BTN_TAB0_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB0_BOTTOM(rcClient));
	m_vbtnTab[1].SetPos(POS_HEADERIMAGES_WND_BTN_TAB1_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB1_BOTTOM(rcClient));
	m_vbtnTab[2].SetPos(POS_HEADERIMAGES_WND_BTN_TAB2_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB2_BOTTOM(rcClient));
	m_vbtnTab[3].SetPos(POS_HEADERIMAGES_WND_BTN_TAB3_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB3_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_TAB3_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_TAB3_BOTTOM(rcClient));

	m_vbtnWndTitle.SetPos(POS_HEADERIMAGES_WND_BTN_WND_TITLE_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_WND_TITLE_BOTTOM(rcClient));
	m_vbtnClose.SetPos(POS_HEADERIMAGES_WND_BTN_CLOSE_LEFT(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_TOP(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_RIGHT(rcClient), POS_HEADERIMAGES_WND_BTN_CLOSE_BOTTOM(rcClient));

	SET_WND_POS(m_wndIE.m_hWnd, POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_LEFT(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_TOP(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_RIGHT(rcClient), POS_HEADERIMGS_WND_IE_HEADERIMGS_VIEW_BOTTOM(rcClient))
#endif
		return 0L;
}

int CWndHeaderImagesView::_OnClickedBtnTab0()
{
	m_vbtnTab[0].ModifyAttribute(0, VB_ATTR_CHECHED);
	m_vbtnTab[1].ModifyAttribute(VB_ATTR_CHECHED, 0);
	m_vbtnTab[2].ModifyAttribute(VB_ATTR_CHECHED, 0);

	SetUrl(m_strHeaderImagesUrl[0].GetBuffer());

	InvalidateRect(NULL);

	return 0;
}

int CWndHeaderImagesView::_OnClickedBtnTab1()
{
	m_vbtnTab[1].ModifyAttribute(0, VB_ATTR_CHECHED);
	m_vbtnTab[0].ModifyAttribute(VB_ATTR_CHECHED, 0);
	m_vbtnTab[2].ModifyAttribute(VB_ATTR_CHECHED, 0);

	SetUrl(m_strHeaderImagesUrl[1].GetBuffer());

	InvalidateRect(NULL);

	return 0;
}

int CWndHeaderImagesView::_OnClickedBtnTab2()
{
	m_vbtnTab[2].ModifyAttribute(0, VB_ATTR_CHECHED);
	m_vbtnTab[0].ModifyAttribute(VB_ATTR_CHECHED, 0);
	m_vbtnTab[1].ModifyAttribute(VB_ATTR_CHECHED, 0);

	SetUrl(m_strHeaderImagesUrl[2].GetBuffer());

	InvalidateRect(NULL);

	return 0;
}

//以GET形式发送数据给网页端 
void CWndHeaderImagesView::UrlSendToServer(CString url)
{
	HINTERNET hSession = InternetOpen(_T("UrlSendToServer"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession!=NULL)
	{
		HINTERNET hHttp = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		InternetCloseHandle(hSession);
		hSession = NULL;
	}
}

int CWndHeaderImagesView::_OnClickedBtnClose()
{
	HideWnd();
	
	//以GET形式发送给网页端，处理保存头像信息到数据库的操作。 
	CString strSaveHeaderUrl;
	strSaveHeaderUrl.Format(_T("%s?userid=%d&headerid=%d"),_GlobalSetting.m_pGlobalEnviron->m_szHeaderPage, _GlobalSetting.m_localUser.m_nUserId, m_nCurSelUserId);		//头像修改URL  改成 读取配置项  By Jason
	_GlobalSetting.m_localUser.m_nHeadId = m_nCurSelUserId ; //头像ID 保存到全局变量中  By Jason
	UrlSendToServer(strSaveHeaderUrl);

	return 0;
}
