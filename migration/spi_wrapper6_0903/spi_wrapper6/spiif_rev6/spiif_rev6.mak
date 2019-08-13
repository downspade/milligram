# Microsoft Developer Studio Generated NMAKE File, Based on spiif_rev6.dsp
!IF "$(CFG)" == ""
CFG=spiif_rev6 - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの spiif_rev6 - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "spiif_rev6 - Win32 Release" && "$(CFG)" != "spiif_rev6 - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "spiif_rev6.mak" CFG="spiif_rev6 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "spiif_rev6 - Win32 Release" ("Win32 (x86) Static Library" 用)
!MESSAGE "spiif_rev6 - Win32 Debug" ("Win32 (x86) Static Library" 用)
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

!IF  "$(CFG)" == "spiif_rev6 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\spiif_rev6.lib"


CLEAN :
	-@erase "$(INTDIR)\regs.obj"
	-@erase "$(INTDIR)\spiif.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\spiif_rev6.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\spiif_rev6.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\spiif_rev6.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\spiif_rev6.lib" 
LIB32_OBJS= \
	"$(INTDIR)\regs.obj" \
	"$(INTDIR)\spiif.obj"

"$(OUTDIR)\spiif_rev6.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "spiif_rev6 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\spiif_rev6.lib"


CLEAN :
	-@erase "$(INTDIR)\regs.obj"
	-@erase "$(INTDIR)\spiif.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\spiif_rev6.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\spiif_rev6.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\spiif_rev6.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\spiif_rev6.lib" 
LIB32_OBJS= \
	"$(INTDIR)\regs.obj" \
	"$(INTDIR)\spiif.obj"

"$(OUTDIR)\spiif_rev6.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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
!IF EXISTS("spiif_rev6.dep")
!INCLUDE "spiif_rev6.dep"
!ELSE 
!MESSAGE Warning: cannot find "spiif_rev6.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "spiif_rev6 - Win32 Release" || "$(CFG)" == "spiif_rev6 - Win32 Debug"
SOURCE=.\regs.c

"$(INTDIR)\regs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\spiif.cpp

"$(INTDIR)\spiif.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

