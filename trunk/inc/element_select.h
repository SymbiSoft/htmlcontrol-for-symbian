#ifndef HTMLELEMENTSELECT_H
#define HTMLELEMENTSELECT_H

#include "elementimpl.h"

#ifdef __UIQ__
#include <QikCommandManager.h>
#include <QikCommand.h>
#include <QikMenuPopout.h>
#endif

class CHtmlElementSelect : public CHtmlElementImpl
#ifdef __UIQ__
,MQikCommandHandler
#endif
{
public:
	enum TFlags
	{
		EDisabled = 10
	};
	
	CHtmlElementSelect(CHtmlControl* aOwner);
	virtual ~CHtmlElementSelect();

	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void InvokeL(TRefByValue< const TDesC16 >, ...);
	virtual TBool CanFocus() const;
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);
	virtual void HandleButtonEventL(TInt aButtonEvent);
	
	CDesCArray* TextArray() const;
	CDesCArray* ValueArray() const;

	void PopupListBox();
	void DrawFocus(CFbsBitGc& aGc) const;

#ifdef __UIQ__
	void HandleCommandL(CQikCommand &aCommand);
#endif
	
	//properties
	CDesCArray* iTextArray;
	CDesCArray* iValueArray;
	HBufC* iTitle;
	RHcStyle iStyle;
	RHcStyle iButtonStyle;
	TInt iSelected;
	
private:
	//runtime
#ifdef __UIQ__
	CQikMenuPopout* iPopout;
	RPointerArray<CQikCommand> iCommands;
#endif
};

inline TBool CHtmlElementSelect::CanFocus() const
{
	return !iFlags.IsSet(EDisabled);
}

inline CDesCArray* CHtmlElementSelect::TextArray() const
{
	return iTextArray;
}

inline CDesCArray* CHtmlElementSelect::ValueArray() const
{
	return iValueArray;
}

#endif
