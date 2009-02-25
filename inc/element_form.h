#ifndef HTMLELEMENTFORM_H
#define HTMLELEMENTFORM_H

#include "elementimpl.h"

class CHtmlElementFormEnd;

class CHtmlElementForm : public CHtmlElementImpl
{
public:
	CHtmlElementForm(CHtmlControl* aOwner);
	virtual ~CHtmlElementForm();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	
	void Layout(){}
	void EndMeasure(CHcMeasureStatus& aStatus);
	
	//properties
	HBufC* iAction;
	TBuf<10> iMethod;
	HBufC* iEncodeType;
	RHcStyle iStyle;
	CHtmlElementFormEnd* iEnd;
};

class CHtmlElementFormEnd : public CHtmlElementImpl
{
public:
	CHtmlElementFormEnd(CHtmlControl* aOwner);

	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
	
	CHtmlElementForm* iStart;
};

#endif
