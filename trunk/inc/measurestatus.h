#ifndef HCMEASURESTATUS_H
#define HCMEASURESTATUS_H

#include "htmlcontrol.hrh"
#include "style.h"
#include "stack.h"

class CHtmlControl;
class CHtmlElementImpl;
class CHtmlElementDiv;

#define NO_BELOW_ZERO_MINUS(x,y) ((x)>(y)?(x)-(y):0)

class THcLineInfo
{
public:
	TInt iHeight;
	TInt iWidth;
	TInt iXOffset;
	
	inline THcLineInfo();
	inline TInt SetHeightIfGreater(TInt aValue);
};

class CHcMeasureStatus : public CBase
{
public:
	static CHcMeasureStatus* NewL();
	virtual ~CHcMeasureStatus();
	
	inline CHtmlElementImpl* Current();
	
	THcTextStyle CurrentTextStyle() const;
	void PushTextStyleL(const THcTextStyle& aStyle);
	void PopTextStyle();
	
	TAlign CurrentLineAlign() const;
	void SetCurrentLineAlign(TAlign aAlign);
	void PushLineAlign(TAlign aAlign);
	void PopLineAlign();

	void NewLine(TBool aForce=EFalse);
	inline TInt LineNumber() const;
	inline THcLineInfo& LineInfo(TInt aLineNumber);
	inline THcLineInfo& CurrentLineInfo();
	
	void WrapTextL(const TDesC& aSource, TInt aFirstLineWidth, TInt aOtherLineWidth, const CFont& aFont, CArrayFix<TPtrC>& aWrappedArray);
	inline TBool CanWrapText() const;
	
	void Measure(CHtmlControlImpl* aControl);
	void SkipElements(CHtmlElementImpl* aEndElement);

	TPoint iPosition;
	TInt iInLink;
	TInt iHidden;

private:
	CHcMeasureStatus();
	void ConstructL();

private:
	CHtmlControlImpl* iControl;
	CHtmlElementImpl* iCurrent;
	TInt iWrappedTextLines;
	RHcStack<THcTextStyle> iTextStyletack;
	RHcStack<TAlign> iLineAlign;
	RArray<THcLineInfo>* iLines;
	TInt iLineNumber;

	friend class CHtmlElementDiv;
};

inline THcLineInfo::THcLineInfo()
{
	iHeight = 0;
	iWidth = 0;
	iXOffset = 0;
}

inline TInt THcLineInfo::SetHeightIfGreater(TInt aValue)
{
	if(aValue>iHeight)
		iHeight = aValue;
	return iHeight;
}

inline CHtmlElementImpl* CHcMeasureStatus::Current()
{
	return iCurrent;
}

inline TInt CHcMeasureStatus::LineNumber() const
{
	return iLineNumber;
}

inline THcLineInfo& CHcMeasureStatus::LineInfo(TInt aLineNumber)
{
	return (*iLines)[aLineNumber];
}

inline THcLineInfo& CHcMeasureStatus::CurrentLineInfo()
{
	return (*iLines)[iLineNumber];
}

inline TBool CHcMeasureStatus::CanWrapText() const
{
	return iWrappedTextLines<80;
}

inline TAlign CHcMeasureStatus::CurrentLineAlign() const
{
	return iLineAlign.Top();
}

inline void CHcMeasureStatus::SetCurrentLineAlign(TAlign aAlign)
{
	iLineAlign.SetTop(aAlign);
}

inline void CHcMeasureStatus::PushLineAlign(TAlign aAlign)
{
	iLineAlign.Push(aAlign);
}

inline void CHcMeasureStatus::PopLineAlign()
{
	iLineAlign.Pop();
}


#endif
