#ifndef HCSCROLLBAR_H
#define HCSCROLLBAR_H

#include "htmlcontrol.hrh"

#include <coemain.h>
#include <fbs.h>
#ifdef __UIQ__
#include <QUiConfigClient.h>
#endif

#define KScrollStep 35

class CHtmlElementDiv;
class CHcScrollbarDrawer;
class CWritableBitmap;

class CHcScrollbar : public CBase
{
public:
	enum TFlags
	{
		ELoop,
		EDisplayNone,
		EAlwaysVisible,
		EScrollOnSlide,
		EDimmed
	};
	enum TState
	{
		EHidden,
		EShaftPressed,
		EThumbPressed,
		EUpArrowPressed,
		EDownArrowPressed,
	};
	CHcScrollbar(CHtmlElementDiv* aDiv);
	
	TBool GetProperty(const TDesC& aName, TDes& aBuffer);
	TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	void Refresh();
	void Draw(CFbsBitGc& aGc) const;
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	void ScrollBy(TInt aVal);
	
	inline TInt Pos() const;
	inline void SetPos(TInt aVal);
	
	inline TInt Max() const;
	inline TInt RealPos() const;
	
	inline void SetTopPos();
	inline void SetBottomPos();
	TBool RemoveStepPos(TBool aAllowLoop=EFalse);
	TBool AddStepPos(TBool aAllowLoop=EFalse);
	void AddPos(TInt aDelta);
	
	inline TBool IsDimmed();
	inline void SetDimmed(TBool aVal);
	
	inline TBool IsDisplayNone() const;
	inline void SetDisplayNone(TBool aVal);

	inline TBool IsLoop() const;
	inline void SetLoop(TBool aVal);

	inline void SetStep(TInt aVal);
	inline void SetAlwaysVisible(TBool aVal);
	inline TBool IsSrollOnSlide() const;
	
	inline TBool IsVisible() const;

	void AdjustScrollPos();
	
	inline TRect Rect() const;
	inline TInt Width() const;

	//properties
	TInt iStep;
	TBitFlags32 iFlags;

private:
	//runtime
	TInt iPos;
	TInt iRealPos;
	TInt iMax;
	TInt iContentHeight;
	TRect iRect;
	TRect iThumbRect;
	TRect iUpArrowRect;
	TRect iDownArrowRect;
	TBitFlags32 iState;
	TInt iDragStartPos;
	
	CHtmlElementDiv* iDiv;	
	CHcScrollbarDrawer* iDrawer;
	friend class CHcScrollbarDrawer;
};

class CSkinPatch;
class CHcScrollbarDrawer : public CBase
{
public:
	static CHcScrollbarDrawer* NewL();
	virtual ~CHcScrollbarDrawer();
	
	void Draw(CFbsBitGc& aGc, const CHcScrollbar& aScrollbar) const;	
	void HandleResourceChange();
	
	void SetCustomImagesL(const TDesC& aDefinition);
	
private:
	void ConstructL();
	void DrawCustom(CFbsBitGc& aGc, const CHcScrollbar& aScrollbar) const;
	void InitDrawer();
	
private:
	TInt iWidth;
	TInt iArrowHeight;
	TInt iDefaultScrollBarBreadth;
	
#ifdef __UIQ__
	const CSkinPatch* iSkinPatchs[4];
	const CSkinPatch* iSkinBackground;
#endif
	struct TComponentImage
	{
		CHcImage* iImage;
		TRect iScale9Grid;
	};
	TComponentImage iComponents[8];
	TBool iCustomDraw;

	CWritableBitmap* iBgBmp;
	CWritableBitmap* iBgBmpMask;
	CWritableBitmap* iHandleBmp;
	CWritableBitmap* iHandleBmpMask;
	
	friend class CHcScrollbar;
};

inline TInt CHcScrollbar::Pos() const
{
	return iPos;
}
inline TInt CHcScrollbar::Max() const
{
	return iMax;
}
inline void CHcScrollbar::SetPos(TInt aVal)
{
	if(!iFlags.IsSet(EDimmed))
		iPos = aVal;
}
inline TInt CHcScrollbar::RealPos() const
{
	return iRealPos;
}
inline void CHcScrollbar::SetTopPos()
{
	iPos = 0;
}
inline void CHcScrollbar::SetBottomPos()
{
	if(!iFlags.IsSet(EDimmed))
		iPos = iMax;
}

inline void CHcScrollbar::SetStep(TInt aVal)
{
	iStep = aVal;
}

inline void CHcScrollbar::SetDimmed(TBool aVal)
{
	iFlags.Assign(EDimmed, aVal);
}
inline void CHcScrollbar::SetAlwaysVisible(TBool aVal)
{
	iFlags.Assign(EAlwaysVisible, aVal);
}
inline void CHcScrollbar::SetDisplayNone(TBool aVal)
{
	iFlags.Assign(EDisplayNone, aVal);
}
inline void CHcScrollbar::SetLoop(TBool aVal)
{
	iFlags.Assign(ELoop, aVal);
}
inline TBool CHcScrollbar::IsDimmed()
{
	return iFlags.IsSet(EDimmed);
}
inline TBool CHcScrollbar::IsDisplayNone() const
{
	return iFlags.IsSet(EDisplayNone);
}

inline TBool CHcScrollbar::IsLoop() const
{
	return iFlags.IsSet(ELoop);
}

inline TBool CHcScrollbar::IsSrollOnSlide() const
{
	return iFlags.IsSet(EScrollOnSlide);
}

inline TBool CHcScrollbar::IsVisible() const
{
	return !iState.IsSet(EHidden);
}

inline TInt CHcScrollbar::Width() const
{
	return iDrawer->iWidth;
}

inline TRect CHcScrollbar::Rect() const
{
	return iRect;
}

#endif

