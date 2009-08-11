#include "htmlcontrol.hrh"
#include "htmlcontrol.h"
#include "htmlelement.h"
#include "htmlelementiter.h"

#include "element_div.h"

THtmlElementIter::THtmlElementIter(const CHtmlElement* aParent, const TDesC& aId, const TDesC& aTagName)
{
	iParent = (CHtmlElementImpl*)aParent;
	iId.Copy(aId);
	iTagName.Copy(aTagName);
	
	if(((CHtmlElementImpl*)iParent)->TypeId()==EElementTypeDiv)
		iCurrent = (CHtmlElementImpl*)iParent;
	else
		iCurrent = NULL;
}

TBool THtmlElementIter::Next()
{
	if(!iCurrent)
		return EFalse;

	iCurrent = ((CHtmlElementImpl*)iCurrent)->iNext;
	
	while(iCurrent!=((CHtmlElementDiv*)iParent)->iEnd)
	{
		if((iId.Length()==0 || iCurrent->Id().CompareF(iId)==0)
				&& (iTagName.Length()==0 || iTagName.CompareF(iCurrent->TagName())==0)
			)
			return ETrue;

		iCurrent = ((CHtmlElementImpl*)iCurrent)->iNext;
	}
	
	return EFalse;
}

CHtmlElement* THtmlElementIter::operator()() const
{
	return iCurrent;
}

void THtmlElementIter::Remove()
{
	CHtmlElement* toRemove = iCurrent;
	iCurrent = ((CHtmlElementImpl*)iCurrent)->iPrev;
	iParent->Owner()->RemoveElement(toRemove);
}
