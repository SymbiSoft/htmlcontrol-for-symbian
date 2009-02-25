/*
* ============================================================================
*  Name     : CFadeTransition
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef FADETRANS_H
#define FADETRANS_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib

class CFbsBitmap;
class CWritableBitmap;
class CHtmlControl;

class CFadeTransition : public CActive
{
public:
	// Cancel and destroy
	~CFadeTransition();

	// Two-phased constructor.
	static CFadeTransition* NewL(CHtmlControl* aControl);

public:
	// New functions
	// Function for making the initial request
	void StartL();
	
	/**
	 * @param alpha value increasing by every step. Alpha value is range from 0 to 255.
	 * @param aInterval interval between every step.
	 */
	void SetParams(TInt aStep, TTimeIntervalMicroSeconds32 aInterval);

private:
	// C++ constructor
	CFadeTransition(CHtmlControl* aControl);

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
	TInt iStep;
	CWritableBitmap* iMask;
	CWritableBitmap* iOldScreen;
	const CFbsBitmap* iNewScreen;
	TInt iOpacity;
	TInt iContentVersion;

};

#endif // CSLIDETRANSITION_H
