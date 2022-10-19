# Microsoft Developer Studio Project File - Name="traindir3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=traindir3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "traindir3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "traindir3.mak" CFG="traindir3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "traindir3 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "traindir3 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "traindir3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gi /GX /O1 /I "..\wxwidgets-2.6.3\lib\vc_lib\msw" /I "..\wxwidgets-2.6.3\include" /I "." /I "..\wxwidgets-2.6.3\samples" /I "..\utils" /I "..\wxWidgets-2.6.3\contrib\include" /I "..\wxWidgets-2.6.3\include" /I "..\wxWidgets-2.6.3\lib\vc_dll\msw" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "__WXMSW__" /D "WXUSINGDLL" /D "NOPCH" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wxmsw26_html.lib wxmsw26_adv.lib wxmsw26_core.lib wxbase26_xml.lib wxbase26_net.lib wxmsw26_core.lib wxbase26.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"binrel/traindir3.exe" /libpath:"..\wxWidgets-2.6.3\lib\vc_dll"

!ELSEIF  "$(CFG)" == "traindir3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\cg\projects\wxwidgets-2.6.3\lib\vc_lib\mswd" /I "\cg\projects\wxwidgets-2.6.3\include" /I "." /I "\cg\projects\wxwidgets-2.6.3\samples" /I "..\utils" /I "..\wxWidgets-2.6.3\contrib\include" /I "..\wxWidgets-2.6.3\include" /I "..\wxWidgets-2.6.3\lib\vc_dll\mswd" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "NOPCH" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxmsw26d_html.lib wxmsw26d_adv.lib wxmsw26d_core.lib wxbase26d_xml.lib wxbase26d_net.lib wxmsw26d_core.lib wxbase26d.lib wxbase26d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"bin/traindir3.exe" /pdbtype:sept /libpath:"..\wxWidgets-2.6.3\lib\vc_dll"

!ENDIF 

# Begin Target

# Name "traindir3 - Win32 Release"
# Name "traindir3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AlertList.cpp
# End Source File
# Begin Source File

SOURCE=.\AssignDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Canvas.cpp
# End Source File
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DaysDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\espanol.cpp
# End Source File
# Begin Source File

SOURCE=.\FontManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphView.cpp
# End Source File
# Begin Source File

SOURCE=.\html.cpp
# End Source File
# Begin Source File

SOURCE=.\HtmlView.cpp
# End Source File
# Begin Source File

SOURCE=.\http.cpp
# End Source File
# Begin Source File

SOURCE=.\italiano.cpp
# End Source File
# Begin Source File

SOURCE=.\Itinerary.cpp
# End Source File
# Begin Source File

SOURCE=.\ItineraryDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ItineraryView.cpp
# End Source File
# Begin Source File

SOURCE=.\LateGraphView.cpp
# End Source File
# Begin Source File

SOURCE=.\loadsave.cpp
# End Source File
# Begin Source File

SOURCE=.\Localize.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mongoose.c
# End Source File
# Begin Source File

SOURCE=.\NotebookMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PlatformGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\Puzzles.cpp
# End Source File
# Begin Source File

SOURCE=.\ReportBase.cpp
# End Source File
# Begin Source File

SOURCE=.\run.cpp
# End Source File
# Begin Source File

SOURCE=.\ScenarioInfoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# Begin Source File

SOURCE=.\SignalDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SkinColorsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StationInfoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBoard.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchboardNameDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchboardView.cpp
# End Source File
# Begin Source File

SOURCE=.\TConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\TDFile.cpp
# End Source File
# Begin Source File

SOURCE=.\tdscript.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeTblView.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolsView.cpp
# End Source File
# Begin Source File

SOURCE=.\track.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackScriptDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Train.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainInfoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainInfoList.cpp
# End Source File
# Begin Source File

SOURCE=.\TriggerDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\trsim.cpp
# End Source File
# Begin Source File

SOURCE=.\TSignal.cpp
# End Source File
# Begin Source File

SOURCE=.\TVector.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AlertList.h
# End Source File
# Begin Source File

SOURCE=.\AssignDialog.h
# End Source File
# Begin Source File

SOURCE=.\Canvas.h
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\DaysDialog.h
# End Source File
# Begin Source File

SOURCE=.\defs.h
# End Source File
# Begin Source File

SOURCE=.\FontManager.h
# End Source File
# Begin Source File

SOURCE=.\GraphView.h
# End Source File
# Begin Source File

SOURCE=.\HtmlView.h
# End Source File
# Begin Source File

SOURCE=.\imovers.h
# End Source File
# Begin Source File

SOURCE=.\Itinerary.h
# End Source File
# Begin Source File

SOURCE=.\ItineraryDialog.h
# End Source File
# Begin Source File

SOURCE=.\ItineraryView.h
# End Source File
# Begin Source File

SOURCE=.\LateGraphView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\mongoose.h
# End Source File
# Begin Source File

SOURCE=.\NotebookMgr.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDialog.h
# End Source File
# Begin Source File

SOURCE=.\PlatformGraphView.h
# End Source File
# Begin Source File

SOURCE=.\ReportBase.h
# End Source File
# Begin Source File

SOURCE=.\ScenarioInfoDialog.h
# End Source File
# Begin Source File

SOURCE=.\SignalDialog.h
# End Source File
# Begin Source File

SOURCE=.\SkinColorsDialog.h
# End Source File
# Begin Source File

SOURCE=.\StationInfoDialog.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBoard.h
# End Source File
# Begin Source File

SOURCE=.\SwitchboardNameDialog.h
# End Source File
# Begin Source File

SOURCE=.\SwitchboardView.h
# End Source File
# Begin Source File

SOURCE=.\TConfig.h
# End Source File
# Begin Source File

SOURCE=.\TDFile.h
# End Source File
# Begin Source File

SOURCE=.\tdscript.h
# End Source File
# Begin Source File

SOURCE=.\TimeTblView.h
# End Source File
# Begin Source File

SOURCE=.\ToolsView.h
# End Source File
# Begin Source File

SOURCE=.\Track.h
# End Source File
# Begin Source File

SOURCE=.\TrackDialog.h
# End Source File
# Begin Source File

SOURCE=.\TrackScriptDialog.h
# End Source File
# Begin Source File

SOURCE=.\Train.h
# End Source File
# Begin Source File

SOURCE=.\traindir3.h
# End Source File
# Begin Source File

SOURCE=.\TrainInfoDialog.h
# End Source File
# Begin Source File

SOURCE=.\TrainInfoList.h
# End Source File
# Begin Source File

SOURCE=.\TriggerDialog.h
# End Source File
# Begin Source File

SOURCE=.\trsim.h
# End Source File
# Begin Source File

SOURCE=.\TSignal.h
# End Source File
# Begin Source File

SOURCE=.\TVector.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\TD.ico
# End Source File
# End Group
# End Target
# End Project
