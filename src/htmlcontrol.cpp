#include <eikenv.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <aknutils.h>
#endif

#include "htmlcontrol.h"
#include "controlimpl.h"
#include "elementimpl.h"
#include "htmlctlenv.h"
#include "htmlctlevent.h"
#include "htmlparser.h"
#include "scrollbar.h"
#include "style.h"
#include "imagepool.h"
#include "transimpl.h"
#include "element_body.h"

CHtmlControl* CHtmlControl::NewL(CCoeControl* aParent) 
{
	CHtmlControl* self = new (ELeave)CHtmlControl();
	CleanupStack::PushL(self);
	self->ConstructL(aParent);
	CleanupStack::Pop();
	return self;
}

CHtmlControl::CHtmlControl()
{

}

CHtmlControl::~CHtmlControl()
{
	delete iImpl;
}

void CHtmlControl::ConstructL(CCoeControl* aParent)
{
	if(aParent)
	{
		SetContainerWindowL(*aParent);
#ifdef __SYMBIAN_9_ONWARDS__
		SetParent(aParent);
#endif
		SetMopParent(aParent);
	}
	else
		CreateWindowL();
	SetFocusing(ETrue);
	
	EnableDragEvents();
	DrawableWindow()->SetPointerGrab(ETrue);
	
	iImpl = CHtmlControlImpl::NewL(this);
	iImpl->CreateBodyL();
}

CHtmlElement* CHtmlControl::Body() const
{
	return (CHtmlElement*)iImpl->Body();
}

void CHtmlControl::SetEventObserver(MHtmlCtlEventObserver* aObserver) 
{
	iImpl->SetEventObserver(aObserver);
}

void CHtmlControl::ScrollToView(CHtmlElement* aElement) 
{
	iImpl->ScrollToView((CHtmlElementImpl*)aElement);
}

CHtmlElement* CHtmlControl::InsertContentL(CHtmlElement* aTarget, TInsertPosition aPosition, const TDesC& aSource, TInt aFlags)
{
	return iImpl->InsertContentL((CHtmlElementImpl*)aTarget, aPosition, aSource, aFlags);
}

CHtmlElement* CHtmlControl::AppendContentL(const TDesC& aSource, TInt aFlags)
{
	return InsertContentL(Body(), EBeforeEnd, aSource, aFlags);
}

void CHtmlControl::ClearContent()
{
	iImpl->Body()->SetProperty(KHStrInnerHtml, KNullDesC);
}

void CHtmlControl::RefreshAndDraw()
{
	Refresh();
	//DrawNow();
	this->Window().Invalidate(Rect());
}

CHtmlElement* CHtmlControl::Element(const TDesC& aId, TInt aIndex) const
{
	return iImpl->Body()->Element(aId, aIndex);
}

CHtmlElement* CHtmlControl::ElementByTag(const TDesC& aTagName, TInt aIndex) const
{
	return iImpl->Body()->ElementByTag(aTagName, aIndex);
}

CHtmlElement* CHtmlControl::FocusedElement() const
{
	return iImpl->FocusedElement();
}

void CHtmlControl::SetFocusTo(CHtmlElement* aElement)
{	
	iImpl->SetFocusTo((CHtmlElementImpl*)aElement);
}

TInt CHtmlControl::CountComponentControls() const
{
	return iImpl->CountComponentControls();
}

CCoeControl* CHtmlControl::ComponentControl(TInt aIndex) const
{
	return iImpl->ComponentControl(aIndex);
}

void CHtmlControl::RemoveElement(CHtmlElement* aElement)
{
	iImpl->RemoveElement((CHtmlElementImpl*)aElement);
}

void CHtmlControl::AddStyleSheetL(const TDesC& aSource) 
{
	iImpl->AddStyleSheetL(aSource);
}

void CHtmlControl::RemoveStyleClass(const TDesC& aSelectorStr)
{
	iImpl->RemoveStyleClass(aSelectorStr);
}

void CHtmlControl::ClearStyleSheet()
{
	iImpl->ClearStyleSheet();
}

void CHtmlControl::Refresh()
{
	iImpl->Refresh();
}

void CHtmlControl::Draw(const TRect& aRect) const
{
	iImpl->Draw(aRect);
}

TKeyResponse CHtmlControl::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	return iImpl->OfferKeyEventL(aKeyEvent, aType);
}

void CHtmlControl::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	CCoeControl::HandlePointerEventL(aPointerEvent);
	
	iImpl->HandlePointerEventL(aPointerEvent);
}

void CHtmlControl::HandleResourceChange(TInt aType)
{
	if(aType==KUidValueCoeColorSchemeChangeEvent
#ifdef __SYMBIAN_9_ONWARDS__
			|| aType==KUidValueCoeFontChangeEvent
#endif
#ifdef __SERIES60_3_ONWARDS__
			|| aType==KAknsMessageSkinChange || aType==KEikDynamicLayoutVariantSwitch
#endif
	) 
	{
		iImpl->iState.Set(EHCSNeedRedraw);
	}
}

MTransition* CHtmlControl::Transition() const
{
	return static_cast<MTransition*>(iImpl->Transition());
}

const CFbsBitmap* CHtmlControl::OffScreenBitmap() const
{
	return (const CFbsBitmap*)iImpl->OffScreenBitmap();
}

void HtmlCtlLib::AddSearchPathL(const TDesC& aPath)
{
	CHtmlCtlEnv::Static()->AddSearchPathL(aPath);
}

TBool HtmlCtlLib::ResolvePath(TDes& aPath)
{
	return CHtmlCtlEnv::Static()->ResolvePath(aPath);	
}

void HtmlCtlLib::AddGlobalStyleSheetL(const TDesC& aSource)
{
	HtmlParser::ParseStyleSheetL(aSource, CHtmlCtlEnv::Static()->DefaultStyleSheet());
}

void HtmlCtlLib::ReportResourceChange(TInt aType)
{
	if(aType==KUidValueCoeColorSchemeChangeEvent
#ifdef __SYMBIAN_9_ONWARDS__
			|| aType==KUidValueCoeFontChangeEvent
#endif
#ifdef __SERIES60_3_ONWARDS__
			|| aType==KAknsMessageSkinChange || aType==KEikDynamicLayoutVariantSwitch
#endif
	) 
	{
		CHtmlCtlEnv::Static()->ImagePool().RefreshAll(ESystemResourceChanged);
		
#ifdef __SERIES60_3_ONWARDS__
		if(aType!=KEikDynamicLayoutVariantSwitch)
#endif
			CHtmlCtlEnv::Static()->ScrollbarDrawer().HandleResourceChange();
	}
}

void HtmlCtlLib::CreateCustomScrollbarL(const TDesC& aDefinition)
{
	CHtmlCtlEnv::Static()->ScrollbarDrawer().SetCustomImagesL(aDefinition);
}

void HtmlCtlLib::AddObjectFactory(NewHtmlObjectL aFactory)
{
	CHtmlCtlEnv::Static()->AddObjectFactory(aFactory);
}

TBool HtmlCtlLib::RefreshImagePool()
{
	return CHtmlCtlEnv::Static()->ImagePool().RefreshAll(ELocalResourceChanged);
}



