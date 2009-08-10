#include "controlimpl.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "utils.h"

#include "element_object.h"
#include "element_text.h"
#include "element_div.h"

CHtmlElementObject::CHtmlElementObject(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeObject;
	iTagName.Set(KHStrObject);
	iParamKeys = new CDesCArrayFlat(2);
	iParamValues = new CDesCArrayFlat(2);
	iStyle.SetTag(iTagName);
}

CHtmlElementObject::~CHtmlElementObject()
{
	if(iEmbedObject) 
	{
		iOwner->Impl()->UnregisterControl(iEmbedObject);
		delete iEmbedObject;
	}
	iStyle.Close();
	delete iParamKeys;
	delete iParamValues;
	delete iAlterText;
}

TBool CHtmlElementObject::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
	{
		for(TInt i=0;i<iParamKeys->Count();i++)
		{
			if((*iParamKeys)[i].CompareF(aName)==0)
			{
				aBuffer.Copy((*iParamValues)[i].Left(aBuffer.MaxLength()));
				return ETrue;
			}
		}
	}
	
	return EFalse;
}

TBool CHtmlElementObject::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrWidth)==0) 
		iStyle.SetSingleStyleL(KHStrWidth, aValue);
	else if(aName.CompareF(KHStrHeight)==0) 
		iStyle.SetSingleStyleL(KHStrHeight, aValue);
	else if(aName.CompareF(KHStrAlign)==0) 
		iStyle.SetSingleStyleL(KHStrAlign, aValue);
	else if(aName.CompareF(KHStrClass)==0) 
		iStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrStyle)==0) 
		iStyle.SetStyleL(aValue);
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.SetSingleStyleL(aName.Mid(6), aValue);
	else
	{
		iParamKeys->AppendL(aName);
		iParamValues->AppendL(aValue);
	}
	
	return ETrue;
}

void CHtmlElementObject::AppendAlterTextL(HBufC* aText)
{
	if(!iAlterText)
	{
		iAlterText = new (ELeave)CHtmlElementText(iOwner);
		iAlterText->iParent = iParent;
		iAlterText->iPrev = iPrev;
		iAlterText->iNext = iNext;
	}
	THcTextStyle style;
	iAlterText->AppendTextL(style, aText);
}

void CHtmlElementObject::PrepareL() 
{
	CHtmlElementImpl::PrepareL();
	
	TPtrC classId, type, data;
	for(TInt i=0;i<iParamKeys->Count();i++)
	{
		if((*iParamKeys)[i].CompareF(KHStrClassId)==0)
			classId.Set((*iParamValues)[i]);
		else if((*iParamKeys)[i].CompareF(KHStrType)==0)
			type.Set((*iParamValues)[i]);
		else if((*iParamKeys)[i].CompareF(KHStrData)==0)
			data.Set((*iParamValues)[i]);
		else if((*iParamKeys)[i].CompareF(KHStrSrc)==0)
			data.Set((*iParamValues)[i]);
	}
	iEmbedObject = iOwner->Impl()->Env()->CreateObjectL(classId, type, data, *iOwner, *this);
	if(iEmbedObject)
		iOwner->Impl()->RegisterControl(iEmbedObject);
	
	if(iAlterText)
		iAlterText->PrepareL();
}

void CHtmlElementObject::SetFocus(TBool aFocus) 
{
	CHtmlElementImpl::SetFocus(aFocus);
	
	if(iEmbedObject) 
		iEmbedObject->SetFocus(aFocus);
}

CHtmlElementImpl* CHtmlElementObject::CloneL() const
{
	CHtmlElementObject* e = new (ELeave)CHtmlElementObject(iOwner);
	BaseClone(*e);
	e->iStyle.CopyL(iStyle);
	for(TInt i=0;i<iParamKeys->Count();i++)
		e->iParamKeys->AppendL((*iParamKeys)[i]);
	for(TInt i=0;i<iParamValues->Count();i++)
		e->iParamValues->AppendL((*iParamValues)[i]);
	if(iAlterText)
		e->iAlterText = (CHtmlElementText*)iAlterText->CloneL();
	
	return e;
}

void CHtmlElementObject::Measure(CHcMeasureStatus& aStatus)
{
	iStyle.Update(iOwner->Impl());
	if(iStyle.Style().IsDisplayNone()) 
	{
		iState.Set(EElementStateHidden);
		return;
	}
	
	if(iEmbedObject)
		DoMeasure(aStatus, iStyle.Style(), iEmbedObject->MinimumSize());
	else
	{
		if(iStyle.Style().IsClearLeft())
			aStatus.NewLine();
		
		if(iAlterText)
		{
			iAlterText->Measure(aStatus);
			iPosition = iAlterText->iPosition;
			iSize = iAlterText->iSize;
		}
		
		if(iStyle.Style().IsClearRight())
			aStatus.NewLine();
	}
}

void CHtmlElementObject::Refresh()
{
	if(iEmbedObject)
	{	
		TBool hidden = iState.IsSet(EElementStateHidden);
		if(iEmbedObject->IsVisible() && hidden
				|| !iEmbedObject->IsVisible() && !hidden)
		{
			iEmbedObject->MakeVisible(!hidden);
			
			if(hidden && iEmbedObject->IsFocused())
				iEmbedObject->SetFocus(EFalse);
		}
		
		if(!hidden)
		{
			TRect rect = TRect(iPosition, iSize);
			iStyle.Style().iBorders.ShrinkRect(rect);
			iStyle.Style().iPaddings.ShrinkRect(rect);
			
			rect.Move(iOwner->Rect().iTl);
			if(rect!=iEmbedObject->Rect())
				iEmbedObject->SetRect(rect);
		}
	}
	else if(iAlterText)
	{
		iAlterText->iPosition = iPosition;
		iAlterText->Refresh();
	}	
}

void CHtmlElementObject::Draw(CFbsBitGc& aGc)  const
{
	if(iEmbedObject)
		HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, Rect(), iStyle.Style());
	else if(iAlterText)
		iAlterText->Draw(aGc);
}

TKeyResponse CHtmlElementObject::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	if(iEmbedObject)
		return iEmbedObject->OfferKeyEventL(aKeyEvent, aType);
	else
		return EKeyWasNotConsumed;
}
