# Microsoft Developer Studio Generated NMAKE File, Based on spiany.dsp
!IF "$(CFG)" == ""
CFG=spiany - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの spiany - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "spiany - Win32 Release" && "$(CFG)" != "spiany - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "spiany.mak" CFG="spiany - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "spiany - Win32 Release" ("Win32 (x86) Console Application" 用)
!MESSAGE "spiany - Win32 Debug" ("Win32 (x86) Console Application" 用)
!MESSAGE 
!ERROR 無効な構成が指定されています。
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "spiany - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\spiany.exe"

!ELSE 

ALL : "spiif_rev6 - Win32 Release" "$(OUTDIR)\spiany.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\spiany.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\spiany.exe"
	-@erase "$(OUTDIR)\spiany.map"
	-@erase "$(OUTDIR)\spiany.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /Zi /O2 /Oy /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\spiany.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\spiany.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\spiany.pdb" /map:"$(INTDIR)\spiany.map" /debug /machine:I386 /out:"$(OUTDIR)\spiany.exe" 
LINK32_OBJS= \
	"$(INTDIR)\spiany.obj" \
	"..\Release\spiif_rev6.lib"

"$(OUTDIR)\spiany.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "spiany - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\spiany.exe"

!ELSE 

ALL : "spiif_rev6 - Win32 Debug" "$(OUTDIR)\spiany.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\spiany.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\spiany.exe"
	-@erase "$(OUTDIR)\spiany.ilk"
	-@erase "$(OUTDIR)\spiany.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\spiany.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\spiany.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\spiany.pdb" /debug /machine:I386 /out:"$(OUTDIR)\spiany.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\spiany.obj" \
	"..\Debug\spiif_rev6.lib"

"$(OUTDIR)\spiany.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("spiany.dep")
!INCLUDE "spiany.dep"
!ELSE 
!MESSAGE Warning: cannot find "spiany.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "spiany - Win32 Release" || "$(CFG)" == "spiany - Win32 Debug"
SOURCE=.\spiany.cpp

"$(INTDIR)\spiany.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "spiany - Win32 Release"

"spiif_rev6 - Win32 Release" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" 
   cd ".\spiany"

"spiif_rev6 - Win32 ReleaseCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" RECURSE=1 CLEAN 
   cd ".\spiany"

!ELSEIF  "$(CFG)" == "spiany - Win32 Debug"

"spiif_rev6 - Win32 Debug" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" 
   cd ".\spiany"

"spiif_rev6 - Win32 DebugCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\spiany"

!ENDIF 


!ENDIF 

