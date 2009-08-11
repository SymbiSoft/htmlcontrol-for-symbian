#ifndef HCLIST_H
#define HCLIST_H

#include <coemain.h>
#include <coedef.h>

#include "htmlcontrol.hrh"
#include "htmlctlenv.h"
#include "lirenderer.h"
#include "imagepool.h"

enum THcListLayout
{
	ELayoutSlides,
	ELayoutList,
	ELayoutGrid
};

class CHtmlElementDiv;
class CHtmlElementImpl;
class CHcMeasureStatus;

class CHcList : public CBase
{
public:
	static CHcList* NewL(CHtmlElementDiv* aDiv, THcListLayout aLayout);
	~CHcList();

	TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	void Measure(CHcMeasureStatus& aStatus);
	void DoCreateL();
	void UpdateL(TInt aIndex);
	TInt Height() const;
	TBool IsEmpty() const;
	TInt ListIndexOf(const CHtmlElementImpl* aElement) const;
	TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType);
	void NotifyScrollPosChanged();
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	
	void SetRenderer(MListItemRenderer* aRenderer);
	void OnTimerL(TInt aError, TInt aIndex);
	void NotifyChildSetImage(CHcImage* aImage);
	
	void ScrollTop();
	void ScrollBottom();
private:
	CHcList(CHtmlElementDiv* aDiv, THcListLayout aLayout);
	void ConstructL();
	
	void HandleUp();
	void HandleDown();
	void HandleLeft();
	void HandleRight();
		
	void MoveItemsUp();
	void MoveItemsDown();
	void MoveItemsTop();
	void MoveItemsBottom();
	
	void FreeTemplate();
	void ClearContainer();
	
	void BuildItemsL();

	enum TBodyStatus
	{
		ENotCreate,
		EItems,
		EEmpty
	};
	
	enum TAutoScrollingDirection
	{
		EAutoScrollingNone,
		EAutoScrollingUp,
		EAutoScrollingDown
	};
	
	THcListLayout iLayout;
	TInt iItemCount;
	TInt iSelectedIndex;
	TInt iBodyStatus;
	MListItemRenderer* iRenderer;
	
	TInt iViewRowCount;
	TInt iViewColCount;
	TInt iItemHeight;
	TInt iTopItem;
	RPointerArray<CHtmlElementDiv> iItems;
	TSize iContainerSize;
	TPoint iTapPoint;
	
	CHtmlElementDiv* iTemplate;
	CHtmlElementImpl* iEmptyBody;
	CHtmlElementDiv* iDiv;
	
	CTimer* iTimer;
	TInt iAutoScrollingDirection;
	
	RPointerArray<CHcImage> iCachedImages;
};

#endif


