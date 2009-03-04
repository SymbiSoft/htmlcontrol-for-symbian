#ifndef HTMLELEMENTTEXT_H
#define HTMLELEMENTTEXT_H

#include "elementimpl.h"

class CTextBlock;
class CHtmlElementText : public CHtmlElementImpl
{
public:
	CHtmlElementText(CHtmlControl* aOwner);
	virtual ~CHtmlElementText();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
	
	void AppendTextL(const THcTextStyle& aStyle, HBufC* aText);
	void SetTextL(const TDesC& aText);
	void GetTextL(TDes& aBuffer) const;
	TInt GetTextLengthL() const;
	
	TBool IsEndingWithBr() const;
	TBool IsBeginningWithBr() const;
	
	inline TInt TextRectCount() const;
	inline TRect TextRect(TInt aIndex) const;
	
	//properties
	TBool iStandAlone;
	RPointerArray<CTextBlock> iBlockArray;

private:
	void ClearCache();
	
	void RefreshSingleLine();
	void RefreshMultiLine();
	void DrawSingleLine(CFbsBitGc& aGc) const;
	void DrawMultiLine(CFbsBitGc& aGc) const;
	void DrawBorder(CFbsBitGc& aGc) const;
	
	class TTextRect
	{
	public:
		TRect iRect;
		CTextBlock* iBlock;
		TInt iLineIndex;
	};
	//runtime
	THcTextStyle iStyle;
	RArray<TTextRect> iRects;
	TInt iCacheWidth;
	TInt iClippedRectIndex;
	TInt iClippedCharCount;
};

inline TInt CHtmlElementText::TextRectCount() const
{
	return iRects.Count();
}
inline TRect CHtmlElementText::TextRect(TInt aIndex) const
{
	return iRects[aIndex].iRect;
}

class CTextBlock : public CBase
{
public:
	virtual ~CTextBlock();

	CTextBlock* CloneL() const;
	void AppendTextL(HBufC* aText);
	void ClearCache();

	inline TBool FullWrapped() const;
	inline TBool IsBeginningWithBr() const;
	inline TBool IsEndingWithBr() const;
	inline TBool IsEndingWithTwoBr() const;
	void RemoveLastLineIfEmpty();

	void WrapTextL(CHcMeasureStatus& aStatus, TBool aForceFullWrap);

private:
	//properties
	THcTextStyle iStyle;
	RPointerArray<HBufC> iTextArray;
	
	//runtime
	CArrayFix<TPtrC>* iLines;
	TInt iWrappingIndex;
	TBool iNewLineAtTheEnd;
	TInt iLeftToParent;
	TInt iTopToBeginning;
	TInt iLineNumber;
	TInt iLineHeight;
	TInt iBaseLine;
	CFont* iFont;
	
	friend class CHtmlElementText;
};


inline TBool CTextBlock::FullWrapped() const
{
	return iWrappingIndex>=iTextArray.Count();
}

inline TBool CTextBlock::IsBeginningWithBr() const
{
	return iTextArray.Count()>0 && iTextArray[0]==NULL;
}

inline TBool CTextBlock::IsEndingWithBr() const
{
	return iTextArray.Count()>0 && iTextArray[iTextArray.Count()-1]==NULL;
}

inline TBool CTextBlock::IsEndingWithTwoBr() const
{
	return iTextArray.Count()>1 && iTextArray[iTextArray.Count()-1]==NULL 
		&& iTextArray[iTextArray.Count()-2]==NULL;
}

#endif

