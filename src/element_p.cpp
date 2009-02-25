#include "controlimpl.h"
#include "measurestatus.h"

#include "element_p.h"

CHtmlElementP::CHtmlElementP(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeP;
	iTagName.Set(KHStrP);
}

CHtmlElementP::~CHtmlElementP()
{
	
}

CHtmlElementImpl* CHtmlElementP::CloneL()  const
{
	CHtmlElementP* e = new (ELeave)CHtmlElementP(iOwner);
	BaseClone(*e);
	
	e->iAlign = iAlign;
	return e;
}

void CHtmlElementP::Measure(CHcMeasureStatus& aStatus)
{
	aStatus.NewLine();
	aStatus.PushLineAlign(iAlign);
}

void CHtmlElementP::Draw(CFbsBitGc&) const
{
	
}

_LIT(KHStrPEnd, "p-end");
CHtmlElementPEnd::CHtmlElementPEnd(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypePEnd;
	iTagName.Set(KHStrPEnd);
}

CHtmlElementImpl* CHtmlElementPEnd::CloneL() const
{
	CHtmlElementPEnd* e = new (ELeave)CHtmlElementPEnd(iOwner);
	BaseClone(*e);
	return e;
}

void CHtmlElementPEnd::Measure(CHcMeasureStatus& aStatus) 
{
	aStatus.NewLine(ETrue);
	aStatus.PopLineAlign();
}

void CHtmlElementPEnd::Draw(CFbsBitGc& )  const
{
}
