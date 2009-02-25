/*
* ============================================================================
*  Name     : CWritableBitmap
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef WRITABLEBITMAP_H
#define WRITABLEBITMAP_H

#include <fbs.h>

class CFbsBitGc;
class CFbsBitmapDevice;

class CWritableBitmap : public CFbsBitmap
{
public:
	static CWritableBitmap* NewL(const TSize& aSizeInPixels, TDisplayMode aDispMode);
	virtual ~CWritableBitmap();

	inline CFbsBitGc& Gc();

	void ResizeWithGcL(const TSize& aSizeInPixels);
	
protected:
	CWritableBitmap();
	virtual void ConstructL(const TSize& aSizeInPixels, TDisplayMode aDispMode);
	
private:
	CFbsBitmapDevice* iDevice;
	CFbsBitGc* iGc;
};

//-----------------------------------------------
inline CFbsBitGc& CWritableBitmap::Gc()
{
	return *iGc;
}

#endif
