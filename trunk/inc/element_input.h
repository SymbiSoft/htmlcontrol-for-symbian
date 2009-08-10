#ifndef HTMLELEMENTINPUT_H
#define HTMLELEMENTINPUT_H

#include "elementimpl.h"

class CHtmlElementInput : public CHtmlElementImpl
{
public:
	enum TFlags
	{
		EChecked = 10,
		EDisabled,
		EReadOnly,
		EDirectFocus
	};
	
	enum TInputType 
	{
		EText,
		ECheckBox,
		ERadio,
		EPassword,
		EButton,
		EHidden,
		ESubmit
	};
	
	CHtmlElementInput(CHtmlControl* aOwner);
	virtual ~CHtmlElementInput();

	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void PrepareL();
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	inline virtual TBool CanFocus() const;
	virtual void SetFocus(TBool aFocus);
	virtual TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent, TEventCode aType);
	virtual void HandleButtonEventL(TInt aButtonEvent);

	inline TInputType InputType() const;
	inline TBool IsChecked() const;
	
	void MeasureTextEditor(CHcMeasureStatus& aStatus);
	void MeasureCheckBox(CHcMeasureStatus& aStatus);
	void MeasureRadio(CHcMeasureStatus& aStatus);
	void MeasureButton(CHcMeasureStatus& aStatus);
	
	void DrawTextEditor(CFbsBitGc& aGc) const;
	void DrawCheckBox(CFbsBitGc& aGc) const;
	void DrawRadio(CFbsBitGc& aGc) const;
	void DrawButton(CFbsBitGc& aGc) const;
	
	void DrawFocus(CFbsBitGc& aGc) const;
	
	void SetEditorTextL(const TDesC& aText);
	void PopupEditorL();
	
	//properties
	HBufC* iText;
	HBufC* iTitle;
	TInputType iInputType;
	TInt iInputMode;
	TInt iMaxLength;
	RHcStyle iStyle;
};

inline TBool CHtmlElementInput::CanFocus() const
{
	return !iFlags.IsSet(EDisabled) && iInputType!=EHidden;
}

CHtmlElementInput::TInputType CHtmlElementInput::InputType() const
{
	return iInputType;
}

inline TBool CHtmlElementInput::IsChecked() const
{
	return iFlags.IsSet(EChecked);
}

#endif
