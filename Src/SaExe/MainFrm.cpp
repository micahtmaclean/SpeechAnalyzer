/////////////////////////////////////////////////////////////////////////////
// mainfrm.cpp:
// Implementation of the CMainFrame class.  
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//        SDM Added support for CDlgEditor
//   1.06.6U2
//        SDM Added Add segment and Insert Word toolbar buttons
//   1.06.6U4
//        SDM Added Save/Copy Screen/Graph AsBMP
//        SDM Moved functionality of OnSaveAsBMP to CDib::CaptureWindow & CDib::Save
//        SDM Added GetCurrSaView
//   1.06.6U5
//        SDM Added Class CSaMDIChildWindow
//        SDM Added paramenters to settings DefaultView Maximized/Height/Width
//        SDM Added CopyWindowAsBMP & SaveWindowAsBMP
//        SDM Changed GraphAsBMP to refer to client area 
//   1.06.8
//        SDM Changed IsSMRunning to check ALL windows
//   1.5Test8.1
//        SDM added support for Reference annotation
//   1.5Test8.2
//        SDM added support for ToneAbove check box
//        SDM added support for saving TE position (m_wplDlgEditor)
//   1.5Test8.5
//        SDM disabled context sensitive help
//        SDM added support for dynamically changing menu, popups, and accelerators
//        SDM added support for display plot
//   1.5Test10.0
//        SDM added m_parseParm & m_segmentParm
//        SDM disabled memory page unless WIN32
//   1.5Test10.3
//        SDM set window order for tiling
//   1.5Test11.1A
//        RLJ Added m_parseParmDefaults.   (Use in addition to SaDoc->m_parseParm.) 
//        RLJ Added m_segmentParmDefaults. (Use in addition to SaDoc->m_segmentParm.) 
//        RLJ Added m_pitchParmDefaults, m_spectrumParmDefaults, 
//                  m_spectrogramParmDefaults, m_snapshotParmDefaults   
//   06/13/2000
//        RLJ Disabled Toolbar's "?" button (Help Index), since this 
//            feature has not yet been implemented. 
//   06/22/2000
//        RLJ Moved some "Global Help Commands" to CSaApp
//
//   08/07/00
//        DDO Completed the work on being able to save settings both
//            permanently and temporarily. Hopefully, the process has
//            been simplified on the dialog box.
//        DDO Fixed a problem with the current caption style on the Tools/
//            Options/View dialog not necessarily matching the current
//            caption sytle. The dialog's member var. wasn't being
//            initialized before showing it modally.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "mainfrm.h"
#include "sa_graph.h"
#include "playerRecorder.h"
#include "toolsOptions.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_spg.h"

#include "sa_wbch.h"
#include "sa_edit.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "doclist.h"
#include "math.h" 
#include "sa_find.h"    
#include "sa_cdib.h"
#include "targview.h"
#include "settings\obstream.h"
using std::ifstream;
using std::ofstream;
using std::ios;

#include "sa_dplot.h"
#include "autorecorder.h"
#include "waveformGenerator.h"

#include "synthesis\DlgSynthesis.h"
#include "synthesis\DlgKlattAll.h"
#include "synthesis\DlgVocalTract.h"
#include "SelfTest.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CMainFrameBase)


//###########################################################################
// CMainFrame

/////////////////////////////////////////////////////////////////////////////
// CMainFrame static members definition

static TCHAR szFINDMSGSTRING[] = FINDMSGSTRING;
static const UINT NEAR nMsgFindReplace = ::RegisterWindowMessage(szFINDMSGSTRING);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CMainFrameBase)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_INITMENU()
    ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_COMMAND(ID_TOOLS_SELFTEST, OnToolsSelfTest)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
    ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateEditReplace)
    ON_WM_CLOSE()
    ON_WM_SYSCOLORCHANGE()
    ON_COMMAND(ID_EDIT_EDITOR, OnEditor)
    ON_MESSAGE(WM_USER_IDLE_UPDATE, OnIdleUpdate)
    ON_UPDATE_COMMAND_UI(ID_EDIT_EDITOR, OnUpdateEditEditor)
    ON_COMMAND(ID_GRAPHS_SCREENCAPTURE, OnSaveScreenAsBMP)
    ON_COMMAND(ID_GRAPHS_SCREENCOPY, OnCopyScreenAsBMP)
    ON_COMMAND(ID_GRAPHS_WINDOWSAVE, OnSaveWindowAsBMP)
    ON_COMMAND(ID_GRAPHS_WINDOWCOPY, OnCopyWindowAsBMP)
    ON_COMMAND(ID_GRAPHS_GRAPHSCAPTURE, OnSaveGraphsAsBMP)
    ON_COMMAND(ID_GRAPHS_GRAPHSCOPY, OnCopyGraphsAsBMP)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_GRAPHSCAPTURE, OnUpdateGraphsAsBMP)
    ON_WM_ACTIVATEAPP()
    ON_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
    ON_COMMAND(ID_WINDOW_TILE_VERT, OnWindowTileVert)
	ON_COMMAND(ID_WAVEFORM_GENERATOR, OnWaveformGenerator)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OPTIONS, OnUpdateToolsOptions)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_GRAPHSCOPY, OnUpdateGraphsAsBMP)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_SYNTHESIS, OnSynthesis)
	ON_UPDATE_COMMAND_UI(ID_SYNTHESIS, OnUpdateSynthesis)
	ON_COMMAND(ID_KLATT_IPA_BLEND, OnSynthesisKlattIpa)
	ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_BLEND, OnUpdateSynthesis)
	ON_COMMAND(ID_KLATT_FRAGMENTS, OnSynthesisKlattFragments)
	ON_UPDATE_COMMAND_UI(ID_KLATT_FRAGMENTS, OnUpdateSynthesis)
	ON_COMMAND(ID_SYNTHESIS_VTRACT, OnSynthesisVocalTract)
	ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT, OnUpdateSynthesis)
    ON_COMMAND_EX(IDR_BAR_BASIC, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDR_BAR_BASIC, OnUpdateControlBarMenu)
    ON_COMMAND_EX(IDR_BAR_ADVANCED, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDR_BAR_ADVANCED, OnUpdateControlBarMenu)
    ON_COMMAND_EX(ID_VIEW_TASKBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TASKBAR, OnUpdateControlBarMenu)
    // Global help commands
    //ON_COMMAND(ID_CONTEXT_HELP, CMainFrameBase::OnContextHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CMainFrameBase::OnHelpIndex) // SDM 1.5Test8.5 disable context sensitive help
    // Custom messages
    ON_MESSAGE(WM_USER_APPLY_TOOLSOPTIONS, OnApplyToolsOptions)
    ON_MESSAGE(WM_USER_PLAYER, OnPlayer)
    ON_MESSAGE(WM_USER_CHANGEVIEW, OnChangeView)
    ON_MESSAGE(WM_USER_SPEECHAPPLICATION, OnSpeechAppCall) 
    ON_COMMAND(ID_GRAPHS_EQUALIZELENGTH, OnEqualizeLength)
    ON_UPDATE_COMMAND_UI(ID_GRAPHS_EQUALIZELENGTH, OnUpdateEqualizeLength)
    //ON_COMMAND(ID_SETDEFAULT, OnSetDefault)
    // status bar update
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_1, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_2, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_3, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_STATUSPANE_4, OnUpdateDataPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_1, OnUpdateProgressPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_2, OnUpdateProgressPane)
    ON_UPDATE_COMMAND_UI(ID_PROGRESSPANE_3, OnUpdateProgressPane)
    ON_COMMAND(ID_POPUP_RECORD_OVERLAY, OnRecordOverlay)
    ON_UPDATE_COMMAND_UI(ID_POPUP_RECORD_OVERLAY, OnUpdateRecordOverlay)
END_MESSAGE_MAP()

static UINT BASED_CODE dataIndicators[] =
{
    ID_SEPARATOR,           // data status line indicator
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
    ID_SEPARATOR,
};

static UINT BASED_CODE progressIndicators[] =
{
    ID_SEPARATOR,           // progress status line indicator
    ID_SEPARATOR,
    ID_SEPARATOR,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction/creation

/***************************************************************************/
// CMainFrame::CMainFrame Constructor
/***************************************************************************/
CMainFrame::CMainFrame()
{
  // otions default settings
  m_bStatusBar = TRUE;               // statusbar enabled at startup
  m_nStatusPosReadout = TIME;        // position readout mode is time
  m_nStatusPitchReadout = HERTZ;     // pitch readout mode is hertz
  m_bToneAbove = FALSE;              // tone below phonetic by default SDM 1.5Test8.2
  m_bScrollZoom = TRUE;              // scrollbar zooming enabled at startup
  m_nCaptionStyle = MiniWithCaption; // graph caption style is minicaption with caption text
  
  // init the gridlines
  m_grid.Init();
  
  // other settings
  m_bMenuModified = FALSE;          // menu not modified at startup
  m_nNumberOfViews = 0;             // no views at startup
  
  // define modeless dialog and other object pointers
  m_pWorkbenchView = NULL;          // workbench
  m_pDlgFind = NULL;                // find/replace
  m_pDlgPlayer = NULL;              // player
  
  //SDM 1.06.5
  m_pDlgEditor = NULL;              // editor
  m_bIsPrinting = FALSE;
  
  // reset workbench processes and filter IDs
  for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++) 
  {
    for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++) 
    {
      m_apWbProcess[nLoop][nFilterLoop] = NULL;
      m_aWbFilterID[nLoop][nFilterLoop] = 0;
    }
  }
  
  m_szPermDefaultGraphs = "";  // DDO - 08/08/00
  m_szTempDefaultGraphs = "";  // DDO - 08/08/00
  m_nPermDefaultLayout = 0;    // DDO - 08/07/00
  m_nTempDefaultLayout = 0;    // DDO - 08/07/00
  m_nStartDataMode = 0;        // DDO - 08/08/00
  //m_bSaveOnExit = FALSE;     // DDO - 08/03/00 Don't need this setting anymore.
  m_bShowStartupDlg = TRUE;    // DDO - 08/03/00
  m_bSaveOpenFiles = FALSE;    // tdg - 09/03/97
  m_pGraphFontFaces = new CSaStringArray; // create graph font face strings array object
  m_pGraphFontSizes = new CUIntArray; // create graph font size array object
  
  // init the graph fonts with default
  try
  {
    if (!m_pGraphFontFaces->GetSize())
    {
      m_pGraphFontFaces->Add(PHONETIC_DEFAULT_FONT);
      m_pGraphFontFaces->Add(TONE_DEFAULT_FONT);
      m_pGraphFontFaces->Add(PHONEMIC_DEFAULT_FONT);
      m_pGraphFontFaces->Add(ORTHOGRAPHIC_DEFAULT_FONT);
      m_pGraphFontFaces->Add(GLOSS_DEFAULT_FONT);
      m_pGraphFontFaces->Add(REFERENCE_DEFAULT_FONT);
      m_pGraphFontFaces->Add(MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->Add(MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->Add(MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->Add(MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontSizes->Add(PHONETIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(TONE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(PHONEMIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(ORTHOGRAPHIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(GLOSS_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(REFERENCE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->Add(MUSIC_PHRASE_DEFAULT_FONTSIZE);
    }
    else
    {
      m_pGraphFontFaces->SetAt(PHONETIC, PHONETIC_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(TONE, TONE_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(PHONEMIC, PHONEMIC_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(ORTHO, ORTHOGRAPHIC_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(GLOSS, GLOSS_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(REFERENCE, REFERENCE_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(MUSIC_PL1, MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(MUSIC_PL2, MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(MUSIC_PL3, MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontFaces->SetAt(MUSIC_PL4, MUSIC_PHRASE_DEFAULT_FONT);
      m_pGraphFontSizes->SetAt(PHONETIC, PHONETIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(TONE, TONE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(PHONEMIC, PHONEMIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(ORTHO, ORTHOGRAPHIC_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(GLOSS, GLOSS_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(REFERENCE, REFERENCE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(MUSIC_PL1, MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(MUSIC_PL2, MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(MUSIC_PL3, MUSIC_PHRASE_DEFAULT_FONTSIZE);
      m_pGraphFontSizes->SetAt(MUSIC_PL4, MUSIC_PHRASE_DEFAULT_FONTSIZE);
    }
  }
  catch(CMemoryException e)
  {
    // handle memory fail exception
    CSaApp* pApp = (CSaApp*)AfxGetApp();
    pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
    return;
  }
  
  
  m_bPrintPreviewInProgress = FALSE;
  m_wplDlgEditor.length = 0; // SDM 1.5Test8.2
  m_pDisplayPlot = NULL; // SDM 1.5Test8.5
  m_hNewMenu = NULL; // SDM 1.5Test8.5
  m_hNewAccel = NULL; // SDM 1.5Test8.5
  m_nPopup = 0; // SDM 1.5Test8.5
  m_hExpMenu = NULL;
  m_szExpMenuTitle = "";
  
  // Initialize parsing, segmenting, pitch, spectrum and spectrogram parameter defaults // RLJ 11.1A
  m_parseParmDefaults.Init();
  m_segmentParmDefaults.Init();
  m_pitchParmDefaults.Init();
  m_musicParmDefaults.Init();
  m_formantParmDefaults.Init();
  m_spectrumParmDefaults.Init();
  m_spectrogramParmDefaults.Init();
  m_snapshotParmDefaults.Init();
  m_pDefaultViewConfig = NULL;
  
  // SDM 1.06.6U5
  m_bDefaultMaximizeView = TRUE;
  m_nDefaultHeightView = 0;
  m_nDefaultWidthView = 0;
  m_bDefaultViewExists = FALSE;       // DDO - 08/07/00
  
  m_nGraphUpdateMode = STATIC_UPDATE;
  m_bAnimate = FALSE;
  m_nAnimationRate = MAX_ANIMATION_RATE;
  m_nCursorAlignment = ALIGN_AT_FRAGMENT;
}

/***************************************************************************/
// CMainFrame::~CMainFrame Destructor
/***************************************************************************/
CMainFrame::~CMainFrame()
{
  // delete modeless dialog and other objects
  if (m_pWorkbenchView)
  {
	delete m_pWorkbenchView; // workbench
	m_pWorkbenchView = NULL;
  }
  if (m_pDlgFind)
  {
	delete m_pDlgFind;
	m_pDlgFind = NULL;
  }
  if (m_pDlgPlayer)
  {
	delete m_pDlgPlayer; // player
	m_pDlgPlayer = NULL;
  }
  if (m_pDlgEditor)
  {
	delete m_pDlgEditor; // SDM 1.06.5 editor
	m_pDlgEditor = NULL;
  }
  if (m_pGraphFontFaces)
  {
	delete m_pGraphFontFaces;
	m_pGraphFontFaces = NULL;
  }
  if (m_pGraphFontSizes)
  {
	delete m_pGraphFontSizes;
	m_pGraphFontSizes = NULL;
  }
  if (m_pDefaultViewConfig)
  {
	delete m_pDefaultViewConfig;
	m_pDefaultViewConfig = NULL;
  }

  DestroySynthesizer();
  CDlgKlattAll::DestroySynthesizer();
  CDlgVocalTract::DestroySynthesizer();

  // delete workbench processes
  for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++) 
  {
    for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
      if (m_apWbProcess[nLoop][nFilterLoop]) delete m_apWbProcess[nLoop][nFilterLoop];
  }
  
  if (m_pDisplayPlot) 
  {
    // SDM 1.5Test8.5
    delete m_pDisplayPlot;
    m_pDisplayPlot = NULL;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame helper functions

/***************************************************************************/
// CMainFrame::SwapInOverlayColors
/***************************************************************************/
void CMainFrame::SwapInOverlayColors(int index)
{
  // save away for restore later on.
  m_colors.cBackupColor   = m_colors.cPlotData[0];
  
  // bring in the overlay colors                                    
  m_colors.cPlotData[0]   = m_colors.cPlotData[index];
}   


/***************************************************************************/
// CMainFrame::SwapOutOverlayColors
/***************************************************************************/
void CMainFrame::SwapOutOverlayColors()
{
  m_colors.cPlotData[0]   = m_colors.cBackupColor;
}

/***************************************************************************/
// CMainFrame::GetVisibleMenuItemCount
/***************************************************************************/
UINT CMainFrame::GetVisibleMenuItemCount(CMenu* pMenu)
{
	UINT count = 0;
	UINT maxCount = pMenu->GetMenuItemCount();
	CSaString rString;
	for (UINT i = 1; i <= maxCount; i++)
	{
		pMenu->GetMenuString(i, rString, MF_BYPOSITION);
		if (rString.IsEmpty())
			break;
		count++;
	}

	return count;
}

/***************************************************************************/
// CMainFrame::ShowDataStatusBar Show or hide data status bar
/***************************************************************************/
void CMainFrame::ShowDataStatusBar(BOOL bShow)
{
  if (m_bStatusBar) // status bar is on
  {
    if (bShow)
    {
      // show data status bar, hide process status bar
      m_progressStatusBar.ShowWindow(SW_HIDE);
      m_dataStatusBar.ShowWindow(SW_SHOW);
    }
    else
    {
      // show process status bar, hide data status bar
      m_dataStatusBar.ShowWindow(SW_HIDE);
      // move progress status bar over data status bar
      CRect rWnd;
      m_dataStatusBar.GetWindowRect(rWnd);
      ScreenToClient(rWnd);
      m_progressStatusBar.MoveWindow(rWnd);
      m_progressStatusBar.InitProgress();
      if (!m_bIsPrinting) 
      {
        m_progressStatusBar.ShowWindow(SW_SHOW);
        m_progressStatusBar.Invalidate();
        m_progressStatusBar.UpdateWindow();
      }
    }
  }
}

/***************************************************************************/
// CMainFrame::GetFnKeys Returns a copy of the function keys structure
// The function returns a pointer to the function key structure. If the
// parameter contains a valid pointer, it will also copy the function key
// setting to this pointer.
/***************************************************************************/
FnKeys* CMainFrame::GetFnKeys(FnKeys* pfnKeys)
{
  if (pfnKeys) *pfnKeys = m_fnKeys;
  return &m_fnKeys;
}

/***************************************************************************/
// CMainFrame::SetFnKeys Sets the function keys structure
/***************************************************************************/
void CMainFrame::SetFnKeys(FnKeys* pfnKeys)
{
  m_fnKeys = *pfnKeys;
}

/***************************************************************************/
// CMainFrame::IsPlayerPlaying Returns TRUE if player is playing
/***************************************************************************/
BOOL CMainFrame::IsPlayerPlaying()
{
  if (CDlgPlayer::bPlayer) // player launched
  {
    return GetPlayer(false)->IsPlaying(); // return TRUE if player is playing
  }
  else return FALSE;
}

/***************************************************************************/
// CMainFrame::IsPlayerTestRun Returns TRUE if player runs function key test
/***************************************************************************/
BOOL CMainFrame::IsPlayerTestRun()
{
  if (CDlgPlayer::bPlayer) // player launched
  {
    return GetPlayer(false)->IsTestRun(); // return TRUE if player runs Fn test
  }
  else return FALSE;
}

/***************************************************************************/
// CMainFrame::SetPlayerTimes  sets time indicators for layer                   
/***************************************************************************/
void CMainFrame::SetPlayerTimes()
{
  if (CDlgPlayer::bPlayer)                            // if player dialogue launched
  {
    CRect rWnd;
    GetPlayer(false)->SetPositionTime();                          // set the start time
    CWnd* pWnd = GetPlayer(false)->GetDlgItem(IDC_POSITIONTIME);  // get start LED indicator
    pWnd->GetWindowRect(rWnd);                                // get coordinates
    pWnd->RedrawWindow(rWnd);  
    GetPlayer(false)->SetTotalTime();                             // set the stop time
    pWnd = GetPlayer(false)->GetDlgItem(IDC_TOTALTIME);           // get stop LED indicator
    pWnd->GetWindowRect(rWnd);                                // get coordinates
    pWnd->RedrawWindow(rWnd);                                 // update
  }
}

/***************************************************************************/
// CMainFrame::GetBufferSize Start setup Fn-keys dialog in player
/***************************************************************************/
void CMainFrame::SetupFunctionKeys()
{ 
  if (GetPlayer(false))
      GetPlayer(false)->SendMessage(WM_USER_SETUP_FNKEYS, 0, 0L);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMainFrameBase::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMainFrameBase::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

/***************************************************************************/
// CMainFrame::OnCreate Window creation
// Called by framework to initially create the one and only mainframe window.
// Creates the toolbar and statusbar. Returns FALSE on error.
/***************************************************************************/
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{   
  if (CMainFrameBase::OnCreate(lpCreateStruct) == -1) return -1;

  // create toolbars
  if (!m_wndToolBarBasic.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
        CRect(0,0,0,0), IDR_BAR_BASIC) ||
      !m_wndToolBarBasic.LoadToolBar(IDR_BAR_BASIC))
  {
    TRACE(_T("Failed to create toolbar\n"));
    return -1; // failed to create
  }
  
  if (!m_wndToolBarAdvanced.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
        CRect(0,0,0,0), IDR_BAR_ADVANCED) ||
      !m_wndToolBarAdvanced.LoadToolBar(IDR_BAR_ADVANCED))
  {
    TRACE(_T("Failed to create toolbar\n"));
    return -1; // failed to create
  }
  
  // create data statusbar
  if (!m_dataStatusBar.Create(this) ||
      !m_dataStatusBar.SetIndicators(dataIndicators, sizeof(dataIndicators)/sizeof(UINT)))
  {
    TRACE(_T("Failed to create data status bar\n"));
    return -1; // failed to create
  }

  // initialize data statusbar
  m_dataStatusBar.Init();
  // create progress statusbar
  if (!m_progressStatusBar.Create(this) ||
      !m_progressStatusBar.SetIndicators(progressIndicators, sizeof(progressIndicators)/sizeof(UINT)))
  {
    TRACE(_T("Failed to create progress status bar\n"));
    return -1; // failed to create
  }   

  // initialize progress statusbar
  m_progressStatusBar.ShowWindow(SW_HIDE);
  m_progressStatusBar.Init();                          

  m_wndToolBarBasic.EnableDocking(CBRS_ALIGN_ANY);
  m_wndToolBarAdvanced.EnableDocking(CBRS_ALIGN_ANY);
  EnableDocking(CBRS_ALIGN_ANY);  // Creates CDockBar objects
  DockControlBar(&m_wndToolBarBasic, AFX_IDW_DOCKBAR_TOP);
  DockControlBar(&m_wndToolBarAdvanced, AFX_IDW_DOCKBAR_TOP);
  ShowControlBar(&m_wndToolBarAdvanced, FALSE, FALSE);

  // Create Task Bar last this affects its position Z-Order and therefore layout behavior
  // Last in the Z-Order is preferrable for the task bar
  if(!m_wndTaskBar.Create(this, IDD_TASKBAR, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT
        | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED, ID_VIEW_TASKBAR))
  {
    TRACE(_T("Failed to create data task bar\n"));
    return -1; // failed to create
  }

  ShowControlBar(&m_wndTaskBar, TRUE, FALSE);
  SetupTaskBar(m_wndTaskBar);

  // init the colors
  m_colors.SetupDefault();
  // setup function keys default
  m_fnKeys.SetupDefault();
  return 0;
}

/***************************************************************************/
// CMainFrame::OnInitMenu Menu creation
// Called by framework to initially create the mainframe menu. Attaches
// the graph layout menu onto the mainframe menu (if not already done).
/***************************************************************************/
void CMainFrame::OnInitMenu(CMenu* pMenu)
{
  CMainFrameBase::OnInitMenu(pMenu);
	UINT nMenuCount = GetVisibleMenuItemCount(pMenu);
  if ((nMenuCount > 6) // standalone menu loaded
      && (pMenu == GetMenu())) // don't modify floating popup menu
  {
    if (!m_bMenuModified) // attach layout menu on standalone menu
		{
			TCHAR szString[256]; // don't change the string
			pMenu->GetMenuString(ID_GRAPHS_LAYOUT, szString, sizeof(szString)/sizeof(TCHAR), MF_BYCOMMAND);
			if (szString[0] != 0)
			{
				VERIFY(pMenu->ModifyMenu(ID_GRAPHS_LAYOUT, MF_BYCOMMAND | MF_POPUP, (UINT)m_LayoutMenu.m_hMenu, szString));
				m_bMenuModified = TRUE; // modification done
			}
		}

		// Show Experimental Features submenu on control-click
		CMenu* pToolsMenu = pMenu->GetSubMenu(5);
		ASSERT_VALID(pToolsMenu);
    bool bExpMenuPresent = (pToolsMenu->GetSubMenu(5) != NULL);
		if (EXPERIMENTAL_ACCESS && !bExpMenuPresent)
			pToolsMenu->InsertMenu(5, MF_BYPOSITION | MF_POPUP, (UINT)m_hExpMenu, m_szExpMenuTitle);
		// remove it if control key not pressed
		if (!EXPERIMENTAL_ACCESS && bExpMenuPresent)
		{
			m_hExpMenu = pToolsMenu->GetSubMenu(5)->m_hMenu;
      pToolsMenu->GetMenuString(5, m_szExpMenuTitle, MF_BYPOSITION);
			pToolsMenu->RemoveMenu(5, MF_BYPOSITION);
		}
  }
}

/***************************************************************************/
// CMainFrame::OnToolsOptions Opens options dialog
// The user wants to see the options dialog. Creates the property sheet, sets
// up some initial values and sends a apply message, if the user hits the OK
// button.
/***************************************************************************/
void CMainFrame::OnToolsOptions()
{
  // set property sheet caption
  CSaString szCaption;
  szCaption.LoadString(IDS_DLGTITLE_TOOLSOPTIO); // load caption string
  // create property sheet object
  m_pDlgToolsOptions = new CDlgToolsOptions(szCaption, NULL);
  // get pointer to active view and document
  CSaView* pView = (CSaView*)GetCurrSaView();
  // setup initial dialog values
  m_pDlgToolsOptions->m_dlgViewPage.m_nCaptionStyle = m_nCaptionStyle; // DDO - 08/07/00
  m_pDlgToolsOptions->m_dlgViewPage.m_bStatusbar = m_bStatusBar;       // setup checkboxes
  m_pDlgToolsOptions->m_dlgViewPage.m_nPosMode = m_nStatusPosReadout;
  m_pDlgToolsOptions->m_dlgViewPage.m_nPitchMode = m_nStatusPitchReadout;
  m_pDlgToolsOptions->m_dlgViewPage.m_bToolbar = bToolBar();
  m_pDlgToolsOptions->m_dlgViewPage.m_bTaskbar = bTaskBar();
  m_pDlgToolsOptions->m_dlgViewPage.m_bToneAbove = m_bToneAbove;
  m_pDlgToolsOptions->m_dlgViewPage.m_bScrollZoom = m_bScrollZoom;
  m_pDlgToolsOptions->m_dlgViewPage.m_nCursorAlignment = pView->GetCursorAlignment();
  m_pDlgToolsOptions->m_dlgViewPage.m_nGraphUpdateMode = pView->GetGraphUpdateMode();
  m_pDlgToolsOptions->m_dlgViewPage.m_bAnimate = pView->IsAnimationRequested();
  m_pDlgToolsOptions->m_dlgViewPage.m_nAnimationRate = pView->GetAnimationFrameRate();
  m_pDlgToolsOptions->m_dlgViewPage.m_bXGrid = m_grid.bXGrid;
  m_pDlgToolsOptions->m_dlgViewPage.m_bYGrid = m_grid.bYGrid;
  m_pDlgToolsOptions->m_dlgViewPage.SetGridStyle(m_grid.nXStyle, m_grid.nYStyle);
  m_pDlgToolsOptions->m_dlgColorPage.m_cColors = m_colors;
  
  try
  {
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
      m_pDlgToolsOptions->m_dlgFontPage.m_GraphFonts.Add(GetFontFace(nLoop));
      m_pDlgToolsOptions->m_dlgFontPage.m_GraphFontSizes.Add(GetFontSize(nLoop));
    }
  }
  catch(CMemoryException e)
  {
    // memory allocation error
    CSaApp* pApp = (CSaApp*)AfxGetApp();
    pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
    if (m_pDlgToolsOptions) delete m_pDlgToolsOptions;
    return;
  }
  
  
  // create the modal dialog box
  if (m_pDlgToolsOptions->DoModal() == IDOK)         // OK button pressed
  {
    SendMessage(WM_USER_APPLY_TOOLSOPTIONS, 0, 0);   // do apply changes
  }                 
  // delete the dialog object
  if (m_pDlgToolsOptions) delete m_pDlgToolsOptions;
  m_pDlgToolsOptions = NULL;
}

void CMainFrame::OnUpdateToolsOptions(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(GetCurrSaView() != NULL);	
}

/***************************************************************************/
// CMainFrame::OnToolsSelfTest Starts SelfTest
/***************************************************************************/
void CMainFrame::OnToolsSelfTest()
{
  CSASelfTest Test;
}

/***************************************************************************/
// CMainFrame::SendMessageToMDIDescendants  Send message to all mdi children
// Just calling SendMessageToDescendants caused a crash in 32-bit version
// apparently because either the status bar or toolbar windows (descendants
// of CMainFrame) choked on a WM_USER_? message sent to them.
/***************************************************************************/
void CMainFrame::SendMessageToMDIDescendants(UINT message, WPARAM wParam, LPARAM lParam)
{
  CDocList doclst; // list of currently open docs
  for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
  {
    POSITION pos = pdoc->GetFirstViewPosition();
    while ( pos )
    {
      // ramble thru all views for this document
      CSaView* pview = (CSaView*)pdoc->GetNextView(pos); // increments pos
      pview->SendMessage(message, wParam, lParam);
      // send to child windows after parent
      pview->SendMessageToDescendants(message, wParam, lParam, TRUE, TRUE);
    }
  }
}

/***************************************************************************/
// CMainFrame::OnApplyToolsOptions Applies the changes in options dialog
// The user wants to apply the changes from options dialog. Applies all the
// changes and sends the necessary messages.
/***************************************************************************/
LRESULT CMainFrame::OnApplyToolsOptions(WPARAM, LPARAM)
{
  // apply to statusbar
  if (m_pDlgToolsOptions->m_dlgViewPage.m_bStatusbar != m_bStatusBar)
  {
    SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); // change statusbar status
    m_bStatusBar = !m_bStatusBar;
  }

  if(m_nStatusPosReadout != m_pDlgToolsOptions->m_dlgViewPage.m_nPosMode)
  {
    m_nStatusPosReadout = m_pDlgToolsOptions->m_dlgViewPage.m_nPosMode;
    CSaView *pView = GetCurrSaView();
    CGraphWnd *pGraph = pView ? pView->GetFocusedGraphWnd() : NULL;

    if(pGraph)
      pGraph->UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), TRUE); // update the status bar
  }

  m_nStatusPitchReadout = m_pDlgToolsOptions->m_dlgViewPage.m_nPitchMode;
  // apply to toolbar
  if (m_pDlgToolsOptions->m_dlgViewPage.m_bToolbar != bToolBar())
  {
    BOOL bAdvanced = m_pDlgToolsOptions->m_dlgViewPage.m_bToolbar;
    ShowControlBar(GetControlBar(IDR_BAR_BASIC),!bAdvanced, FALSE); // change toolbar status
    ShowControlBar(GetControlBar(IDR_BAR_ADVANCED), bAdvanced, FALSE); // change toolbar status
  }
  // apply to taskbar
  if (m_pDlgToolsOptions->m_dlgViewPage.m_bTaskbar != bTaskBar())
  {
    BOOL bTaskbar = m_pDlgToolsOptions->m_dlgViewPage.m_bTaskbar;
    ShowControlBar(GetControlBar(ID_VIEW_TASKBAR),bTaskbar, FALSE); // change taskbar status
  }
  // apply tone position
  if (m_pDlgToolsOptions->m_dlgViewPage.m_bToneAbove != m_bToneAbove)
  {
    m_bToneAbove = !m_bToneAbove;
    if(m_bToneAbove)
    {
      CGraphWnd::m_anAnnWndOrder[1] = TONE;
      CGraphWnd::m_anAnnWndOrder[2] = PHONETIC;
    }
    else
    {
      CGraphWnd::m_anAnnWndOrder[1] = PHONETIC;
      CGraphWnd::m_anAnnWndOrder[2] = TONE;
    }
    SendMessageToMDIDescendants(WM_USER_GRAPH_ORDERCHANGED, 0, 0L);
  }
  // apply to scrolling zoom
  if (m_pDlgToolsOptions->m_dlgViewPage.m_bScrollZoom != m_bScrollZoom)
  {
    m_bScrollZoom = !m_bScrollZoom;
    // tell about the change to all views
    SendMessageToMDIDescendants(WM_USER_VIEW_SCROLLZOOMCHANGED, m_bScrollZoom, 0L); 
  }

  // update cursor alignment for the active view
  CURSOR_ALIGNMENT nCursorAlignmentSetting = (CURSOR_ALIGNMENT) m_pDlgToolsOptions->m_dlgViewPage.m_nCursorAlignment;
  CSaView* pView = (CSaView*)GetCurrSaView();
  if (pView && nCursorAlignmentSetting != pView->GetCursorAlignment())
    pView->ChangeCursorAlignment(nCursorAlignmentSetting);

  // This becomes the new default setting
  m_nCursorAlignment = nCursorAlignmentSetting;

  // update graph mode
  int nGraphUpdateModeSetting = m_pDlgToolsOptions->m_dlgViewPage.m_nGraphUpdateMode;
  if (nGraphUpdateModeSetting != GetGraphUpdateMode())
  {
    SetGraphUpdateMode(nGraphUpdateModeSetting);
    SendMessageToMDIDescendants(WM_USER_VIEW_GRAPHUPDATECHANGED, 0L, 0L);
  }
  // update animation
  int bAnimateSetting = m_pDlgToolsOptions->m_dlgViewPage.m_bAnimate;
  int nAnimationRateSetting = m_pDlgToolsOptions->m_dlgViewPage.m_nAnimationRate;
  if (bAnimateSetting != IsAnimationRequested() ||   
      nAnimationRateSetting != GetAnimationFrameRate())
  {
    m_bAnimate = bAnimateSetting;
    m_nAnimationRate = nAnimationRateSetting;
    SendMessageToMDIDescendants(WM_USER_VIEW_ANIMATIONCHANGED, 0, 0); 
  }
  // apply graph caption style
  if (m_pDlgToolsOptions->m_dlgViewPage.m_nCaptionStyle != m_nCaptionStyle)
  {
    if (AfxMessageBox(IDS_QUESTION_RECREATEGRAPHS, MB_YESNO | MB_ICONQUESTION,0) == IDYES)
    {
      m_nCaptionStyle = m_pDlgToolsOptions->m_dlgViewPage.m_nCaptionStyle;                  
      // tell about the change to all views
      SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
    }
  }
  // apply graph grid style
  int nDlgXStyle, nDlgYStyle;
  
  m_pDlgToolsOptions->m_dlgViewPage.GetGridStyle(&nDlgXStyle, &nDlgYStyle);
  
  if ((m_pDlgToolsOptions->m_dlgViewPage.m_bXGrid != m_grid.bXGrid)
      || (m_pDlgToolsOptions->m_dlgViewPage.m_bYGrid != m_grid.bYGrid)
      || (nDlgXStyle != m_grid.nXStyle) || (nDlgYStyle != m_grid.nYStyle))
  {
    m_grid.bXGrid = m_pDlgToolsOptions->m_dlgViewPage.m_bXGrid;
    m_grid.bYGrid = m_pDlgToolsOptions->m_dlgViewPage.m_bYGrid;
    m_grid.nXStyle = nDlgXStyle;
    m_grid.nYStyle = nDlgYStyle;
    // tell about the change to all views
    SendMessageToMDIDescendants(WM_USER_GRAPH_GRIDCHANGED, 0, 0L); 
  }
  // apply graph colors
  if (m_pDlgToolsOptions->m_dlgColorPage.m_bColorsChanged)
  {
    m_pDlgToolsOptions->m_dlgColorPage.m_bColorsChanged = FALSE;
    m_colors = m_pDlgToolsOptions->m_dlgColorPage.m_cColors;
    // tell about the change to all views
    SendMessageToMDIDescendants(WM_USER_GRAPH_COLORCHANGED, 0, 0L); 
  }
  // apply graph fonts
  if (m_pDlgToolsOptions->m_dlgFontPage.m_bFontChanged)
  {
    // get pointer to active document
    CSaDoc* pDoc = (CSaDoc*)GetCurrSaView()->GetDocument();
    m_pDlgToolsOptions->m_dlgFontPage.m_bFontChanged = FALSE;
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
      SetFontFace(nLoop, m_pDlgToolsOptions->m_dlgFontPage.m_GraphFonts.GetAt(nLoop));
      SetFontSize(nLoop, m_pDlgToolsOptions->m_dlgFontPage.m_GraphFontSizes.GetAt(nLoop));
    }
    pDoc->CreateFonts();
    // tell about the change to all views
    SendMessageToMDIDescendants(WM_USER_GRAPH_FONTCHANGED, 0, 0L); 
  }
  CSaApp::m_bUseUnicodeEncoding = m_pDlgToolsOptions->m_dlgFontPage.m_bUseUnicodeEncoding;


  //***********************************************************
  // Save the values from the startup tab
  //***********************************************************
  m_bShowStartupDlg = m_pDlgToolsOptions->m_dlgSavePage.m_showStartupDlg; // DDO - 08/03/00
  m_bSaveOpenFiles = m_pDlgToolsOptions->m_dlgSavePage.m_saveOpenFiles;   // tdg - 09/03/97
  return 0;
}

CDlgPlayer* CMainFrame::GetPlayer(BOOL bCreate)
{
  if (!CDlgPlayer::bPlayer) // player dialog not launched
  {
    if(!bCreate)
      return NULL;
    
    if (m_pDlgPlayer) delete m_pDlgPlayer; // delete old dialog object
    m_pDlgPlayer = new CDlgPlayer; // create new player object
    
    if (!CDlgPlayer::bPlayer) // player dialog not launched
    {
      m_pDlgPlayer->Create(); // create player
    }
  }
  return m_pDlgPlayer;
}

/***************************************************************************/
// CMainFrame::OnPlayer Launches the player
// The user wants to see the modeless player dialog. If not already done
// it creates the player dialog, otherwise it just sets the player active
// and on top of all other windows. lParam delivers the player mode to be
// set.
/***************************************************************************/
LRESULT CMainFrame::OnPlayer(WPARAM wParam, LPARAM lParam)
{
  return OnPlayer(wParam, lParam, NULL);
}

/***************************************************************************/
// CMainFrame::OnPlayer Launches the player
// The user wants to see the modeless player dialog. If not already done
// it creates the player dialog, otherwise it just sets the player active
// and on top of all other windows. lParam delivers the player mode to be
// set.
/***************************************************************************/
LRESULT CMainFrame::OnPlayer(WPARAM wParam, LPARAM lParam, SSpecific *pSpecific)
{
  TRACE(_T("OnPlayer %x %x %x\n"),wParam,HIWORD(lParam),LOWORD(lParam));
  
  CWnd* pWnd = GetActiveWindow(); // retrieve pointer to active window
  if (!CDlgPlayer::bPlayer) // player dialog not launched
  {
    GetPlayer(true); // get or create player object
    BOOL bFnKey = FALSE;
    // RLJ 09/14/2000: Update for 32-bit
    //      if (HIWORD(lParam) == -1)
    if (HIWORD(lParam) == (WORD) -1)
    {
      // function key call
      bFnKey = TRUE;
      lParam = MAKELONG(LOWORD(lParam), FALSE);
    }
    // if player will not have size, set the old active window
    if (!(BOOL)HIWORD(lParam)) pWnd->SetActiveWindow();
    GetPlayer(false)->SetPlayerMode(wParam, LOWORD(lParam), (BOOL)HIWORD(lParam), bFnKey, pSpecific); // set player mode
  }
  else
  {
    if (!GetPlayer(false)->IsTestRun())
    {
      // player not running function key test
      BOOL bFnKey = FALSE;
      //kg 32 bit cast needed
      if (HIWORD(lParam) == (WORD)-1)
      {
        // function key call
        bFnKey = TRUE;
        lParam = MAKELONG(LOWORD(lParam), FALSE);
      }
      GetPlayer(false)->SetPlayerMode(wParam, LOWORD(lParam), (BOOL)HIWORD(lParam), bFnKey, pSpecific); // set player mode
      if (GetPlayer(false)->IsFullSize()) // player has full size, set it the active window
      {
        GetPlayer(false)->SetActiveWindow(); // set focus on player
        GetPlayer(false)->ShowWindow(SW_SHOW);
      }
    }
  }
  return 0;
}

// SDM 1.06.5
/**************************************************************************/
// CMainFrame::OnEditor Launches the transcription editor
// The user wants to see the modeless transcription editor. If not already done
// it creates the editor dialog, otherwise it just sets the editor active
// and on top of all other windows. 
/***************************************************************************/
void CMainFrame::OnEditor()
{
  if(!IsEditAllowed()) return;
  if(!m_pDlgEditor) 
  {
    m_pDlgEditor = new CDlgEditor(this);  // New Editor with view parent
  }
  if(m_pDlgEditor) 
  {
    // Create window if necessary
    m_pDlgEditor->CreateSafe(CDlgEditor::IDD, this, &m_wplDlgEditor);
  }
  
  if (m_pDlgEditor  && !m_pDlgEditor->IsWindowVisible())
    m_pDlgEditor->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
  else if(m_pDlgEditor)
    m_pDlgEditor->ShowWindow(SW_HIDE);
}

/**************************************************************************/
/**************************************************************************/
void CMainFrame::OnUpdateEditEditor(CCmdUI* pCmdUI)
{
  if(m_pDlgEditor  && m_pDlgEditor->IsWindowVisible())  
    pCmdUI->SetText(_T("Hide Transcription Editor\tF4"));
  else
    pCmdUI->SetText(_T("Show Transcription Editor\tF4"));
}

// SDM 1.06.5
/***************************************************************************/
// CMainFrame::OnIdleUpdate handles idle time update needs
/***************************************************************************/
LRESULT CMainFrame::OnIdleUpdate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  if(m_pDlgEditor) m_pDlgEditor->UpdateDialog();
  
  return 0;
}

// SDM 1.5Test8.3
/***************************************************************************/
// CMainFrame::IsEditAllowed
/***************************************************************************/
BOOL CMainFrame::IsEditAllowed()
{
  if(m_pDisplayPlot) return FALSE;
  else return TRUE;
}


/***************************************************************************/
// CMainFrame::OnEditFind Launches the find dialog
/***************************************************************************/
void CMainFrame::OnEditFind()
{
  MaybeCreateFindOrReplaceDlg(TRUE);
}

/***************************************************************************/
// CMainFrame::OnUpdateEditFind Menu Update
/***************************************************************************/
void CMainFrame::OnUpdateEditFind(CCmdUI* pCmdUI)
{
  OnUpdateEditReplace(pCmdUI);
}

/***************************************************************************/
// CMainFrame::OnEditReplace Launches the replace dialog
/***************************************************************************/
void CMainFrame::OnEditReplace()
{
  MaybeCreateFindOrReplaceDlg(FALSE);
}

/***************************************************************************/
// CMainFrame::OnUpdateEditReplace Menu Update
/***************************************************************************/
void CMainFrame::OnUpdateEditReplace(CCmdUI* pCmdUI)
{
  CSaDoc* pDoc = GetCurrDoc();
  
  if (pDoc) 
  {
    pCmdUI->Enable(pDoc->GetUnprocessedDataSize() != 0); // enable if data is available
  } 
  else 
  {
    pCmdUI->Enable(FALSE);
  }
}


#if 0
/***************************************************************************/
// CMainFrame::OnFindNextCmd Message from the find/replace dialog
/***************************************************************************/
void CMainFrame::OnFindNextCmd()
{
  ASSERT(m_pDlgFind != NULL);
  
  int annotationSetID = m_pDlgFind->AnnotationSetID();
  // Look for the string in the appropriate view.
  CSaView* pView = (CSaView*)GetCurrSaView();
  if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) 
  {
    pView->FindNextAnnotation(annotationSetID, m_pDlgFind->GetFindString());
  }
}


/***************************************************************************/
// CMainFrame::OnFindPrevCmd Message from the find/replace dialog
/***************************************************************************/
void CMainFrame::OnFindPrevCmd()
{
  ASSERT(m_pDlgFind != NULL);
  
  int annotationSetID = m_pDlgFind->AnnotationSetID();
  // Look for the string in the appropriate view.
  CSaView* pView = (CSaView*)GetCurrSaView();
  if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) 
  {
    pView->FindPrevAnnotation(annotationSetID, m_pDlgFind->GetFindString());
  }
}
#endif


/***************************************************************************/
// CMainFrame::OnChangeView Message from a view
// A view is loosing focus. If the player is actually launched, it has to be
// informed to stop playing immediatly and to change caption. The statusbar
// panes have to be cleared if there is no more view.
/***************************************************************************/
LRESULT CMainFrame::OnChangeView(WPARAM wParam, LPARAM lParam)
{
  if (m_pDlgFind) 
  {
    if (wParam) 
    {
      m_pDlgFind->ChangeView();
    } 
    else 
    {
      m_pDlgFind->SendMessage(WM_CLOSE);
    }
  }
  if (CDlgPlayer::bPlayer) // player dialog launched
  {
    if (wParam) GetPlayer(false)->ChangeView((CSaView*)lParam); // inform player
    else GetPlayer(false)->SendMessage(WM_CLOSE); // last view closed, close player too
  }
  if (!wParam) // last view closed
  {
    // get pointer to status bar
    CDataStatusBar* pStat = GetDataStatusBar();
    // turn off symbols
    pStat->SetPaneSymbol(ID_STATUSPANE_1, FALSE);
    pStat->SetPaneSymbol(ID_STATUSPANE_2, FALSE);
    pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
    pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
    // clear the panes
    pStat->SetPaneText(ID_STATUSPANE_1, _T(""));
    pStat->SetPaneText(ID_STATUSPANE_2, _T(""));
    pStat->SetPaneText(ID_STATUSPANE_3, _T(""));
    pStat->SetPaneText(ID_STATUSPANE_4, _T(""));
  }
  
  return 0;
}

/***************************************************************************/
// CMainFrame::OnClose Mainframe closing
// Called from the framework. The mainframe window is closing. If the player
// is actually launched, it has to be informed to stop playing immediatly.
// The find/replace dialog has also to be closed and the same for the
// workbench view. If SA runs in batch mode, don't close, but call the return
// to SM menu item.
/***************************************************************************/
void CMainFrame::OnClose()
{
  CSaApp* pApp = (CSaApp*)AfxGetApp();
  if (pApp->GetBatchMode() == 1) 
  {
    // TODO: this can probably be completely removed at some point
		SendMessage(WM_COMMAND, ID_FILE_RETURN, 0L);
    return;
  }

  if (!AfxGetApp()->SaveAllModified()) // user canceled close operation
    return; 
  
  // check if workbench document is open
  CDocument* pWbDoc = ((CSaApp *)AfxGetApp())->GetWbDoc();
  
  //******************************************************
  // close the workbench 
  //******************************************************
  if (pWbDoc) 
  {
    POSITION pos = pWbDoc->GetFirstViewPosition();
    CView* pView = pWbDoc->GetNextView(pos);
    pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0); // close view
  }
  
  ((CSaApp *)AfxGetApp())->SetWbOpenOnExit(pWbDoc != NULL);
  
  //******************************************************
  // Workbench is still there, don't close
  //******************************************************
  if (((CSaApp *)AfxGetApp())->GetWbDoc()) return;
  
  //******************************************************
  //******************************************************
  //if (m_bSaveOnExit && !m_bPrintPreviewInProgress)
  if (!m_bPrintPreviewInProgress)
    ((CSaApp *)AfxGetApp())->WriteSettings();
  
  //******************************************************
  // If player dialog open then close it.
  //******************************************************
  if (CDlgPlayer::bPlayer) m_pDlgPlayer->SendMessage(WM_CLOSE);
  
  //******************************************************
  // If find dialog open then close it.
  //******************************************************
  if (m_pDlgFind) m_pDlgFind->SendMessage(WM_CLOSE);
  
  CMainFrameBase::OnClose();
}

/***************************************************************************/
// CMainFrame::OnSysColorChange System colors changed
/***************************************************************************/
void CMainFrame::OnSysColorChange()
{
  CMainFrameBase::OnSysColorChange();
  // update system colors
  m_colors.SetupSystemColors();
  // apply color changes to graphs
  SendMessageToMDIDescendants(WM_USER_GRAPH_STYLECHANGED, 0, 0L);
}

/***************************************************************************/
// CMainFrame::OnUpdateDataPane Data statusbar pane updating
/***************************************************************************/
void CMainFrame::OnUpdateDataPane(CCmdUI *pCmdUI)
{
  if (m_bIsPrinting) 
  {
  } 
  else 
  {
    pCmdUI->Enable();
  }
}

/***************************************************************************/
// CMainFrame::OnUpdateProgressPane Progress statusbar pane updating
/***************************************************************************/
void CMainFrame::OnUpdateProgressPane(CCmdUI *pCmdUI)
{
  if (m_bIsPrinting) 
  {
  } 
  else 
  {
    pCmdUI->Enable();
  }
}

/***************************************************************************/
// CMainFrame::OnSpeechAppCall Call from another application
// Another application wants to start up SA. Check out what to do. If SA
// is to close, be sure to first take SA out of batch mode to enable standard
// exit procedure and then send the ID_APP_EXIT.
/***************************************************************************/
LRESULT CMainFrame::OnSpeechAppCall(WPARAM wParam, LPARAM lParam)
{
  // ASSERT(FALSE); // Assert to debug this function
  CSaApp* pApp = (CSaApp*)AfxGetApp();
  if (wParam == SPEECH_WPARAM_CLOSE)
  {
    pApp->CancelBatchMode(); // cancel batch mode for exit
    PostMessage(WM_COMMAND, ID_APP_EXIT, 0L); // close SA
  }
  CSaString szCmd;
  UNUSED_ALWAYS(lParam);
  if (wParam == SPEECH_WPARAM_SHOWSA) pApp->ExamineCmdLine(szCmd, wParam); // start up SA in batch mode with list file
  if (wParam == SPEECH_WPARAM_SHOWSAREC) pApp->ExamineCmdLine(szCmd, wParam); // start up SA in batch mode in recording mode
  return 0;
}

/***************************************************************************/
// CMainFrame::GetAnnotation
// Gets the annotation set index annotSetID for the current view/doc.
/***************************************************************************/
CSegment * CMainFrame::GetAnnotation(int annotSetID)
{
  CSegment * pSeg = NULL;
  CSaView* pView = (CSaView*)GetCurrSaView();
  
  if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) 
  {
    pSeg = pView->GetAnnotation(annotSetID);
  }
  
  return pSeg;
}       


//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::GetCurrSaView
// returns the active SaView.
/***************************************************************************/
CSaView * CMainFrame::GetCurrSaView(void)
{
  CMDIChildWnd* pMDIWnd = MDIGetActive();  // get active child window
  
  if (pMDIWnd)
  {
    CSaView* pView = (CSaView*)MDIGetActive()->GetActiveView();
    
    if ((pView != NULL) && pView->IsKindOf(RUNTIME_CLASS(CSaView))) 
      return pView;
  }    
  
  return NULL;
}


/***************************************************************************/
// CMainFrame::GetCurrDoc
// returns the doc for the active view.
/***************************************************************************/
CSaDoc * CMainFrame::GetCurrDoc()
{
  CSaDoc * pDoc = NULL;
  CSaView* pView = GetCurrSaView();
  
  if (pView) pDoc = pView->GetDocument();
  
  return pDoc;
}




/***************************************************************************/
// CMainFrame::SetPrintingFlag
// Sets the "is printing" state to true.  Passes on the setting to the 
// progress bar.
/***************************************************************************/
void  CMainFrame::SetPrintingFlag() 
{
  m_bIsPrinting = TRUE; 
  m_progressStatusBar.SetIsPrintingFlag(m_bIsPrinting);
};




/***************************************************************************/
// CMainFrame::ClearPrintingFlag
/***************************************************************************/
void  CMainFrame::ClearPrintingFlag()
{
  m_bIsPrinting = FALSE; 
  m_progressStatusBar.SetIsPrintingFlag(m_bIsPrinting);
};


//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::OnSaveScreenAsBMP
/***************************************************************************/
void CMainFrame::OnSaveScreenAsBMP()
{
  CDib * pCDib = new CDib;
  CWnd* pWnd = GetTopLevelParent();
  if(!pWnd)
    pWnd = this;
  if(pCDib) pCDib->CaptureWindow(pWnd);
  if(pCDib) pCDib->Save();
  if(pCDib) delete pCDib;
}

//SDM 1.06.6U5
/***************************************************************************/
// CMainFrame::OnSaveWindowAsBMP
/***************************************************************************/
void CMainFrame::OnSaveWindowAsBMP()
{
  CWnd* pWnd = this;

  if(!pWnd) return;
  CDib * pCDib = new CDib;

  CRect rectCrop(0,0,0,0);

  CRect rectToolbar, rectMainWnd;
  GetControlBar(IDR_BAR_BASIC)->GetWindowRect(&rectToolbar);
  AfxGetMainWnd()->GetWindowRect(&rectMainWnd);
  int nHeight = rectToolbar.bottom - rectToolbar.top;
  int nWidth = rectToolbar.right - rectToolbar.left;
  if (!GetControlBar(IDR_BAR_BASIC)->IsFloating())
  {
    if ((nWidth > nHeight) && (rectToolbar.top < (rectMainWnd.top + rectMainWnd.bottom) / 2))
    {
      rectCrop.top = nHeight - 2;
    }
    if ((nWidth < nHeight) && (rectToolbar.left < (rectMainWnd.left + rectMainWnd.right) / 2))
    {
      rectCrop.left = nWidth - 2;
    }
  }
  
  if(pCDib) pCDib->CaptureWindow(pWnd, rectCrop, TRUE);
  if(pCDib) pCDib->Save();
  if(pCDib) delete pCDib;
}

//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::OnSaveGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnSaveGraphsAsBMP()
{
  CSaView* pSaView = GetCurrSaView();
  if(!pSaView) return;
  CDib * pCDib = new CDib;
  // SDM 1.06.6U5 capture client area
  if(pCDib) pCDib->CaptureWindow(pSaView);
  if(pCDib) pCDib->Save();
  if(pCDib) delete pCDib;
}

//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::OnCopyScreenAsBMP
/***************************************************************************/
void CMainFrame::OnCopyScreenAsBMP()
{
  CDib * pCDib = new CDib;
  CWnd* pWnd = GetTopLevelParent();
  if(!pWnd)
    pWnd = this;
  if(pCDib) pCDib->CaptureWindow(pWnd);
  if(pCDib) pCDib->CopyToClipboard(pWnd);
  if(pCDib) delete pCDib;
}

//SDM 1.06.6U5
/***************************************************************************/
// CMainFrame::OnCopyWindowAsBMP
/***************************************************************************/
void CMainFrame::OnCopyWindowAsBMP()
{
  CWnd* pWnd = this;

  if(!pWnd) return;
  CDib * pCDib = new CDib;

  CRect rectCrop(0,0,0,0);

  CRect rectToolbar, rectMainWnd;
  GetControlBar(IDR_BAR_BASIC)->GetWindowRect(&rectToolbar);
  AfxGetMainWnd()->GetWindowRect(&rectMainWnd);
  int nHeight = rectToolbar.bottom - rectToolbar.top;
  int nWidth = rectToolbar.right - rectToolbar.left;
  if (!GetControlBar(IDR_BAR_BASIC)->IsFloating())
  {
    if ((nWidth > nHeight) && (rectToolbar.top < (rectMainWnd.top + rectMainWnd.bottom) / 2))
    {
      rectCrop.top = nHeight - 2;
    }
    if ((nWidth < nHeight) && (rectToolbar.left < (rectMainWnd.left + rectMainWnd.right) / 2))
    {
      rectCrop.left = nWidth - 2;
    }
  }
  
  if(pCDib) pCDib->CaptureWindow(pWnd, rectCrop, TRUE);
  if(pCDib) pCDib->CopyToClipboard(pWnd);
  if(pCDib) delete pCDib;
}

//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::OnCopyGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnCopyGraphsAsBMP()
{
  CSaView* pSaView = GetCurrSaView();
  if (!pSaView) return;
  CDib * pCDib = new CDib;
  // SDM 1.06.6U5 capture client area
  if (pCDib) pCDib->CaptureWindow(pSaView);
  if (pCDib) pCDib->CopyToClipboard(pSaView);
  if (pCDib) delete pCDib;
}

//SDM 1.06.6U4 
/***************************************************************************/
// CMainFrame::OnUpdateGraphsAsBMP
/***************************************************************************/
void CMainFrame::OnUpdateGraphsAsBMP(CCmdUI* pCmdUI)
{
  pCmdUI->Enable((GetCurrSaView() != NULL));
}

/***************************************************************************/
// CMainFrame::OnSetDefault
/***************************************************************************/
void CMainFrame::OnSetDefaultGraphs(BOOL bPermanent)
{
  //**********************************************************
  // Get pointers to the active view and it's document.
  //**********************************************************
  CSaView* pSrcView = (CSaView*)GetCurrSaView();
  
  //**********************************************************
  // If a view class for the default view has exists then
  // delete it so we can create a new default from the
  // current view.  DDO - 08/07/00
  //**********************************************************
  if (m_pDefaultViewConfig) delete m_pDefaultViewConfig;
  m_pDefaultViewConfig = new CSaView(pSrcView);  
  
  //**********************************************************
  // Build a string of graph names used to display on the
  // dialog to tell the user the current default view
  // graphs. Also get the layout (used to show the user an
  // icon of the current layout).
  //**********************************************************
  m_szTempDefaultGraphs = "";
  m_nTempDefaultLayout = (bPermanent) ? 0 : pSrcView->GetLayout();
  
  if (bPermanent) 
  {
    m_szPermDefaultGraphs = "";
    m_nPermDefaultLayout = pSrcView->GetLayout();
  }
  
  //**********************************************************
  // Build a string from graph captions.
  //**********************************************************
  ((bPermanent) ? m_szPermDefaultGraphs : m_szTempDefaultGraphs) = m_pDefaultViewConfig->GetGraphsDescription();
  
  //**********************************************************
  // SDM 1.5Test8.1
  //**********************************************************
  BOOL bMaximized = m_bDefaultMaximizeView;

  if (MDIGetActive(&bMaximized)) 
  {
    m_bDefaultMaximizeView = bMaximized;
    WINDOWPLACEMENT WP;
    WP.length = sizeof(WINDOWPLACEMENT);
    if (MDIGetActive()->GetWindowPlacement(&WP)) 
    {
      // SDM 32 bit conversion
      m_nDefaultHeightView = WP.rcNormalPosition.bottom - WP.rcNormalPosition.top;
      m_nDefaultWidthView  = WP.rcNormalPosition.right - WP.rcNormalPosition.left;
    }
  }
  
  //**********************************************************
  // If the user clicked on the permanent button then save
  // the default settings to the file that holds the
  // temporary default view settings. DDO - 08/08/00
  //**********************************************************
  if (bPermanent) 
  {
    m_nStartDataMode = 0;
    WriteReadDefaultViewToTempFile(TRUE);
  }
}

/***************************************************************************/
// CMainFrame::OnSetDefaultParameters
/***************************************************************************/
void CMainFrame::OnSetDefaultParameters()
{
  //**********************************************************
  // Get pointers to the active view and it's document.
  //**********************************************************
  CSaView* pSrcView = (CSaView*)GetCurrSaView();
  CSaDoc*  pDoc = pSrcView->GetDocument();
  
  //**********************************************************
  // RLJ 1.5Test11.1A - Copy current pitch parameter values
  // to defaults.
  //**********************************************************
  const PitchParm* pPitchParm                    = pDoc->GetPitchParm();   // Get pitch parameter values.
  m_pitchParmDefaults.nRangeMode           = pPitchParm->nRangeMode;
  m_pitchParmDefaults.nScaleMode           = pPitchParm->nScaleMode;
  m_pitchParmDefaults.nUpperBound          = pPitchParm->nUpperBound;
  m_pitchParmDefaults.nLowerBound          = pPitchParm->nLowerBound;
  m_pitchParmDefaults.bUseCepMedianFilter  = pPitchParm->bUseCepMedianFilter;
  m_pitchParmDefaults.nCepMedianFilterSize = pPitchParm->nCepMedianFilterSize;  
  
  //**********************************************************
  // Copy current music parameter values
  // to defaults.
  //**********************************************************
  const MusicParm* pMusicParm                    = pDoc->GetMusicParm();   // Get pitch parameter values.
  m_musicParmDefaults.nRangeMode           = pMusicParm->nRangeMode;
  m_musicParmDefaults.nUpperBound          = pMusicParm->nUpperBound;
  m_musicParmDefaults.nLowerBound          = pMusicParm->nLowerBound;

  m_intensityParmDefaults = pDoc->GetIntensityParm();

  //**********************************************************
  // RLJ 1.5Test11.1A - Get pointer to view.
  //**********************************************************
  CSaView* pView = (CSaView*)GetCurrSaView();
  UINT* pnID = pView->GetGraphIDs();
  
  //**********************************************************
  // Check to see which (if any) of spectrum, Spectrogram,
  // and Snapshot graphs are displayed.
  //**********************************************************
  BOOL bFormantGraphUsed      = FALSE;
  BOOL bSpectrumGraphUsed     = FALSE;
  BOOL bSpectrogramGraphUsed = FALSE;
  BOOL bSnapshotGraphUsed = FALSE;
  
  for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) 
  {
    if (pnID[nLoop] == IDD_3D || 
				pnID[nLoop] == IDD_F1F2 || 
				pnID[nLoop] == IDD_F2F1 || 
				pnID[nLoop] == IDD_F2F1F1 ||
				pnID[nLoop] == IDD_WAVELET)			// 7/31/01 ARH - Added for the wavelet scalogram view

      bFormantGraphUsed = TRUE;
    else if (pnID[nLoop] == IDD_SPECTRUM) bSpectrumGraphUsed = TRUE;
    else if (pnID[nLoop] == IDD_SPECTROGRAM) bSpectrogramGraphUsed = TRUE;
    else if (pnID[nLoop] == IDD_SNAPSHOT) bSnapshotGraphUsed = TRUE;
  }
  
  //**********************************************************
  // AKE 1.5Test13.1 - Copy current formant chart parameter
  // values to defaults.
  //**********************************************************
  if (bFormantGraphUsed) 
  {
    CProcessFormants* pFormants                 = (CProcessFormants*)pDoc->GetFormants(); 
    FormantParm* pFormantParm                   = pFormants->GetFormantParms();   // Get formant chart parameter values.
    m_formantParmDefaults.bFromLpcSpectrum      = pFormantParm->bFromLpcSpectrum;
    m_formantParmDefaults.bFromCepstralSpectrum = pFormantParm->bFromCepstralSpectrum;
    m_formantParmDefaults.bTrackFormants        = pFormantParm->bTrackFormants;
    m_formantParmDefaults.bSmoothFormants       = pFormantParm->bSmoothFormants;
    m_formantParmDefaults.bMelScale             = pFormantParm->bMelScale;
  }
  
  //**********************************************************
  // RLJ 1.5Test11.1A - Copy current spectrum parameter
  // values to defaults.
  //**********************************************************
  if (bSpectrumGraphUsed) 
  {
    CProcessSpectrum* pSpectrum            = (CProcessSpectrum*)pDoc->GetSpectrum();
    SpectrumParm* pSpectrumParm            = pSpectrum->GetSpectrumParms();   // Get spectrum parameter values.
    m_spectrumParmDefaults.nScaleMode      = pSpectrumParm->nScaleMode;
    m_spectrumParmDefaults.nPwrUpperBound  = pSpectrumParm->nPwrUpperBound;
    m_spectrumParmDefaults.nPwrLowerBound  = pSpectrumParm->nPwrLowerBound;
    m_spectrumParmDefaults.nFreqUpperBound = pSpectrumParm->nFreqUpperBound;
    m_spectrumParmDefaults.nFreqLowerBound = pSpectrumParm->nFreqLowerBound;
    m_spectrumParmDefaults.nFreqScaleRange = pSpectrumParm->nFreqScaleRange;
    m_spectrumParmDefaults.nSmoothLevel    = pSpectrumParm->nSmoothLevel;
    m_spectrumParmDefaults.nPeakSharpFac   = pSpectrumParm->nPeakSharpFac;
    m_spectrumParmDefaults.cWindow      = pSpectrumParm->cWindow;
    m_spectrumParmDefaults.bShowLpcSpectrum  = pSpectrumParm->bShowLpcSpectrum;
	m_spectrumParmDefaults.bShowCepSpectrum  = pSpectrumParm->bShowCepSpectrum;
    m_spectrumParmDefaults.bShowFormantFreq  = pSpectrumParm->bShowFormantFreq;
	m_spectrumParmDefaults.bShowFormantBandwidth  = pSpectrumParm->bShowFormantBandwidth;
    m_spectrumParmDefaults.bShowFormantPower = pSpectrumParm->bShowFormantPower;
  }
  
  //**********************************************************
  // RLJ 1.5Test11.1A - Get process index of Spectrogram
  // graph.
  //**********************************************************
  if (bSpectrogramGraphUsed) 
  {
    CGraphWnd* pGraph = NULL;
    
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) 
    {
      if (pnID[nLoop] == IDD_SPECTROGRAM) pGraph = pView->GetGraph(nLoop);
    }
    
    ASSERT(pGraph);
    
    //********************************************************
    // Copy current Spectrogram parameter values to object
    // containing Spectrogram defaults.
    //********************************************************
    CProcessSpectrogram* pSpectro               = (CProcessSpectrogram*)pDoc->GetSpectrogram(TRUE);
    m_spectrogramParmDefaults = pSpectro->GetSpectroParm();

    if(m_spectrogramParmDefaults.nFrequency >= int(pDoc->GetFmtParm()->dwSamplesPerSec*45/100))
      // This spectrogram is set to near nyquist
      // Assume the user wants all spectrograms to be display at nyquist
      // Set frequency above any reasonable sampling nyquist to force clipping to nyquist
      m_spectrogramParmDefaults.nFrequency = 99999;
  }
  
  if (bSnapshotGraphUsed) 
  {
    CGraphWnd* pGraph = NULL;
    
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) 
    {
      if (pnID[nLoop] == IDD_SNAPSHOT) pGraph = pView->GetGraph(nLoop);
    }
    
    ASSERT(pGraph);
    
    //********************************************************
    // Copy current Snapshot parameter values to object
    // containing Snapshot defaults.
    //********************************************************
    CProcessSpectrogram* pSpectro               = (CProcessSpectrogram*)pDoc->GetSpectrogram(FALSE);
    m_snapshotParmDefaults = pSpectro->GetSpectroParm();

    if(m_snapshotParmDefaults.nFrequency >= int(pDoc->GetFmtParm()->dwSamplesPerSec*45/100))
      // This spectrogram is set to near nyquist
      // Assume the user wants all spectrograms to be display at nyquist
      // Set frequency above any reasonable sampling nyquist to force clipping to nyquist
      m_snapshotParmDefaults.nFrequency = 99999;
  }
  
}

/***************************************************************************/
// CMainFrame::OnEqualizeLength
/***************************************************************************/
void CMainFrame::OnEqualizeLength()
{
  CTargViewDlg tView(this);
  
  if (tView.DoModal()==IDOK) 
  {
    CSaView * pSrcView = (CSaView*)GetCurrSaView();
    CSaView * pTarg = tView.Targ();
    if (pTarg) 
    {
      CSaDoc  * pTargDoc = pTarg->GetDocument();
      if (pTargDoc)
      {
        CSaDoc *pSrcDoc = pSrcView->GetDocument();
        FmtParm * pSrcFmtParm = pSrcDoc->GetFmtParm();
        WORD wSrcSmpSize = (WORD)(pSrcFmtParm->wBlockAlign / pSrcFmtParm->wChannels);
        DWORD SrcLen = (pSrcView->GetStopCursorPosition() -
          pSrcView->GetStartCursorPosition() + wSrcSmpSize) / wSrcSmpSize;
        FmtParm * pTargFmtParm = pTargDoc->GetFmtParm();
        WORD wTargSmpSize = (WORD)(pTargFmtParm->wBlockAlign / pTargFmtParm->wChannels);
        DWORD TargStop = pTarg->GetStartCursorPosition() +
                         (DWORD)((double)SrcLen * (double)pTargFmtParm->dwSamplesPerSec /
                         (double)pSrcFmtParm->dwSamplesPerSec + 0.5) * wTargSmpSize;
        if (TargStop > pTargDoc->GetUnprocessedDataSize()) 
        {
          AfxMessageBox(IDS_EQUALIZE_TOO_FAR_RIGHT);
        } 
        else 
        {
          pTarg->SetStopCursorPosition(TargStop);
        }
      }
    }
  }
}

/***************************************************************************/
// CMainFrame::OnUpdateEqualizeLength
/***************************************************************************/
void CMainFrame::OnUpdateEqualizeLength(CCmdUI* pCmdUI)
{
  int count = 0;
  CDocList doclst; // list of currently open docs
  
  for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() ) 
  {
    if (pdoc != NULL) 
    {
      count++;
    }
  }
  
  pCmdUI->Enable(count>1);
}


/***************************************************************************/
// CMainFrame::CreateFindReplaceDlg Launches the find/replace dialog
// The user wants to see the modeless find dialog.
/***************************************************************************/
void CMainFrame::CreateFindOrReplaceDlg()
{
  // Get the string segment represented;
  CSaString sToFind;
  int  annotWndIndex = 0;
  CSaView* pView = (CSaView*)GetCurrSaView();
  if (pView->IsAnyAnnotationSelected()) 
  {
    sToFind = pView->GetSelectedAnnotationString();
    annotWndIndex = pView->FindSelectedAnnotationIndex();
  }
  CSaString sFields;
  sFields.LoadString(IDS_FINDFIELDS);   
  m_pDlgFind = new CSaFindDlg((CWnd*)this,sFields,
                               sToFind,m_bFindOnly,CSaString(_T("")),annotWndIndex,this);
  if (!m_pDlgFind->Created()) 
  {
    delete m_pDlgFind;
    m_pDlgFind = NULL;
    ASSERT_VALID(this);
  }
}



/***************************************************************************/
// CMainFrame::MaybeCreateFindOrReplaceDlg 
// Brings up the find or replace dialog, creating it if neccessary.
/***************************************************************************/
void CMainFrame::MaybeCreateFindOrReplaceDlg(BOOL bWantFindOnly)
{
  if (m_pDlgFind) 
  {
    delete m_pDlgFind;
    m_pDlgFind = NULL;
  }
  m_bFindOnly = bWantFindOnly;
  CreateFindOrReplaceDlg();
}

/***************************************************************************/
// CMainFrame::SetWbProcess Set new pointer to workbench process 
// Sets a new pointer to a workbench process and returns the old one.
/***************************************************************************/
CDataProcess* CMainFrame::SetWbProcess(int nProcess, int nFilter, CDataProcess* pProcess)
{
  CDataProcess* pTempProcess = m_apWbProcess[nProcess][nFilter];
  m_apWbProcess[nProcess][nFilter] = pProcess;
  return pTempProcess; 
}             

/***************************************************************************/
// CMainFrame::SetWbFilterID Set new filter ID to workbench 
// Sets a new filter ID into the workbench and returns the old one.
/***************************************************************************/
int CMainFrame::SetWbFilterID(int nProcess, int nFilter, int nID)
{
  int nTempID = m_aWbFilterID[nProcess][nFilter];
  m_aWbFilterID[nProcess][nFilter] = nID;
  return nTempID; 
}              

/***************************************************************************/
// CMainFrame::IsProcessUsed Checks if process used by a document
// Returns a pointer to the document that uses the process if there is one,
// else it returns NULL.
/***************************************************************************/
CDocument* CMainFrame::IsProcessUsed(int nProcess)
{
  CDocList doclst; // list of currently open docs
  for (CSaDoc* pDoc = doclst.pdocFirst(); pDoc; pDoc = doclst.pdocNext())
  {
    if (pDoc != NULL)
    {
      if (nProcess == pDoc->GetWbProcess())
      {
        return (CDocument*)pDoc;
      }
    }
  }
  return NULL;
}

/***************************************************************************/
// CMainFrame::DeleteWbProcesses Deletes all workbench processes
// This function also checks, if a document had used a process and switches
// it back to plain with recalculating if the flag bSwitchBack is TRUE
// (default).
/***************************************************************************/
void CMainFrame::DeleteWbProcesses(BOOL bSwitchBack)
{
  // delete workbench processes
  for (int nLoop = 0; nLoop < MAX_PROCESS_NUMBER; nLoop++)
  {
    for (int nFilterLoop = 0; nFilterLoop < MAX_FILTER_NUMBER; nFilterLoop++)
    {
      if (m_apWbProcess[nLoop][nFilterLoop])
      {
        CDocument* pDoc = IsProcessUsed(nLoop);
        if (pDoc && bSwitchBack) ((CSaDoc*)pDoc)->SetWbProcess(0); // switch back to plain
        delete m_apWbProcess[nLoop][nFilterLoop];
        m_apWbProcess[nLoop][nFilterLoop] = NULL;
      }
      m_aWbFilterID[nLoop][nFilterLoop] = 0;
    }
  }
}

//********************************************************************
//
//********************************************************************
static const char* psz_mainframe          = "mainframe";
static const char* psz_placementMain      = "placementMain";
static const char* psz_placementEditor    = "placementEditor";
static const char* psz_showstartupdlg     = "showstartupdlg";
static const char* psz_saveopenfiles      = "saveopenfiles";   //tdg 09/03/97
static const char* psz_statusbar          = "statusbar";
static const char* psz_statusposreadout   = "statusposreadout";
static const char* psz_statuspitchreadout = "statuspitchreadout";
static const char* psz_toolbar            = "toolbar";      
static const char* psz_taskbar            = "taskbar";      
static const char* psz_toneAbove          = "toneAbove";      
static const char* psz_scrollzoom         = "scrollzoom";
static const char* psz_captionstyle       = "captionstyle";
static const char* psz_buffersize         = "buffersize";
static const char* psz_cursoralignment    = "cursoralignment";
static const char* psz_graphfontarray     = "graphfontarray";
static const char* psz_graphfont          = "graphfont";
static const char* psz_graphfontsize      = "graphfontsize";
static const char* psz_defaultgraphlist   = "DefaultGraphList";
static const char* psz_startmode          = "StartMode";
static const char* psz_bAnimate           = "Animate";
static const char* psz_animationRate      = "AnimationRate";
static const char* psz_graphUpdateMode    = "GraphUpdateMode";
static const char* psz_cursorMode         = "CursorMode";

// SDM 1.06.6U5 save maximized state of MDIChild (SaView)
static const char* psz_bMaxView           = "DefaultMaximizeView";
static const char* psz_WidthView          = "DefaultWidthView";
static const char* psz_HeightView         = "DefaultHeightView";

//********************************************************************
//
//********************************************************************
void CMainFrame::WriteProperties(Object_ostream& obs)
{
  //*****************************************************
  // Before beginning to write the properties, make sure
  // that if there has been a permanent default view
  // defined, it is read from the temp file. This is to
  // make sure any temp. view stored in memory gets
  // replaced with the permanent view the user wants to
  // keep in the settings file. i.e. This will make sure
  // the temp. default view isn't the one written to the
  // settings file.  DDO - 08/07/00
  //*****************************************************
  if (m_bDefaultViewExists) WriteReadDefaultViewToTempFile(FALSE);
  
  obs.WriteBeginMarker(psz_mainframe);
  obs.WriteNewline();
  
  // write out properties
  WINDOWPLACEMENT wpl;
  this->GetWindowPlacement(&wpl);
  obs.WriteWindowPlacement(psz_placementMain, wpl);
  
  if(m_pDlgEditor)
  {
    m_pDlgEditor->GetWindowPlacement(&wpl);
    obs.WriteWindowPlacement(psz_placementEditor, wpl);
  }
  else if(m_wplDlgEditor.length)
    obs.WriteWindowPlacement(psz_placementEditor, m_wplDlgEditor);
  
  //*****************************************************
  // DDO - 08/03/00 Always save on exit so don't need
  // to save a setting that tells future sessions of
  // SA to save settings.
  // obs.WriteBool(psz_saveonexit, m_bSaveOnExit);
  //*****************************************************
  obs.WriteInteger(psz_startmode, m_nStartDataMode);      // DDO - 08/03/00
  obs.WriteBool(psz_showstartupdlg, m_bShowStartupDlg);   // DDO - 08/03/00
  obs.WriteBool(psz_saveopenfiles, m_bSaveOpenFiles);     // tdg - 09/03/97
  obs.WriteBool(psz_statusbar , m_bStatusBar);
  obs.WriteInteger(psz_statusposreadout, m_nStatusPosReadout);
  obs.WriteInteger(psz_statuspitchreadout, m_nStatusPitchReadout);
  obs.WriteBool(psz_toolbar, bToolBar()); 
  obs.WriteBool(psz_taskbar, bTaskBar()); 
  obs.WriteBool(psz_toneAbove, m_bToneAbove);  //SDM 1.5Test8.2
  obs.WriteBool(psz_scrollzoom, m_bScrollZoom);
  obs.WriteInteger(psz_graphUpdateMode, m_nGraphUpdateMode);
  obs.WriteBool(psz_bAnimate, m_bAnimate); 
  obs.WriteInteger(psz_animationRate, m_nAnimationRate);
  obs.WriteInteger(psz_cursorMode, m_nCursorAlignment);
  
  m_colors.WriteProperties(obs);
  m_fnKeys.WriteProperties(obs);
  m_grid.WriteProperties(obs);
  dlgWaveformGenerator.current.WriteProperties(obs);
  
  // Write parsing, segmenting, pitch, spectrum, and spectrogram parameter defaults to *.PSA file.      // RLJ 11.1A
  m_parseParmDefaults.WriteProperties(obs);
  m_segmentParmDefaults.WriteProperties(obs);
  m_pitchParmDefaults.WriteProperties(obs);
  m_musicParmDefaults.WriteProperties(obs);
  m_intensityParmDefaults.WriteProperties(obs);
  m_spectrumParmDefaults.WriteProperties(obs);
  m_spectrogramParmDefaults.WritePropertiesA(obs);
  m_snapshotParmDefaults.WritePropertiesB(obs);
  
  obs.WriteInteger(psz_captionstyle, m_nCaptionStyle);
  
  obs.WriteBeginMarker(psz_graphfontarray);
  for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) 
  {
    CSaString szFont = m_pGraphFontFaces->GetAt(nLoop);
    obs.WriteString(psz_graphfont, szFont  );
    obs.WriteUInt(psz_graphfontsize, m_pGraphFontSizes->GetAt(nLoop));
  }
  obs.WriteEndMarker(psz_graphfontarray);
  
  //  CLayoutMenu   m_LayoutMenu;   // layout menu embedded object
  BOOL bMaximized;
  
  //SDM 1.06.6U5 if there is an active child save its maximized state and normal size as the defaults
  if(MDIGetActive(&bMaximized))
  {
    m_bDefaultMaximizeView = bMaximized;
    WINDOWPLACEMENT WP;
    WP.length = sizeof(WINDOWPLACEMENT);
    if(MDIGetActive()->GetWindowPlacement(&WP)) // SDM 32bit conversion
    {
      m_nDefaultHeightView = WP.rcNormalPosition.bottom - WP.rcNormalPosition.top;
      m_nDefaultWidthView = WP.rcNormalPosition.right - WP.rcNormalPosition.left;
    }
  }
  
  obs.WriteBool(psz_bMaxView,  m_bDefaultMaximizeView);
  obs.WriteInteger(psz_HeightView, m_nDefaultHeightView);
  obs.WriteInteger(psz_WidthView, m_nDefaultWidthView);
  
  obs.WriteEndMarker(psz_mainframe);
}

//********************************************************************
// Read the open databases and windows
//********************************************************************
BOOL CMainFrame::bReadProperties(Object_istream& obs)
{
  if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_mainframe) ) 
  {
    return FALSE;
  }                         
  
  BOOL b = FALSE;
  int nValue;
  
  while ( !obs.bAtEnd() ) 
  {
    WINDOWPLACEMENT wpl;
    //     if ( obs.bReadBool(psz_saveonexit, m_bSaveOnExit));                        // DDO - 08/03/00 Don't need setting anymore.
    
    if (obs.bReadInteger(psz_startmode, m_nStartDataMode));                     // DDO - 08/03/00
    else if (obs.bReadBool(psz_showstartupdlg, m_bShowStartupDlg));             // DDO - 08/03/00
    else if (obs.bReadBool(psz_saveopenfiles, m_bSaveOpenFiles));               // tdg - 09/03/97
    else if (obs.bReadWindowPlacement(psz_placementMain, wpl))
    {
      if ( wpl.showCmd == SW_SHOWMINIMIZED )
        wpl.showCmd = SW_SHOWNORMAL;
      wpl.ptMinPosition.x = -1;
      wpl.ptMinPosition.y = -1;
      
      this->SetWindowPlacement(&wpl);
      
      if ( wpl.showCmd == SW_SHOWMAXIMIZED )
      {
        this->ShowWindow(wpl.showCmd);
      }
    }

    else if (obs.bReadWindowPlacement(psz_placementEditor, m_wplDlgEditor));    // SDM - 1.5Test8.2
    else if (obs.bReadBool(psz_statusbar, b)) 
    {
      if (b != m_bStatusBar) 
      {
        SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0); // change statusbar status
        m_bStatusBar = !m_bStatusBar; 
      }
    } 
    
    else if (obs.bReadInteger(psz_statusposreadout, m_nStatusPosReadout));
    else if (obs.bReadInteger(psz_statuspitchreadout, m_nStatusPitchReadout));
    else if (obs.bReadBool(psz_toolbar, b)) 
    {
      if (b != bToolBar()) 
      {
        BOOL bAdvanced = b;
        ShowControlBar(GetControlBar(IDR_BAR_BASIC),!bAdvanced, TRUE); // change toolbar status
        ShowControlBar(GetControlBar(IDR_BAR_ADVANCED), bAdvanced, TRUE); // change toolbar status
      }
    } 
    else if (obs.bReadBool(psz_taskbar, b)) 
    {
      if (b != bTaskBar()) 
      {
        ShowControlBar(GetControlBar(ID_VIEW_TASKBAR),b, TRUE); // change toolbar status
      }
    } 
    else if ( obs.bReadBool(psz_toneAbove, b))  
    {
      //SDM 1.5Test8.2
      if (b != m_bToneAbove) 
      {
        m_bToneAbove = !m_bToneAbove; 
        
        if (m_bToneAbove) 
        {
          CGraphWnd::m_anAnnWndOrder[1] = TONE;
          CGraphWnd::m_anAnnWndOrder[2] = PHONETIC;
        }
        else 
        {
          CGraphWnd::m_anAnnWndOrder[1] = PHONETIC;
          CGraphWnd::m_anAnnWndOrder[2] = TONE;
        }
        SendMessageToMDIDescendants(WM_USER_GRAPH_ORDERCHANGED, 0, 0L);
      }
    } 
    
    else if (obs.bReadBool(psz_scrollzoom, m_bScrollZoom));
    else if (obs.bReadInteger(psz_graphUpdateMode, m_nGraphUpdateMode));
    else if (obs.bReadBool(psz_bAnimate, m_bAnimate));
    else if (obs.bReadInteger(psz_animationRate, m_nAnimationRate));
    else if (obs.bReadInteger(psz_cursorMode, nValue))
      m_nCursorAlignment = (CURSOR_ALIGNMENT) nValue;
    else if (m_colors.bReadProperties(obs));
    else if (m_fnKeys.bReadProperties(obs));
    else if (m_grid.bReadProperties(obs));
    else if (dlgWaveformGenerator.current.bReadProperties(obs));
    else if (obs.bReadInteger(psz_captionstyle, m_nCaptionStyle));
    else if (m_parseParmDefaults.bReadProperties(obs));
    else if (m_segmentParmDefaults.bReadProperties(obs));
    else if (m_pitchParmDefaults.bReadProperties(obs));
    else if (m_musicParmDefaults.bReadProperties(obs));
    else if (m_intensityParmDefaults.bReadProperties(obs));
    else if (m_spectrumParmDefaults.bReadProperties(obs));
    else if (m_spectrogramParmDefaults.bReadPropertiesA(obs));
    else if (m_snapshotParmDefaults.bReadPropertiesB(obs));
    else if (obs.bReadBeginMarker(psz_graphfontarray)) 
    {
      for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) 
      {
        UINT u = 0;
        CSaString str;
        if (obs.bReadString(psz_graphfont, &str)) m_pGraphFontFaces->SetAtGrow(nLoop, str);
        if (obs.bReadUInt(psz_graphfontsize, u)) m_pGraphFontSizes->SetAtGrow(nLoop, u);
      }
      obs.SkipToEndMarker(psz_graphfontarray);
    }
    
    
    //SDM 1.06.6U5 read if there is an active child save its maximized, height, and width state as the default
    else if ( obs.bReadBool(psz_bMaxView, m_bDefaultMaximizeView));
    else if ( obs.bReadInteger(psz_HeightView, m_nDefaultHeightView));
    else if ( obs.bReadInteger(psz_WidthView, m_nDefaultWidthView));
    
    else if ( obs.bEnd(psz_mainframe) ) 
      break;
  }                        
  
  // Maximize this window if it had been when SA was closed
  
  return TRUE;
}

static const char* psz_defaultviewconfig = "defaultviewconfig";

//********************************************************************
//
//********************************************************************
BOOL CMainFrame::bReadDefaultView(Object_istream& obs)
{
  if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_defaultviewconfig))
    return FALSE;
  
  if (m_pDefaultViewConfig)
    delete m_pDefaultViewConfig; // always start from scratch
  m_pDefaultViewConfig = new CSaView();
  m_nPermDefaultLayout = 0;
  m_szPermDefaultGraphs  = "";

  while ( !obs.bAtEnd() ) 
  {
    if (m_pDefaultViewConfig ? m_pDefaultViewConfig->bReadProperties(obs, FALSE) : FALSE)
    {
      if(m_pDefaultViewConfig->GetLayout() < ID_LAYOUT_FIRST ||
         m_pDefaultViewConfig->GetLayout() > ID_LAYOUT_LAST || 
         m_pDefaultViewConfig->GetGraphIDs()[0] == 0)
      {
        // This is a corrupted default view
        delete m_pDefaultViewConfig;
        m_pDefaultViewConfig = NULL;
      }
      else
      {
        m_nPermDefaultLayout = m_pDefaultViewConfig->GetLayout();
        m_szPermDefaultGraphs  = m_pDefaultViewConfig->GetGraphsDescription();
      }
    }
    else if ( obs.bEnd(psz_defaultviewconfig) ) 
      break;
  }
  
  // Now that documents and views have been opened, activate the top window
  // and maximize it if it had been when SA was closed.
  CSaView* pviewT = ((CSaApp *)AfxGetApp())->pviewTop();
  if (pviewT) pviewT->ShowInitialTopState();
  
  WriteReadDefaultViewToTempFile(TRUE);
  
  return TRUE;
}

//********************************************************************
//
//********************************************************************
void CMainFrame::WriteDefaultView(Object_ostream& obs)
{
  if (m_bDefaultViewExists && m_pDefaultViewConfig) 
  {
    obs.WriteBeginMarker(psz_defaultviewconfig);
    obs.WriteString(psz_defaultgraphlist, "");   // We don't use this anymore but the old code expected/required it
    m_pDefaultViewConfig->WriteProperties(obs);
    obs.WriteEndMarker(psz_defaultviewconfig);
  }
}

static const char* pszTmpDfltSettingsFile = "~!SA!~.tmp";

//********************************************************************
// WriteReadDefaultViewToTempFile()
// writes defaultView properties to predefined temp file
// WARNING ....read destroys temp file
//********************************************************************
void CMainFrame::WriteReadDefaultViewToTempFile(BOOL bWrite)
{
  CSaString szPath;

  GetTempPath(_MAX_PATH, szPath.GetBuffer(_MAX_PATH));
  szPath.ReleaseBuffer();
  
  CSaString szTmpDfltSettingsFile;
  szTmpDfltSettingsFile.Format(_T("~!SA!~ %08X.tmp"), AfxGetThread()->m_nThreadID); // Use thread id to keep different threads from interfering with this file
  
  szPath += szTmpDfltSettingsFile;
    
  try
  {
    if (bWrite) 
    {
      if (!m_pDefaultViewConfig) return;
      Object_ostream obs(szPath);
      
      obs.WriteBool(psz_bMaxView,  m_bDefaultMaximizeView);
      obs.WriteInteger(psz_HeightView, m_nDefaultHeightView);
      obs.WriteInteger(psz_WidthView, m_nDefaultWidthView);
      m_pDefaultViewConfig->WriteProperties(obs);
      obs.getIos().close();
      m_bDefaultViewExists = TRUE;
    }
    else 
    {
      Object_istream obs(szPath);
      
      obs.bReadBool(psz_bMaxView, m_bDefaultMaximizeView);
      obs.bReadInteger(psz_HeightView, m_nDefaultHeightView);
      obs.bReadInteger(psz_WidthView, m_nDefaultWidthView);
      if(m_pDefaultViewConfig)
        delete m_pDefaultViewConfig;
      m_pDefaultViewConfig = new CSaView();
      
      m_pDefaultViewConfig->bReadProperties(obs, FALSE);
      obs.getIos().close();
      std::string szUtf8 = szPath.utf8();
      remove(szUtf8.c_str());
    }
  }
  catch(...)
  {
  }
}

// SDM 1.5Test8.5
/***************************************************************************/
// CMainFrame::OnActivateApp delete display plot on loss of focus
/***************************************************************************/
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask)
{
  CMainFrameBase::OnActivateApp(bActive, hTask);
  
  if(!bActive && m_pDisplayPlot) 
  {
    delete m_pDisplayPlot;
    m_pDisplayPlot = NULL;
  }
}

// SDM 1.5Test10.3
/***************************************************************************/
// CMainFrame::OnWindowTileHorz set window order then tile
/***************************************************************************/
void CMainFrame::OnWindowTileHorz()
{
  CDocList doclst; // list of currently open docs
  int nLoop;
  int maxID = -1;
  
  for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
  {
    if(pdoc->GetID() > maxID) maxID = pdoc->GetID();
  }
  for(nLoop = maxID; nLoop >= 0; nLoop--)
  {
    for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
    {
      if(pdoc->GetID() == nLoop)
      {
        POSITION pos = pdoc->GetFirstViewPosition();
        CView* pFirstView = pdoc->GetNextView( pos );
        CWnd* pWnd = pFirstView->GetParent();  // MDIChildWnd
        pWnd->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOREDRAW);
        break;
      }
    }
  }
  MDITile(MDITILE_HORIZONTAL);
}

// SDM 1.5Test10.3
/***************************************************************************/
// CMainFrame::OnWindowTileVert set window order then tile
/***************************************************************************/
void CMainFrame::OnWindowTileVert()
{
  CDocList doclst; // list of currently open docs
  int nLoop;
  int maxID = -1;
  
  for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
  {
    if(pdoc->GetID() > maxID) maxID = pdoc->GetID();
  }
  for(nLoop = maxID; nLoop >= 0; nLoop--)
  {
    for ( CSaDoc* pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext() )
    {
      if(pdoc->GetID() == nLoop)
      {
        POSITION pos = pdoc->GetFirstViewPosition();
        CView* pFirstView = pdoc->GetNextView( pos );
        CWnd* pWnd = pFirstView->GetParent();  // MDIChildWnd
        pWnd->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOREDRAW);
        break;
      }
    }
  }
  MDITile(MDITILE_VERTICAL);
}

/***************************************************************************/
// CMainFrame::NotifyFragmentDone  respond to fragment processing done 
/***************************************************************************/
void CMainFrame::NotifyFragmentDone(void * /*pCaller*/ )
{
  // kg - prevent exception
  if ((CDlgPlayer::bPlayer) && (GetPlayer(false)->IsWindowVisible()))
    GetPlayer(false)->EnableSpeedSlider();
}

/***************************************************************************/
// CMainFrame::DestroyPlayer
/***************************************************************************/
void CMainFrame::DestroyPlayer()
{
  delete m_pDlgPlayer;
  CDlgPlayer::bPlayer = FALSE;
  m_pDlgPlayer = NULL;
}

void CMainFrame::OnWaveformGenerator() 
{
  dlgWaveformGenerator.DoModal();
}

static CDlgSynthesis* s_pDlgSynthesis = NULL;
/***************************************************************************/
// CMainFrame::OnSynthesis Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesis() 
{
  DestroySynthesizer();
  ASSERT(s_pDlgSynthesis == NULL);
  s_pDlgSynthesis = new CDlgSynthesis(_T("Synthesis"));
  if(s_pDlgSynthesis)
    s_pDlgSynthesis->Create(this);
}

/***************************************************************************/
// CMainFrame::OnUpdateSynthesis Menu update
/***************************************************************************/
void CMainFrame::OnUpdateSynthesis(CCmdUI* /*pCmdUI*/) 
{
  // pCmdUI->Enable(EXPERIMENTAL_ACCESS); // enable if control key pressed
}

/***************************************************************************/
// CMainFrame::DestroySynthesizer
/***************************************************************************/
void CMainFrame::DestroySynthesizer()
{
  if(s_pDlgSynthesis)
  {
    s_pDlgSynthesis->DestroyWindow();
    delete s_pDlgSynthesis;
    s_pDlgSynthesis = NULL;
  }
}

/***************************************************************************/
// CMainFrame::OnSynthesisKlattIpa Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisKlattIpa() 
{
  CDlgKlattAll::CreateSynthesizer(this, CDlgKlattAll::kSegment);
}

/***************************************************************************/
// CMainFrame::OnSynthesisKlattFragments Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisKlattFragments() 
{
  CDlgKlattAll::CreateSynthesizer(this, CDlgKlattAll::kFragment);
}

/***************************************************************************/
// CMainFrame::OnSynthesis Open synthesis dialog
/***************************************************************************/
void CMainFrame::OnSynthesisVocalTract() 
{
  CDlgVocalTract::CreateSynthesizer(this);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
  CMainFrame* pFrameWnd = (CMainFrame*) GetMDIFrame();
  
  if(!pFrameWnd) return;
  
  if(pFrameWnd->MDIGetActive()) 
  {
    CMDIChildWnd::ActivateFrame(nCmdShow);  // maintain current state
  }
  else if(pFrameWnd->IsDefaultViewMaximized()) 
  {
    CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED); 
  }
  else
  {
    CMDIChildWnd::ActivateFrame(nCmdShow);  // default behavior
  }
  
  // Adjust size of child
  CPoint Size = pFrameWnd->GetDefaultViewSize();
  WINDOWPLACEMENT WP;
  WP.length = sizeof(WINDOWPLACEMENT);
  CRect rParent;
  
  pFrameWnd->GetClientRect(&rParent);
  
  GetWindowPlacement(&WP);
  if((Size.x < rParent.Width())&&(Size.x > (rParent.Width()/10)))
  {
    WP.rcNormalPosition.right = WP.rcNormalPosition.left+Size.x;
  }
  else if(Size.x >= rParent.Width())
  {
    WP.rcNormalPosition.right = rParent.Width();
  }
  
  if((Size.y < rParent.Height())&&(Size.y > (rParent.Height()/10)))
  {
    WP.rcNormalPosition.bottom = WP.rcNormalPosition.top+Size.y;
  }
  else if(Size.y >= rParent.Height())
  {
    WP.rcNormalPosition.bottom = rParent.Height();
  }
  
  SetWindowPlacement(&WP);
};

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
  CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
  CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

/******************************************************************************
* used to create a new document, record and overlay in it and load it
* in another document.
******************************************************************************/
void CMainFrame::OnRecordOverlay() 
{
  // find the view that launched this message
  CSaView * pSourceView = GetCurrSaView();
  if (!pSourceView) return;
  
  // if the focused graph is mergeable, bring up a list of all other
  // graphs that can be merged with it, (m_pPickOverlay) then
  // call AssignGraph to merge them in.
  if ((!pSourceView->GetFocusedGraphWnd()) ||
		  (!CGraphWnd::IsMergeableGraph(pSourceView->GetFocusedGraphWnd())))
  {
    return;
  }
  
  CSaApp * pApp = (CSaApp*)AfxGetApp();
  CSaDoc * pDoc = (CSaDoc *)pApp->OpenBlankView(true);
  
  if (pDoc)
  {
    //hide the temporary overlay document
    MDIActivate(pSourceView->pwndChildFrame());
    
    // launch recorder in this new view
    pDoc->vSetTempOverlay();	//mark this for reuse.
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaDoc * pSourceDoc = pSourceView->GetDocument();
    CAlignInfo alignInfo;
    memset(&alignInfo,0,sizeof(alignInfo));
    pSourceDoc->vGetAlignInfo(alignInfo);
    if (pos)
    {
      CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
      if (pView->IsKindOf(RUNTIME_CLASS(CSaView)))
      {
        CDlgAutoRecorder *pDlgAutoRecorder = new CDlgAutoRecorder(pDoc,(CSaView*)pView,pSourceView,alignInfo);

        if(m_pDisplayPlot)
          m_pDisplayPlot->m_pModal = pDlgAutoRecorder;

        pDlgAutoRecorder->DoModal();

        if(m_pDisplayPlot)
          m_pDisplayPlot->m_pModal = NULL;

        delete pDlgAutoRecorder;
      }
    }
    else
    {
      pDoc->OnCloseDocument();
    }
  }
}

void CMainFrame::OnUpdateRecordOverlay(CCmdUI* pCmdUI) 
{
  // if the recorder is up, we can't restart
  BOOL bEnable = FALSE;
  
  {
    CSaView * pSourceView = GetCurrSaView();
    if (pSourceView)
    {
      // if the focused graph is mergeable, bring up a list of all other
      // graphs that can be merged with it, (m_pPickOverlay) then
      // call AssignGraph to merge them in.
      if ((pSourceView->GetFocusedGraphWnd()) &&
        (CGraphWnd::IsMergeableGraph(pSourceView->GetFocusedGraphWnd())))
      {
        bEnable = TRUE;
      }
    }
  }
  
  pCmdUI->Enable(bEnable);
}