-------------htmlcontrol 1.1.7-------------------

1.Improved: Added KHStrScrollMax.

2.Fixed: DrawNow;

3.Fixed: margin-right;

4.Improved: Increased default SELECT height;

5.Improved: Changed default focusing style of form elements.

6.Improved: Added STYLE tag support.

7.Improved: css attributes: display/clear/visibility/fade filter.

8.Improved: Added KHStrClick command on elements.

9.Improved: Immediately enter edit mode after alpha keys pressed.

10.Improved: Move the macros from MMP to standalone header file, htmlcontrol.hrh

-------------htmlcontrol 1.1.6-------------------

1.Improved: Add accesskey support.

2.Fixed: return EKeyWasNotConsumed if htmlcontrol didn't handle the key, in OfferKeyEventL.

3.Fixed: incorrect offset value in CHtmlControlImpl::ScrollToView (reported and fixed by Mimmo).

4.Fixed: bug in navigation among containers.(reported by Mimmo).

-------------htmlcontrol 1.1.5-------------------

1.Improved: Change the input(type='text')/textarea element focusing behavior. The user should hit selection key to activate focus of edit control. However, you can set the 'direct-focus' property to be true to avoid it.

2.Improved: Add `<`sup`>` and <`sub`> support.

-------------htmlcontrol 1.1.4-------------------

1.Improved: Remove not used function ViewRect in CHtmlElementImpl.

2.Improved: Add iFlags member to CHtmlElementImpl.

3.Improved: Add iData member to THtmlCtlEvent. Refer to Nokia Test criteria TPOT-02: "Touch interaction must follow the S60 UI Style Guide. The first tap selects the item and the second tap opens the item unless another item is selected." On responding to EOnClick, you can check the value of iData, if iData=0, it is the first tap; if iData=1, it is the second tap. It is due to you to determine the action on each cases. My suggestion is you must follow the criteria for list/grid, i.e. only open the item when iData=1; While for other elments, such as hyperlink, button, etc, you can just ignore iData. Please note that if EOnClick is triggered by selection key down event, iData is always 1.

4.Fixed: Forgot calling PrepareL of base class in CHtmlElementDiv::PrepareL

5.Improved: Add tabstop property. it is default to be true. If tabstop is false, the element wont have focus, however, it can be touched/clicked.



-------------htmlcontrol 1.1.3-------------------

1.Fixed: error when img element change its src from none empty to empty

2.Fixed: display error for div with borders in scrollable container(border drawn outside)

3.Fixed: in some special cases, text input/textarea failed to input text when using fullscreen qwerty.

4.Fixed: some compile error in using RVCT complier. (by tanjinkun@163.com)

5.Fixed: application quite after switch variant layout during loading large text

6.Fixed: ParseImageSrc failed to parse the path contains chinese chars.



-------------htmlcontrol 1.1.2-------------------

1.Improved: add Parent() to CHtmlElement.

2.Fixed: error when img element change its src from animated gif to none animated images.

3.Fixed: underline not visible in 5th1.0 emulator. fixed by adding one more pixel to the bottom of draw text rect.

4.Fixed: use VerticalPixelsToTwips instead of HorizontalPixelsToTwips to get font height.

5.Improved: add font-size to SELECT default style.



-------------htmlcontrol 1.1.1-------------------

1.Improved: performance of control size changing frequently

2.Fixed: fly transition effect display distorted after control size changed

3.Fixed: for div element, margin-right and margin-bottom have no effect in case margin-left and margin-top is 'auto'.

4.Fixed: line-wrap='clip' not clip multiline text correctly.

5.Improved: img with empty src will not use error image style.

6.Fixed: CFont::FontMaxHeight works fine in 3rd FP1 and later, but return 0 in 3rd\_MR. Added macro to differentiate.



-------------htmlcontrol 1.1-------------------

1.Fixed: the scrollbar display problem in some theme.

2.Removed: the old transition class and add new transition+easing support.

3.Improved: the line-wrap='clip' behavior.



-------------htmlcontrol 1.0-------------------

First release