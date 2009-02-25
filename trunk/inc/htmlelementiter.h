/*
* ============================================================================
*  Name     : THtmlElementIter
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLELEMENTITER_H
#define HTMLELEMENTITER_H

#include "htmlelement.h"

class THtmlElementIter
{
public:
	THtmlElementIter(const CHtmlElement* aParent, const TDesC& aId=KNullDesC, const TDesC& aTagName=KNullDesC);
	
	TBool Next();
	CHtmlElement* operator()() const;
	void Remove();

private:
	const CHtmlElement* iParent;
	CHtmlElement* iCurrent;
	TBuf<KMaxElementIdLength> iId;
	TBuf<KMaxTagNameLength> iTagName;
};

#endif
