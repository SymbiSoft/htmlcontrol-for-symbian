/*
* ============================================================================
*  Name     : THtmlCtlEvent
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLCTLEVENT_H
#define HTMLCTLEVENT_H

//Forward declaration
class CHtmlControl;
class CHtmlElement;

class THtmlCtlEvent
{
public:
	enum TEventType
	{
		EOnClick,
		EOnChanged,
		EOnSubmit,
		EOnGainFocus,
		EOnLostFocus,
		EOnSlideLeft,
		EOnSlideRight
	};
	
	TEventType iType;
	CHtmlControl* iControl;
	CHtmlElement* iElement;
	TInt iData;
	
	inline THtmlCtlEvent();
};

class MHtmlCtlEventObserver
{
public:
	virtual void HandleHtmlCtlEventL(const THtmlCtlEvent& aEvent) = 0;
};

inline THtmlCtlEvent::THtmlCtlEvent()
{
	iType = EOnClick;
	iControl = NULL;
	iElement = NULL;
	iData = 0;
}

#endif

