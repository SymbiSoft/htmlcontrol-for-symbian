#ifndef ELEMENTIMPL_H
#define ELEMENTIMPL_H

#include <coecntrl.h>
#include <babitflags.h>

#include "rstyle.h"

//Forward declaration
class CFbsBitGc;
class CHtmlControl;
class CHtmlElementDiv;
class CHcMeasureStatus;

enum THtmlElementTypeId 
{
	EElementTypeA,
	EElementTypeAEnd,
	EElementTypeDiv,
	EElementTypeDivEnd,
	EElementTypeForm,
	EElementTypeFormEnd,
	EElementTypeHr,
	EElementTypeImg,
	EElementTypeInput,
	EElementTypeObject,
	EElementTypeP,
	EElementTypePEnd,
	EElementTypeSelect,
	EElementTypeText,
	EElementTypeTextArea
};

enum THtmlElementState
{
	EElementStateRuntime,
	EElementStateFocused,
	EElementStateInFocus,
	EElementStateFocusChanged,
	EElementStateHidden,
	EElementStateFaded,
	EElementStateInLink,
	EElementStateError,
	EElementStateStatic,
	EElementStateFixedX,
	EElementStateFixedY,
	EElementStateDown,
};

enum THtmlButtonEvent
{
	EButtonEventDown,
	EButtonEventClick,
	EButtonEventUp
};

class CHtmlElementImpl : public CHtmlElement
{
public:
	inline TInt TypeId() const;
	virtual TBool CanFocus() const;
	virtual void SetFocus(TBool aFocus);
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const ;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void PrepareL();
	virtual void Measure(CHcMeasureStatus& aStatus) = 0;
	virtual void Layout();
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const = 0;
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);
	virtual void HandleButtonEventL(TInt aButtonEvent);
	virtual CHtmlElementImpl* CloneL() const = 0;

	virtual TRect ViewRect() const;
	inline TRect Rect() const;
	inline TBool IsFocused() const;

	//runtime
	TPoint iPosition;
	TSize iSize;
	TBitFlags32 iState;
	TInt iLineNumber;
	TInt iLineHeight;
	
	//properties
	CHtmlElementImpl* iNext;
	CHtmlElementImpl* iPrev;
	CHtmlElementDiv* iParent;

protected:
	CHtmlElementImpl(CHtmlControl* aOwner);
	void BaseClone(CHtmlElementImpl& aTarget) const;

	void DoMeasure(CHcMeasureStatus& aStatus, const CHcStyle& aStyle, const TSize& aDefaultSize, TBool aForceSize=EFalse);

	//properties
	TInt iTypeId;
	TVAlign iLineVAlign;
};

//--inline functions---
inline TInt CHtmlElementImpl::TypeId() const
{
	return iTypeId;
}

inline TRect CHtmlElementImpl::Rect() const
{
	return TRect(iPosition, iSize);
}

inline TBool CHtmlElementImpl::IsFocused() const
{
	return iState.IsSet(EElementStateFocused);
}

#endif
