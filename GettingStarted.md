# Modify MMP file #
HtmlControl is provided as static library, so add the following statement to your MMP file:
```
STATICLIBRARY htmlcontrol.lib
```
Some system libraries must be included at the same time, for S60 3rd, they are
```
LIBRARY	bafl.lib eikdlg.lib eikcoctl.lib eikctl.lib
LIBRARY	gdi.lib fbscli.lib bitgdi.lib ws32.lib egul.lib 
LIBRARY form.lib etext.lib uiklaf.lib
LIBRARY aknskins.lib aknskinsrv.lib aknswallpaperutils.lib aknicon.lib
LIBRARY imageconversion.lib charconv.lib
LIBRARY inetprotutil.lib
```

And for S60 5th, they are
```
LIBRARY	bafl.lib eikdlg.lib eikcoctl.lib eikctl.lib
LIBRARY	gdi.lib fbscli.lib bitgdi.lib ws32.lib egul.lib 
LIBRARY form.lib etext.lib uiklaf.lib
LIBRARY aknskins.lib aknskinsrv.lib aknswallpaperutils.lib aknicon.lib
LIBRARY imageconversion.lib charconv.lib
LIBRARY inetprotutil.lib
LIBRARY touchfeedback.lib
```

And for S60 2nd, they are
```
LIBRARY	bafl.lib eikdlg.lib eikcoctl.lib eikctl.lib apgrfx.lib
LIBRARY	gdi.lib fbscli.lib bitgdi.lib ws32.lib egul.lib 
LIBRARY form.lib etext.lib uiklaf.lib
LIBRARY aknskins.lib aknskinsrv.lib
LIBRARY imageconversion.lib
LIBRARY inetprotutil.lib
```

And for UIQ, they are
```
LIBRARY	bafl.lib eikdlg.lib eikctl.lib apgrfx.lib 
LIBRARY	gdi.lib fbscli.lib bitgdi.lib ws32.lib egul.lib 
LIBRARY form.lib etext.lib uiklaf.lib
LIBRARY skins.lib quiconfigclient.lib qikutils.lib qikdlg.lib qikctl.lib
LIBRARY imageconversion.lib  charconv.lib 
LIBRARY inetprotutil.lib
```

# Create the control #
```
#include <html\htmlcontrol.h>

iControl = CHtmlControl::NewL(NULL);
iControl->SetRect(ClientRect());
iControl->SetMopParent(this);
iControl->ActivateL();
AddToStackL(iControl );
```

You should pass the parent control pointer as the only parameter of **CHtmlControl::NewL**. Use NULL if it is the top level control.

In order to allow HtmlControl to response the system resource changes, you should override **HandleResourceChangeL** of AppUi, and add following statement,
```
HtmlCtlLib::ReportResourceChange(aType);
```

# Add HTML content #
The simple way is to add HTML content is **AppendContentL**, which add the HTML to the end of the control, e.g.
```
_LIT(KHtml, "<b>Hello world</b>");
iControl->AppendContentL(KHtml);
iControl->RefreshAndDraw();
```
You can keep calling **AppendContentL** to add new content, but the content of each call must be well-formed HTML text. The following code is wrong,
```
//WRONG
_LIT(KHtml1, "<b>Hello world");
iControl->AppendContentL(KHtml1);
_LIT(KHtml2, "</b>");
iControl->AppendContentL(KHtml2);
```
note: HtmlControl can handle WRONG HTML and produce no error.

The second way is to use **InsertContentL**, which has additional parameters to specify the position, e.g.
```
_LIT(KHtml1, "<div id='a'>world</div>");
iControl->AppendContentL(KHtml1);
_LIT(KHtml2, "Hello ");
iControl->InsertContentL(iControl->Element(_L("a")), EBeforeBegin, KHtml2);
iControl->RefreshAndDraw();
```

The third way is to use **innerText** or **innerHtml** property of elements，e.g.
```
_LIT(KHtml1, "<div id='a'></div>");
iControl->AppendContentL(KHtml1);
_LIT(KHtml2, "<b>Hello world</b>");
iControl->Element(_L("a"))->SetProperty(KHStrInnerHtml, KHtml2);
iControl->RefreshAndDraw();
```

No matter what method you use，please keep in mind that you need to call Refresh or RefreshAndDraw(Refresh+DrawNow) to update the control state.

The following table shows the supported HTML tags and properties.
| Tag | HTML Property | Extended Property |
|:----|:--------------|:------------------|
| body | style class   | -                 |
| b   | -             | -                 |
| strong | -             | -                 |
| i   | -             | -                 |
| u   | -             | -                 |
| font | color size style | -                 |
| br  | -             | -                 |
| p   | align         | -                 |
| hr  | width height color size align | -                 |
| img | id name src width height align style class | focus-src animation |
| a   | id name style class | -                 |
| div | id name width height align style class | focusing focus-style focus-class down-style down-class line-wrap |
| form | id name action method enctype style class | -                 |
| input | id name type value checked disabled readonly  style class| max-length input-mode |
| select | id name value disabled style class | selectedIndex textArray valueArray optionArray |
| textarea | id name disabled style class | draw-lines line-color |
| object | id name width height align classid data type style class | -                 |
| embed | id name width height align src style class | -                 |

# Access Elements #
HtmlControl use BODY element as the top level element，call **Body** to get the pointer of it. e.g.
```
  CHtmlElement* body = iControl->Body();
```

To get element pointer,
```
CHtmlElement* Element(const TDesC& aId, TInt aIndex = 0) const;
CHtmlElement* ElementByTag(const TDesC& aTagName, TInt aIndex = 0) const;
```
The return pointer will remain valid until the related element is removed. Caller must not delete this return pointer.
Note: In CHtmlControl，property **name** will treat as property **id**, and **id** is not demand to be unique.
e.g.：
```
_LIT(KHtml, "<div id='a'>Hello</div> <div id='a'>world</div>");
iControl->AppendContentL(KHtml);
CHtmlElement* e1 = iControl->Element(_L("a"), 0); //hello
CHtmlElement* e2 = iControl->Element(_L("a"), 1); //world
```

Following code demonstrated how to get element pointer by tag name.
```
_LIT(KHtml, "<div>Hello world </div>");
iControl->AppendContentL(KHtml);
CHtmlElement* e = iControl->ElementByTag(KHStrDiv);
```

To enumerate elements,
```
#include <html\htmlelementiter.h>
_LIT(KHtml, "<img src='1.gif'><img src='2.gif'><img src='3.gif'>");
iControl->AppendContentL(KHtml);
THtmlElementIter iter(iControl->Body(), KNullDesC, KHStrImg);
while(it.Next())
{
  CHtmlElement* e = it();
}
```

After you got the element pointer, you can change the element properties and styles. e.g.
```
CHtmlElement* e = iControl->Element(_L("a"), 0);
e->SetStyle(KHStrVisibility, KHStrHidden); //e->SetStyle(KHStrDisplay, KHStrNone); 
iControl->RefreshAndDraw();
```
The above code will hide the specific element.

To delete element from control,
```
_LIT(KHtml, "<div id='a'>Hello</div> <div id='a'>world</div>");
iControl->AppendContentL(KHtml);
CHtmlElement* e1 = iControl->Element(_L("a"), 0); //hello
iControl->RemoveElement(e1);
iControl->RefreshAndDraw();
```

To clear the content of DIV/A/FORM element,
```
e1->SetProperty(KHStrInnerHtml, KNullDesC);
iControl->RefreshAndDraw();
```

To clear all content of the control,
```
iControl->ClearContent();
iControl->RefreshAndDraw();
```

# Using styles #
HtmlControl support CSS，e.g.
```
_LIT(KHtml, "<div style='font-size:large;color:red'>Hello world</div>");
iControl->AppendContentL(KHtml);
iControl->RefreshAndDraw();
```

StyleSheet is also supprted, e.g.
```
_LIT(KStyleSheet, ".noname { font-size:large;color:red }");
iControl->AddStyleSheetL(KStyleSheet);
_LIT(KHtml, "<div class='noname'>Hello world</div>");
iControl->AppendContentL(KHtml);
iControl->RefreshAndDraw();
```

To delete CSS class,
```
iControl->RemoveStyleClass(_L("noname"));
```
To clear all CSS classes,
```
iControl->ClearStyleSheet();
```

To add CSS styles that available for all CHtmlControl in the application, you can use **HtmlCtlLib::AddGlobalStyleSheetL**, e.g.
```
_LIT(KStyleSheet, "INPUT { border:2 }"
                  "TEXTAREA { border:2 }"
);
HtmlCtlLib::AddGlobalStyleSheetL(KStyleSheet);
```

The following table shows the supported CSS properties.
  * Font
color font-family font-size font-weight font-style text-decoration line-height
  * Text
text-align
  * Background
background-color background-image background-position background-repeat
  * Position
top left
  * Dimension
height max-height width max-width
  * Layout
clear overflow display visibility
  * Margin
margin margin-top margin-right margin-bottom margin-left
  * Border
border border-color border-style border-width border-top border-right border-bottom border-left
  * Padding
padding padding-top padding-right padding-bottom padding-left
  * Misc
filter

Note: All length unit is px, e.g. style='width:50'，here '50' is equal to '50px'; and style='width:50pt', here '50pt' is equal to '50px' too.

# Handle events #
Call **CHtmlControl::SetEventObserver** to setup the event callback, e.g.
```
iControl->SetEventObserver(this);
```
The supported event types are:
  * **EOnClick**      Click event. Fired by A/INPUT/DIV;
  * **EOnChanged**    Fired by SELECT after selection changed.
  * **EOnSubmit**     Fired by FORM.
  * **EOnGainFocus**  Fired by any focusing element after receiving focus.
  * **EOnLostFocus**  Fired by any focusing element after focus lost.
  * **EOnSlideLeft**  Fired after the finger sliding from right to left.
  * **EOnSlideRight** Fired after the finger sliding from left to right.

# Create List and Grid #
HtmlControl support standard Symbian List and Grid. The following are the steps to create such UI.
  1. Use DIV as container.
```
_LIT(KHtml, "<div id='list' style='width:100%;height:100%;'></div>");
iControl->AppendContentL(KHtml);
CHtmlElement* list = iControl->Element(_L("list"));
```
  1. Create list.
```
list->InvokeL(KHStrCreateList, (MListItemRenderer*)this);
```
  1. Set list item template.
```
_LIT(KItemHtml, 
"<div id='item' style='width:100%; height:22;' class='item' focus-class='focus-item'>"
	"<div style='width:20;height:100%;border-right:1 dotted;padding:auto auto'><img src='' width='16' height='16'></div>"
	"<div id='title' style='padding:auto 0 auto 2; width:auto; height:100%' line-wrap='scroll'></div>"
"</div>");
list->SetProperty(KHStrListItemHtml, KItemHtml);
```
  1. Render the list item in callback.
```
void CBasicDemoAppUi::RenderListItemL(CHtmlElement* aElement, TInt aIndex)
{
  const TMyEntry& entry = iFileList[aIndex];
	
  CHtmlElement* icon = aElement->ElementByTag(KHStrImg);
  CHtmlElement* title = aElement->Element(_L("title"));
	
  if(entry.iAtt & KEntryAttVolume)
    icon->SetProperty(KHStrSrc, _L("images/drive.mbm"));
  else if(entry.iAtt & KEntryAttDir)
    icon->SetProperty(KHStrSrc, _L("images/folder.mbm"));
  else
    icon->SetProperty(KHStrSrc, _L("images/file.mbm"));

  title->SetProperty(KHStrInnerText, entry.iName);
}
```
> > Note: Don't call **Refresh** or **RefreshAndDraw** in **RenderListItemL**.
  1. Set list item count, and then update the whole list.
```
list->SetProperty(KHStrListItemCount, 100);
list->InvokeL(KHStrUpdateAll);
iControl->RefreshAndDraw();
```

You can create grid in same way, only to replace KHStrCreateList with KHStrCreateGrid.
The columns of grid depends on the width of the list item.

To update a single item in the list(e.g. the status of an SMS message change from unread to read), you can call **KHStrUpdate** on the list, e.g.
```
list->InvokeL(KHStrUpdate, 50);
iControl->RefreshAndDraw();
```
If the 50th item is in the view, then the list will fire **RenderListItemL** to rerender this item.

To delete one or more items in the list, just set **KHStrListItemCount** to the new count，and then call **KHStrUpdateAll** on the list, e.g.
```
list->SetProperty(KHStrListItemCount, 0);
list->InvokeL(KHStrUpdateAll);
iControl->RefreshAndDraw();
```

When there is no item in the list, the list is blank by default. However, you can set some content to display, e.g.
```
_LIT(KEmptyHtml, "<p align='center'>(No Data)</p>");
list->SetProperty(KHStrListEmptyHtml, KEmptyHtml);
```

# Using Transition #
Transition is the effect on content change. There are 8 transition effects built in CHtmlControl.

The following example shows how simple is to use transition class. When user press the left key, the screen will slide from right to left.

```
#include <html\transition.h>

_LIT(KHtml, "<div id='a'>Hello world</a>");
iControl->AppendContentL(KHtml);
iControl->RefreshAndDraw();
```

```
TKeyResponse CSomeClass::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType  )
{
  if (aKeyEvent.iCode == EKeyLeftArrow)
  {
     iControl->ClearContent();

     _LIT(KHtml, "Another Page");
     iControl->AppendContentL(KHtml);

     iControl->Tansition()->Perform(iControl->Body(), MTransition::ESlideLeft, 200*1000, 5);

     return EKeyWasConsumed;
  }
  else
    return CBaseClass::OfferKeyEventL(aKeyEvent, aType);
}
```