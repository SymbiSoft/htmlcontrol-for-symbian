#ifndef HTMLELEMENTOBJECT_H
#define HTMLELEMENTOBJECT_H

#include "elementimpl.h"

class CHtmlElementText;

class CHtmlElementObject : public CHtmlElementImpl
{
public:
	CHtmlElementObject(CHtmlControl* aOwner);
	virtual ~CHtmlElementObject();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void PrepareL();
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	inline virtual TBool CanFocus() const;
	virtual void SetFocus(TBool aFocus);
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);

	void AppendAlterTextL(HBufC* aText);
	
	//properties
	RHcStyle iStyle;
	CDesCArray* iParamKeys;
	CDesCArray* iParamValues;
	CHtmlElementText* iAlterText;
};

inline TBool CHtmlElementObject::CanFocus() const
{
	return iEmbedObject && !iEmbedObject->IsDimmed() && iEmbedObject->IsVisible() && !iEmbedObject->IsNonFocusing();
}

#endif /*ELEMENT_OBJECT_H_*/
