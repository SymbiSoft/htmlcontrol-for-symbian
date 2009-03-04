#ifndef HTMLELEMENTA_H
#define HTMLELEMENTA_H

#include "elementimpl.h"

class CHcMeasureStatus;
class CHtmlElementAEnd;

class CHtmlElementA : public CHtmlElementImpl
{
public:	
	CHtmlElementA(CHtmlControl* aOwner);
	virtual ~CHtmlElementA();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);
	virtual void HandleButtonEventL(TInt aButtonEvent);
	virtual inline TBool CanFocus() const;
	virtual CHtmlElementImpl* CloneL() const;
	
	void Layout(){}
	void ClearContent();
	
	//properties
	HBufC* iHref;
	HBufC* iTarget;
	RHcStyle iStyle;
	CHtmlElementAEnd* iEnd;
};

class CHtmlElementAEnd : public CHtmlElementImpl
{
public:
	CHtmlElementAEnd(CHtmlControl* aOwner);
	
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
	CHtmlElementA* iStart;
};

inline TBool CHtmlElementA::CanFocus() const
{
	return iNext->TypeId()!=EElementTypeAEnd && iHref && iHref->Length()>0;
}

#endif
