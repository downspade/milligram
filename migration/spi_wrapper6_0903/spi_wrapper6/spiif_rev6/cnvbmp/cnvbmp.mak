# Microsoft Developer Studio Generated NMAKE File, Based on cnvbmp.dsp
!IF "$(CFG)" == ""
CFG=cnvbmp - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの cnvbmp - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "cnvbmp - Win32 Release" && "$(CFG)" != "cnvbmp - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "cnvbmp.mak" CFG="cnvbmp - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "cnvbmp - Win32 Release" ("Win32 (x86) Console Application" 用)
!MESSAGE "cnvbmp - Win32 Debug" ("Win32 (x86) Console Application" 用)
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

!IF  "$(CFG)" == "cnvbmp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\cnvbmp.exe"

!ELSE 

ALL : "spiif_rev6 - Win32 Release" "$(OUTDIR)\cnvbmp.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\cnvbmp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cnvbmp.exe"
	-@erase "$(OUTDIR)\cnvbmp.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\cnvbmp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cnvbmp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cnvbmp.pdb" /map:"$(INTDIR)\cnvbmp.map" /machine:I386 /out:"$(OUTDIR)\cnvbmp.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cnvbmp.obj" \
	"..\Release\spiif_rev6.lib"

"$(OUTDIR)\cnvbmp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cnvbmp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\cnvbmp.exe"

!ELSE 

ALL : "spiif_rev6 - Win32 Debug" "$(OUTDIR)\cnvbmp.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\cnvbmp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cnvbmp.exe"
	-@erase "$(OUTDIR)\cnvbmp.ilk"
	-@erase "$(OUTDIR)\cnvbmp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\cnvbmp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cnvbmp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cnvbmp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cnvbmp.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cnvbmp.obj" \
	"..\Debug\spiif_rev6.lib"

"$(OUTDIR)\cnvbmp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("cnvbmp.dep")
!INCLUDE "cnvbmp.dep"
!ELSE 
!MESSAGE Warning: cannot find "cnvbmp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cnvbmp - Win32 Release" || "$(CFG)" == "cnvbmp - Win32 Debug"
SOURCE=.\cnvbmp.cpp

"$(INTDIR)\cnvbmp.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "cnvbmp - Win32 Release"

"spiif_rev6 - Win32 Release" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" 
   cd ".\cnvbmp"

"spiif_rev6 - Win32 ReleaseCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" RECURSE=1 CLEAN 
   cd ".\cnvbmp"

!ELSEIF  "$(CFG)" == "cnvbmp - Win32 Debug"

"spiif_rev6 - Win32 Debug" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" 
   cd ".\cnvbmp"

"spiif_rev6 - Win32 DebugCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\cnvbmp"

!ENDIF 


!ENDIF 

