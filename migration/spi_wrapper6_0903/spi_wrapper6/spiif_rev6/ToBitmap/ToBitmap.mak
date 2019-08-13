# Microsoft Developer Studio Generated NMAKE File, Based on ToBitmap.dsp
!IF "$(CFG)" == ""
CFG=ToBitmap - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの ToBitmap - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "ToBitmap - Win32 Release" && "$(CFG)" != "ToBitmap - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "ToBitmap.mak" CFG="ToBitmap - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "ToBitmap - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "ToBitmap - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 
!ERROR 無効な構成が指定されています。
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ToBitmap - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ToBitmap.exe" "$(OUTDIR)\ToBitmap.pch"

!ELSE 

ALL : "spiif_rev6 - Win32 Release" "$(OUTDIR)\ToBitmap.exe" "$(OUTDIR)\ToBitmap.pch"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\convert_thread.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\susie_drop.obj"
	-@erase "$(INTDIR)\ToBitmap.obj"
	-@erase "$(INTDIR)\ToBitmap.pch"
	-@erase "$(INTDIR)\ToBitmap.res"
	-@erase "$(INTDIR)\ToBitmapDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ToBitmap.exe"
	-@erase "$(OUTDIR)\ToBitmap.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x411 /fo"$(INTDIR)\ToBitmap.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ToBitmap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\ToBitmap.pdb" /map:"$(INTDIR)\ToBitmap.map" /machine:I386 /out:"$(OUTDIR)\ToBitmap.exe" 
LINK32_OBJS= \
	"$(INTDIR)\convert_thread.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\susie_drop.obj" \
	"$(INTDIR)\ToBitmap.obj" \
	"$(INTDIR)\ToBitmapDlg.obj" \
	"$(INTDIR)\ToBitmap.res" \
	"..\Release\spiif_rev6.lib"

"$(OUTDIR)\ToBitmap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ToBitmap - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ToBitmap.exe" "$(OUTDIR)\ToBitmap.pch"

!ELSE 

ALL : "spiif_rev6 - Win32 Debug" "$(OUTDIR)\ToBitmap.exe" "$(OUTDIR)\ToBitmap.pch"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"spiif_rev6 - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\convert_thread.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\susie_drop.obj"
	-@erase "$(INTDIR)\ToBitmap.obj"
	-@erase "$(INTDIR)\ToBitmap.pch"
	-@erase "$(INTDIR)\ToBitmap.res"
	-@erase "$(INTDIR)\ToBitmapDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ToBitmap.exe"
	-@erase "$(OUTDIR)\ToBitmap.ilk"
	-@erase "$(OUTDIR)\ToBitmap.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x411 /fo"$(INTDIR)\ToBitmap.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ToBitmap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\ToBitmap.pdb" /debug /machine:I386 /out:"$(OUTDIR)\ToBitmap.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\convert_thread.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\susie_drop.obj" \
	"$(INTDIR)\ToBitmap.obj" \
	"$(INTDIR)\ToBitmapDlg.obj" \
	"$(INTDIR)\ToBitmap.res" \
	"..\Debug\spiif_rev6.lib"

"$(OUTDIR)\ToBitmap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("ToBitmap.dep")
!INCLUDE "ToBitmap.dep"
!ELSE 
!MESSAGE Warning: cannot find "ToBitmap.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ToBitmap - Win32 Release" || "$(CFG)" == "ToBitmap - Win32 Debug"
SOURCE=.\convert_thread.cpp

!IF  "$(CFG)" == "ToBitmap - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\convert_thread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ToBitmap - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\convert_thread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "ToBitmap - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\ToBitmap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ToBitmap - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ToBitmap.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\ToBitmap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\susie_drop.cpp

"$(INTDIR)\susie_drop.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ToBitmap.cpp

"$(INTDIR)\ToBitmap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ToBitmap.rc

"$(INTDIR)\ToBitmap.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\ToBitmapDlg.cpp

"$(INTDIR)\ToBitmapDlg.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "ToBitmap - Win32 Release"

"spiif_rev6 - Win32 Release" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" 
   cd ".\ToBitmap"

"spiif_rev6 - Win32 ReleaseCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Release" RECURSE=1 CLEAN 
   cd ".\ToBitmap"

!ELSEIF  "$(CFG)" == "ToBitmap - Win32 Debug"

"spiif_rev6 - Win32 Debug" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" 
   cd ".\ToBitmap"

"spiif_rev6 - Win32 DebugCLEAN" : 
   cd "\users\kana\SRC\spi_wrapper\spi_wrapper6\spiif_rev6"
   $(MAKE) /$(MAKEFLAGS) /F .\spiif_rev6.mak CFG="spiif_rev6 - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\ToBitmap"

!ENDIF 


!ENDIF 

