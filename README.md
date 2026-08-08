patheditor
==========
Edit PATH environment on Windows conveniently

The default environment editor is not great at editing PATH if there are lot of entries and many of them are similar looking.
* Implemented using Win32, **Does not depend on MFC or .NET**
* Missing folders are highlighted with different icon.
* To edit System PATH, offers to be launched with elevated admin privilege, similar to "Run as Administrator"
* Double clicking any path entry attempts to open in Windows Explorer.

![picture](http://farm6.staticflickr.com/5528/9216019339_ea67407001_z.jpg)

## Extended version features
* Window resizing
* Apply/Reload
* Insert after selected (Shift+Add)
* Clipboard operations
* Manual in-place edit
* Hotkeys:
  | Hotkeys            | Action            |
  |--------------------|-------------------|
  |       (Ctrl+Enter) | Commit and exit   |
  |      (Shift+Enter) | Commit            |
  |               (F5) | Reload            |
  |         (Ctrl+Tab) | Switch b/w Lists  |
  |                    |                   |
  |  (Ctrl+C Ctrl+Ins) | Copy item         |
  | (Ctrl+X Shift+Del) | Cut item          |
  | (Ctrl+V Shift+Ins) | Paste item        |
  |               (F2) | Manual edit       |
  |              (Ins) | Add item (Dialog) |
  |              (Del) | Remove item       |
  |          (Ctrl+Up) | Move item up      |
  |         (Ctr+Down) | Move item down    |
