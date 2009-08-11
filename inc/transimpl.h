#ifndef TRANSIMPL_H
#define TRANSIMPL_H

#include <e32base.h>

#include "htmlcontrol.hrh"
#include "transition.h"

class CHtmlElement;
class CHtmlElementImpl;
class CWritableBitmap;

class CTransition : public CActive, public MTransition
{
public:
	static CTransition* NewL();
	virtual ~CTransition();
	
	virtual void Perform(CHtmlElement* aElement,
			TTransitionType aType, 
			TTimeIntervalMicroSeconds32 aDuration, 
			TInt aFrames,
			EasingFunction aEasingFunc=&TEasingLinear::EaseNone);
private:
	CTransition();
	void ConstructL();
	
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();
	
	void DoTransition();
	void DoSlideLeft();
	void DoSlideRight();
	void DoSlideUp();
	void DoSlideDown();
	void DoFade();
	void DoFly();
	void DoScale();
	void DoFlyAndScale();
	
private:
	CHtmlElementImpl* iElement;
	TTransitionType iType;
	TInt iFrames;
	TTimeIntervalMicroSeconds32 iDuration;
	EasingFunction iEasingFunc;
	TInt iCurrentFrame;
	TRect iOriginRect;
	TRect iTargetRect;
	RTimer iTimer;
	TInt iPos;
	CWritableBitmap* iHelperBitmap;
};

#endif
