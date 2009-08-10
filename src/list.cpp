#include "controlimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "list.h"
#include "imagepool.h"
#include "scrollbar.h"
#include "utils.h"
#include "timer.h"

#include "element_hr.h"
#include "element_div.h"
#include "element_img.h"

#ifdef __UIQ__
#include <DeviceKeys.h>
#endif

#define KPointerRepeatInerval TTimeIntervalMicroSeconds32(100000)

CHcList* CHcList::NewL(CHtmlElementDiv* aDiv, THcListLayout aLayout)
{
	CHcList* self = new (ELeave)CHcList(aDiv, aLayout);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}

CHcList::CHcList(CHtmlElementDiv* aDiv, THcListLayout aLayout) 
{
	iDiv = aDiv;
	iLayout = aLayout;
	iViewColCount = 1;
}

CHcList::~CHcList()
{
	delete iTimer;
	iItems.Close();
	if(iTemplate)
		iDiv->Owner()->Impl()->FreeElementList(iTemplate, iTemplate->iEnd);
	if(iEmptyBody)
		iDiv->Owner()->Impl()->FreeElementList(iEmptyBody, iEmptyBody->iPrev);
	for(TInt i=0;i<iCachedImages.Count();i++)
		iDiv->Owner()->Impl()->Env()->ImagePool().Remove(iCachedImages[i]);
	iCachedImages.Close();
}

void CHcList::ConstructL()
{
	iTimer = CHcTimer<CHcList>::NewL(*this);
}

TBool CHcList::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(aName.Compare(KHStrListSelectedIndex)==0) 
	{
		if(iBodyStatus==EEmpty)
		{
			if(iDiv->iFocusedElement)
				aBuffer.Num(iDiv->iFocusedElement->Index(iDiv));
			else
				aBuffer.Num(-1);
		}
		else
		{
			if(iDiv->iFocusedElement)
			{
				if(iSelectedIndex==-1)
					aBuffer.Num(ListIndexOf(iDiv->iFocusedElement));
				else
					aBuffer.Num(iSelectedIndex);
			}
			else
				aBuffer.Num(-1);
		}
	}
	else if(aName.Compare(KHStrListTopIndex)==0)
	{
		aBuffer.Num(iTopItem);
	}
	else
		return EFalse;
	return ETrue;
}

TBool CHcList::SetProperty(const TDesC& aName, const TDesC& aValue) 
{	
	if(aName.Compare(KHStrListItemCount)==0)
	{
		iItemCount  = HcUtils::StrToInt(aValue);
		if(iSelectedIndex>=iItemCount)
			iSelectedIndex = iItemCount - 1;
	}
	else if(aName.Compare(KHStrListSelectedIndex)==0)
	{
		TInt index = HcUtils::StrToInt(aValue);
		if(index<0 || index>=iItemCount)
			return EFalse;
		
		iSelectedIndex = index;
	}
	else if(aName.Compare(KHStrListItemHtml)==0)
	{
		if(iTemplate)
			iDiv->Owner()->Impl()->FreeElementList(iTemplate, iTemplate->iEnd);
		iTemplate = NULL;

		iTemplate = (CHtmlElementDiv*)iDiv->Owner()->Impl()->ParseL(iDiv, aValue, 0);
		iTemplate->iFlags.Set(CHtmlElementDiv::EFocusing);

		CHtmlElementImpl* e = iTemplate->iNext;
		while(e!=iTemplate->iEnd)
		{
			if(e->TypeId()==EElementTypeImg)
				((CHtmlElementImg*)e)->iOwnerList = this;
			e = e->iNext;
		}

		if(iBodyStatus==EItems)
			iBodyStatus = ENotCreate;
	}
	else if(aName.Compare(KHStrListEmptyHtml)==0) 
	{
		if(iEmptyBody)
			iDiv->Owner()->Impl()->FreeElementList(iEmptyBody, iEmptyBody->iPrev);
		
		iEmptyBody = iDiv->Owner()->Impl()->ParseL(iDiv, aValue, 0);
		
		if(iBodyStatus==EEmpty)
			iBodyStatus = ENotCreate;
	}
	else
		return EFalse;
	
	return ETrue;
}

void CHcList::SetRenderer(MListItemRenderer* aRenderer)
{
	iRenderer = aRenderer;
}

void CHcList::NotifyChildSetImage(CHcImage* aImage)
{
	if(iCachedImages.Find(aImage)==KErrNotFound)
	{
		if(iCachedImages.Count()>iItems.Count()*3)
		{
			iDiv->Owner()->Impl()->Env()->ImagePool().Remove(iCachedImages[0]);
			iCachedImages.Remove(0);
		}
		aImage->AddRef();
		iCachedImages.Append(aImage);
	}
}

void CHcList::ClearContainer()
{
	if(iDiv->iNext!=iDiv->iEnd)
		iDiv->Owner()->Impl()->FreeElementList(iDiv->iNext, iDiv->iEnd->iPrev);
	iDiv->iScrollbar->SetTopPos();
	iItems.Reset();
}

void CHcList::Measure(CHcMeasureStatus& )
{
	if(iTemplate && iTemplate->iStyle.Update(iDiv->Owner()->Impl()))
	{
		iTemplate->iFocusStyle.Update(iDiv->Owner()->Impl());
			
		CHtmlElementImpl* e = iTemplate->iNext;
		while(e!=iTemplate->iEnd)
		{
			if(e->TypeId()==EElementTypeImg)
				((CHtmlElementImg*)e)->iStyle.Update(iDiv->Owner()->Impl());
			else if(e->TypeId()==EElementTypeDiv)
				((CHtmlElementDiv*)e)->iStyle.Update(iDiv->Owner()->Impl());
				
			e = e->iNext;
		}
	}
			
	if(iBodyStatus==EItems && iContainerSize!=iDiv->iDisplayRect.Size())
	{
		if(iDiv->iFocusedElement)
			iSelectedIndex = ListIndexOf(iDiv->iFocusedElement);
		iBodyStatus = ENotCreate;
	}
	
	if(iBodyStatus==ENotCreate)
		DoCreateL();
}

void CHcList::DoCreateL()
{
	if(iBodyStatus==ENotCreate)
	{
		ClearContainer();
		
		if(iItemCount==0) 
		{
			iBodyStatus = EEmpty;
			if(iEmptyBody)
			{
				iDiv->Owner()->Impl()->CopyElementListL(iEmptyBody, iEmptyBody->iPrev, iDiv, EAfterBegin);
				iDiv->SetFirstFocus();
			}
		}
		else
		{
			iBodyStatus = EItems;
			BuildItemsL();
		}
	}
	else {
		if(iItemCount==0) {
			if(iBodyStatus==EItems) {
				ClearContainer();

				iBodyStatus = EEmpty;
				if(iEmptyBody)
				{
					iDiv->Owner()->Impl()->CopyElementListL(iEmptyBody, iEmptyBody->iPrev, iDiv, EAfterBegin);
					iDiv->SetFirstFocus();
				}
			}
		}
		else {
			if(iBodyStatus==EEmpty) {
				ClearContainer();
				
				iBodyStatus = EItems;
				BuildItemsL();
			}
		}
	}
	
	if(iBodyStatus!=EItems)
		return;

	TInt viewCount = iViewRowCount*iViewColCount;
	if(iSelectedIndex!=-1) 
	{
		iTopItem = iSelectedIndex/iViewColCount*iViewColCount;
		if(iTopItem<viewCount)
			iTopItem = 0;
	}
	else if(iDiv->iFocusedElement)
		iSelectedIndex = ListIndexOf(iDiv->iFocusedElement);
	
	if(iTopItem+viewCount>iItemCount) 
	{
		TInt remainder = iItemCount%iViewColCount;
		if(remainder==0)
			iTopItem = iItemCount - viewCount;
		else
			iTopItem = iItemCount - iViewColCount*(iViewRowCount-1) - remainder;
		if(iTopItem<0)
			iTopItem = 0;
	}

	if(iSelectedIndex<iTopItem || iSelectedIndex>=Min(iItemCount, iTopItem+viewCount))
	{
		if(iItems[0]->CanFocus()) 
		{
			iDiv->FocusChangingTo(iItems[0]);
			iDiv->iScrollbar->SetTopPos();
			iTopItem = 0;
		}
	}
	
	for(TInt i=0;i<viewCount;i++)
	{
		CHtmlElementDiv* e = iItems[i];
		if(i+iTopItem<iItemCount) 
		{
			e->iFlags.Clear(CHtmlElementDiv::EDisabled);
			if(iRenderer)
				iRenderer->RenderListItemL(e, i+iTopItem);
			
			if(iSelectedIndex==i+iTopItem)
			{
				iDiv->FocusChangingTo(e);
				iDiv->iScrollbar->SetPos(iTopItem/iViewColCount*iItemHeight);	
			}
		}
		else if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
		{
			iItems[i] = (CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, e, EAfterEnd);
			iItems[i]->iFlags.Set(CHtmlElementDiv::EDisabled);
			iDiv->Owner()->Impl()->FreeElementList(e, e->iEnd);
		}
	}
	
	iSelectedIndex = -1;
}

void CHcList::BuildItemsL()
{
	iContainerSize = iDiv->iDisplayRect.Size();
	TMargins margins = iTemplate->iStyle.Style().iMargins.GetMargins(iContainerSize);
	TInt itemWidth = iTemplate->iStyle.Style().iWidth.GetRealValue(iContainerSize.iWidth, iContainerSize.iWidth)
		+ margins.iLeft + margins.iRight;
	iItemHeight = iTemplate->iStyle.Style().iHeight.GetRealValue(iContainerSize.iHeight, iContainerSize.iHeight)
		+ margins.iTop + margins.iBottom;
	
	iDiv->iScrollbar->SetStep(iItemHeight);
	if(iLayout==ELayoutGrid)
	{
		iViewRowCount = iContainerSize.iHeight/iItemHeight;
		if(iViewRowCount==0)
			iViewRowCount = 1;
	
		iViewColCount = iContainerSize.iWidth/itemWidth;
		if(iViewColCount==0)
			iViewColCount = 1;
	}
	else if(iLayout==ELayoutSlides) 
	{
		iViewRowCount = iContainerSize.iWidth/itemWidth;
		if(iViewRowCount==0)
			iViewRowCount = 1;
		
		iViewColCount = 1;
	}
	else
	{
		iViewRowCount = iContainerSize.iHeight/iItemHeight;
		if(iViewRowCount==0)
			iViewRowCount = 1;
		
		iViewColCount = 1;
	}
	
	TInt total = iViewRowCount*iViewColCount;
	for(TInt i=0;i<total;i++) 
	{
		CHtmlElementDiv* e = (CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, iDiv, EBeforeEnd);
		//if(iLayout!=ELayoutSlides && (i+1)%iViewColCount==0)
		//{
		//	e->iStyle.Set(EClear);
		//	e->iClear = EClearRight;
		//}
		e->iFlags.Set(CHtmlElementDiv::EDisabled);
		iItems.Append(e);
	}

	if(iSelectedIndex==-1)
		iSelectedIndex = 0;
}

void CHcList::UpdateL(TInt aIndex)
{
	if(iBodyStatus==ENotCreate)
		return;
	
	if(aIndex==-1) //update all
		DoCreateL();
	else if(iBodyStatus==EItems)
	{
		if(aIndex>=iTopItem && aIndex<iTopItem+iItems.Count())
		{
			CHtmlElementDiv* e = iItems[aIndex-iTopItem];
			if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
			{
				if(iRenderer)
					iRenderer->RenderListItemL(e, aIndex);
			}
		}
	}
}

TInt CHcList::ListIndexOf(const CHtmlElementImpl* aElement)  const
{
	TInt ret = iItems.Find((CHtmlElementDiv*)aElement);
	if(ret!=KErrNotFound)
		return iTopItem + ret;
	else
		return ret;
}

TInt CHcList::Height()  const
{
	if(iBodyStatus==EItems) {
		TInt i = (iItemCount+iViewColCount-1)/iViewColCount;
		if(i<iViewRowCount)
			return iDiv->iDisplayRect.Height();
		else
			return iDiv->iDisplayRect.Height() + iItemHeight * (i-iViewRowCount);
	}
	else {
		return iDiv->iDisplayRect.Height();
	}
}

TBool CHcList::IsEmpty() const
{
	return iBodyStatus!=EItems;
}

void CHcList::MoveItemsTop() 
{
	if(iTopItem!=0) 
	{
		iTopItem = 0;

		for(TInt i=0;i<iItems.Count();i++) 
		{
			CHtmlElementDiv* e = iItems[i];
			if(i + iTopItem<iItemCount)
			{
				e->iFlags.Clear(CHtmlElementDiv::EDisabled);
				if(iRenderer)
					iRenderer->RenderListItemL(e, i + iTopItem);
			}
			else if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
			{
				iItems[i] = ((CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, e, EAfterEnd));
				iItems[i]->iFlags.Set(CHtmlElementDiv::EDisabled);
				iDiv->Owner()->Impl()->FreeElementList(e, e->iEnd);
			}
		}
		iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
	}
	iDiv->iScrollbar->SetTopPos();
}

void CHcList::MoveItemsBottom() 
{
	TInt remainder = iItemCount%iViewColCount;
	TInt newTopItem;
	if(remainder==0)
		newTopItem = iItemCount - iViewColCount*iViewRowCount;
	else
		newTopItem = iItemCount - iViewColCount*(iViewRowCount-1) - remainder;
	if(newTopItem<0)
		newTopItem = 0;
	
	if(newTopItem!=iTopItem) 
	{
		iTopItem = newTopItem;
		
		for(TInt i=0;i<iItems.Count();i++) 
		{
			CHtmlElementDiv* e = iItems[i];
			if(i + iTopItem<iItemCount)
			{
				e->iFlags.Clear(CHtmlElementDiv::EDisabled);
				if(iRenderer)
					iRenderer->RenderListItemL(e, i + iTopItem);
			}
			else if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
			{
				iItems[i] = (CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, e, EAfterEnd);
				iItems[i]->iFlags.Set(CHtmlElementDiv::EDisabled);
				iDiv->Owner()->Impl()->FreeElementList(e, e->iEnd);
			}
		}
		iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
	}
	
	iDiv->iScrollbar->SetBottomPos();
}

void CHcList::MoveItemsUp()
{
	iTopItem -= iViewColCount;
	for(TInt i=0;i<iViewColCount;i++) 
	{
		CHtmlElementDiv* ee = iItems[iItems.Count()-1];
		iDiv->Owner()->Impl()->MoveElementL(ee, iItems[0], EBeforeBegin);
		iItems.Remove(iItems.Count()-1);
		iItems.Insert(ee,0);
	}

	for(TInt i=0;i<iViewColCount;i++)
	{
		CHtmlElementDiv* ee = iItems[i];
		ee->iFlags.Clear(CHtmlElementDiv::EDisabled);
		if(iRenderer)
			iRenderer->RenderListItemL(ee, iTopItem + i);	
	}
	
	iDiv->iScrollbar->RemoveStepPos();
	iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
}

void CHcList::MoveItemsDown()
{
	iTopItem += iViewColCount;
	for(TInt i=0;i<iViewColCount;i++) 
	{
		iDiv->Owner()->Impl()->MoveElementL(iItems[0], iItems[iItems.Count()-1], EAfterEnd);
		iItems.Append(iItems[0]);
		iItems.Remove(0);
	}
	
	for(TInt i=iItems.Count()-iViewColCount;i<iItems.Count();i++)
	{
		CHtmlElementDiv* e = iItems[i];
		if(i + iTopItem<iItemCount)
		{
			e->iFlags.Clear(CHtmlElementDiv::EDisabled);
			if(iRenderer)
				iRenderer->RenderListItemL(e, i + iTopItem);
		}
		else if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
		{
			iItems[i] = ((CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, e, EAfterEnd));
			iItems[i]->iFlags.Set(CHtmlElementDiv::EDisabled);
			iDiv->Owner()->Impl()->FreeElementList(e, e->iEnd);
		}
	}
	
	iDiv->iScrollbar->AddStepPos();
	iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
}

void CHcList::ScrollTop() 
{
	MoveItemsTop();
	
	if(iItemCount>0)
		iDiv->FocusChangingTo(iItems[0]);
}

void CHcList::ScrollBottom() 
{
	MoveItemsBottom();

	if(iItemCount>iTopItem)
		iDiv->FocusChangingTo(iItems[iItemCount-iTopItem-1]);	
}

void CHcList::HandleLeft()
{
	TInt index = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
	if(index==KErrNotFound)
		index = 0;
	else
		index--;
	if(index<0)
	{
		if(iTopItem>0) 
		{
			MoveItemsUp();
			index += iViewColCount;

			iDiv->FocusChangingTo(iItems[index]);
		}
		else
			ScrollBottom();
	}
	else
		iDiv->FocusChangingTo(iItems[index]);
}

void CHcList::HandleRight()
{
	TInt index = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
	if(index==KErrNotFound)
		index = 0;
	else			
		index++;
	if(index>=iItemCount - iTopItem)
		ScrollTop();
	else 
	{
		if(index>iViewRowCount*iViewColCount-1)
		{
			MoveItemsDown();
			index -= iViewColCount;
		}
		iDiv->FocusChangingTo(iItems[index]);
	}
}

void CHcList::HandleUp() 
{
	TInt index = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
	if(index==KErrNotFound)
		index = 0;
	else
		index -= iViewColCount;
	if(index<0)
	{
		if(iTopItem>0) 
		{
			MoveItemsUp();
			index += iViewColCount;
			iDiv->FocusChangingTo(iItems[index]);
		}
		else
		{
			if(iDiv->iScrollbar->IsLoop())  
			{
				MoveItemsBottom();
				index += ((iItemCount-iTopItem+iViewColCount)/iViewColCount)*iViewColCount;
				if(index + iTopItem>iItemCount-1)
					index -= iViewColCount;
				iDiv->FocusChangingTo(iItems[index]);
			}				
		}
	}
	else
		iDiv->FocusChangingTo(iItems[index]);
}

void CHcList::HandleDown()
{
	TInt index = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
	if(index==KErrNotFound)
		index = 0;
	else
		index += iViewColCount;
	if(index>iItemCount - iTopItem-1) 
	{
		if(iDiv->iScrollbar->IsLoop())
		{
			MoveItemsTop();
			iDiv->FocusChangingTo(iItems[index%iViewColCount]);
		}
	}
	else if(index>iViewRowCount*iViewColCount-1)
	{
		MoveItemsDown();
		index -= iViewColCount;
		iDiv->FocusChangingTo(iItems[index]);
	}
	else
		iDiv->FocusChangingTo(iItems[index]);
}

TKeyResponse CHcList::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	if(!iTemplate)
		return EKeyWasNotConsumed;
	
	if(aType!=EEventKey)
		return EKeyWasNotConsumed;
	
	iDiv->iState.Clear(EElementStateFocusChanged);
	TInt keyCode = HcUtils::TranslateKey(aKeyEvent.iCode);
#ifdef __UIQ__
	if(iLayout!=ELayoutList)
	{
		if(aKeyEvent.iCode==EDeviceKeyTwoWayUp)
			keyCode = EKeyLeftArrow;
		else if(aKeyEvent.iCode==EDeviceKeyTwoWayDown)
			keyCode = EKeyRightArrow;
	}
#endif
	switch(keyCode) 
	{
		case EKeyLeftArrow:
			if(iLayout!=ELayoutList) 
			{
				HandleLeft();
				return EKeyWasConsumed;
			}
			break;
			
		case EKeyRightArrow:
			if(iLayout!=ELayoutList)
			{
				HandleRight();
				return EKeyWasConsumed;
			}

			break;
			
		case EKeyUpArrow:
			if(iLayout!=ELayoutSlides ) 
			{
				HandleUp();
				return EKeyWasConsumed;
			}
			break;

		case EKeyDownArrow:
			if(iLayout!=ELayoutSlides)
			{
				HandleDown();
				return EKeyWasConsumed;
			}
			break;
			
		case '*':
			ScrollTop();
			return EKeyWasConsumed;

		case '#':
			ScrollBottom();
			return EKeyWasConsumed;
	}
	
	if(iDiv->iState.IsSet(EElementStateFocusChanged))
		return EKeyWasConsumed;
	else if(iDiv->iScrollbar->Pos()!=iDiv->iScrollbar->RealPos())
	{
		iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
		return EKeyWasConsumed;
	}
	else
		return EKeyWasNotConsumed;	
}

void CHcList::NotifyScrollPosChanged()
{
	if(iBodyStatus!=EItems)
		return;
	
	TInt topItem = (iDiv->iScrollbar->Pos() + iItemHeight/2)/iItemHeight*iViewColCount;
	if(topItem==iTopItem)
	{
		if(iDiv->iScrollbar->Pos()!=iDiv->iScrollbar->RealPos())
		{
			iDiv->iScrollbar->AdjustScrollPos();
			iDiv->Refresh();
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
		}
	}
	else if(topItem<iTopItem)
	{
		TInt offset = iTopItem-topItem;
		if(iDiv->iFocusedElement)
		{
			 TInt index = ListIndexOf(iDiv->iFocusedElement);
			 if(index<topItem || index>=topItem+iItems.Count())
			 {
				 iDiv->iFocusedElement->SetFocus(EFalse);
				 iDiv->iFocusedElement = NULL;
			 }
		}
		iTopItem = topItem;
		//scrollup
		if(offset<iItems.Count())
		{
			for(TInt i=0;i<offset;i++) 
			{
				CHtmlElementDiv* e = iItems[iItems.Count()-1];
				iDiv->Owner()->Impl()->MoveElementL(e, iItems[0], EBeforeBegin);
				iItems.Remove(iItems.Count()-1);
				iItems.Insert(e,0);
			}
		}
		else
			offset = iItems.Count();
		
		for(TInt i=0;i<offset;i++)
		{
			CHtmlElementDiv* e = iItems[i];
			e->iFlags.Clear(CHtmlElementDiv::EDisabled);
			if(iRenderer)
				iRenderer->RenderListItemL(e, i + iTopItem);		
		}
		
		iDiv->Owner()->Impl()->iState.Set(EHCSFocusValid);
		iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
	}
	else
	{
		TInt offset = topItem - iTopItem;
		if(iDiv->iFocusedElement)
		{
			 TInt index = ListIndexOf(iDiv->iFocusedElement);
			 if(index<topItem || index>=topItem+iItems.Count())
			 {
				 iDiv->iFocusedElement->SetFocus(EFalse);
				 iDiv->iFocusedElement = NULL;
			 }
		}
		iTopItem = topItem;
		//scrolldown
		if(offset<iItems.Count())
		{
			for(TInt i=0;i<offset;i++) 
			{
				iDiv->Owner()->Impl()->MoveElementL(iItems[0], iItems[iItems.Count()-1], EAfterEnd);
				iItems.Append(iItems[0]);
				iItems.Remove(0);
			}
		}
		else
			offset = iItems.Count();

		for(TInt i=iItems.Count()-offset;i<iItems.Count();i++)
		{
			CHtmlElementDiv* e = iItems[i];
			if(i + iTopItem<iItemCount)
			{
				e->iFlags.Clear(CHtmlElementDiv::EDisabled);
				if(iRenderer)
					iRenderer->RenderListItemL(e, i + iTopItem);
			}
			else if(!e->iFlags.IsSet(CHtmlElementDiv::EDisabled))
			{
				iItems[i] = ((CHtmlElementDiv*)iDiv->Owner()->Impl()->CopyElementListL(iTemplate, iTemplate->iPrev, e, EAfterEnd));
				iItems[i]->iFlags.Set(CHtmlElementDiv::EDisabled);
				iDiv->Owner()->Impl()->FreeElementList(e, e->iEnd);
			}
		}
		
		iDiv->Owner()->Impl()->iState.Set(EHCSFocusValid);
		iDiv->Owner()->Impl()->iState.Set(EHCSNeedRefresh);
	}
}

void CHcList::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	if(aPointerEvent.iType==TPointerEvent::EDrag)
	{
		if(iAutoScrollingDirection==EAutoScrollingNone)
		{
			TInt yOffset = aPointerEvent.iPosition.iY - iTapPoint.iY;
			TInt tapping = 0;
			for(;tapping<iItems.Count();tapping++)
			{
				if(iItems[tapping]->CanFocus() 
						&& iItems[tapping]->Rect().Contains(aPointerEvent.iPosition))
					break;
			}
			TInt current = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
			TInt direction = EAutoScrollingNone;
			
			if(current>=0)
			{
				if(current<iViewColCount) //first row items
				{
					if(yOffset<-20)
						direction = EAutoScrollingUp;
				}
				else if(current<iViewColCount*2) //second row items
				{
					if(tapping<iViewColCount)
						direction = EAutoScrollingUp;
				}
				
				if(current>=iItems.Count()-iViewColCount) //last row items
				{
					if(yOffset>20)
						direction = EAutoScrollingDown;
				}
				else if(current>=iItems.Count()-iViewColCount*2) //last second row items
				{
					if(tapping>=iItems.Count()-iViewColCount)
						direction = EAutoScrollingDown;
				}
			}

			if(direction==EAutoScrollingUp && iTopItem>0)
			{
				if(!iTimer->IsActive())
					iTimer->After(KPointerRepeatInerval);
				iAutoScrollingDirection = direction;
				iTapPoint = aPointerEvent.iPosition;
			}
			else if(direction==EAutoScrollingDown && iTopItem+iItems.Count()<iItemCount)
			{
				if(!iTimer->IsActive())
					iTimer->After(KPointerRepeatInerval);
				iAutoScrollingDirection = direction;
				iTapPoint = aPointerEvent.iPosition;
			}
			else
			{
				iTimer->Cancel();
				
				if(tapping<iItems.Count()) 
				{
					CHtmlElementImpl* e = iItems[tapping];
					if(!e->IsFocused())
					{
						iDiv->FocusChangingTo(e);				
#ifdef TOUCH_FEEDBACK_SUPPORT
						iDiv->Owner()->Impl()->Env()->TouchFeedback()->InstantFeedback(ETouchFeedbackSensitive);
#endif
					}
				}
			}
		}
		else if(iAutoScrollingDirection==EAutoScrollingUp)
		{
			TInt yOffset = aPointerEvent.iPosition.iY - iTapPoint.iY;
			if(yOffset>0)
			{
				iAutoScrollingDirection = EAutoScrollingNone;
				iTimer->Cancel();
			}
			else
				iTapPoint = aPointerEvent.iPosition;
		}
		else
		{
			TInt yOffset = aPointerEvent.iPosition.iY - iTapPoint.iY;
			if(yOffset<0)
			{
				iAutoScrollingDirection = EAutoScrollingNone;
				iTimer->Cancel();
			}
			else
				iTapPoint = aPointerEvent.iPosition;
		}
	}
	else if(aPointerEvent.iType==TPointerEvent::EButton1Down)
	{
		iTapPoint = aPointerEvent.iPosition;
	}
	else if(aPointerEvent.iType==TPointerEvent::EButton1Up)
	{
		iAutoScrollingDirection = EAutoScrollingNone;
		iTimer->Cancel();
	}
}

void CHcList::OnTimerL(TInt aError, TInt)
{
	if(aError!=KErrNone)
		return;
	
	if(iAutoScrollingDirection==EAutoScrollingUp)
	{
#ifdef TOUCH_FEEDBACK_SUPPORT
		iDiv->Owner()->Impl()->Env()->TouchFeedback()->InstantFeedback(ETouchFeedbackSensitive);
#endif
		TInt current = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
		if(iTopItem>0)
		{
			MoveItemsUp();
			if(iItems[current]->CanFocus())
				iDiv->FocusChangingTo(iItems[current]);
			iTimer->After(KPointerRepeatInerval);
		}
		else
			iDiv->FocusChangingTo(iItems[current%iViewColCount]);
		iDiv->Owner()->RefreshAndDraw();
	}
	else if(iAutoScrollingDirection==EAutoScrollingDown)
	{
#ifdef TOUCH_FEEDBACK_SUPPORT
		iDiv->Owner()->Impl()->Env()->TouchFeedback()->InstantFeedback(ETouchFeedbackSensitive);
#endif
		TInt current = iItems.Find((CHtmlElementDiv*)iDiv->iFocusedElement);
		if(iTopItem<iItemCount-iViewColCount*iViewRowCount)
		{
			MoveItemsDown();
			if(iItems[current]->CanFocus())
				iDiv->FocusChangingTo(iItems[current]);
			iTimer->After(KPointerRepeatInerval);
			
			iDiv->Owner()->RefreshAndDraw();
		}
		else
		{
			current = iViewColCount*(iViewRowCount-1)+ current%iViewColCount;
			if(iTopItem + current<iItemCount)
			{
				if(iItems[current]->CanFocus())
					iDiv->FocusChangingTo(iItems[current]);
				iDiv->Owner()->RefreshAndDraw();
			}
		}
		
	}
}
