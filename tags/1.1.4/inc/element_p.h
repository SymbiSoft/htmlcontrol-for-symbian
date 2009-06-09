#ifndef HTMLELEMENTP_H
#define HTMLELEMENTP_H

#include "elementimpl.h"

class CHcMeasureStatus;
class CHtmlElementPEnd;

class CHtmlElementP : public CHtmlElementImpl
{
public:	
	CHtmlElementP(CHtmlControl* aOwner);
	virtual ~CHtmlElementP();

	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
	
	//properties
	TAlign iAlign;
	CHtmlElementPEnd* iEnd;
};

class CHtmlElementPEnd : public CHtmlElementImpl
{
public:
	CHtmlElementPEnd(CHtmlControl* aOwner);
	
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	void Layout(){}
};


#endif
