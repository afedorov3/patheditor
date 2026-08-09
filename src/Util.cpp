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

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "Util.h"

BOOL _CheckTokenMembership( HANDLE hToken, PSID pAdminSID, PBOOL pIsMember)
{
    DWORD dwSize = 0;
    if( FALSE == ::GetTokenInformation( hToken, TokenGroups, NULL, dwSize, &dwSize))
    {
        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return FALSE;
    }

    std::vector<BYTE> pBuffer(dwSize);
    PTOKEN_GROUPS pGroupInfo = reinterpret_cast<PTOKEN_GROUPS>( pBuffer.data());
    if( FALSE == ::GetTokenInformation( hToken, TokenGroups, pGroupInfo, dwSize, &dwSize)) 
        return FALSE;

    for( DWORD i = 0; i < pGroupInfo->GroupCount; i++)
    {
        if( ::EqualSid( pAdminSID, pGroupInfo->Groups[i].Sid))
        {
            if( pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED)
            {
                *pIsMember = TRUE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool IsProcessAdmin( HANDLE hProcess)
{
    HANDLE hToken;
    if( FALSE == ::OpenProcessToken( hProcess, TOKEN_QUERY, &hToken))
        return false;
    std::shared_ptr<void> afHandle( hToken, ::CloseHandle);

    PSID pAdminSID = 0;
    SID_IDENTIFIER_AUTHORITY sidAuthority = SECURITY_NT_AUTHORITY;
    if( FALSE == ::AllocateAndInitializeSid(
        &sidAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdminSID))
        return false;
    std::shared_ptr<void> afFreeSID( pAdminSID, ::FreeSid);

    BOOL isMember = FALSE;
    if( FALSE == _CheckTokenMembership( hToken, pAdminSID, &isMember))
        return false;
    return isMember == TRUE;
}

std::wstring wsExpandEnvironmentStrings(const std::wstring& sVar)
{
    if (std::wstring::npos == sVar.find(L'%'))
        return sVar;

    DWORD dwLen = 0;
    dwLen = ExpandEnvironmentStrings(sVar.c_str(), nullptr, dwLen);
    if (dwLen == 0)
        return L"";

    std::wstring strValue(dwLen, 0);
    dwLen = ExpandEnvironmentStrings(sVar.c_str(), &strValue[0], dwLen);
    if (dwLen == 0)
        return L"";

    strValue.resize(strValue.find_first_of(L'\0'));
    return strValue;
}

bool Str2Clipboard( const std::wstring &Str)
{
    SIZE_T iLen = wcslen(Str.c_str());
    if (iLen == 0) return false;

    ::SetLastError(ERROR_SUCCESS);
    if( !::OpenClipboard(NULL)) return false;
    do {
        iLen += 1;
        HGLOBAL hMem =  ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, iLen * sizeof(WCHAR));
        if (hMem == NULL) break;
        LPWSTR clipStr = static_cast<LPWSTR>(::GlobalLock(hMem));
        if (clipStr == NULL) break;
        wcscpy_s(clipStr, iLen, Str.c_str());
        ::GlobalUnlock(hMem);
        if (!::EmptyClipboard()) break;
        if (::SetClipboardData(CF_UNICODETEXT, hMem) == NULL) break;
        ::CloseClipboard();

        return true;
    } while (0);

    DWORD err = ::GetLastError();
    ::CloseClipboard();
    ::SetLastError(err);

    return false;
}

bool Clipboard2Str( std::wstring &Str)
{
    Str.clear();
    ::SetLastError(ERROR_SUCCESS);
    if (!::IsClipboardFormatAvailable(CF_UNICODETEXT)) return false;
    if (!::OpenClipboard(NULL)) return false;
    do {
        HGLOBAL hMem = ::GetClipboardData(CF_UNICODETEXT);
        if (hMem == NULL) break;
        LPWSTR clipStr = static_cast<LPWSTR>(::GlobalLock(hMem));
        if (clipStr == NULL) break;
        Str = clipStr;
        ::GlobalUnlock(hMem);
        ::CloseClipboard();

        return true;
    } while (0);

    DWORD err = ::GetLastError();
    ::CloseClipboard();
    ::SetLastError(err);

    return false;
}

// https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file
bool IsAbsoluteLocalPathValid( const std::wstring &Path)
{
    static auto is_sep = [](wchar_t ch) { return ch == L'\\' || ch == L'/'; };
    static auto is_dot = [](wchar_t ch) { return ch == L'.'; };
    static auto is_elem_last_char = []
        (std::wstring::const_iterator cur, std::wstring::const_iterator end) {
        return ++cur == end || *cur == L'\0' || is_sep(*cur);
    };

    auto it = Path.cbegin(), end = Path.cend();
    // skip namespace if any
    if (Path.rfind(L"\\\\?\\", 0) == 0 || Path.rfind(L"\\\\.\\", 0) == 0)
        it += 4;
    size_t PathLen = end - it;
    // should not be less than root of a drive or greater than MAX_PATH
    if (PathLen < 3 || PathLen > MAX_PATH) return false;
    // must start with a drive leter, colon, separator
    if (!iswalpha(*it++) || *it++ != L':' || !is_sep(*it)) return false;

    for (auto sep = it++; it != end && *it; it++)
    {
        if (*it < 32) return false; // control character
        switch (*it) {
        case L':':
        case L'*':
        case L'?':
        case L'>':
        case L'<':
        case L'"':
        case L'|': return false;    // special character
        case L'\\':
        case L'/':
            sep = it;
            break;
        case L' ':
        case L'.':
            if (is_elem_last_char(it, end) &&
                !(it - sep <= 2 && std::all_of(sep + 1, it + 1, is_dot)))
                return false;       // element ends with space or dot and not is . or ..
            break;
        }
    }

    return true;
}

#ifdef PICKFOLDER_FALLBACK
#include <shlobj_core.h>

#pragma comment(lib, "ComDlg32.Lib")

static int CALLBACK _BrowseForFolderCbProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    UNREFERENCED_PARAMETER(lParam);

    if (uMsg == BFFM_INITIALIZED && lpData != 0)
        SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
    return 0;
}

static HRESULT _PickFolderDlgCompat(HWND Owner, std::wstring &Path, FILEOPENDIALOGOPTIONS Options,
                  std::wstring const &InitialFolder)
{
    BROWSEINFO bi{ };
    bi.hwndOwner = Owner;
    bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
    if (Options & FOS_FORCEFILESYSTEM)
        bi.ulFlags |= BIF_RETURNONLYFSDIRS;

    if (!InitialFolder.empty()) {
        bi.lpfn = _BrowseForFolderCbProc;
        bi.lParam = reinterpret_cast<LPARAM>(InitialFolder.c_str());
    }

    PIDLIST_ABSOLUTE strList = SHBrowseForFolder(&bi);
    if (strList == NULL) return HRESULT_FROM_WIN32(ERROR_CANCELLED);

    HRESULT hRes = S_OK;
    Path.resize(MAX_PATH, L'\0');
    if (!SHGetPathFromIDList(strList, &Path[0])) hRes = E_FAIL;
    Path.resize(Path.find_first_of(L'\0'));
    CoTaskMemFree(strList);

    return hRes;
}
#endif // PICKFOLDER_FALLBACK

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Ole32.lib")

HRESULT PickFolderDlg(HWND Owner, std::wstring &Path, FILEOPENDIALOGOPTIONS Options,
                  std::wstring const &DefFolder, bool ForceDefFolder)
{
    HRESULT hRes;
    IFileOpenDialog *pFileDlg;

    hRes = CoCreateInstance(CLSID_FileOpenDialog, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDlg));
    if (!SUCCEEDED(hRes))
    {
#ifdef PICKFOLDER_FALLBACK
        hRes = _PickFolderDlgCompat(Owner, Path, Options, ForceDefFolder ? DefFolder : std::wstring {});
#endif // PICKFOLDER_FALLBACK
        return hRes;
    }

    do
    {
        Options &= ~FOS_ALLOWMULTISELECT;
        FILEOPENDIALOGOPTIONS fosOptions = 0;
        pFileDlg->GetOptions(&fosOptions);
        fosOptions |= FOS_PICKFOLDERS | Options;
        hRes = pFileDlg->SetOptions(fosOptions);
        if (!SUCCEEDED(hRes)) break;

        if (!DefFolder.empty())
        {
            IShellItem *pItem = nullptr;
            SHCreateItemFromParsingName(DefFolder.c_str(), nullptr, IID_PPV_ARGS(&pItem));
            if (pItem)
            {
                if (ForceDefFolder)
                    pFileDlg->SetFolder(pItem);
                else
                    pFileDlg->SetDefaultFolder(pItem);

                pItem->Release();
            } // Don't fail if the initial folder couldn't be set 
        }

        hRes = pFileDlg->Show(Owner);
        if (!SUCCEEDED(hRes)) break;

        IShellItem *pItem;
        hRes = pFileDlg->GetResult(&pItem);
        if (!SUCCEEDED(hRes)) break;

        SIGDN sigdnName = (fosOptions & FOS_FORCEFILESYSTEM) != 0 ?
                          SIGDN_FILESYSPATH : SIGDN_DESKTOPABSOLUTEPARSING;
        LPWSTR strName;
        hRes = pItem->GetDisplayName(sigdnName, &strName);
        if (SUCCEEDED(hRes))
        {
            Path = strName;
            CoTaskMemFree(strName);
        }
        pItem->Release();
    } while(false);
    pFileDlg->Release();

    return hRes;
}
