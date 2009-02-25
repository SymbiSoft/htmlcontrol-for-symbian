/*
* ============================================================================
*  Name     : CSlideTransition
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom
* ============================================================================
*/

#ifndef SLIDETRANS_H
#define SLIDETRANS_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib

class CFbsBitmap;
class CWritableBitmap;
class CHtmlControl;

class CSlideTransition : public CActive
{
public:
	enum TMode
	{
		ESlideLeft,
		ESlideRight,
		ESlideUp,
		ESlideDown,
	};
	// Cancel and destroy
	~CSlideTransition();

	// Two-phased constructor.
	static CSlideTransition* NewL(CHtmlControl* aControl);

public:
	// New functions
	// Function for making the initial request
	void StartL(TMode aMode);
	
	/**
	 * @param aStep pixel count to move every step.
	 * @param aInterval interval between every step.
	 */
	void SetParams(TInt aStep, TTimeIntervalMicroSeconds32 aInterval);

private:
	// C++ constructor
	CSlideTransition(CHtmlControl* aControl);

	// Second-phase constructor
	void ConstructL();

	void Draw();
private:
	// From CActive
	// Handle completion
	void RunL();

	// How to cancel me
	void DoCancel();

	// Override to handle leaves from RunL(). Default implementation causes
	// the active scheduler to panic.
	TInt RunError(TInt aError);

private:
	CHtmlControl* iControl;
	RTimer iTimer; // Provides async timing service
	TTimeIntervalMicroSeconds32 iInterval;
	TMode iMode;
	TInt iStep;
	CWritableBitmap* iOldScreen;
	const CFbsBitmap* iNewScreen;
	TInt iPos;
	TInt iContentVersion;
};

#endif // SLIDETRANS_H
