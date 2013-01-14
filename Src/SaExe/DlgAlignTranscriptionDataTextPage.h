#pragma once

#include "resource.h"
#include "AppDefs.h"
#include "CSaString.h"

class CSaDoc;

class CDlgAlignTranscriptionDataTextPage : public CPropertyPage {

public:
	CDlgAlignTranscriptionDataTextPage( CSaDoc * pSaDoc, Annotations type, int IDD);
	virtual ~CDlgAlignTranscriptionDataTextPage();

private:
	CSaDoc * m_pSaDoc;

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnSetActive();
	afx_msg void OnClickedImport();
	afx_msg void OnClickedRevert();

public:
	bool m_bModified;
	CSaString m_szText;
	Annotations m_Type;
	int IDD;

private:
	void SaveAnnotation();
	void OnUpdateAnnotation();
	void SetEnable(int nItem, BOOL bEnable);
	void SetAnnotation();
	void SetText(int nItem, CSaString szText);
public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
};

