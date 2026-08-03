#pragma once

class CDlgCtrl {
protected:
    HWND m_hWnd;
    UINT m_x, m_y, m_w, m_h;
public:
    CDlgCtrl() {}
    CDlgCtrl(HWND hWnd) { Init(hWnd); }
    virtual void Init(HWND hWnd) {
        m_hWnd = hWnd;
        RECT r;
        ::GetWindowRect(hWnd, &r);
        ::MapWindowPoints(NULL, GetParent(hWnd), (LPPOINT)&r, 2);
        m_x = r.left;
        m_y = r.top;
        m_w = r.right - r.left;
        m_h = r.bottom - r.top;
    };
    virtual void Move(UINT x, UINT y) {
        m_x = x;
        m_y = y;
        ::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    };
    virtual void Resize(UINT w, UINT h) {
        m_w = w;
        m_h = h;
        ::SetWindowPos(m_hWnd, NULL, 0, 0, w, h, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
    };
    virtual void MoveAndResize(UINT x, UINT y, UINT w, UINT h) {
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;
        ::SetWindowPos(m_hWnd, NULL, x, y, w, h, SWP_NOACTIVATE|SWP_NOZORDER);
    };
	virtual void Show(int nCmdShow) { ::ShowWindow( m_hWnd, nCmdShow); }
	virtual void Enable(bool bEnable) { ::EnableWindow( m_hWnd, bEnable); }
	virtual bool IsSelected() { return ::GetFocus() == m_hWnd; }
    UINT x() { return m_x; }
    UINT y() { return m_y; }
    UINT w() { return m_w; }
    UINT h() { return m_h; }
};
