#ifndef HTMLELEMENTDIV_H
#define HTMLELEMENTDIV_H

#include "elementimpl.h"

class CHtmlElementDiv;
class CHtmlElementDivEnd;
class CHcScrollbar;
class CHcList;

class CHtmlElementDiv : public CHtmlElementImpl
{
public:
	enum TFlags
	{
		EFocusing = 10,
		EDisabled,
		EBody
	};
	
	CHtmlElementDiv(CHtmlControl* aOwner);
	virtual ~CHtmlElementDiv();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual TBool CanFocus() const;
	virtual void SetFocus(TBool aFocus);
	virtual void InvokeL(TRefByValue< const TDesC16 >, ...);
	virtual void PrepareL();
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Layout();
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);
	virtual void HandleButtonEventL(TInt aButtonEvent);
	
	inline TBool IsContainer() const;
	inline THcLineWrap LineWrapMode() const;
	
	void ClearContent();
	void UpdateScrollbarsL();

	void EndMeasure(CHcMeasureStatus& aStatus);
	void EndDraw(CFbsBitGc& aGc) const;

	void FocusChangingTo(CHtmlElementImpl* aElement);
	void SetFirstFocus();
	void SetLastFocus();
	
	//properties
	RHcStyle iStyle;
	RHcLightStyle iFocusStyle;
	RHcLightStyle iDownStyle;
	THcLineWrap iLineWrapMode;
	
	CHtmlElementImpl* iFocusedElement;
	CHcScrollbar* iScrollbar;
	CHcList* iList;
	CHtmlElementDivEnd* iEnd;

	//runtime
	TRect iDisplayRect;
	TSize iContentSize;
	TRect iClippingRect;
	TMargins iMargins;
	TMargins iPaddings;
};

class CHtmlElementDivEnd : public CHtmlElementImpl
{
public:
	CHtmlElementDivEnd(CHtmlControl* aOwner);

	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
};

inline TBool CHtmlElementDiv::CanFocus() const
{
	return (iFlags.IsSet(EFocusing) || IsContainer()) && !iFlags.IsSet(EDisabled); 
}

inline TBool CHtmlElementDiv::IsContainer() const
{
	return iScrollbar!=NULL;
}

inline THcLineWrap CHtmlElementDiv::LineWrapMode() const
{
	return iLineWrapMode;
}

#endif
