#include "controlimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "utils.h"

#include "element_form.h"
#include "element_div.h"
#include "element_input.h"
#include "element_select.h"
#include "element_textarea.h"

CHtmlElementForm::CHtmlElementForm(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeForm;
	iTagName.Set(KHStrForm);
	iStyle.SetTag(iTagName);
}

CHtmlElementForm::~CHtmlElementForm()
{
	delete iAction;
	delete iEncodeType;
	iStyle.Close();
}

TBool CHtmlElementForm::GetProperty(const TDesC& aName, TDes& aBuffer)  const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrAction)==0) 
	{
		if(iAction)
			aBuffer.Copy(iAction->Left(aBuffer.MaxLength()));
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrMethod)==0) 
	{
		aBuffer.Copy(iMethod.Left(aBuffer.MaxLength()));
	}
	else if(aName.CompareF(KHStrEncType)==0) 
	{
		if(iEncodeType)
			aBuffer.Copy(iEncodeType->Left(aBuffer.MaxLength()));
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else if(aName.CompareF(KHStrBufSize)==0) 
	{
		CHtmlElementImpl* e = this->iNext;
		TInt bufSize;
		TInt totalSize = 0;
		while(e!=this->iEnd)
		{
			if(e->Id().Length()>0 && 
				((e->TypeId()==EElementTypeInput
						&& (((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EText 
							|| ((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EPassword 
							|| ((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EHidden
							|| ((CHtmlElementInput*)e)->IsChecked())
				)						
				|| e->TypeId()==EElementTypeSelect 
				|| e->TypeId()==EElementTypeTextArea))
			{
				totalSize += e->Id().Length() + 2;
				((CHtmlElement*)e)->GetProperty(KHStrBufSize, bufSize);
				totalSize += bufSize;
			}
			e = e->iNext;
		}
		aBuffer.Num(totalSize);
	}
	else if(aName.CompareF(KHStrFormData)==0)
	{
		CHtmlElementImpl* e = this->iNext;
		TInt bufSize;
		TInt pos;
		TPtr ptr(NULL, 0);
		while(e!=this->iEnd)
		{
			if(e->Id().Length()>0 &&
					((e->TypeId()==EElementTypeInput 
						&& (((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EText 
						|| ((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EPassword 
						|| ((CHtmlElementInput*)e)->InputType()==CHtmlElementInput::EHidden
						|| ((CHtmlElementInput*)e)->IsChecked())
				)
				|| e->TypeId()==EElementTypeSelect 
				|| e->TypeId()==EElementTypeTextArea))
			{
				aBuffer.Append(e->Id());
				aBuffer.Append('\0');
				((CHtmlElement*)e)->GetProperty(KHStrBufSize, bufSize);
				if(bufSize>0)
				{
					pos = aBuffer.Length();
					aBuffer.SetLength(pos + bufSize);
					ptr.Set(aBuffer.MidTPtr(pos));
					e->GetProperty(KHStrValue, ptr);
				}
				aBuffer.Append('\0');
			}
			e = e->iNext;
		}
	}
	else
		return EFalse;
	return ETrue;
}

TBool CHtmlElementForm::SetProperty(const TDesC& aName, const TDesC& aValue) 
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrAction)==0) 
	{
		delete iAction;
		iAction = NULL;
		iAction = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrMethod)==0) 
	{
		iMethod.Copy(aValue.Left(iMethod.MaxLength()));
	}
	else if(aName.CompareF(KHStrEncType)==0) 
	{
		delete iEncodeType;
		iEncodeType = NULL;
		iEncodeType = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrClass)==0) 
		iStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrStyle)==0) 
		iStyle.SetStyleL(aValue);
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.SetSingleStyleL(aName.Mid(6), aValue);
	else
		return EFalse;
	
	return ETrue;
}

CHtmlElementImpl* CHtmlElementForm::CloneL() const
{
	CHtmlElementForm* e = new (ELeave)CHtmlElementForm(iOwner);
	BaseClone(*e);
	
	if(iAction)
		e->iAction = (*iAction).AllocL();
	e->iMethod.Copy(iMethod);
	if(iEncodeType)
		e->iEncodeType = (*iEncodeType).AllocL();
	e->iStyle.CopyL(iStyle);
	return e;
}

void CHtmlElementForm::Measure(CHcMeasureStatus& aStatus)
{
	iStyle.Update(iOwner->Impl());	
	if(iStyle.Style().IsDisplayNone())
	{
		aStatus.SkipElements(iEnd);
		return;
	}
	
	if(iStyle.Style().IsHidden())
		iState.Set(EElementStateHidden);
	
	if(iState.IsSet(EElementStateHidden))
		aStatus.iHidden++;
	
	aStatus.PushTextStyleL(iStyle.Style().iTextStyle);
	aStatus.NewLine();
}
	
void CHtmlElementForm::EndMeasure(CHcMeasureStatus& aStatus)
{
	iPosition = iNext->iPosition;
	iSize.iWidth = iParent->iDisplayRect.Width();
	iSize.iHeight = iEnd->iPosition.iY - iPosition.iY;
	if(iEnd->iPosition.iX!=iParent->iDisplayRect.iTl.iX)
		iSize.iHeight += aStatus.CurrentLineInfo().iHeight;
	
	if(iState.IsSet(EElementStateHidden))
		aStatus.iHidden--;

	aStatus.PopTextStyle();
}

void CHtmlElementForm::Draw(CFbsBitGc& )  const
{

}

_LIT(KHStrFormEnd, "form-end");
CHtmlElementFormEnd::CHtmlElementFormEnd(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeFormEnd;
	iTagName.Set(KHStrFormEnd);
}

CHtmlElementImpl* CHtmlElementFormEnd::CloneL() const
{
	CHtmlElementFormEnd* e = new (ELeave)CHtmlElementFormEnd(iOwner);
	BaseClone(*e);
	return e;
}

void CHtmlElementFormEnd::Measure(CHcMeasureStatus& aStatus) 
{
	aStatus.NewLine();
	
	iPosition = aStatus.iPosition;
	iStart->EndMeasure(aStatus);
}

void CHtmlElementFormEnd::Draw(CFbsBitGc& )  const
{
}

