#include "controlimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"

#include "element_hr.h"
#include "element_div.h"

_LIT(KDefaultWidth, "%98");
_LIT(KDefaultHeight, "1");

CHtmlElementHr::CHtmlElementHr(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeHr;
	iTagName.Set(KHStrHr);
	iColor.iRgb = KRgbBlack;
	HtmlParser::ParseLength(KDefaultWidth,iWidth);
	HtmlParser::ParseLength(KDefaultHeight,iHeight);
	iAlign = ECenter;
}

TBool CHtmlElementHr::GetProperty(const TDesC& aName, TDes& aBuffer)  const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	return EFalse;
}

TBool CHtmlElementHr::SetProperty(const TDesC& aName, const TDesC& aValue) 
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrWidth)==0) 
	{
		if(aValue.Length()!=0)
			HtmlParser::ParseLength(aValue, iWidth);
		else
			HtmlParser::ParseLength(KDefaultWidth, iWidth);
	}
	else if(aName.CompareF(KHStrSize)==0) 
	{
		if(aValue.Length()!=0)
			HtmlParser::ParseLength(aValue, iHeight);
		else
			HtmlParser::ParseLength(KDefaultHeight, iHeight);
	}
	else if(aName.CompareF(KHStrColor)==0)
		HtmlParser::ParseColor(aValue, iColor);
 	else if(aName.CompareF(KHStrAlign)==0) 
 		HtmlParser::ParseAlign(aValue, iAlign);
 	else
		return EFalse;
	
	return ETrue;
}

CHtmlElementImpl* CHtmlElementHr::CloneL() const
{
	CHtmlElementHr* e = new (ELeave)CHtmlElementHr(iOwner);
	BaseClone(*e);
	
	e->iWidth = iWidth;
	e->iHeight = iHeight;
	e->iAlign = iAlign;
	e->iColor = iColor;
	
	return e;
}

void CHtmlElementHr::Measure(CHcMeasureStatus& aStatus)
{
	aStatus.NewLine();

	const TRect& parentRect = iParent->iDisplayRect;
	iSize.iWidth = iWidth.GetRealValue(parentRect.Width(),parentRect.Width());
	iSize.iHeight = iHeight.GetRealValue(parentRect.Height(),parentRect.Height());

	iPosition = aStatus.iPosition;
	iPosition.iY += 3;
	if(iAlign==ECenter)
	{
		if(iSize.iWidth<parentRect.Width())
			iPosition.iX += (parentRect.Width() - iSize.iWidth)/2;
	}
	else if(iAlign==ERight)
	{
		if(iSize.iWidth<parentRect.Width())
			iPosition.iX = parentRect.iBr.iX - iSize.iWidth;
	}
	aStatus.iPosition.iX = iPosition.iX + iSize.iWidth;
	aStatus.iPosition.iY = iPosition.iY + iSize.iHeight + 3;
	
	aStatus.NewLine();
}

void CHtmlElementHr::Draw(CFbsBitGc& aGc) const
{
	aGc.SetPenStyle(CGraphicsContext::ESolidPen);
	aGc.SetPenSize(TSize(iSize.iHeight, iSize.iHeight));
	aGc.SetPenColor(iColor.Rgb());
	
	TPoint p(iPosition);
	p.iY += iSize.iHeight/2;
	aGc.MoveTo(p);
	p.iX += iSize.iWidth;
	aGc.DrawLineTo(p);
}

