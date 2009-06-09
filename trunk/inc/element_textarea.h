#ifndef HTMLELEMENTTEXTAREA_H
#define HTMLELEMENTTEXTAREA_H

#include <eikedwin.h> 

#include "elementimpl.h"

class CHcInputEditor;

class CHtmlElementTextArea : public CHtmlElementImpl
#ifdef __SYMBIAN_9_ONWARDS__
, MCoeControlBackground
#endif
{
public:
	enum TFlags
	{
		EDisabled = 10,
		EDrawLines,
		EReadOnly,
		EDirectFocus
	};
	
	CHtmlElementTextArea(CHtmlControl* aOwner);
	virtual ~CHtmlElementTextArea();
	
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
	virtual void HandleButtonEventL(TInt aButtonEvent);
	
	void SetEditorTextL(const TDesC& aText);
	void Draw(CWindowGc& aGc, const CCoeControl& aControl, const TRect& aRect) const;
	void DrawFocus(CFbsBitGc& aGc) const;

	//properties
	HBufC* iText;
	HBufC* iTitle;
	RHcStyle iStyle;
	TInt iRows;
	THcColor iLineColor;
};

inline TBool CHtmlElementTextArea::CanFocus() const
{
	return !iFlags.IsSet(EDisabled);
}

#endif
