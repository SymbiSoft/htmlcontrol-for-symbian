#ifndef HTMLELEMENTHR_H
#define HTMLELEMENTHR_H

#include "elementimpl.h"

class CHtmlElementHr : public CHtmlElementImpl
{
public:
	CHtmlElementHr(CHtmlControl* aOwner);

	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
	
	//properties
	THcLength iWidth;
	THcLength iHeight;
	THcColor iColor;
	TAlign iAlign;
};

#endif
