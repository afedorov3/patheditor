SHELL := cmd
.SHELLFLAGS := /c

CC = @cl.exe -nologo
CFLAGS = -c -EHsc -W4 -WX- -O2 -Oi -GL -Gm- -EHsc -GS -Gy -fp:precise -Zc:wchar_t -Zc:forScope -Gd -errorReport:queue
DEFINES = -DUNICODE -D_UNICODE -D WIN32 -D NDEBUG -D _WINDOWS -DPICKFOLDER_FALLBACK

LD = @link.exe -nologo
LDFLAGS = -LTCG

RC = @rc.exe -nologo
RCFLAGS = -l 0x0409 -DUNICODE -D_UNICODE

STDLIBS = kernel32.lib user32.lib advapi32.lib gdi32.lib shell32.lib

SRCDIR = ../src
BUILDDIR = build

PROJECT = PathEditor

SRCS = $(SRCDIR)/PathEditor.cpp \
       $(SRCDIR)/PathEditorDlg.cpp \
       $(SRCDIR)/PathListCtrl.cpp \
       $(SRCDIR)/PathReader.cpp \
       $(SRCDIR)/Util.cpp
INCLUDES = $(SRCDIR)/DlgCtrl.hpp \
           $(SRCDIR)/PathEditorDlg.h \
           $(SRCDIR)/PathListCtrl.h \
           $(SRCDIR)/PathReader.h \
           $(SRCDIR)/resource.h \
           $(SRCDIR)/Util.h
RCRC = $(SRCDIR)/$(PROJECT).rc

OBJS = $(addprefix $(BUILDDIR)/,$(addsuffix .obj, $(notdir $(basename $(SRCS)))))
RCRES = $(BUILDDIR)/$(PROJECT).res
MANIFEST = $(SRCDIR)/$(PROJECT).manifest

TARGET = $(BUILDDIR)/$(PROJECT).exe

all: $(TARGET)

$(RCRES): $(RCRC) | $(BUILDDIR)
	$(RC) $(RCFLAGS) /Fo$@ $<

clean:
	@if exist $(BUILDDIR) rmdir /S /Q $(BUILDDIR)

$(TARGET): $(OBJS) $(RCRES) $(MANIFEST)
	$(LD) $(LDFLAGS) $(STDLIBS) $(OBJS) $(RCRES) /manifest:embed /manifestinput:$(MANIFEST) /out:$@

$(BUILDDIR)/%.obj: $(SRCDIR)/%.cpp $(INCLUDES) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(DEFINES) /Fo$@ $<

$(BUILDDIR):
	@mkdir $@
