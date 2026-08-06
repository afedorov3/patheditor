#pragma once

class CDlgCtrl {
protected:
    HWND m_hWnd;
    LONG m_x, m_y, m_w, m_h;
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
    virtual void Move(LONG x, LONG y) {
        m_x = x;
        m_y = y;
        ::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    };
    virtual void Resize(LONG w, LONG h, bool redraw = false) {
        m_w = w;
        m_h = h;
        ::SetWindowPos(m_hWnd, NULL, 0, 0, w, h, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        if (redraw) Redraw();
    };
    virtual void MoveAndResize(LONG x, LONG y, LONG w, LONG h, bool redraw = false) {
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;
        ::SetWindowPos(m_hWnd, NULL, x, y, w, h, SWP_NOACTIVATE|SWP_NOZORDER);
        if (redraw) Redraw();
    };
    virtual void Show(int nCmdShow = SW_SHOW) { ::ShowWindow( m_hWnd, nCmdShow); }
    virtual void Enable(bool bEnable = true)  { ::EnableWindow( m_hWnd, bEnable); }
    virtual bool IsSelected() { return ::GetFocus() == m_hWnd; }
    virtual void Redraw() { RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE); }
    LONG x() { return m_x; }
    LONG y() { return m_y; }
    LONG w() { return m_w; }
    LONG h() { return m_h; }
    HWND hWnd() { return m_hWnd; }
};
