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
