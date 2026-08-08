/* Copyright (c) 2013, Masoom Shaikh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "PathEditorDlg.h"
#include "resource.h"
#include "Util.h"
#include "DlgCtrl.hpp"

#include <algorithm>

#pragma comment(lib, "comctl32.lib")

CPathEditorDlg::CPathEditorDlg()
{
}

CPathEditorDlg::~CPathEditorDlg()
{
    DestroyIcon(m_hIcon);
    std::for_each( m_ButtonIcons.begin(), m_ButtonIcons.end(), DestroyIcon);
    ImageList_Destroy(m_hImageList);
}

// copied whole stock from atlwin.h
// trimmed ATL dependent macros, mostly asserts.
BOOL CPathEditorDlg::_CenterWindow()
{
    // determine owner window to center against
    DWORD dwStyle = (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE);
    HWND hWndCenter = NULL;
    if(dwStyle & WS_CHILD)
        hWndCenter = ::GetParent(m_hWnd);
    else
        hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    ::GetWindowRect(m_hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if(!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if(hWndCenter != NULL)
        {
            DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
            if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = NULL;
        }

        HMONITOR hMonitor = NULL;
        if(hWndCenter != NULL)
        {
            hMonitor = ::MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        }

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(hMonitor, &minfo);

        rcArea = minfo.rcWork;
        if(hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
        // center within parent client coordinates
        hWndParent = ::GetParent(m_hWnd);

        ::GetClientRect(hWndParent, &rcArea);
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if(xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;
    if(xLeft < rcArea.left)
        xLeft = rcArea.left;

    if(yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;
    if(yTop < rcArea.top)
        yTop = rcArea.top;

    // map screen coordinates to child coordinates
    return ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CPathEditorDlg::_CreateImageList()
{
    m_hImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 2);
    if( m_hImageList == NULL)
        return FALSE;

    HICON hIconExist = LoadIconW( m_hInstance, MAKEINTRESOURCE(IDI_ICON_EXIST));
    ImageList_ReplaceIcon( m_hImageList, -1, hIconExist);
    if( DeleteObject(hIconExist))
        return FALSE;

    HICON hIconNonExist = LoadIconW( m_hInstance, MAKEINTRESOURCE(IDI_ICON_NON_EXIST));
    ImageList_ReplaceIcon( m_hImageList, -1, hIconNonExist);
    if( DeleteObject(hIconNonExist))
        return FALSE;
    return TRUE;
}

BOOL CPathEditorDlg::_SetButtonIcons()
{
    HICON hAddIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_DIR_ADD), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_usrAddBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hAddIcon);
    SendMessage( m_sysAddBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hAddIcon);
    m_ButtonIcons.push_back(hAddIcon);

    HICON hDelIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_DIR_DEL), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_usrRemoveBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDelIcon);
    SendMessage( m_sysRemoveBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDelIcon);
    m_ButtonIcons.push_back(hDelIcon);

    HICON hEditIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_DIR_EDIT), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_usrEditBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hEditIcon);
    SendMessage( m_sysEditBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hEditIcon);
    m_ButtonIcons.push_back(hEditIcon);

    HICON hUpIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_DIR_UP), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_usrUpBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUpIcon);
    SendMessage( m_sysUpBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUpIcon);
    m_ButtonIcons.push_back(hUpIcon);

    HICON hDownIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_DIR_DOWN), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_usrDownBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDownIcon);
    SendMessage( m_sysDownBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDownIcon);
    m_ButtonIcons.push_back(hDownIcon);

    HICON hRefreshIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_ICON_REFRESH), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage( m_refreshBtn.hWnd(), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hRefreshIcon);
    m_ButtonIcons.push_back(hRefreshIcon);

    return TRUE;
}

BOOL CPathEditorDlg::_Reload()
{
    static LPCWSTR reqs[] = {
        NULL,
        L"User PATH was modified, OK to reload anyway.",
        L"System PATH was modified, OK to reload anyway.",
        L"Both User and System PATHs were modified, OK to reload anyway.",
    };
    LPCWSTR req = reqs[int(m_sysListCtrl.IsModified()) << 1 | int(m_usrListCtrl.IsModified())];
    if (req && MessageBox( m_hWnd, req, L"Path Editor: Uncommitted changes", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
        return FALSE;

    BOOL ret = TRUE;
    if (!m_usrListCtrl.Reload()) ret = FALSE;
    if (!m_sysListCtrl.Reload()) ret = FALSE;

    return ret;
}

BOOL CPathEditorDlg::_Commit()
{
    static LPCWSTR errs[] = {
        NULL,
        L"Failed to save User PATH",
        L"Failed to save System PATH",
        L"Failed to save both System and User PATHs",
    };

    bool usrOk = m_usrListCtrl.Commit();
    bool sysOk = !m_bIsAdmin || m_sysListCtrl.Commit();

    int err = !sysOk << 1 | !usrOk;
    if (err)
        MessageBox( m_hWnd, errs[err], L"Path Editor", MB_OK);

    // broadcast path change messages to interested parties
    if (usrOk || (m_bIsAdmin && sysOk))
    {
        DWORD_PTR dwResult = 0;
        SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 1, &dwResult);
    }
    return !err;
}

void CPathEditorDlg::_StatusMessage(LPCWSTR Text, DWORD Style, UINT Timeout)
{
    constexpr DWORD STYLE_MASK = (SS_LEFT|SS_CENTER|SS_RIGHT);

    // handle multiline vertical alignment
    LONG_PTR dwStyle = ::GetWindowLongPtr(m_statusLbl.hWnd(), GWL_STYLE);
    if (wcschr(Text, L'\n'))
        dwStyle &= ~SS_CENTERIMAGE;
    else
        dwStyle |= SS_CENTERIMAGE;
    dwStyle = (dwStyle & ~STYLE_MASK) | (Style & STYLE_MASK);
    ::SetWindowLongPtr(m_statusLbl.hWnd(), GWL_STYLE, dwStyle);

    ::SetWindowText(m_statusLbl.hWnd(), Text);
    m_statusLbl.Show(SW_SHOW);
    ::SetTimer(m_hWnd, TIMERID_STATUS, Timeout, _TimerProc);
}

template <typename CbT, typename... Args>
bool CPathEditorDlg::_ListViewDispatch(CbT&& memberCallback, Args&&... args) {
    if (m_usrListCtrl.IsFocused())
        std::invoke(std::forward<CbT>(memberCallback), *this, m_usrListCtrl, std::forward<Args>(args)...);
    else if (m_sysListCtrl.IsFocused())
        std::invoke(std::forward<CbT>(memberCallback), *this, m_sysListCtrl, std::forward<Args>(args)...);
    else
        return false;

    return true;
}

void CPathEditorDlg::_TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dwTime);

    ::KillTimer(hWnd, idEvent);
    if (idEvent == TIMERID_STATUS) {
        auto PathEditorDlg = reinterpret_cast<CPathEditorDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (PathEditorDlg)
            PathEditorDlg->m_statusLbl.Show(SW_HIDE);
    }
}

BOOL CPathEditorDlg::OnInitDialog( HINSTANCE hInstance, HWND hWnd)
{
    // save interesting handles
    m_hWnd = hWnd;
    m_hInstance = hInstance;
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<ULONG_PTR>(this));

    m_okBtn       .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_OK));
    m_cancelBtn   .Init( ::GetDlgItem(m_hWnd, IDCANCEL));
    m_gainPrivBtn .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_GAIN_PRIVILEGE));
    m_applyBtn    .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_APPLY));
    m_refreshBtn  .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_REFRESH));
    m_statusLbl   .Init( ::GetDlgItem(m_hWnd, IDC_STATIC_STATUS));

    m_usrGroup    .Init( ::GetDlgItem(m_hWnd, IDC_STATIC_USER));
    m_usrAddBtn   .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_USER_ADD));
    m_usrEditBtn  .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_USER_EDIT));
    m_usrRemoveBtn.Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_USER_REMOVE));
    m_usrUpBtn    .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_USER_UP));
    m_usrDownBtn  .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_USER_DOWN));

    m_sysGroup    .Init( ::GetDlgItem(m_hWnd, IDC_STATIC_SYSTEM));
    m_sysAddBtn   .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_SYSTEM_ADD));
    m_sysEditBtn  .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_SYSTEM_EDIT));
    m_sysRemoveBtn.Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_SYSTEM_REMOVE));
    m_sysUpBtn    .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_SYSTEM_UP));
    m_sysDownBtn  .Init( ::GetDlgItem(m_hWnd, IDC_BUTTON_SYSTEM_DOWN));

    // set application icon
    m_hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
    SendMessage(m_hWnd, WM_SETICON, TRUE, (LPARAM)m_hIcon);
    SendMessage(m_hWnd, WM_SETICON, FALSE, (LPARAM)m_hIcon);

    if( _SetButtonIcons() == FALSE)
        return FALSE;
    if( _CreateImageList() == FALSE)
        return FALSE;

    m_bIsAdmin = IsProcessAdmin( ::GetCurrentProcess());
    m_usrListCtrl.Init( ::GetDlgItem(m_hWnd, IDC_LIST_USER), m_hImageList,
        HKEY_CURRENT_USER, L"Environment", L"Path");
    if(m_bIsAdmin)
    {
        m_gainPrivBtn.Show(SW_HIDE);
        m_sysListCtrl.Init( ::GetDlgItem( m_hWnd, IDC_LIST_SYSTEM), m_hImageList,
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
            L"Path");
    }
    else
    {
        m_sysListCtrl.Init( ::GetDlgItem( m_hWnd, IDC_LIST_SYSTEM));

        m_sysListCtrl.Show(SW_HIDE);
        m_gainPrivBtn.Show(SW_SHOW);
        Button_SetElevationRequiredState( m_gainPrivBtn.hWnd(), TRUE);

        m_sysListCtrl .Enable(FALSE);
        m_sysUpBtn    .Enable(FALSE);
        m_sysDownBtn  .Enable(FALSE);
        m_sysAddBtn   .Enable(FALSE);
        m_sysRemoveBtn.Enable(FALSE);
        m_sysEditBtn  .Enable(FALSE);
    }

    // initial resize
    RECT rcArea;
    ::GetWindowRect(hWnd, &rcArea);
    int width  = rcArea.right  - rcArea.left;
    int height = rcArea.bottom - rcArea.top;
    // ensure window is within limits
    if (width  < m_sizeLimits.left) width  = m_sizeLimits.left;
    if (height < m_sizeLimits.top)  height = m_sizeLimits.top;
    ::SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);

    if( _CenterWindow() == FALSE)
        return FALSE;

    m_usrListCtrl.Focus();
    m_usrListCtrl.SelectItem(0);
    m_sysListCtrl.SelectItem(0);

    return TRUE;
}

BOOL CPathEditorDlg::OnMinMaxInfo(LPMINMAXINFO mmi)
{
    mmi->ptMinTrackSize.x = m_sizeLimits.left;
    mmi->ptMinTrackSize.y = m_sizeLimits.top;

    return TRUE;
}

BOOL CPathEditorDlg::OnSize(UINT uType, UINT width, UINT height)
{
    UNREFERENCED_PARAMETER(uType);

    // order is important
    m_okBtn.Move((width - m_okBtn.w() - m_applyBtn.w() - m_cancelBtn.w() - DLG_BTN_SPACING * 2) / 2, height - MARGIN - m_okBtn.h());
    m_applyBtn.Move(m_okBtn.x() + m_okBtn.w() + DLG_BTN_SPACING, m_okBtn.y());
    m_cancelBtn.Move(m_applyBtn.x() + m_applyBtn.w() + DLG_BTN_SPACING, m_okBtn.y());

    m_usrGroup.MoveAndResize(MARGIN, MARGIN, width - MARGIN * 2, (m_okBtn.y() - MARGIN) / 2 - MARGIN);
    m_usrAddBtn.Move(m_usrGroup.x() + EDIT_BTN_POS, m_usrGroup.y() + m_usrGroup.h() - m_usrAddBtn.h() - MARGIN);
    m_usrEditBtn.Move(m_usrAddBtn.x() + m_usrAddBtn.w() + EDIT_BTN_SPASING, m_usrAddBtn.y());
    m_usrRemoveBtn.Move(m_usrEditBtn.x() + m_usrEditBtn.w() + EDIT_BTN_SPASING, m_usrAddBtn.y());
    m_usrUpBtn.Move(m_usrRemoveBtn.x() + m_usrRemoveBtn.w() + MOVE_BTN_POS, m_usrAddBtn.y());
    m_usrDownBtn.Move(m_usrUpBtn.x() + m_usrUpBtn.w() + EDIT_BTN_SPASING, m_usrAddBtn.y());
    m_usrListCtrl.MoveAndResize(m_usrGroup.x() + MARGIN, m_usrGroup.y() + LIST_MARGIN, m_usrGroup.w() - MARGIN * 2, m_usrGroup.h() - m_usrAddBtn.h() - LIST_MARGIN - MARGIN * 2);

    m_sysGroup.MoveAndResize(m_usrGroup.x(), m_usrGroup.y() + m_usrGroup.h() + MARGIN, m_usrGroup.w(), m_usrGroup.h());
    m_sysAddBtn.Move(m_usrAddBtn.x(), m_sysGroup.y() + m_sysGroup.h() - m_sysAddBtn.h() - MARGIN);
    m_sysEditBtn.Move(m_usrEditBtn.x(), m_sysAddBtn.y());
    m_sysRemoveBtn.Move(m_usrRemoveBtn.x(), m_sysAddBtn.y());
    m_sysUpBtn.Move(m_usrUpBtn.x(), m_sysAddBtn.y());
    m_sysDownBtn.Move(m_usrDownBtn.x(), m_sysAddBtn.y());
    m_sysListCtrl.MoveAndResize(m_usrListCtrl.x(), m_sysGroup.y() + LIST_MARGIN, m_usrListCtrl.w(), m_usrListCtrl.h());

    m_refreshBtn.Move(m_sysGroup.x(), m_okBtn.y());
    m_statusLbl.MoveAndResize(m_refreshBtn.x() + m_refreshBtn.w() + MARGIN, m_okBtn.y() - MARGIN / 4, m_okBtn.x() - m_refreshBtn.x() - m_refreshBtn.w() - MARGIN * 2, m_okBtn.h() + MARGIN / 2, true);
    m_gainPrivBtn.Move((width - m_gainPrivBtn.w()) / 2, m_sysListCtrl.y() + (m_sysListCtrl.h() - m_gainPrivBtn.h()) / 2 - MARGIN);

    return TRUE;
}

void CPathEditorDlg::OnButtonGainPrivilege()
{
    std::wstring strBuffer(MAX_PATH, 0);
    if (0 == GetModuleFileName(0, &strBuffer[0], static_cast<DWORD>(strBuffer.size())))
        return;

    SHELLEXECUTEINFO exInfo{ };
    exInfo.cbSize = sizeof(exInfo);
    exInfo.lpVerb = L"runas";
    exInfo.lpFile = strBuffer.c_str();
    exInfo.nShow = SW_SHOW;
    if( TRUE == ShellExecuteEx( &exInfo))
        SendMessage( m_hWnd, WM_CLOSE, 0, 0);
}

BOOL CPathEditorDlg::OnCommand( UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);

    WORD uCode = HIWORD(wParam);
    WORD uId   = LOWORD(wParam);

    /* Control specific commands */
    if (uCode > 1)
    {
        return FALSE;
    }

    // Menu or Accelerator
    switch(uId)
    {
    case IDC_BUTTON_USER_UP:
        m_usrListCtrl.MoveUp();
        break;
    case IDC_BUTTON_USER_DOWN:
        m_usrListCtrl.MoveDown();
        break;
    case IDC_BUTTON_USER_ADD:
        m_usrListCtrl.AddPath((GetKeyState(VK_SHIFT) & 0x80) != 0);
        break;
    case IDC_BUTTON_USER_REMOVE:
        m_usrListCtrl.RemovePath();
        break;
    case IDC_BUTTON_USER_EDIT:
        m_usrListCtrl.EditPath();
        break;
    case IDC_BUTTON_SYSTEM_UP:
        m_sysListCtrl.MoveUp();
        break;
    case IDC_BUTTON_SYSTEM_DOWN:
        m_sysListCtrl.MoveDown();
        break;
    case IDC_BUTTON_SYSTEM_ADD:
        m_sysListCtrl.AddPath((GetKeyState(VK_SHIFT) & 0x80) != 0);
        break;
    case IDC_BUTTON_SYSTEM_REMOVE:
        m_sysListCtrl.RemovePath();
        break;
    case IDC_BUTTON_SYSTEM_EDIT:
        m_sysListCtrl.EditPath();
        break;
    case IDC_BUTTON_GAIN_PRIVILEGE:
        OnButtonGainPrivilege();
        break;
    case ID_ACC_COPY_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { Str2Clipboard(lv.GetItemPath()); });
        break;
    case ID_ACC_CUT_ITEM:
        m_accHit = _ListViewDispatch(&CPathEditorDlg::OnCut);
        break;
    case ID_ACC_PASTE_ITEM:
        m_accHit = _ListViewDispatch(&CPathEditorDlg::OnPaste);
        break;
    case ID_ACC_EDIT_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { lv.EditItem(); });
        break;
    case ID_ACC_INSERT_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { lv.AddPath(true); });
        break;
    case ID_ACC_DELETE_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { lv.RemovePath(); });
        break;
    case ID_ACC_MOVEUP_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { lv.MoveUp(); });
        break;
    case ID_ACC_MOVEDN_ITEM:
        m_accHit = _ListViewDispatch([](auto&, auto &lv) { lv.MoveDown(); });
        break;
    case ID_ACC_APPLY:
        m_accHit = TRUE;
    case IDC_BUTTON_APPLY:
        if (_Commit()) _StatusMessage(L"Applied", SS_RIGHT);
        break;
    case ID_ACC_REFRESH:
        m_accHit = TRUE;
    case IDC_BUTTON_REFRESH:
        if (_Reload()) _StatusMessage(L"Reloaded", SS_LEFT);
        break;
    case ID_ACC_LIST_TOGGLE:
        if (m_usrListCtrl.IsFocused()) m_sysListCtrl.Focus(); else m_usrListCtrl.Focus();
        m_accHit = TRUE;
        break;
    case IDOK:
        _StatusMessage(L"Ctrl-Enter\r\nto save and exit", SS_RIGHT);
        break;
    case ID_ACC_OK:
        m_usrListCtrl.FinishEditItem();
        m_sysListCtrl.FinishEditItem();
        m_accHit = TRUE;
        /* fall through */
    case IDC_BUTTON_OK:
        if (!OnOK()) break;
        /* fall through */
    case IDCANCEL:
        PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    default:
        /* not handled */
        return FALSE;
    }
    return TRUE;
}

BOOL CPathEditorDlg::OnNotify( LPNMHDR lpNMHDR)
{
    switch(lpNMHDR->code)
    {
    case NM_DBLCLK:
        OnListDoubleClick(reinterpret_cast<LPNMITEMACTIVATE>(lpNMHDR));
        break;
    case LVN_GETDISPINFO:
        OnListGetDispInfo(reinterpret_cast<NMLVDISPINFO*>(lpNMHDR));
        break;
    case LVN_BEGINLABELEDIT:
        return OnListBeginLabelEdit(reinterpret_cast<NMLVDISPINFO*>(lpNMHDR));
    case LVN_ENDLABELEDIT:
        return OnListEndLabelEdit(reinterpret_cast<NMLVDISPINFO*>(lpNMHDR));
    }
    return TRUE;
}

void CPathEditorDlg::OnListGetDispInfo(NMLVDISPINFO *pDispInfo)
{
    switch(pDispInfo->hdr.idFrom)
    {
    case IDC_LIST_USER:
        m_usrListCtrl.OnGetdispinfo(pDispInfo);
        break;
    case IDC_LIST_SYSTEM:
        m_sysListCtrl.OnGetdispinfo(pDispInfo);
        break;
    }
}

void CPathEditorDlg::OnListDoubleClick(LPNMITEMACTIVATE lpNMItemActivate)
{
    switch(lpNMItemActivate->hdr.idFrom)
    {
    case IDC_LIST_USER:
        m_usrListCtrl.OnDoubleClick(lpNMItemActivate);
        break;
    case IDC_LIST_SYSTEM:
        m_sysListCtrl.OnDoubleClick(lpNMItemActivate);
        break;
    }
}

BOOL CPathEditorDlg::OnListBeginLabelEdit(NMLVDISPINFO *pDispInfo)
{
    switch(pDispInfo->hdr.idFrom)
    {
    case IDC_LIST_USER:
        return m_usrListCtrl.OnBeginLabelEdit(pDispInfo);
    case IDC_LIST_SYSTEM:
        return m_sysListCtrl.OnBeginLabelEdit(pDispInfo);
    }
    return TRUE;
}

BOOL CPathEditorDlg::OnListEndLabelEdit(NMLVDISPINFO *pDispInfo)
{
    switch(pDispInfo->hdr.idFrom)
    {
    case IDC_LIST_USER:
        return m_usrListCtrl.OnEndLabelEdit(pDispInfo);
    case IDC_LIST_SYSTEM:
        return m_sysListCtrl.OnEndLabelEdit(pDispInfo);
    }
    return FALSE;
}

BOOL CPathEditorDlg::OnOK()
{
    return _Commit();
}

BOOL CPathEditorDlg::OnClose()
{
    static LPCWSTR reqs[] = {
        NULL,
        L"User PATH was modified, OK to exit anyway.",
        L"System PATH was modified, OK to exit anyway.",
        L"Both User and System PATHs were modified, OK to exit anyway.",
    };
    LPCWSTR req = reqs[int(m_sysListCtrl.IsModified()) << 1 | int(m_usrListCtrl.IsModified())];
    if (req && MessageBox( m_hWnd, req, L"Path Editor: Uncommitted changes", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
        return FALSE;

    return TRUE;
}

void CPathEditorDlg::OnCut(CPathListCtrl &ListCtrl)
{
    auto Item = ListCtrl.GetItemPath();
    if (Item.empty()) return;
    if (Str2Clipboard(Item))
        ListCtrl.RemovePath();
}

void CPathEditorDlg::OnPaste(CPathListCtrl &ListCtrl)
{
    std::wstring Str;
    if (!Clipboard2Str(Str)) return;

    size_t nlPos = Str.find_first_of(L"\r\n\0");
    if (nlPos != std::wstring::npos)
        Str.resize(nlPos);

    if (Str.size() >= MAX_PATH)
    {
        MessageBox(m_hWnd, L"The path you're trying to paste is too long", L"Path Editor", MB_ICONERROR | MB_OK);
        return;
    }
    if (!IsAbsoluteLocalPathValid(Str))
    {
        std::wstring req(L"The path doesn't seem to be a valid absolute local path:\n");
        req += Str;
        req += L"\nOK to add it anyway.";
        if (MessageBox(m_hWnd, req.c_str(), L"Path Editor", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
            return;
    }

    ListCtrl.AddPath(Str, true);
}
