#ifndef HTMLCONTROLIMPL_H
#define HTMLCONTROLIMPL_H

#include <coecntrl.h>
#include <coemain.h>
#include <babitflags.h>

#include "htmlcontrol.hrh"
#include "image.h"
#include "htmlcontrol.h"
#include "measurestatus.h"

//Forward declaration
class CHtmlCtlEnv;
class CHtmlElementImpl;
class CHtmlElementDiv;
class CHtmlElementBody;
class CHcScrollbar;
class CWritableBitmap;
class CTransition;

enum THtmlControlState
{
	EHCSAutoFocusDisabled,
	EHCSInTransition,
	EHCSFocusValid,
	EHCSSlideOccured,
	EHCSNeedRefresh,
	EHCSNeedRedraw,
	EHCSExiting,
	EHCSNavKeyPrev,
	EHCSNavKeyNext
};

class CHtmlControlImpl : public CBase, public MImageLoadedEventSubscriber
{
public:
	static CHtmlControlImpl* NewL(CHtmlControl* aControl);
	virtual ~CHtmlControlImpl();
	void CreateBodyL();	
	
	inline CHtmlElementBody* Body() const;
	inline CWritableBitmap* OffScreenBitmap() const;
	inline void SetOffScreenBitmap(CWritableBitmap* aBitmap);
	inline CHtmlCtlEnv* Env() const;
	inline CTransition* Transition() const;
	
	void Refresh(TBool aInTransition=EFalse);
	void DrawOffscreen();
	void Draw(const TRect& aRect);

	CHtmlElementImpl* FocusedElement() const;
	void SetFocusTo(CHtmlElementImpl* aElement);
	void ScrollToView(CHtmlElementImpl* aElement);
	
	CHtmlElementImpl* InsertContentL(CHtmlElementImpl* aTarget, TInsertPosition aPosition, const TDesC& aSource, TInt aFlags);
	void InsertContent(CHtmlElementImpl* aHead, CHtmlElementImpl* aTail, CHtmlElementImpl* aTarget, TInsertPosition aPosition);
	CHtmlElementImpl* ParseL(CHtmlElementDiv* aParent, const TDesC& aSource, TInt aFlags);
	void RemoveElement(CHtmlElementImpl* aElement);
	void FreeElementList(CHtmlElementImpl* aStart, CHtmlElementImpl* aEnd);
	CHtmlElementDiv* FindContainer(CHtmlElementImpl* aElement) const;
	CHtmlElementImpl* FindLink(CHtmlElementImpl* aElement) const;
	void MoveElementL(CHtmlElementImpl* aElement, CHtmlElementImpl* aTarget, TInsertPosition aPosition);
	CHtmlElementImpl* CopyElementListL(CHtmlElementImpl* aHead, CHtmlElementImpl* aTail, CHtmlElementImpl* aTarget, TInsertPosition aPosition);
	
	void RegisterControl(CCoeControl* aControl);
	void UnregisterControl(CCoeControl* aControl);
	
	void ScrollText(TInt aTextWidth, const TRect& aBox);
	inline TInt TextScrollPos() const;
	void MeasureTextContinued();
	
	inline const THcLineInfo& LineInfo(TInt aLineNumber) const;
	inline RArray<THcLineInfo>& Lines();

	void AddStyleSheetL(const TDesC& aSource);
	void RemoveStyleClass(const TDesC& aSelector);
	void ClearStyleSheet();
	TUint32 StyleSheetVersion();
	TBool GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute, CHcStyle& aStyle) const;
	const CHcStyle& GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute) const;
	CFont* GetFont(const THcTextStyle& aStyle);

	inline void SetEventObserver(MHtmlCtlEventObserver* aObserver);
	void FireEventL(const THtmlCtlEvent& aEvent);
	
	TBitFlags32 iState;
public: //callbacks
	void OnTimerL(TInt aError, TInt aIndex);	//call by timers
	void NotifyImageLoaded(CHcImage* aImage); //call by imagepool
	void NotifyScrollPosChanged(); //call by scrollbar
	
public:
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl(TInt aIndex) const;
	TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType); 
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	
private:
	class TTextScrollInfo
	{
	public:
		enum TState
		{
			EStarting,
			EScrolling,
			EEnding
		};
		TState iState;
		TInt iScrollPos;
		TInt iTextWidth;
		TRect iBox;
	};
	
	enum TTapArea
	{
		ETapAreaMain,
		ETapAreaList,
		ETapAreaScrollbar
	};
	
	struct TTapInfo
	{
		CHtmlElementDiv* iContainer;
		CHtmlElementImpl* iStartElement;
		TTapArea iStartArea;
		TPoint iStartPoint;
		TPoint iPoint;
		TBool iNewlyGainFocus;
	};
	
	struct THcFontCacheItem
	{
		CFont* iFont;
		TInt iMatchCode;
	};
	
	CHtmlControlImpl(CHtmlControl* aControl);
	void ConstructL();
	
	//key navigation handling
	TBool VisibilityTest(CHtmlElementImpl* aElement, const TRect& aDisplayRect);
	void UpdateVFEs(CHtmlElementDiv* aContainer);
	void SkipLeftSameLine(TInt& aIndex);
	void SkipRightSameLine(TInt& aIndex);
	CHtmlElementImpl* FindAboveElement(TInt aToIndex, TInt aX);
	CHtmlElementImpl* FindBelowElement(TInt aFromIndex, TInt aX);
	void ResolveFocus(CHtmlElementDiv* aContainer);
	void HandleKeyLeft(CHtmlElementDiv* aContainer, TBool aScrolled=EFalse);
	void HandleKeyRight(CHtmlElementDiv* aContainer, TBool aScrolled=EFalse);
	void HandleKeyUp(CHtmlElementDiv* aContainer, TBool aScrolled=EFalse);
	void HandleKeyDown(CHtmlElementDiv* aContainer, TBool aScrolled=EFalse);
	TKeyResponse OfferKeyEventL2 (CHtmlElementDiv* aContainer, const TKeyEvent &aKeyEvent, TEventCode aType);
	
	//pointer events handling
	TBool HitTest(CHtmlElementImpl* aElement, const TPoint& aPoint) const;
	TBool HitTest(CHcScrollbar* aScrollbar, const TPoint& aPoint) const;
	void GetElementByPosition(CHtmlElementDiv* aTopContainer, const TPoint& aPoint, CHtmlElementImpl*& aElement, CHtmlElementDiv*& aContainer, TTapArea& aArea);
	void HandlePointerEventL2(const TPointerEvent& aPointerEvent);

	//scrolling
	void QuickScroll(CHtmlElementDiv* aContainer);
	void DoScrollText();
	
	//transition
	void DrawTransition();
	
private:	
	CHtmlControl* iControl;
	CHtmlElementBody* iBody;
	CHcStyleSheet* iStyleSheet;
	CHcStyle* iTempUsageStyle;
	CWritableBitmap* iOffScreenBitmap;
	RPointerArray<CCoeControl> iControls;
	RArray<THcLineInfo> iLines;
	TTextScrollInfo iTextScrollInfo;
	TTapInfo iTapInfo;
	RPointerArray<CHtmlElementImpl> iVFEs;
	RArray<THcFontCacheItem> iFontCache;
	CTimer* iDelayRefreshTimer;
	CTimer* iTextScrollTimer;
	CHtmlCtlEnv* iEnv;
	MHtmlCtlEventObserver* iObserver;
	CTransition* iTransition;
};

inline CHtmlElementBody* CHtmlControlImpl::Body() const
{
	return iBody;
}

inline TInt CHtmlControlImpl::TextScrollPos() const
{
	return iTextScrollInfo.iScrollPos;
}

inline const THcLineInfo& CHtmlControlImpl::LineInfo(TInt aLineNumber) const
{
	return iLines[aLineNumber];
}

inline RArray<THcLineInfo>& CHtmlControlImpl::Lines()
{
	return iLines;
}

inline CWritableBitmap* CHtmlControlImpl::OffScreenBitmap() const
{
	return iOffScreenBitmap;
}

inline void CHtmlControlImpl::SetOffScreenBitmap(CWritableBitmap* aBitmap)
{
	iOffScreenBitmap = aBitmap;
}

inline CHtmlCtlEnv* CHtmlControlImpl::Env() const
{
	return iEnv;
}

inline void CHtmlControlImpl::SetEventObserver(MHtmlCtlEventObserver* aObserver)
{
	iObserver = aObserver;
}

inline CTransition* CHtmlControlImpl::Transition() const
{
	return iTransition;
}

#endif
