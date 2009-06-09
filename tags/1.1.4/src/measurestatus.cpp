#include <eikenv.h>

#include "controlimpl.h"
#include "elementimpl.h"
#include "measurestatus.h"
#include "image.h"
#include "utils.h"

#include "element_div.h"
#include "element_body.h"

CHcMeasureStatus* CHcMeasureStatus::NewL()
{
	CHcMeasureStatus* self = new (ELeave)CHcMeasureStatus();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CHcMeasureStatus::CHcMeasureStatus()
{
}

CHcMeasureStatus::~CHcMeasureStatus()
{
	iTextStyletack.Close();
	iLineAlign.Close();
}

void CHcMeasureStatus::ConstructL()
{
}

void CHcMeasureStatus::Measure(CHtmlControlImpl* aControl)
{
	iControl = aControl;
	iPosition = TPoint(0,0);
	iInLink = 0;
	iWrappedTextLines = 0;
	iHidden = 0;
	iTextStyletack.Reset();
	iLineAlign.Reset();
	iLines = &aControl->Lines();
	iLines->Reset();
	iLines->Append(THcLineInfo());
	
	TInt i=0;
	iCurrent = iControl->Body();
	do
	{
		iCurrent->iState.Assign(EElementStateHidden, iHidden);
		iCurrent->iState.Assign(EElementStateInLink, iInLink);
		iCurrent->Measure(*this);
		iCurrent = iCurrent->iNext;
		i++;
	}
	while(iCurrent!=iControl->Body());
	#ifdef __WINSCW__
	RDebug::Print(_L("CHcMeasureStatus: elements=%i, lines=%i"), i, iLines->Count());
	#endif
}

void CHcMeasureStatus::SkipElements(CHtmlElementImpl* aEndElement)
{
	while(iCurrent!=aEndElement)
	{
		iCurrent->iState.Set(EElementStateHidden);
		iCurrent = iCurrent->iNext;
	}
	iCurrent->iState.Set(EElementStateHidden);
}

void CHcMeasureStatus::NewLine(TBool aForce)
{
	if(iPosition.iX==iCurrent->iParent->iDisplayRect.iTl.iX && !aForce)
		return;
	
	THcLineInfo& currentLine = (*iLines)[iLineNumber];
	if(iPosition.iX> iCurrent->iParent->iDisplayRect.iBr.iX)
		currentLine.iWidth = iCurrent->iParent->iDisplayRect.Width();
	else
		currentLine.iWidth = iPosition.iX - iCurrent->iParent->iDisplayRect.iTl.iX;
	if(currentLine.iWidth>iCurrent->iParent->iContentSize.iWidth)
		iCurrent->iParent->iContentSize.iWidth = currentLine.iWidth;
	iPosition.iX = iCurrent->iParent->iDisplayRect.iTl.iX;
	iPosition.iY += currentLine.iHeight;
	
	TAlign align = iLineAlign.Top();
	if(align==ECenter)
	{
		currentLine.iXOffset = (iCurrent->iParent->iDisplayRect.Width() - currentLine.iWidth)/2;
		iCurrent->iParent->iContentSize.iWidth = iCurrent->iParent->iDisplayRect.Width();
	}
	else if(align==ERight)
	{
		currentLine.iXOffset = iCurrent->iParent->iDisplayRect.Width() - currentLine.iWidth;
		iCurrent->iParent->iContentSize.iWidth = iCurrent->iParent->iDisplayRect.Width();
	}
	
	if(iCurrent->TypeId()!=EElementTypeDivEnd)
	{
		iLines->Append(THcLineInfo());
		iLineNumber = iLines->Count()-1;
	}
}

void CHcMeasureStatus::PushTextStyleL(const THcTextStyle& aStyle)
{
	if(iTextStyletack.Count()>0) 
	{
		THcTextStyle style = iTextStyletack.Top();
		style.Add(aStyle);
		iTextStyletack.Push(style);
	}
	else
		iTextStyletack.Push(aStyle);
}

THcTextStyle CHcMeasureStatus::CurrentTextStyle() const
{
	if(iTextStyletack.Count()>0)
		return iTextStyletack.Top();
	else
		return THcTextStyle();
}

void CHcMeasureStatus::PopTextStyle() 
{
	if(iTextStyletack.Count()>0)
		iTextStyletack.Pop();
}

void CHcMeasureStatus::WrapTextL(const TDesC& aSource, TInt aFirstLineWidth, TInt aOtherLineWidth, const CFont& aFont, CArrayFix<TPtrC>& aWrappedArray)
{
	HcUtils::WrapTextL(aSource, aFirstLineWidth, aOtherLineWidth, aFont, aWrappedArray);
	iWrappedTextLines += aWrappedArray.Count();
}



