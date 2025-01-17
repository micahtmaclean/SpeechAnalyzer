/////////////////////////////////////////////////////////////////////////////
// sa_p_fra.cpp:
// Implementation of the CProcessFragments
// Author: Alec Epting
// copyright 1999 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "sa_p_fra.h"
#include "sa_p_gra.h"
#include "isa_doc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessFragments construction/destruction/creation

/***************************************************************************/
// CProcessFragments::CProcessFragments Constructor
/***************************************************************************/
CProcessFragments::CProcessFragments() {
    m_pFragmenter = NULL;
    m_dwFragmentIndex = 0;
    m_dwFragmentCount = 0;
    m_dwWaveIndex = 0;
    m_dwPitchIndex = 0;
}

/***************************************************************************/
// CProcessFragments::~CProcessFragments Destructor
/***************************************************************************/
CProcessFragments::~CProcessFragments() {
    if (m_pFragmenter!=NULL) {
        delete m_pFragmenter;
        m_pFragmenter = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProcessFragments helper functions

/***************************************************************************/
// CProcessFragments::Process Processing fragment data
// The processed fragment data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessFragments::Process(void * pCaller, ISaDoc * pDoc, int nProgress,
                                int nLevel) {
    //TRACE(_T("Process: CProcessFragments\n"));
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    BOOL bBackground = pDoc->IsBackgroundProcessing();
    if (!bBackground) {
        BeginWaitCursor();    // wait cursor
    }

    // generate pitch contour
    CProcessGrappl * pAutoPitch = pDoc->GetGrappl();
    short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || (nResult == PROCESS_CANCELED)) {
        if ((nResult == PROCESS_CANCELED)) {
            CancelProcess();    // set your own cancel flag
        }
        if (!bBackground) {
            EndWaitCursor();
        }
        return MAKELONG(nResult, nProgress);
    }
    DWORD dwOldPitchBlock = pAutoPitch->GetProcessBufferIndex();  // save current pitch buffer block offset

    // start processing fragments
    if (!(bBackground ? StartProcess(pCaller, IDS_STATTXT_BACKGNDFRA) : StartProcess(pCaller, IDS_STATTXT_PROCESSFRA))) {
        // memory allocation failed
        // end data processing
        EndProcess();
        if (!bBackground) {
            EndWaitCursor();
        }
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // get sample size in bytes
    DWORD wSmpSize = pDoc->GetSampleSize();

    // save current wave buffer block offset
    DWORD dwOldWaveBlock = pDoc->GetWaveBufferIndex();

    // if file has not been created
    if (!GetProcessFileName()[0]) {
        if (m_pFragmenter!=NULL) {
            delete m_pFragmenter;
            m_pFragmenter = NULL;
        }
        // create the temporary fragment file
        if (!CreateTempFile(_T("FRA"))) { // creating error
            EndProcess(); // end data processing
            SetDataInvalid();
            if (!bBackground) {
                EndWaitCursor();
            }
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        m_dwWaveIndex = 0;
        m_dwPitchIndex = 0;

        short * pPitchBuffer = (short *)pAutoPitch->GetProcessedData(m_dwPitchIndex, TRUE);   //!!run pitch first?

        // set pitch calculation and buffer parameters
        SPitchParms CPitchParm;
        CPitchParm.dwContourLength = pAutoPitch->GetDataSize();
        CPitchParm.wSmpIntvl = Grappl_calc_intvl;
        CPitchParm.wScaleFac = Grappl_scale_factor;
        SGrapplParms CalcParm = pAutoPitch->GetCalcParms();
        CPitchParm.wCalcRangeMin = CalcParm.minpitch;
        CPitchParm.wCalcRangeMax = CalcParm.maxpitch;

        DWORD dwPitchBufferLen = pAutoPitch->GetProcessBufferSize() / sizeof(*pPitchBuffer);

        // set signal and wave buffer parameters
        void * pWaveBuffer = (void *)pDoc->GetWaveData(m_dwWaveIndex * wSmpSize, TRUE);

        DWORD dwWaveBufferLen;
        SSigParms SigParm;
        SigParm.SmpRate = pDoc->GetSamplesPerSec();
        if (wSmpSize == 1) {
            SigParm.SmpDataFmt = PCM_UBYTE;
            SigParm.Length = pDoc->GetDataSize();
            dwWaveBufferLen = GetBufferSize();
        } else {
            SigParm.SmpDataFmt = PCM_2SSHORT;
            SigParm.Length = pDoc->GetDataSize() / 2;
            dwWaveBufferLen = GetBufferSize() / 2;
        }

        // set fragment buffer parameters
        SFragParms * pFragParmBuffer = (SFragParms *)m_lpBuffer;
        DWORD dwFragBufferLen = GetProcessBufferSize() / sizeof(SFragParms);


        // construct fragmenter
        dspError_t Err = CFragment::CreateObject(&m_pFragmenter, pFragParmBuffer, dwFragBufferLen, SigParm, pWaveBuffer, dwWaveBufferLen, CPitchParm, pPitchBuffer, dwPitchBufferLen);
        if (Err) {
            EndProcess(); // end data processing
            SetDataInvalid();
            if (!bBackground) {
                EndWaitCursor();
            }
            return MAKELONG(PROCESS_ERROR, nProgress);
        }

    } else {
        // open file to append data
        if (!OpenFileToAppend()) {
            EndProcess(); // end data processing
            if (!bBackground) {
                EndWaitCursor();
            }
            if (m_pFragmenter!=NULL) {
                delete m_pFragmenter;
                m_pFragmenter = NULL;
            }
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        pAutoPitch->GetProcessedData(m_dwPitchIndex, TRUE);
        pDoc->GetWaveData(m_dwWaveIndex * wSmpSize, TRUE);
    }


    // fragment waveform
    dspError_t lStatus;
    do {
        lStatus = m_pFragmenter->Fragment();
        switch (lStatus) {
        case WAVE_BUFFER_CALLBACK:
            // reload waveform buffer
            m_dwWaveIndex = m_pFragmenter->GetWaveBlockIndex();
            if (!bBackground) {
                pDoc->GetWaveData(m_dwWaveIndex * wSmpSize, TRUE);
            }
            break;
        case PITCH_BUFFER_CALLBACK:
            // reload pitch buffer
            m_dwPitchIndex = m_pFragmenter->GetPitchBlockIndex();
            if (!bBackground) {
                pAutoPitch->GetProcessedData(m_dwPitchIndex, TRUE);    //!!check byte offset
            }
            break;
        case FRAG_BUFFER_FULL:
        case DONE:
            // write the processed data block
            try {
                Write(m_lpBuffer, m_pFragmenter->GetFragmentBlockLength() * sizeof(SFragParms));
            } catch (CFileException * e) {
                // error writing file
                ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				// error, writing failed
				e->Delete();
				return Exit(PROCESS_ERROR);
            }
            break;
        }
        if (bBackground) {
            break;
        }
    } while (lStatus != DONE);


    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    EndProcess((nProgress >= 95)); // end data processing

    // restore wave and pitch blocks
    if (dwOldWaveBlock != UNDEFINED_OFFSET) {
        pDoc->GetWaveData(dwOldWaveBlock, TRUE);
    }
    pAutoPitch->GetProcessedData(dwOldPitchBlock, TRUE);

    // close the temporary file and read the status
    CloseTempFile(); // close the file

    // check for end of fragmetation
    m_dwFragmentCount = m_pFragmenter->GetFragmentCount();
    if (lStatus == DONE) {
        m_dwFragmentIndex = (m_dwFragmentCount - 1) / 2;  // initialize for GetFragmentIndex() binary search
        if (m_pFragmenter!=NULL) {
            delete m_pFragmenter;
            m_pFragmenter = NULL;
        }
        SetDataReady();
        if (GetDataSize() == 0) {
            return Exit(PROCESS_ERROR);    // error, not enough data
        }
        pDoc->NotifyFragmentDone(this);
    }

    // if foreground processing and data is not ready, return a process error
    if (!bBackground) {
        EndWaitCursor();
        if (!IsDataReady()) {
            SetDataInvalid(); // delete the temporary file
            return MAKELONG(PROCESS_ERROR, 100);
        }
    }

    return MAKELONG(nLevel, nProgress);
}


/***************************************************************************/
// CProcessFragments::GetBufferLength Retrieves buffer fragment parm capacity
// Returns the number of fragment parameter structures the buffer can contain.
/***************************************************************************/
ULONG CProcessFragments::GetBufferLength() {
    return GetProcessBufferSize() / sizeof(SFragParms);
}


/***************************************************************************/
// CProcessFragments::GetFragmentBlock Loads buffer with block of fragment
// parms starting at specified index.
/***************************************************************************/
SFragParms * CProcessFragments::GetFragmentBlock(ULONG dwFragmentIndex) {
    return (SFragParms *)GetProcessedWaveData(dwFragmentIndex*sizeof(SFragParms), TRUE);
}


/***************************************************************************/
// CProcessFragments::GetFragmentParms Returns reference to fragment parms
// at specified index.
/***************************************************************************/
const SFragParms & CProcessFragments::GetFragmentParms(ULONG dwFragmentIndex) {
    return *(SFragParms *) GetProcessedObject(dwFragmentIndex,sizeof(SFragParms));
}

/***************************************************************************/
// CProcessFragments::GetFragmentIndex Returns index of fragment containing
// sample at waveform index.
/***************************************************************************/
ULONG CProcessFragments::GetFragmentIndex(ULONG dwWaveIndex) {

    // Set bounds for binary search.
    ULONG dwMaxSearchIndex = m_dwFragmentCount - 1;
    ULONG dwMinSearchIndex = 0;

    // But first check to see if current fragment or adjacent fragments contain the waveform sample.
    // This is the fastest method if cursor is being dragged across fragments.
    SFragParms stFragment = GetFragmentParms(m_dwFragmentIndex);
    if (dwWaveIndex >= stFragment.dwOffset) {
        if (dwWaveIndex < stFragment.dwOffset + stFragment.wLength) {
            return m_dwFragmentIndex;
        }
        if (m_dwFragmentIndex + 1 >= m_dwFragmentCount) {
            return UNDEFINED_OFFSET;
        }
        dwMinSearchIndex = m_dwFragmentIndex++;
    } else {
        if (m_dwFragmentIndex == 0) {
            return UNDEFINED_OFFSET;
        }
        dwMaxSearchIndex = m_dwFragmentIndex--;
    }

    // Otherwise resort to a binary search.
    while (dwMinSearchIndex < dwMaxSearchIndex) { // true as long as dwWaveIndex within range spanned by fragments
        SFragParms stFragment2 = GetFragmentParms(m_dwFragmentIndex);
        if (dwWaveIndex >= stFragment2.dwOffset) {
            if (dwWaveIndex < stFragment2.dwOffset + stFragment2.wLength) {
                return m_dwFragmentIndex;
            }
            dwMinSearchIndex = m_dwFragmentIndex;   // previous fragment index becomes lower end of search range
            m_dwFragmentIndex = (dwMaxSearchIndex + dwMinSearchIndex + 1) / 2;   // go half the range, rounding up if necessary
        } else {
            dwMaxSearchIndex = m_dwFragmentIndex;   // previous fragment index becomes upper end of search range
            m_dwFragmentIndex = (dwMaxSearchIndex + dwMinSearchIndex) / 2;  // go half the range, rounding down if necessary
        }
    }

    return UNDEFINED_OFFSET;
}

ULONG CProcessFragments::GetFragmentCount() {
    return m_dwFragmentCount;
}
