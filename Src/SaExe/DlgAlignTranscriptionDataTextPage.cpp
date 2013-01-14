// DlgAlignTranscriptionDataTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "DlgAlignTranscriptionDataTextPage.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "settings\obstream.h"
#include "Sa_Doc.h"
#include "CSaString.h"
#include "Sa_segm.h"
#include "DlgImportSFM.h"

static const char* psz_ImportEnd = "import";

CDlgAlignTranscriptionDataTextPage::CDlgAlignTranscriptionDataTextPage(  CSaDoc * pSaDoc,  Annotations type, int aIDD) :
CPropertyPage(aIDD),
m_pSaDoc(pSaDoc),
m_Type(type),
m_bModified(false),
m_szText(""),
IDD(aIDD) 
{
}

CDlgAlignTranscriptionDataTextPage::~CDlgAlignTranscriptionDataTextPage() 
{
}


BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataTextPage, CPropertyPage)
	ON_BN_CLICKED(IDC_IMPORT, &CDlgAlignTranscriptionDataTextPage::OnClickedImport)
	ON_BN_CLICKED(IDC_REVERT, &CDlgAlignTranscriptionDataTextPage::OnClickedRevert)
	ON_EN_UPDATE(IDC_ANNOTATION, OnUpdateAnnotation)
END_MESSAGE_MAP()

// CDlgAlignTranscriptionDataTextPage message handlers

BOOL CDlgAlignTranscriptionDataTextPage::OnSetActive() 
{
	m_szText = m_pSaDoc->BuildString(m_Type);
	SetAnnotation();
	SetEnable(IDC_REVERT,m_bModified);
	CPropertySheet * pSheet = reinterpret_cast<CPropertySheet *>(GetParent());
	pSheet->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	return CPropertyPage::OnSetActive();
}


void CDlgAlignTranscriptionDataTextPage::OnClickedImport() 
{

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm) |*.sfm| Text Files (*.txt) |*.txt| All Files (*.*) |*.*||"), this );
	if (dlg.DoModal() != IDOK) 
	{
		return;
	}

	CSaString path = dlg.GetPathName();
	Object_istream obs(path);
	if ( obs.getIos().fail() ) 
	{
		return;
	}

	BOOL SFM = FALSE;

	SaveAnnotation();

	if ( obs.bAtBackslash()) 
	{
		// assume SFM
		CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());

		CDlgImportSFM dlg2(pSheet->init.m_bPhonetic,pSheet->init.m_bPhonemic,pSheet->init.m_bOrthographic);
		if (dlg2.DoModal()==IDCANCEL) 
		{
			return;
		}

		SFM = TRUE;

		while ( SFM && !obs.bAtEnd()) 
		{
			if ( dlg2.m_bPhonetic && obs.bReadString(dlg2.m_szPhonetic, & pSheet->phonetic.m_szText));
			else if ( dlg2.m_bPhonemic && obs.bReadString(dlg2.m_szPhonemic, & pSheet->phonemic.m_szText) );
			else if ( dlg2.m_bOrthographic && obs.bReadString(dlg2.m_szOrthographic, & pSheet->ortho.m_szText) );
			else if ( dlg2.m_bGloss  && obs.bReadString(dlg2.m_szGloss, & pSheet->gloss.m_szText) );
			else if ( obs.bEnd(psz_ImportEnd) )
				break;
		}

		if (SFM) 
		{
			SetAnnotation();
		}
	}

	if (!SFM) 
	{
		const int MAXLINE = 32000;
		char* pBuf = new char[MAXLINE];
		obs.getIos().seekg(0);  // start to file start
		obs.getIos().getline(pBuf,MAXLINE, '\000');
		SetText(IDC_ANNOTATION, pBuf);
		OnUpdateAnnotation(); // Set Modified
		delete [] pBuf;
	}
}


void CDlgAlignTranscriptionDataTextPage::OnClickedRevert() 
{
	
	m_bModified = false;
	m_szText = m_pSaDoc->BuildString(m_Type);
	CWnd* pWnd = GetDlgItem(IDC_REVERT);
	if (pWnd!=NULL) 
	{
		pWnd->EnableWindow(m_bModified);
	}
	pWnd = GetDlgItem(IDC_ANNOTATION);
	if (pWnd!=NULL) 
	{
		pWnd->SetWindowText(m_szText);
		CFont * pFont = (CFont*)m_pSaDoc->GetFont(m_Type);
		if (pFont!=NULL)
		{
			pWnd->SetFont(pFont);
		}
	}
}

void CDlgAlignTranscriptionDataTextPage::SaveAnnotation(void) 
{
	CWnd* pWnd = GetDlgItem( IDC_ANNOTATION);
	if (pWnd == NULL)
	{
		return;
	}
	pWnd->GetWindowText(m_szText);
}

void CDlgAlignTranscriptionDataTextPage::OnUpdateAnnotation()
{
	m_bModified = true;
	SetEnable(IDC_REVERT,m_bModified);
}

void CDlgAlignTranscriptionDataTextPage::SetEnable(int nItem, BOOL bEnable)
{
	CWnd* pWnd = GetDlgItem(nItem);
	if (pWnd==NULL) return;

	pWnd->EnableWindow(bEnable);
}

void CDlgAlignTranscriptionDataTextPage::SetAnnotation()
{
	
	SetText(IDC_ANNOTATION, m_szText);
	CFont * pFont = (CFont*)m_pSaDoc->GetFont(m_Type);
	if (pFont==NULL) return;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_ANNOTATION);
	if (pEdit==NULL) return;

	pEdit->SetFont(pFont);

	// disable selection after the control is drawn...
	pEdit->PostMessageW(EM_SETSEL,-1,0);
}

void CDlgAlignTranscriptionDataTextPage::SetText(int nItem, CSaString szText)
{
	CWnd * pWnd = GetDlgItem(nItem);
	if (pWnd==NULL) return;

	pWnd->SetWindowText(szText);
}


LRESULT CDlgAlignTranscriptionDataTextPage::OnWizardNext()
{
	CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
	return pSheet->CalculateNext( IDD);
}


LRESULT CDlgAlignTranscriptionDataTextPage::OnWizardBack()
{
	CDlgAlignTranscriptionDataSheet * pSheet = reinterpret_cast<CDlgAlignTranscriptionDataSheet *>(GetParent());
	return pSheet->CalculateBack( IDD);
}