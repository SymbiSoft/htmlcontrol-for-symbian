#include <eikenv.h>
#include <eikedwin.h>

#include "controlimpl.h"
#include "htmlcontrol.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "scrollbar.h"
#include "imagepool.h"
#include "timer.h"
#include "stack.h"
#include "list.h"
#include "utils.h"
#include "writablebitmap.h"
#include "transimpl.h"

#include "element_body.h"
#include "element_div.h"
#include "element_hr.h"
#include "element_img.h"
#include "element_input.h"
#include "element_select.h"
#include "element_a.h"
#include "element_text.h"
#include "element_form.h"
#include "element_p.h"

CHtmlControlImpl* CHtmlControlImpl::NewL(CHtmlControl* aControl) 
{
	CHtmlControlImpl* self = new (ELeave)CHtmlControlImpl(aControl);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}

CHtmlControlImpl::CHtmlControlImpl(CHtmlControl* aControl):iControl(aControl)
{

}

CHtmlControlImpl::~CHtmlControlImpl()
{
	iState.Set(EHCSExiting);
	
	iEnv->RemoveDestroyMonitor(this);
	
	iObserver = NULL;
	delete iOffScreenBitmap;
	delete iTransition;
	
	FreeElementList(iBody, iBody->iPrev);
	iEnv->ImagePool().RemoveLoadedEventSubscriber(this);
	
	CWsScreenDevice* device = CEikonEnv::Static()->ScreenDevice();
	for(TInt i=0;i<iFontCache.Count();i++)
		device->ReleaseFont(iFontCache[i].iFont);
	iFontCache.Close();
	
	iControls.Close();
	iLines.Close();
	iVFEs.Close();
	delete iDelayRefreshTimer;
	delete iTextScrollTimer;
	delete iTempUsageStyle;
	delete iStyleSheet;
}

void CHtmlControlImpl::ConstructL()
{
	iEnv = CHtmlCtlEnv::Static();
	
	iDelayRefreshTimer = CHcTimer<CHtmlControlImpl>::NewL(*this, 0);
	iTextScrollTimer = CHcTimer<CHtmlControlImpl>::NewL(*this, 1);
	
	iStyleSheet = new (ELeave) CHcStyleSheet();
	iTempUsageStyle = new (ELeave) CHcStyle();
	
	iTransition = CTransition::NewL();
}

void  CHtmlControlImpl::CreateBodyL()
{
	iBody = new (ELeave) CHtmlElementBody(iControl);
	iBody->iParent = iBody;
	
	iBody->iEnd = new (ELeave) CHtmlElementDivEnd(iControl);
	iBody->iEnd->iParent = iBody;
	
	iBody->iEnd->iPrev = iBody;
	iBody->iEnd->iNext = iBody;
	iBody->iPrev = iBody->iEnd;
	iBody->iNext = iBody->iEnd;
	
	iBody->PrepareL();
	iBody->iEnd->PrepareL();
}

void CHtmlControlImpl::RegisterControl(CCoeControl* aControl) 
{
	iControls.Append(aControl);
}

void CHtmlControlImpl::UnregisterControl(CCoeControl* aControl) 
{
	TInt pos = iControls.Find(aControl);
	if(pos!=KErrNotFound)
	{
		if(aControl->IsFocused() && !iState.IsSet(EHCSExiting))
			aControl->SetFocus(EFalse, ENoDrawNow);
		iControls.Remove(pos);
	}
}

void CHtmlControlImpl::AddStyleSheetL(const TDesC& aSource) 
{
	HtmlParser::ParseStyleSheetL(aSource, *iStyleSheet);
}

void CHtmlControlImpl::RemoveStyleClass(const TDesC& aSelectorStr)
{
	THcSelector selector;
	HtmlParser::ParseSelector(aSelectorStr, selector);
	iStyleSheet->Remove(selector);
}

void CHtmlControlImpl::ClearStyleSheet()
{
	iStyleSheet->Clear();
}

TUint32 CHtmlControlImpl::StyleSheetVersion()
{
	return (((TUint32)iEnv->DefaultStyleSheet().Version())<<16) + iStyleSheet->Version();
}

TBool CHtmlControlImpl::GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute, CHcStyle& aStyle) const
{
	CHcStyleSheet& def = iEnv->DefaultStyleSheet();
	TInt ret = def.GetStyle(aTag, aClass, aFakeClass, aAttribute, aStyle);
	ret += iStyleSheet->GetStyle(aTag, aClass, aFakeClass, aAttribute, aStyle);
	return ret;
}

const CHcStyle& CHtmlControlImpl::GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute) const
{
	iTempUsageStyle->ClearAll();
	GetStyle(aTag, aClass, aFakeClass, aAttribute, *iTempUsageStyle);
	return *iTempUsageStyle;
}


CFont* CHtmlControlImpl::GetFont(const THcTextStyle& aTextStyle)
{
	TInt matchCode = aTextStyle.FontMatchCode();
	for(TInt i=0;i<iFontCache.Count();i++) 
	{
		if(iFontCache[i].iMatchCode==matchCode)
			return iFontCache[i].iFont;
	}
	
	CFont* font = aTextStyle.CreateFont();	
	THcFontCacheItem item;
	item.iFont = font;
	item.iMatchCode = matchCode;
	iFontCache.Append(item);
	
	return font;
}

void CHtmlControlImpl::ScrollText(TInt aTextWidth, const TRect& aBox)
{
	iTextScrollInfo.iTextWidth = aTextWidth;
	iTextScrollInfo.iBox = aBox;
	iTextScrollInfo.iScrollPos = 0;
	iTextScrollInfo.iState = TTextScrollInfo::EStarting;
	if(!iTextScrollTimer->IsActive())
		iTextScrollTimer->After(1000*1000);
}

void CHtmlControlImpl::MeasureTextContinued()
{
	if(!iDelayRefreshTimer->IsActive())
		iDelayRefreshTimer->After(500*1000);
}

TInt CHtmlControlImpl::CountComponentControls() const
{
	return iControls.Count();
}

CCoeControl* CHtmlControlImpl::ComponentControl(TInt aIndex) const
{
	if(aIndex<iControls.Count())
		return iControls[aIndex];
	else
		return NULL;
}

void CHtmlControlImpl::FireEventL(const THtmlCtlEvent& aEvent) 
{
	if(iObserver)
	{
		CHtmlCtlEnv* env = iEnv;
		env->AddDestroyMoinitor(this);
		iObserver->HandleHtmlCtlEventL(aEvent);
		if(!env->RemoveDestroyMonitor(this))
			User::Leave(KErrAbort);
	}
}

void CHtmlControlImpl::OnTimerL(TInt aError, TInt aIndex)
{
	if(aError!=KErrNone || iState.IsSet(EHCSInTransition))
		return;

	if(aIndex==0) //iDelayRefreshTimer
	{
		Refresh();
		//iControl->DrawNow();
		((RWindow*)iControl->DrawableWindow())->Invalidate(iControl->Rect());
	}
	else //iTextScrollTimer
		DoScrollText();
}

void CHtmlControlImpl::NotifyImageLoaded(CHcImage*)
{
	if(!iDelayRefreshTimer->IsActive())
		iDelayRefreshTimer->After(50*1000);
}

void CHtmlControlImpl::Refresh(TBool aInTransition)
{
	if(!iOffScreenBitmap)
	{
		if(iControl->Size()==TSize(0,0))
			return;
		
		iOffScreenBitmap = CWritableBitmap::NewL(iControl->Size(), 
				CEikonEnv::Static()->ScreenDevice()->DisplayMode());
	}
	else if(iOffScreenBitmap->SizeInPixels().iWidth<iControl->Size().iWidth
			|| iOffScreenBitmap->SizeInPixels().iHeight<iControl->Size().iHeight)
		iOffScreenBitmap->ResizeWithGcL(iControl->Size());
	
	if(iDelayRefreshTimer->IsActive())
		iDelayRefreshTimer->Cancel();
	if(iTextScrollTimer->IsActive())
		iTextScrollTimer->Cancel();
	if(iTransition && iTransition->IsActive())
		iTransition->Cancel();
	
	if(!iState.IsSet(EHCSFocusValid))
	{
		if(!iControl->IsFocused())
			iControl->SetFocus(ETrue, ENoDrawNow);
		
		SetFocusTo(iBody);
	}
	
	iState.Assign(EHCSInTransition, aInTransition);
	
	iEnv->MeasureStatus().Measure(this);
	
	CHtmlElementImpl* e = iBody;
	do
	{
		if(!e->iState.IsSet(EElementStateHidden))
			e->Layout();
		e->Refresh();
		e = e->iNext;
	}
	while(e!=iBody);
	
	iState.Clear(EHCSNeedRefresh);
	iState.Set(EHCSNeedRedraw);
}

void CHtmlControlImpl::DrawOffscreen()
{
	iOffScreenBitmap->Gc().CancelClippingRect();
	CHtmlElementImpl* current = iBody;
	do
	{
		if(!current->iState.IsSet(EElementStateHidden))
			current->Draw(iOffScreenBitmap->Gc());
		current = current->iNext;
	}
	while(current && current!=iBody);
	
	iState.Clear(EHCSNeedRedraw);
}

void CHtmlControlImpl::Draw(const TRect& aRect)
{
	if(!iOffScreenBitmap)
		return;
	
	if(iState.IsSet(EHCSNeedRedraw))
		DrawOffscreen();

	TPoint offset = aRect.iTl - iControl->Rect().iTl;
	iControl->SystemGc().BitBlt(aRect.iTl, iOffScreenBitmap, TRect(offset, aRect.Size()));
}

CHtmlElementImpl* CHtmlControlImpl::InsertContentL(CHtmlElementImpl* aTarget, TInsertPosition aPosition, const TDesC& aSource, TInt aFlags)
{
	if(aTarget==iBody) 
	{
		if(aPosition == EBeforeBegin)
			aPosition = EAfterBegin;
		else if(aPosition == EAfterEnd)
			aPosition = EBeforeEnd;
	}

	CHtmlElementDiv* parent;
	if(aTarget->TypeId()==EElementTypeDiv
		&& (aPosition==EAfterBegin || aPosition==EBeforeEnd))
		parent = (CHtmlElementDiv*)aTarget;
	else
		parent = iBody;
	
	CHtmlElementImpl* head = ParseL(parent, aSource, aFlags);
	if(!head)
		return NULL;

	InsertContent(head, head->iPrev, aTarget, aPosition);

	return head;	
}

void CHtmlControlImpl::InsertContent(CHtmlElementImpl* aHead, CHtmlElementImpl* aTail, CHtmlElementImpl* aTarget, TInsertPosition aPosition)
{
	aHead->iPrev = aTail;
	aTail->iNext = aHead;
	
	TBool hasLink = EFalse;
	CHtmlElementImpl* e = aHead;
	while(e!=aTail) 
	{
		if(e->TypeId()==EElementTypeA) 
		{
			hasLink = ETrue;
			break;
		}
		e = e->iNext;
	}
	
	switch(aPosition)
	{
		case EBeforeBegin:
		{
			if(aTarget->iPrev->TypeId()==EElementTypeA && hasLink) 
				aTarget = aTarget->iPrev;
					
			CHtmlElementImpl* targetPrev = aTarget->iPrev;
			
			targetPrev->iNext = aHead;
			aHead->iPrev = targetPrev;
			
			aTarget->iPrev = aTail;
			aTail->iNext = aTarget;				
			
			break;
		}
		case EAfterBegin:
		{
			if(aTarget->iNext->TypeId()==EElementTypeAEnd && hasLink)
				aTarget = aTarget->iNext;
			
			CHtmlElementImpl* targetNext = aTarget->iNext;

			aTarget->iNext = aHead;
			aHead->iPrev = aTarget;
			
			targetNext->iPrev = aTail;
			aTail->iNext = targetNext;
			
			break;
		}
		case EBeforeEnd:
			if(aTarget->TypeId()==EElementTypeDiv) 
			{
				aTarget = ((CHtmlElementDiv*)aTarget)->iEnd;
				CHtmlElementImpl* targetPrev = aTarget->iPrev;
				
				targetPrev->iNext = aHead;
				aHead->iPrev = targetPrev;
				
				aTarget->iPrev = aTail;
				aTail->iNext = aTarget;
			}
			else 
			{
				CHtmlElementImpl* targetPrev = aTarget->iPrev;
				
				if(targetPrev->TypeId()==EElementTypeA && hasLink) 
					targetPrev = targetPrev->iPrev;
							
				targetPrev->iNext = aHead;
				aHead->iPrev = targetPrev;
				
				aTarget->iPrev = aTail;
				aTail->iNext = aTarget;
			}
			break;
		case EAfterEnd:
			if(aTarget->TypeId()==EElementTypeDiv)
			{
				aTarget =  ((CHtmlElementDiv*)aTarget)->iEnd;
				CHtmlElementImpl* targetNext = aTarget->iNext;
				
				aTarget->iNext = aHead;
				aHead->iPrev = aTarget;
				
				targetNext->iPrev = aTail;
				aTail->iNext = targetNext;
			}
			else 
			{
				CHtmlElementImpl* targetNext = aTarget->iNext;
				if(targetNext->TypeId()==EElementTypeAEnd && hasLink) 
					targetNext = targetNext->iNext;
			
				aTarget->iNext = aHead;
				aHead->iPrev = aTarget;
				
				targetNext->iPrev = aTail;
				aTail->iNext = targetNext;
			}
			break;
	}
	
	iState.Clear(EHCSFocusValid);
}

CHtmlElementImpl* CHtmlControlImpl::ParseL(CHtmlElementDiv* aParent, const TDesC& aSource, TInt aFlags)
{
	CHtmlElementImpl* head = new (ELeave)CHtmlElementHr(iControl);
	head->iParent = aParent;
	TRAPD(error,
		iEnv->HtmlParser().ParseL(aSource, head, aFlags);
	)
	if(error!=KErrNone)
	{
		FreeElementList(head, head->iPrev);
		User::Leave(error);
	}

	CHtmlElementImpl* t = head->iNext;
	CHtmlElementImpl* tail = head->iPrev;
	delete head;
	if(t==head)
		return NULL;
	else {
		t->iPrev = tail;
		tail->iNext = t; 
		CHtmlElementImpl* e = t;
		do 
		{
			TRAPD(error, e->PrepareL();	)
			e = e->iNext;
		}while(e!=tail->iNext);
		
		return t;
	}
}


void CHtmlControlImpl::MoveElementL(CHtmlElementImpl* aElement, CHtmlElementImpl* aTarget, TInsertPosition aPosition) 
{
	CHtmlElementImpl* head = aElement;
	CHtmlElementImpl* tail;
	if(aElement->TypeId()==EElementTypeDiv) 
		tail = ((CHtmlElementDiv*)head)->iEnd;
	else
		tail = head;
	head->iPrev->iNext = tail->iNext;
	tail->iNext->iPrev = head->iPrev;
	
	InsertContent(head, tail, aTarget, aPosition);
}

CHtmlElementImpl* CHtmlControlImpl::CopyElementListL(CHtmlElementImpl* aHead, CHtmlElementImpl* aTail, CHtmlElementImpl* aTarget, TInsertPosition aPosition)
{	
	RHcStack<CHtmlElementDiv*> blocks;
	CleanupClosePushL(blocks);
	
	CHtmlElementA* link = NULL;
	CHtmlElementForm* form = NULL;

	CHtmlElementImpl* newCurrent = aHead->CloneL();
	if(aHead->iParent==aHead)
		newCurrent->iParent = (CHtmlElementDiv*)newCurrent;
	else 
		newCurrent->iParent = aHead->iParent;
	if(newCurrent->TypeId()==EElementTypeDiv)
		blocks.Push((CHtmlElementDiv*)newCurrent);
	else if(newCurrent->TypeId()==EElementTypeA)
		link = (CHtmlElementA*)newCurrent;
	else if(newCurrent->TypeId()==EElementTypeForm)
		form = (CHtmlElementForm*)newCurrent;

	CHtmlElementImpl* newHead = newCurrent;
	while(aHead!=aTail)
	{
		aHead = aHead->iNext;
		
		CHtmlElementImpl* sub = aHead->CloneL();
		if(blocks.Count()==0)
			sub->iParent = newHead->iParent;
		else
			sub->iParent = blocks.Top();
		sub->iPrev = newCurrent;
		newCurrent->iNext =sub;
		newCurrent = sub;
		
		if(sub->TypeId()==EElementTypeDiv)
			blocks.Push((CHtmlElementDiv*)sub);
		else if(sub->TypeId()==EElementTypeDivEnd) 
		{
			sub->iParent->iEnd = (CHtmlElementDivEnd*)sub;
			blocks.Pop();
		}
		else if(sub->TypeId()==EElementTypeA)
			link = (CHtmlElementA*)sub;
		else if(sub->TypeId()==EElementTypeAEnd) 
		{
			((CHtmlElementAEnd*)sub)->iStart = link;
			link->iEnd = (CHtmlElementAEnd*)sub;
		}
		else if(sub->TypeId()==EElementTypeForm)
			form = (CHtmlElementForm*)sub;
		else if(sub->TypeId()==EElementTypeFormEnd) 
		{
			((CHtmlElementFormEnd*)sub)->iStart = form;
			form->iEnd = (CHtmlElementFormEnd*)sub;
		}
		
		sub->PrepareL();
		sub->iState.Set(EElementStateRuntime);
	}
	CleanupStack::PopAndDestroy();
	
	if(aTarget)
		InsertContent(newHead, newCurrent, aTarget, aPosition);
	else
	{
		newHead->iPrev = newCurrent;
		newCurrent->iNext = newHead;
	}
	
	return newHead;
}

void CHtmlControlImpl::RemoveElement(CHtmlElementImpl* aElement)
{
	if(aElement==iBody) //not allow
		return;
	
	if(aElement->TypeId()==EElementTypeDiv)
	{
		CHtmlElementDiv* p = aElement->iParent;
		CHtmlElementImpl* it = ((CHtmlElementDiv*)aElement)->iNext;
		while(it!=((CHtmlElementDiv*)aElement)->iEnd)
		{
			it->iParent = p;
			it = it->iNext;
		}
		
		FreeElementList(((CHtmlElementDiv*)aElement)->iEnd, ((CHtmlElementDiv*)aElement)->iEnd);
		FreeElementList(aElement, aElement);
	}
	else if(aElement->TypeId()==EElementTypeA)
	{
		FreeElementList(((CHtmlElementA*)aElement)->iEnd, ((CHtmlElementA*)aElement)->iEnd);
		FreeElementList(aElement, aElement);
	}
	else if(aElement->TypeId()==EElementTypeForm)
	{
		FreeElementList(((CHtmlElementForm*)aElement)->iEnd, ((CHtmlElementForm*)aElement)->iEnd);
		FreeElementList(aElement, aElement);
	}
	else if(aElement->TypeId()==EElementTypeP)
	{
		FreeElementList(((CHtmlElementP*)aElement)->iEnd, ((CHtmlElementP*)aElement)->iEnd);
		FreeElementList(aElement, aElement);
	}
	else
		FreeElementList(aElement, aElement);
}

void CHtmlControlImpl::FreeElementList(CHtmlElementImpl* aStart, CHtmlElementImpl* aEnd)
{
	if(aStart->iPrev!=aEnd)
	{
		aStart->iPrev->iNext = aEnd->iNext;
		aEnd->iNext->iPrev = aStart->iPrev;
	}
	
	CHtmlElementImpl* e = aStart, *n;
	aStart->iPrev = aEnd;
	aEnd->iNext = aStart;
	
	CHtmlElementDiv* container = FindContainer(e);
	do
	{
		if(e==container->iFocusedElement)
		{
			container->iFocusedElement = NULL;
			iState.Clear(EHCSFocusValid);
		}
		if(e==iTapInfo.iStartElement)
		{
			iTapInfo.iStartElement = container;
		}
		
		e = e->iNext;
	}
	while(e!=aStart);
	
	e = aStart;
	do
	{
		n = e->iNext;
		delete e;
		e = n;
	}
	while(e!=aStart);
}

CHtmlElementDiv* CHtmlControlImpl::FindContainer(CHtmlElementImpl* aElement) const
{
	if(aElement)
	{
		CHtmlElementDiv* e = aElement->iParent;
		while(e!=iBody)
		{
			if(e->IsContainer())
				return e;
					
			e = e->iParent;
		}
	}

	return iBody;
}

CHtmlElementImpl* CHtmlControlImpl::FindLink(CHtmlElementImpl* aElement) const
{
	if(aElement)
	{
		CHtmlElementImpl* e = aElement->iPrev;
		while(e!=iBody)
		{
			if(e->TypeId()==EElementTypeA)
				return e;
					
			e = e->iPrev;
		}
	}

	return NULL;
}

CHtmlElementImpl* CHtmlControlImpl::FocusedElement() const
{
	CHtmlElementImpl* e = iBody;
	
	while(ETrue)
	{
		if(!((CHtmlElementDiv*)e)->iFocusedElement)
			return e;
		
		e = ((CHtmlElementDiv*)e)->iFocusedElement;
		
		if(e->TypeId()!=EElementTypeDiv || !((CHtmlElementDiv*)e)->IsContainer())
			return e;
	}
}

void CHtmlControlImpl::SetFocusTo(CHtmlElementImpl* aElement)
{
	if(!iControl->IsFocused())
		iControl->SetFocus(ETrue, ENoDrawNow);
	
	if(aElement==iBody) 
	{
		iState.Set(EHCSFocusValid);
		iBody->SetFocus(ETrue);
		return;
	}
	
	if(!aElement->CanFocus())
		return;
	
	RPointerArray<CHtmlElementDiv> path;
	CHtmlElementDiv* e = aElement->iParent;
	while(ETrue)
	{
		if(e->IsContainer())
			path.Append(e);
		
		if(e==iBody)
			break;
	
		e = e->iParent;
	}

	iState.Set(EHCSAutoFocusDisabled);
	if(path.Count()>1)
	{
		for(TInt i=path.Count()-1;i>0;i--) 
		{
			CHtmlElementDiv* e = path[i];
			e->SetFocus(ETrue);
			e->FocusChangingTo(path[i-1]);
		}
	}
	else
		path[0]->SetFocus(ETrue);
	iState.Clear(EHCSAutoFocusDisabled);
	path[0]->FocusChangingTo(aElement);
	
	path.Close();
	
	iState.Set(EHCSFocusValid);
}

void CHtmlControlImpl::ScrollToView(CHtmlElementImpl* aElement) 
{
	if(!aElement)
		return;

	CHtmlElementDiv* container = FindContainer(aElement);
	TInt offset = aElement->iPosition.iY + container->iScrollbar->RealPos()- container->iPosition.iY;
	container->iScrollbar->SetPos(offset - 10);
	if(container->IsFocused())
	{
		if(aElement->CanFocus() && aElement->iFlags.IsSet(CHtmlElementImpl::ETabStop) )
			SetFocusTo(aElement);
		else
			ResolveFocus(container);
	}
	iState.Set(EHCSNeedRefresh);
}

TBool CHtmlControlImpl::VisibilityTest(CHtmlElementImpl* aElement, const TRect& aDisplayRect)
{
	if(aElement->TypeId()==EElementTypeA)
	{
		CHtmlElementImpl* e = aElement->iNext;
		while(e!=((CHtmlElementA*)aElement)->iEnd)
		{
			if(HcUtils::Intersects(e->Rect(), aDisplayRect))
				return ETrue;
			e = e->iNext;
		}
		return EFalse;
	}
	else
		return HcUtils::Intersects(aElement->Rect(), aDisplayRect);
}

void CHtmlControlImpl::UpdateVFEs(CHtmlElementDiv* aContainer)
{
	iVFEs.Reset();
	
	TRect displayRect = aContainer->iDisplayRect;
	displayRect.Move(0,  aContainer->iScrollbar->Pos()-aContainer->iScrollbar->RealPos());
	
	CHtmlElementA* link = NULL;
	CHtmlElementImpl* e = aContainer->iNext;
	TRect linkRect;
	while(e!=aContainer->iEnd)
	{
		if(e->TypeId()==EElementTypeA)
		{
			if(e->CanFocus() && e->iFlags.IsSet(CHtmlElementImpl::ETabStop) && !e->iState.IsSet(EElementStateHidden))
			{
				link = (CHtmlElementA*) e;
				linkRect = TRect();
			}
			e = e->iNext;
			continue;
		}
		else if(e->TypeId()==EElementTypeAEnd)
		{
			if(link && !linkRect.IsEmpty())
			{
				iVFEs.Append(link);
				link->iPosition = linkRect.iTl;
				link->iSize = linkRect.Size();
			}
			link = NULL;
			e = e->iNext;
			continue;
		}
		
		if((e->CanFocus() && e->iFlags.IsSet(CHtmlElementImpl::ETabStop) && !e->iState.IsSet(EElementStateHidden) || link) && HcUtils::Intersects(e->Rect(), displayRect))
		{
			if(link)
			{
				if(linkRect.IsEmpty())
					linkRect = e->Rect();
				else
					linkRect.BoundingRect(e->Rect());
			}
			else
				iVFEs.Append(e);
		}
		
		if(e->TypeId()==EElementTypeDiv 
				&& (((CHtmlElementDiv*)e)->IsContainer() || e->iState.IsSet(EElementStateHidden)))
			e = ((CHtmlElementDiv*)e)->iEnd;
		else
			e = e->iNext;
	}
}

inline TInt CompareLine(const TRect& r1, const TRect& r2) //return >0 if r1 is below r2, <0 if r1 is above r2 
{
	if(r1.iBr.iY<=r2.iTl.iY)
		return -1;
	else if(r1.iTl.iY>=r2.iBr.iY)
		return 1;
	else if(HcUtils::Intersects(r1, r2))
	{
		if(r1.iTl.iY<r2.iTl.iY && r1.iBr.iY<r2.iBr.iY)
			return -1;
		else if(r1.iBr.iY>r2.iBr.iY && r1.iTl.iY<r2.iTl.iY)
			return 1;
		else
			return r1.iTl.iY - r2.iTl.iY - (r2.iBr.iY - r1.iBr.iY);
	}
	else
		return 0;
}

void CHtmlControlImpl::SkipLeftSameLine(TInt& aIndex)
{
	CHtmlElementImpl* lineStart = iVFEs[aIndex];
	TRect rect = lineStart->Rect();
	for(aIndex--;aIndex>=0;aIndex--)
	{
		CHtmlElementImpl* e = iVFEs[aIndex];
		TInt c = CompareLine(e->Rect(), rect);
		if(c!=0)
			break;
	}
}

void CHtmlControlImpl::SkipRightSameLine(TInt& aIndex)
{
	CHtmlElementImpl* lineStart = iVFEs[aIndex];
	TRect rect = lineStart->Rect();
	for(aIndex++;aIndex<iVFEs.Count();aIndex++)
	{
		CHtmlElementImpl* e = iVFEs[aIndex];
		TInt c = CompareLine(e->Rect(), rect);
		if(c!=0)
			break;
	}
}

CHtmlElementImpl* CHtmlControlImpl::FindAboveElement(TInt aToIndex, TInt aX)
{
	CHtmlElementImpl* found = NULL;
	CHtmlElementImpl* lineStart = iVFEs[aToIndex];
	TRect rect = lineStart->Rect();
	TInt nearest = Abs(lineStart->iPosition.iX - aX);
	for(TInt i=aToIndex-1;i>=0;i--)
	{
		CHtmlElementImpl* e = iVFEs[i];
		TInt c = CompareLine(e->Rect(), rect);
		if(c==0)
		{
			TInt offset = Abs(e->iPosition.iX - aX);
			if(offset<nearest)
			{
				found = e;
				nearest = offset;
			}
			else if(offset==nearest && found && e->iPosition.iY<found->iPosition.iY)
				found = e;
		}
		else if(c<0)
			break;
	}
	
	if(found)
		return found;
	else
		return lineStart;
}

CHtmlElementImpl* CHtmlControlImpl::FindBelowElement(TInt aFromIndex, TInt aX)
{
	CHtmlElementImpl* found = NULL;
	CHtmlElementImpl* lineStart = iVFEs[aFromIndex];
	TRect rect = lineStart->Rect();
	TInt nearest = Abs(lineStart->iPosition.iX - aX);
	for(TInt i=aFromIndex+1;i<iVFEs.Count();i++)
	{
		CHtmlElementImpl* e = iVFEs[i];
		TInt c = CompareLine(e->Rect(), rect);
		if(c==0)
		{
			TInt offset = Abs(e->iPosition.iX - aX);
			if(offset<nearest)
			{
				found = e;
				nearest = offset;
			}
			else if(offset==nearest && found && e->iPosition.iY<found->iPosition.iY)
				found = e;
		}
		else if(c>0)
			break;
	}
	
	if(found)
		return found;
	else
		return lineStart;
}

void CHtmlControlImpl::ResolveFocus(CHtmlElementDiv* aContainer)
{
	CHtmlElementImpl* found = NULL;
	
	UpdateVFEs(aContainer);
	if(iVFEs.Count()>0)
	{
		TInt index = iVFEs.Find(aContainer->iFocusedElement);
		if(index==KErrNotFound) //focus is invisible, then set last
			found = iVFEs[0];
		else
			found = aContainer->iFocusedElement;
	}
	
	if(found)
		aContainer->FocusChangingTo(found);
}

void CHtmlControlImpl::HandleKeyLeft(CHtmlElementDiv* aContainer, TBool aScrolled)
{
	CHtmlElementImpl* found = NULL;
	
	UpdateVFEs(aContainer);
	if(iVFEs.Count()==0) //nothing visible
	{
		if(!aScrolled && aContainer->iScrollbar->RemoveStepPos()) //scroll up
		{
			UpdateVFEs(aContainer);
			if(iVFEs.Count()>0) //set last
				found = iVFEs[iVFEs.Count()-1];
		}
	}
	else
	{
		TInt index = iVFEs.Find(aContainer->iFocusedElement);
		if(index==KErrNotFound) //focus is invisible, then set last
			found = iVFEs[iVFEs.Count()-1];
		else if(index>0) //focus visible, and some element before it
			found = iVFEs[index-1];
		else if(!aScrolled && aContainer->iScrollbar->RemoveStepPos()) //focus visible, but nothing before it
			HandleKeyLeft(aContainer, ETrue); 
	}
	
	if(found)
	{
		TInt adjustY = aContainer->iScrollbar->RealPos() - aContainer->iScrollbar->Pos();
		TInt offset = found->iPosition.iY + adjustY - aContainer->iDisplayRect.iTl.iY - 2;
		if(offset<0 && offset<aContainer->iDisplayRect.Height())  //partical visible
			aContainer->iScrollbar->AddPos(offset);	
		aContainer->FocusChangingTo(found);
	}
}

void CHtmlControlImpl::HandleKeyRight(CHtmlElementDiv* aContainer, TBool aScrolled)
{
	CHtmlElementImpl* found = NULL;
	
	UpdateVFEs(aContainer);
	if(iVFEs.Count()==0) //nothing visible
	{
		if(!aScrolled && aContainer->iScrollbar->AddStepPos()) //scroll down
		{
			UpdateVFEs(aContainer);
			if(iVFEs.Count()>0) //set first
				found = iVFEs[0];
		}
	}
	else
	{
		TInt index = iVFEs.Find(aContainer->iFocusedElement);
		if(index==KErrNotFound) //focus is invisible, then set first
			found = iVFEs[0];
		else if(index<iVFEs.Count()-1) //focus visible, and some element after it
			found = iVFEs[index+1];
		else if(!aScrolled && aContainer->iScrollbar->AddStepPos()) //focus visible, but nothing after it
			HandleKeyRight(aContainer, ETrue); 
	}
	
	if(found)
	{
		TInt adjustY = aContainer->iScrollbar->RealPos() - aContainer->iScrollbar->Pos();
		TInt offset = found->iPosition.iY + adjustY + found->iSize.iHeight  - aContainer->iDisplayRect.iBr.iY + 2;
		if(offset>0 && offset<aContainer->iDisplayRect.Height())  //partical visible
			aContainer->iScrollbar->AddPos(offset);
		aContainer->FocusChangingTo(found);
	}
}

void CHtmlControlImpl::HandleKeyUp(CHtmlElementDiv* aContainer, TBool aScrolled)
{
	CHtmlElementImpl* found = NULL;
	
	UpdateVFEs(aContainer);
	TInt index = iVFEs.Find(aContainer->iFocusedElement);
	if(index!=KErrNotFound) //focus is visible
	{
		//if(aScrolled)
		//	return;
		
		//in this case, also implict iFocusedElement is not null
		SkipLeftSameLine(index); //skip same line elements
		if(index>=0 && index<iVFEs.Count()) //find some elements above
			found = FindAboveElement(index, aContainer->iFocusedElement->iPosition.iX);
		if(!found) //nothing above, then scroll
		{
			if(aContainer->iScrollbar->Max()==0) //can't scroll
			{
				//search from the bottom
				if(aContainer->iScrollbar->IsLoop() && iVFEs.Count()>0)
					found = FindBelowElement(iVFEs.Count()-1, aContainer->iFocusedElement->iPosition.iX);
			}
			else if(!aScrolled && aContainer->iScrollbar->RemoveStepPos(ETrue)) //suc scroll
				HandleKeyUp(aContainer, ETrue);
		}
		
		if(!found)
			found = aContainer->iFocusedElement;
	}
	else if(aContainer->iFocusedElement) //focus is invisible
	{
		if(iVFEs.Count()==0) //nothing visible, then scroll
		{
			if(!aScrolled && aContainer->iScrollbar->RemoveStepPos(ETrue)) //suc scroll
				HandleKeyUp(aContainer, ETrue);
		}
		else //set last one
		{
			found = FindBelowElement(iVFEs.Count()-1, aContainer->iFocusedElement->iPosition.iX);
		}
	}
	else //no focus before
	{
		//set last one
		if(iVFEs.Count()>0)
			found = iVFEs[iVFEs.Count()-1];
		else //scroll
		{
			if(!aScrolled && aContainer->iScrollbar->RemoveStepPos(ETrue)) //suc scroll
				HandleKeyUp(aContainer, ETrue);
		}
	}
	
	if(found)
	{
		TInt adjustY = aContainer->iScrollbar->RealPos() - aContainer->iScrollbar->Pos();
		TInt offset = found->iPosition.iY + adjustY - aContainer->iDisplayRect.iTl.iY - 2;
		if(offset<0 && offset<aContainer->iDisplayRect.Height())  //partical visible
			aContainer->iScrollbar->AddPos(offset);
		aContainer->FocusChangingTo(found);
	}
}

void CHtmlControlImpl::HandleKeyDown(CHtmlElementDiv* aContainer, TBool aScrolled)
{
	CHtmlElementImpl* found = NULL;
	
	UpdateVFEs(aContainer);
	TInt index = iVFEs.Find(aContainer->iFocusedElement);
	if(index!=KErrNotFound) //focus is visible
	{
		//if(aScrolled)
			//return;
		
		//in this case, also implict iFocusedElement is not null
		SkipRightSameLine(index); //skip same line elements
		if(index>=0 && index<iVFEs.Count()) //find some elements below
			found = FindBelowElement(index, aContainer->iFocusedElement->iPosition.iX);
		if(!found) //nothing below, then scroll
		{
			if(aContainer->iScrollbar->Max()==0) //can't scroll
			{
				//search from the top
				if(aContainer->iScrollbar->IsLoop() && iVFEs.Count()>0)
					found = FindBelowElement(0, aContainer->iFocusedElement->iPosition.iX);
			}
			else if(!aScrolled && aContainer->iScrollbar->AddStepPos(ETrue)) //suc scroll
				HandleKeyDown(aContainer, ETrue);
		}
		if(!found)
			found = aContainer->iFocusedElement;
	}
	else if(aContainer->iFocusedElement) //focus is invisible
	{
		if(iVFEs.Count()==0) //nothing visible, then scroll
		{
			if(!aScrolled && aContainer->iScrollbar->AddStepPos(ETrue)) //suc scroll
				HandleKeyDown(aContainer, ETrue);
		}
		else //set first one
		{
			found = FindBelowElement(0, aContainer->iFocusedElement->iPosition.iX);
		}
	}
	else //no focus before
	{
		//set first one
		if(iVFEs.Count()>0)
			found = iVFEs[0];
		else //scroll
		{
			if(!aScrolled && aContainer->iScrollbar->AddStepPos(ETrue)) //suc scroll
				HandleKeyDown(aContainer, ETrue);
		}
	}
	
	if(found)
	{
		TInt adjustY = aContainer->iScrollbar->RealPos() - aContainer->iScrollbar->Pos();
		TInt offset = found->iPosition.iY + adjustY + found->iSize.iHeight - aContainer->iDisplayRect.iBr.iY + 2;
		if(offset>0 && offset<aContainer->iDisplayRect.Height())  //partical visible
			aContainer->iScrollbar->AddPos(offset);	
		aContainer->FocusChangingTo(found);	
	}
}

TKeyResponse CHtmlControlImpl::OfferKeyEventL2 (CHtmlElementDiv* aContainer, const TKeyEvent &aKeyEvent, TEventCode aType) 
{	
	if(aContainer->iFocusedElement
		&& aContainer->iFocusedElement->CanFocus() 
		&& !aContainer->iFocusedElement->iState.IsSet(EElementStateHidden))
	{
		if(aContainer->iFocusedElement->TypeId()==EElementTypeDiv
				&& ((CHtmlElementDiv*)aContainer->iFocusedElement)->IsContainer())
		{
			if(OfferKeyEventL2((CHtmlElementDiv*)aContainer->iFocusedElement, aKeyEvent, aType)==EKeyWasConsumed)
				return EKeyWasConsumed;
		}
		 
		if((VisibilityTest(aContainer->iFocusedElement, aContainer->iDisplayRect)
#ifdef __SERIES60_5_ONWARDS__
			||	((aContainer->iFocusedElement->TypeId()==EElementTypeInput || aContainer->iFocusedElement->TypeId()==EElementTypeTextArea)
				&&  aContainer->iFocusedElement->EmbedObject() 
				&& (((CEikEdwin*)aContainer->iFocusedElement->EmbedObject())->AknEdwinFlags() & EAknEditorFlagTouchInputModeOpened))
#endif
			)
			&& aContainer->iFocusedElement->OfferKeyEventL(aKeyEvent, aType)==EKeyWasConsumed)
			return EKeyWasConsumed;
	}
	
	if(aContainer->iList && !aContainer->iList->IsEmpty())
		return aContainer->iList->OfferKeyEventL(aKeyEvent, aType);

	if(aType!=EEventKey || aContainer->iNext==aContainer->iEnd)
		return EKeyWasNotConsumed;
	
	aContainer->iState.Clear(EElementStateFocusChanged);
	TInt keyCode = HcUtils::TranslateKey(aKeyEvent.iCode);
	switch(keyCode)
	{
		case EKeyLeftArrow:
			iState.Set(EHCSNavKeyPrev);
			HandleKeyLeft(aContainer);
			break;
			
		case EKeyRightArrow:
			iState.Set(EHCSNavKeyNext);
			HandleKeyRight(aContainer);
			break;
			
		case EKeyUpArrow:
			iState.Set(EHCSNavKeyPrev);
			HandleKeyUp(aContainer);
			break;
			
		case EKeyDownArrow:
			iState.Set(EHCSNavKeyNext);
			HandleKeyDown(aContainer); 
			break;
//use accesskey property to gain these functions instead
/*			
		case '*':
			aContainer->iScrollbar->SetTopPos();
			aContainer->SetFirstFocus();
			break;
			
		case '#':
			aContainer->iScrollbar->SetBottomPos();
			aContainer->SetLastFocus();
			break;
*/
	}
	iState.Clear(EHCSNavKeyPrev);
	iState.Clear(EHCSNavKeyNext);
	if(aContainer->iScrollbar->Pos()!=aContainer->iScrollbar->RealPos())
	{
		if(!iState.IsSet(EHCSNeedRefresh))
			QuickScroll(aContainer);
		
		return EKeyWasConsumed;
	}
	else if(aContainer->iState.IsSet(EElementStateFocusChanged))
		return EKeyWasConsumed;
	else
		return EKeyWasNotConsumed;
}

TKeyResponse CHtmlControlImpl::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	if(iState.IsSet(EHCSInTransition))
		return EKeyWasNotConsumed;

	TKeyResponse ret = EKeyWasNotConsumed;
	TRAPD(error,
		ret = OfferKeyEventL2(iBody, aKeyEvent, aType);
		if(ret==EKeyWasNotConsumed && aType==EEventKey && aKeyEvent.iCode>32 && aKeyEvent.iCode<128)
		{
			//access key support
			CHtmlElementImpl* e = iBody;
			do
			{
				if(!e->iState.IsSet(EElementStateHidden) && e->iAccessKey==aKeyEvent.iCode)
				{
					ScrollToView(e);
					if(e->CanFocus())
						e->HandleButtonEventL(EButtonEventClick);
					ret = EKeyWasConsumed;
					break;
				}
				e = e->iNext;
			}
			while(e!=iBody);
		}
	);
	
	if(error==KErrAbort)
		return ret;
	else
		User::LeaveIfError(error);
	
	if(iState.IsSet(EHCSNeedRefresh))
		Refresh();
	
	if(iState.IsSet(EHCSNeedRedraw))
		//iControl->DrawNow();
		((RWindow*)iControl->DrawableWindow())->Invalidate(iControl->Rect());

	return ret;
}

TBool CHtmlControlImpl::HitTest(CHtmlElementImpl* aElement, const TPoint& aPoint) const
{	
	if(aElement->iState.IsSet(EElementStateHidden))
		return EFalse;
	
	if(aElement->TypeId()==EElementTypeText)
	{
		for(TInt i=0;i<((CHtmlElementText*)aElement)->TextRectCount();i++) 
		{
			if(((CHtmlElementText*)aElement)->TextRect(i).Contains(aPoint))
				return ETrue;
		}
		
		return EFalse;
	}
	else if(aElement->TypeId()==EElementTypeInput)
	{
		TRect rect = aElement->Rect();
		rect.Grow(10, 10);
		return rect.Contains(aPoint);
	}
	else
		return aElement->Rect().Contains(aPoint);
}

TBool CHtmlControlImpl::HitTest(CHcScrollbar* aScrollbar, const TPoint& aPoint) const
{
	if(aScrollbar->IsVisible())
	{
		TRect rect = aScrollbar->Rect();
		rect.iTl.iX -= aScrollbar->Width();
		return rect.Contains(aPoint);
	}
	else
		return EFalse;
}

void CHtmlControlImpl::GetElementByPosition(CHtmlElementDiv* aTopContainer, const TPoint& aPoint, CHtmlElementImpl*& aElement, CHtmlElementDiv*& aContainer, TTapArea& aArea)
{
	if(HitTest(aTopContainer->iScrollbar,aPoint))
	{
		aElement = aTopContainer;
		aContainer = aTopContainer;
		aArea = ETapAreaScrollbar;
		return;
	}
	
	CHtmlElementImpl* e = aTopContainer->iEnd->iPrev;
	CHtmlElementA* link = NULL;
	aElement = aTopContainer;
	
	while(e!=aTopContainer)
	{
		if(e->TypeId()==EElementTypeAEnd)
		{
			link = ((CHtmlElementAEnd*)e)->iStart;
			if(!link->CanFocus() || link->iState.IsSet(EElementStateHidden))
				link = NULL;
			e = e->iPrev;
			continue;
		}
		else if(e->TypeId()==EElementTypeA)
		{
			link = NULL;
			e = e->iPrev;
			continue;
		}
		else if(e->TypeId()==EElementTypeDivEnd)
		{
			if(e->iParent->IsContainer())
			{
				if(e->iParent->CanFocus()
						&& !e->iParent->iState.IsSet(EElementStateHidden)
						&& HitTest(e->iParent, aPoint))
				{
					GetElementByPosition(e->iParent, aPoint, aElement, aContainer, aArea);
					return;
				}
				else
				{
					e = e->iParent->iPrev;
					continue;
				}
			}
			else
			{
				e = e->iPrev;
				continue;
			}
		}
			
		if((e->CanFocus() && !e->iState.IsSet(EElementStateHidden) || link) && HitTest(e, aPoint))
		{
			if(link)
				aElement = link;
			else
				aElement = e;
			break;
		}
		e = e->iPrev;
	}
	
	if(aTopContainer->iList && !aTopContainer->iList->IsEmpty())
		aArea = ETapAreaList;
	else
		aArea = ETapAreaMain;
	aContainer = aTopContainer;
}

void CHtmlControlImpl::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	if(iState.IsSet(EHCSInTransition))
		return;
	
	TPointerEvent eventCopy = aPointerEvent;
	eventCopy.iPosition -= iControl->Rect().iTl;
	TRAPD(error,
		HandlePointerEventL2(eventCopy);
	);
	
	if(error==KErrAbort)
		return;
	else
		User::LeaveIfError(error);

	if(iState.IsSet(EHCSNeedRefresh))
		Refresh();
	
	if(iState.IsSet(EHCSNeedRedraw))
		//iControl->DrawNow();
		((RWindow*)iControl->DrawableWindow())->Invalidate(iControl->Rect());
}

void CHtmlControlImpl::HandlePointerEventL2(const TPointerEvent& aPointerEvent)
{
	if(aPointerEvent.iType==TPointerEvent::EDrag)
	{
		if(!iTapInfo.iStartElement)
			return;
		
		if(iTapInfo.iStartArea==ETapAreaScrollbar)
			iTapInfo.iContainer->iScrollbar->HandlePointerEventL(aPointerEvent);
		else if(iTapInfo.iStartArea==ETapAreaList)
			iTapInfo.iContainer->iList->HandlePointerEventL(aPointerEvent);
		else
		{
			CHtmlElementImpl* e = NULL;
			if(HitTest(iTapInfo.iContainer, aPointerEvent.iPosition))
			{
				TTapArea tapArea;
				CHtmlElementDiv* notused;
				GetElementByPosition(iTapInfo.iContainer, aPointerEvent.iPosition, e, notused, tapArea);
				if(!e->IsFocused())
				{
					if(e->iFlags.IsSet(CHtmlElementImpl::ETabStop)) {
						SetFocusTo(e);
					
#ifdef TOUCH_FEEDBACK_SUPPORT
						if(e!=iTapInfo.iContainer)
							iEnv->TouchFeedback()->InstantFeedback(ETouchFeedbackSensitive);
#endif
					}
				}
			}
			
			if(iTapInfo.iContainer->iScrollbar->IsSrollOnSlide())
			{
				TInt xOffset = aPointerEvent.iPosition.iX - iTapInfo.iPoint.iX;
				TInt yOffset = aPointerEvent.iPosition.iY - iTapInfo.iPoint.iY;
				iTapInfo.iPoint = aPointerEvent.iPosition;
				
				if(Abs(yOffset)>Abs(xOffset))
				{
					iTapInfo.iContainer->iScrollbar->AddPos(-yOffset*2);
					NotifyScrollPosChanged();
				}
			}
		}
		
		if(Abs(aPointerEvent.iPosition.iX - iTapInfo.iStartPoint.iX)>20 
				|| Abs(aPointerEvent.iPosition.iY - iTapInfo.iStartPoint.iY)>20)
			iState.Set(EHCSSlideOccured);
	}
	else if(aPointerEvent.iType==TPointerEvent::EButton1Down)
	{
		iState.Clear(EHCSSlideOccured);
		GetElementByPosition(iBody, aPointerEvent.iPosition, iTapInfo.iStartElement, iTapInfo.iContainer, iTapInfo.iStartArea);
		if(!iTapInfo.iStartElement->IsFocused())
		{
			iTapInfo.iNewlyGainFocus = ETrue;
			
			if(iTapInfo.iStartElement->iFlags.IsSet(CHtmlElementImpl::ETabStop))
				SetFocusTo(iTapInfo.iStartElement);
		}
		else
			iTapInfo.iNewlyGainFocus = EFalse;
		iTapInfo.iStartElement->HandleButtonEventL(EButtonEventDown);
		
		if(iTapInfo.iStartArea==ETapAreaScrollbar)
			iTapInfo.iContainer->iScrollbar->HandlePointerEventL(aPointerEvent);
		else if(iTapInfo.iStartArea==ETapAreaList)
		{
#ifdef TOUCH_FEEDBACK_SUPPORT
			if(iTapInfo.iStartElement!=iTapInfo.iContainer)
				iEnv->TouchFeedback()->InstantFeedback(ETouchFeedbackBasic);
#endif
			iTapInfo.iContainer->iList->HandlePointerEventL(aPointerEvent);
		}
		else
		{
#ifdef TOUCH_FEEDBACK_SUPPORT
		if(iTapInfo.iStartElement!=iTapInfo.iContainer)
			iEnv->TouchFeedback()->InstantFeedback(ETouchFeedbackBasic);
#endif
		}
		iTapInfo.iStartPoint = aPointerEvent.iPosition;
		iTapInfo.iPoint = iTapInfo.iStartPoint;
	}
	else if(aPointerEvent.iType==TPointerEvent::EButton1Up)
	{
		if(iTapInfo.iStartElement)
		{
			iTapInfo.iStartElement->HandleButtonEventL(EButtonEventUp);
			
			if(iTapInfo.iStartArea==ETapAreaScrollbar)
				iTapInfo.iContainer->iScrollbar->HandlePointerEventL(aPointerEvent);
			else if(iTapInfo.iStartArea==ETapAreaList)
				iTapInfo.iContainer->iList->HandlePointerEventL(aPointerEvent);

			if(!iState.IsSet(EHCSSlideOccured))
			{
				if(iTapInfo.iNewlyGainFocus)
					iTapInfo.iStartElement->HandleButtonEventL(EButtonEventSelect);
				else
					iTapInfo.iStartElement->HandleButtonEventL(EButtonEventClick);
			}
			else
			{
				if(Abs(aPointerEvent.iPosition.iX - iTapInfo.iStartPoint.iX)>60
						&& Abs(aPointerEvent.iPosition.iY - iTapInfo.iStartPoint.iY)<30)
				{
					THtmlCtlEvent event;
					if(aPointerEvent.iPosition.iX<iTapInfo.iStartPoint.iX)
						event.iType = THtmlCtlEvent::EOnSlideLeft;
					else
						event.iType = THtmlCtlEvent::EOnSlideRight;
					event.iControl = iControl;
					event.iElement = iBody;
					FireEventL(event);
				}
			}
			
			iTapInfo.iStartElement = NULL;
		}
	}
	else
	{
		if(iTapInfo.iStartElement)
		{
			if(iTapInfo.iStartArea==ETapAreaScrollbar)
				iTapInfo.iContainer->iScrollbar->HandlePointerEventL(aPointerEvent);
		}
	}
}

void CHtmlControlImpl::QuickScroll(CHtmlElementDiv* aContainer)
{
	TInt adjustY = aContainer->iScrollbar->RealPos() - aContainer->iScrollbar->Pos();
	aContainer->iScrollbar->AdjustScrollPos();
	aContainer->Refresh();
	
	CHtmlElementImpl* e = aContainer->iNext;
	while(e!=aContainer->iEnd)
	{
		e->iPosition.iY += adjustY;
		if(e->TypeId()==EElementTypeDiv)
			((CHtmlElementDiv*)e)->iDisplayRect.Move(0, adjustY);
		e->Refresh();
		e = e->iNext;
	}
	
	iState.Set(EHCSNeedRedraw);
}

void CHtmlControlImpl::DoScrollText()
{
	RWindow& win = *(RWindow*)iControl->DrawableWindow();
	if(win.IsFaded())
	{
		iTextScrollInfo.iScrollPos = -1;
		DrawOffscreen();
		return;
	}
	
	if(iTextScrollInfo.iState==TTextScrollInfo::EStarting)
		iTextScrollInfo.iState = TTextScrollInfo::EScrolling;
			
	if(iTextScrollInfo.iState==TTextScrollInfo::EScrolling)
	{
		iTextScrollInfo.iScrollPos += 10;
		if(iTextScrollInfo.iScrollPos + iTextScrollInfo.iBox.Width() >= iTextScrollInfo.iTextWidth)
		{
			iTextScrollInfo.iScrollPos = iTextScrollInfo.iTextWidth - iTextScrollInfo.iBox.Width();
			iTextScrollInfo.iState = TTextScrollInfo::EEnding;
			
			iTextScrollTimer->After(1000*1000);
		}
		else	
			iTextScrollTimer->After(100*1000);
	}
	
	DrawOffscreen();
	
	TRect rect = iTextScrollInfo.iBox;
	rect.Move(iControl->Rect().iTl);
	CWindowGc& gc = iControl->SystemGc();
	win.Invalidate(rect);
	gc.Activate(win);
	win.BeginRedraw(rect);
	iControl->SystemGc().BitBlt(rect.iTl, iOffScreenBitmap, iTextScrollInfo.iBox);
	win.EndRedraw();
	gc.Deactivate();
	
	if(iTextScrollInfo.iState==TTextScrollInfo::EEnding)
		iTextScrollInfo.iScrollPos = -1;	
}

void CHtmlControlImpl::NotifyScrollPosChanged()
{
	if(iTapInfo.iContainer->iList)
		iTapInfo.iContainer->iList->NotifyScrollPosChanged();
	else if(!iState.IsSet(EHCSNeedRefresh))
		QuickScroll(iTapInfo.iContainer);	
}



