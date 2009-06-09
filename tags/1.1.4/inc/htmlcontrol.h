/*
* ============================================================================
*  Name     : CHtmlControl
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLCONTROL_H
#define HTMLCONTROL_H

#include <coecntrl.h>
#include <coemain.h>

#include "htmlstrings.h"
#include "htmlelement.h"
#include "htmlctlevent.h"

//Forward declaration
class CHtmlControlImpl;
class MTransition;

#define KParserFlagsIgnoreFontSize 		0x0001
#define KParserFlagsRemoveRedundantBr	0x0002
#define KParserFlagsIgnoreLayout	 	0x0004

enum TInsertPosition
{
	EBeforeBegin,
	EAfterBegin,
	EBeforeEnd,
	EAfterEnd
};

class CHtmlControl : public CCoeControl
{
public:
	static CHtmlControl* NewL(CCoeControl* aParent);
	
	virtual ~CHtmlControl();
	
	/**
	 * Insert HTML content to the document.
	 * @param aTarget position related element.
	 * @param aPosition see TInsertPosition.
	 * @param aSource HTML text.
	 * @param aFlags see KParserFlagsXXXX.
	 */
	CHtmlElement* InsertContentL(CHtmlElement* aTarget, TInsertPosition aPosition, const TDesC& aSource, TInt aFlags = 0);
	
	/**
	 * Append HTML content to the end of document.
	 * @param aSource HTML text.
	 * @param aFlags see KParserFlagsXXXX.
	 * @return
	 */
	CHtmlElement* AppendContentL(const TDesC& aSource, TInt aFlags = 0);
	
	/**
	 * Refresh the control layout. Must call after change the content or set element properties.
	 */
	void Refresh();
	
	/**
	 * Refresh() + DrawNow()
	 */
	void RefreshAndDraw();
	
	/**
	 * Clear all content of the document.
	 */
	void ClearContent();
	
	/**
	 * Remove a specific element from the document.
	 * @param aElement
	 */
	void RemoveElement(CHtmlElement* aElement);
	
	/**
	 * Get element by id.
	 * @param aId element id.
	 * @param aIndex the index of the element collection with the id.
	 * @return element pointer. Don't delete it.
	 */
	CHtmlElement* Element(const TDesC& aId, TInt aIndex = 0) const;
	
	/**
	 * Get element by tag name.
	 * @param aTagName tag name.
	 * @param aIndex the index of the element collection with the tag name.
	 * @return element pointer. Don't delete it.
	 */
	CHtmlElement* ElementByTag(const TDesC& aTagName, TInt aIndex = 0) const;
	
	/**
	 * Get current focused element.
	 * @return element pointer. Don't delete it.
	 */
	CHtmlElement* FocusedElement() const;
	
	/**
	 * Set focus to a element.
	 * @param aElement element pointer.
	 */
	void SetFocusTo(CHtmlElement* aElement);
	
	/**
	 * Scroll a elememnt into view.
	 * @param aElement element pointer.
	 */
	void ScrollToView(CHtmlElement* aElement);
	
	/**
	 * Add stylesheet to the document.
	 * @param aSource CSS text. 
	 */
	void AddStyleSheetL(const TDesC& aSource);
	
	/**
	 * Remove a style class.
	 * @param aSelector CSS selector.
	 */
	void RemoveStyleClass(const TDesC& aSelector);
	
	/**
	 * Clear stylesheet.
	 */
	void ClearStyleSheet();
	
	/**
	 * Get body element pointer. It is the top level element of the document.
	 * @return
	 */
	CHtmlElement* Body() const;

	/**
	 * Set event observer
	 * @param aObserver
	 */
	void SetEventObserver(MHtmlCtlEventObserver* aObserver);
	
	/**
	 * Get offscreen bitmap.
	 */
	const CFbsBitmap* OffScreenBitmap() const;
	
	/**
	 * Get transition interface.
	 */
	MTransition* Transition() const;
	
	inline CHtmlControlImpl* Impl() const;
	inline CWindowGc& SystemGc() const;

protected:
	CHtmlControl();
	virtual void ConstructL(CCoeControl* aParent);
	
public: //from CCoeControl
	virtual TInt CountComponentControls() const;
	virtual CCoeControl* ComponentControl(TInt aIndex) const;
	virtual TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType); 
	virtual void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	virtual void HandleResourceChange(TInt aType);
	
private:
	virtual void Draw(const TRect& aRect) const;

private:
	CHtmlControlImpl* iImpl;
};

typedef CCoeControl* (*NewHtmlObjectL)(const TDesC& aClassId, const TDesC& aMimeType, const TDesC& aData, CHtmlControl& aControl, CHtmlElement& aElement);

class HtmlCtlLib
{
public:
	//search manage
	static void AddSearchPathL(const TDesC& aPath);
	static TBool ResolvePath(TDes& aPath);
	
	/**
	 * Add a global stylesheet. It will then make effect in all CHtmlControl.
	 * @param aSource CSS text.
	 */
	static void AddGlobalStyleSheetL(const TDesC& aSource);
	
	/**
	 * Add a object factory. The factory function will be called when object tag is met in the document.
	 * @param aFactory see NewHtmlObjectL.
	 */
	static void AddObjectFactory(NewHtmlObjectL aFactory);
	
	/**
	 * Create custom scrollbar.
	 * @aDefinition CSS text.
	 */
	static void CreateCustomScrollbarL(const TDesC& aDefinition);
	
	/**
	 * This function must be called in CAppUi::HandleResourceChange.
	 */
	static void ReportResourceChange(TInt aType);
	
	/**
	 * Refresh the image pool.
	 */
	static TBool RefreshImagePool();
};

//----------------------------------------------------------
inline CHtmlControlImpl* CHtmlControl::Impl() const
{
	return iImpl;
}

inline CWindowGc& CHtmlControl::SystemGc() const
{
	return CCoeControl::SystemGc();
}

#endif
