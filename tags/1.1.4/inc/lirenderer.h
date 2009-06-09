/*
* ============================================================================
*  Name     : MListItemRenderer
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef LISTITEMRENDERER_H
#define LISTITEMRENDERER_H

class CHtmlElement;

class MListItemRenderer
{
public:
	virtual void RenderListItemL(CHtmlElement* aElement, TInt aIndex) = 0;
};

#endif
