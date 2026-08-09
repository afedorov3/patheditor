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

#define NOMINMAX

#include <algorithm>

#include "PathListCtrl.h"
#include "Util.h"

int CPathListCtrl::_GetImageIndex( std::wstring fname)
{
    std::wstring pathName = wsExpandEnvironmentStrings(fname);
    return GetFileAttributes(pathName.c_str()) == INVALID_FILE_ATTRIBUTES ? 1 : 0;
}

bool CPathListCtrl::_LoadData()
{
    ListView_DeleteAllItems(m_hWnd);
    m_str_list.clear();
    m_modified = false;
    m_failed = !m_reader.Read( m_str_list);
    if (m_failed) return false;
    for( std::size_t count = 0; count < m_str_list.size(); ++count)
    {
        LVITEM lvItem{ };
        lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
        lvItem.iItem = static_cast<int>(count);
        lvItem.iImage = I_IMAGECALLBACK;
        lvItem.pszText = LPSTR_TEXTCALLBACK;
        ListView_InsertItem( m_hWnd, &lvItem);
    }

    return true;
}

void CPathListCtrl::_AdjustColumnWidth()
{
    ListView_SetColumnWidth(m_hWnd, 0, LVSCW_AUTOSIZE_USEHEADER);
}

void CPathListCtrl::Init( HWND hWnd, HIMAGELIST hImageList, HKEY hKey, LPCTSTR lpszKeyName, LPCTSTR lpszValueName)
{
    CDlgCtrl::Init(hWnd);
    ListView_SetImageList( m_hWnd, hImageList, LVSIL_SMALL);

    LVCOLUMN lvColumn{ };
    ListView_InsertColumn( m_hWnd, 0, &lvColumn);

    LONG_PTR dwStyle = GetWindowLongPtr(m_hWnd, GWL_STYLE);
    dwStyle |= LVS_EDITLABELS;
    SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyle);

    DWORD dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
    ListView_SetExtendedListViewStyle( m_hWnd, dwExStyle);

    // load data
    m_reader = CPathReader( hKey, lpszKeyName, lpszValueName);
    _LoadData();

    _AdjustColumnWidth();
}

bool CPathListCtrl::Reload()
{
    // save top position
    int top = ListView_GetTopIndex(m_hWnd);

    if (!_LoadData()) return false;

    // try to scroll saved position back to the top
    ListView_EnsureVisible(m_hWnd, top, FALSE);
    RECT rcItem;
    ListView_GetItemRect(m_hWnd, top, &rcItem, LVIR_BOUNDS);
    ListView_Scroll(m_hWnd, 0, rcItem.top);

    return true;
}

bool CPathListCtrl::Commit()
{
    StringListT strList;
    for( int i = 0; i < ListView_GetItemCount( m_hWnd); ++i)
    {
        int cchTextMax = MAX_PATH;
        std::wstring strValue(cchTextMax, 0);
        ListView_GetItemText(m_hWnd, i, 0, &strValue[0], cchTextMax);

        strValue.resize(strValue.find_first_of(L'\0'));
        strList.push_back(strValue);
    }
    m_failed = !m_reader.Write( strList);
    if (!m_failed) m_modified = false;
    return !m_failed;
}

void CPathListCtrl::AddPath(const std::wstring &strPath, bool insert)
{
    if (strPath.size() >= MAX_PATH) return;

    int iItem = insert ? ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED) : -1;
    if( iItem == -1)
    {
        iItem = ListView_GetItemCount( m_hWnd);
        m_str_list.push_back(strPath), m_modified = true;
    }
    else
    {
        iItem += 1;
        m_str_list.insert(std::next(m_str_list.begin(), iItem), strPath), m_modified = true;
    }

    LVITEM lvItem{ };
    lvItem.mask = LVIF_TEXT | LVIF_STATE;
    lvItem.iItem = iItem;
    lvItem.pszText = LPSTR_TEXTCALLBACK;
    ListView_InsertItem( m_hWnd, &lvItem);

    ListView_EnsureVisible( m_hWnd, iItem, FALSE);
    _AdjustColumnWidth();
}

void CPathListCtrl::AddPath(bool insert)
{
    std::wstring strPath;
    auto hRes = PickFolderDlg(GetAncestor(m_hWnd, GA_ROOT), strPath, FOS_FORCEFILESYSTEM);
    if( SUCCEEDED(hRes))
        AddPath(strPath, insert);
}

void CPathListCtrl::EditPath()
{
    int iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if( iItem == -1)
        return;

    std::wstring strPath = wsExpandEnvironmentStrings(m_str_list[iItem]);
    auto hRes = PickFolderDlg(GetAncestor(m_hWnd, GA_ROOT), strPath, FOS_FORCEFILESYSTEM, strPath, true);
    if( SUCCEEDED(hRes))
    {
        m_str_list[iItem] = strPath, m_modified = true;
        ListView_Update( m_hWnd, iItem);
    }
}

void CPathListCtrl::RemovePath()
{
    int iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if( iItem == -1)
        return;

    m_str_list.erase( std::find( m_str_list.begin(), m_str_list.end(), m_str_list[iItem])), m_modified = true;
    ListView_DeleteItem( m_hWnd, iItem);
    ListView_Update( m_hWnd, iItem);

    if( iItem == int(m_str_list.size()))
        iItem = iItem - 1;
    ListView_SetItemState( m_hWnd, iItem, LVNI_SELECTED, LVNI_SELECTED);

    _AdjustColumnWidth();
}

void CPathListCtrl::MoveUp()
{
    int iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if( iItem == -1 || iItem == 0)
        return;

    m_str_list[iItem].swap( m_str_list[iItem - 1]), m_modified = true;
    ListView_Update( m_hWnd, iItem);
    ListView_Update( m_hWnd, iItem - 1);
    ListView_SetItemState( m_hWnd, iItem - 1,
        LVNI_SELECTED | LVIS_FOCUSED, LVNI_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible( m_hWnd, iItem - 1, FALSE);
}

void CPathListCtrl::MoveDown()
{
    int iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if( iItem == -1 || ( iItem == int( m_str_list.size() - 1)))
        return;

    m_str_list[iItem].swap( m_str_list[iItem + 1]), m_modified = true;
    ListView_Update( m_hWnd, iItem);
    ListView_Update( m_hWnd, iItem + 1);
    ListView_SetItemState( m_hWnd, iItem + 1,
        LVNI_SELECTED | LVIS_FOCUSED, LVNI_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible( m_hWnd, iItem + 1, FALSE);
}

void CPathListCtrl::OnDoubleClick( LPNMITEMACTIVATE lpNMItemActivate)
{
    int iItem = lpNMItemActivate->iItem;
    if( iItem == -1)
        return;

    std::wstring pathName = wsExpandEnvironmentStrings(m_str_list[iItem]);
    if (GetFileAttributes(pathName.c_str()) == INVALID_FILE_ATTRIBUTES)
        return;
    ShellExecute(0, L"open", pathName.c_str(), 0, 0, SW_NORMAL);
}

void CPathListCtrl::OnGetdispinfo( NMLVDISPINFO *pDispInfo)
{
    if( pDispInfo->item.mask & LVIF_TEXT)
    {
        if( pDispInfo->item.iSubItem == 0)
        {
            std::wstring& sItem = m_str_list[pDispInfo->item.iItem];
            pDispInfo->item.pszText = &sItem[0];
        }
    }

    if( pDispInfo->item.mask & LVIF_IMAGE)
        pDispInfo->item.iImage = _GetImageIndex( m_str_list[pDispInfo->item.iItem]);
}

BOOL CPathListCtrl::OnBeginLabelEdit( NMLVDISPINFO *pDispInfo)
{
    UNREFERENCED_PARAMETER(pDispInfo);

    return FALSE;
}

BOOL CPathListCtrl::OnEndLabelEdit( NMLVDISPINFO *pDispInfo)
{
    if( pDispInfo->item.iSubItem == 0 && (pDispInfo->item.mask & LVIF_TEXT) && pDispInfo->item.pszText)
    {
        if (m_str_list[pDispInfo->item.iItem].compare(pDispInfo->item.pszText) != 0)
        {
            m_str_list[pDispInfo->item.iItem] = pDispInfo->item.pszText, m_modified = true;
            return TRUE;
        }
    }

    return FALSE;
}

void CPathListCtrl::SelectItem(int iItem, bool select)
{
    if (m_str_list.size() == 0) return;

    iItem = std::max(-1, std::min(iItem, int( m_str_list.size() - 1)));
    ListView_SetItemState( m_hWnd, iItem,
        select ? LVNI_SELECTED | LVIS_FOCUSED : 0, LVNI_SELECTED | LVIS_FOCUSED);
    if (iItem != -1)
        ListView_EnsureVisible( m_hWnd, iItem, FALSE);
}

std::wstring CPathListCtrl::GetItemPath(int iItem)
{
    std::wstring strItem;
    if (iItem < 0) iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if (iItem < 0) return strItem;

    int cchTextMax = MAX_PATH;
    strItem.resize(cchTextMax, 0);
    ListView_GetItemText(m_hWnd, iItem, 0, &strItem[0], cchTextMax);
    strItem.resize(strItem.find_first_of(L'\0'));

    return strItem;
}

void CPathListCtrl::EditItem(int iItem)
{
    if (iItem < 0) iItem = ListView_GetNextItem( m_hWnd, -1, LVNI_SELECTED);
    if (iItem < 0) return;

    ListView_EditLabel(m_hWnd, iItem);
}

void CPathListCtrl::FinishEditItem()
{
    ::SetFocus(m_hWnd);
    ListView_EditLabel(m_hWnd, -1);
}
