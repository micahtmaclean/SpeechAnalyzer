#ifndef DEPENDENTTEXTSEGMENT_H
#define DEPENDENTTEXTSEGMENT_H

#include "TextSegment.h"

class CDependentTextSegment : public CTextSegment
{
public:
    CDependentTextSegment(int index, int master = -1);

    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, int nMode=LIMIT_MOVING_BOTH) const;
    virtual BOOL SetAt(const CSaString *, bool delimiter, DWORD dwStart, DWORD dwDuration);                 // set text segment
    virtual BOOL Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);    // insert a new segment with text
    virtual BOOL InsertBlank( int nIndex, DWORD dwStart, DWORD dwDuration);									// insert a new segment with no text
    virtual BOOL SetText(int nIndex, LPCTSTR pszString, int nDelimiter, DWORD dwStart, DWORD dwDuration);   // insert a new segment
    int CheckPositionToMaster(CSaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode) const;
    virtual void Add(CSaDoc * pDoc, DWORD dwStart, CSaString & szString, BOOL bDelimiter = FALSE, BOOL bCheck = TRUE); // add a segment
};

#endif

