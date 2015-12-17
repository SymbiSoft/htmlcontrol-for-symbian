## Enable scrollbar ##
Scrollbar can be setup for DIV element, by using CSS property overflow. e.g.
```
<div style='width:80%;height:80%;border:1;overflow:scroll'></div>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_175251.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_175251.jpg)

After added some content

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_175613.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_175613.jpg)

## Scrollbar properties ##
Following are the additional properties of scrollbar.
  * KHStrScrollLoop
Indicates whether the scrollbar will back to top if it reaches the end.
e.g.
```
eDiv->SetProperty(KHStrScrollLoop, ETrue);
iControl->RefreshAndDraw();
```
  * KHStrScrollStep
Delta scroll pos in pixels when one press on up key or down key.
  * KHStrScrollPos
Get or set the scroll position.
  * KHStrScrollVisible
Show or hide the scrollbar. Scrolling remains enabled.
  * KHStrScrollOnSlide
By default, one should touch the scrollbar area to do scroll. If set this value, one can use the finger to slide on the content area to do scroll.

## Customize scrollbar ##
e.g.
```
_LIT(KScrollbar, 
".shaft {background-image: images/shaft.mbm}"
".thumb {background-image:images/thumb.mbm; filter:scale9grid(1,2,15,51)}"
".uparrow {background-image:images/uparrow.mbm}"
".downarrow {background-image:images/downarrow.mbm}"
);
HtmlCtlLib::CreateCustomScrollbarL(KScrollbar);
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_221217.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_221217.jpg)

There are eight components you can define, they are:
  * shaft
  * shaft\_pressed
  * thumb
  * thumb\_pressed
  * uparrow
  * uparrow\_pressed
  * downarrow
  * downarrow\_pressed


