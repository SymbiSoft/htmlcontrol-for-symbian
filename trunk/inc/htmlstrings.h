/*
* ============================================================================
*  Name     : String constants
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLSTRINGS_H
#define HTMLSTRINGS_H

#define __LIT(name,s) extern const TLitC16<sizeof(L##s)/2> name

#define _MAKESTR(s)     #s
#define MAKESTR(s)      _MAKESTR(s)

__LIT(KHStrTrue, "true");
__LIT(KHStrFalse, "false");
__LIT(KHStrEnabled, "enabled");
__LIT(KHStrChecked, "checked");
__LIT(KHStrDisabled, "disabled");
__LIT(KHStrBlack, "black");
__LIT(KHStrRed, "red");
__LIT(KHStrGreen, "green");
__LIT(KHStrBlue, "blue");
__LIT(KHStrYellow, "yellow");
__LIT(KHStrMagenta, "magenta");
__LIT(KHStrCyan, "cyan");
__LIT(KHStrGray, "gray");
__LIT(KHStrWhite, "white");
__LIT(KHStrTransparent, "transparent");
__LIT(KHStrXXSmall, "xx-small");
__LIT(KHStrXSmall, "x-small");
__LIT(KHStrSmall, "small");
__LIT(KHStrMedium, "medium");
__LIT(KHStrLarge, "large");
__LIT(KHStrXLarge, "x-large");
__LIT(KHStrXXLarge, "xx-large"); 
__LIT(KHStrAlign,"align");
__LIT(KHStrLeft, "left");
__LIT(KHStrRight, "right");
__LIT(KHStrCenter, "center");
__LIT(KHStrTop, "top");
__LIT(KHStrMiddle, "middle");
__LIT(KHStrBottom, "bottom");
__LIT(KHStrBoth, "both");
__LIT(KHStrPosition,"position");
__LIT(KHStrRelative,"relative");
__LIT(KHStrAbsolute,"absolute");
__LIT(KHStrNone, "none");
__LIT(KHStrDotted, "dotted");
__LIT(KHStrDashed, "dashed");
__LIT(KHStrOutset, "outset");
__LIT(KHStrInset, "inset");
__LIT(KHStrAuto, "auto");
__LIT(KHStrVisible, "visible");
__LIT(KHStrHidden, "hidden");
__LIT(KHStrScroll, "scroll");
__LIT(KHStrDisplay, "display");
__LIT(KHStrBlock, "block");
__LIT(KHStrAnnotation, "annotation");
__LIT(KHStrTitle, "title");
__LIT(KHStrLegend, "legend");
__LIT(KHStrSymbol, "symbol");
__LIT(KHStrDense, "dense");
__LIT(KHStrItalic, "italic");
__LIT(KHStrOblique, "oblique");
__LIT(KHStrBold, "bold");
__LIT(KHStrUnderline, "underline");
__LIT(KHStrLineThrough, "line-through");
__LIT(KHStrColor, "color");
__LIT(KHStrFontSize, "font-size");
__LIT(KHStrFontStyle, "font-style");
__LIT(KHStrFontWeight, "font-weight");
__LIT(KHStrTextDecoration, "text-decoration");
__LIT(KHStrFontFamily, "font-family");
__LIT(KHStrLineHeight, "line-height");
__LIT(KHStrTextAlign, "text-align");
__LIT(KHStrLineVAlign, "line-valign");
__LIT(KHStrBackgroundColor, "background-color");
__LIT(KHStrBackgroundImage,  "background-image");
__LIT(KHStrBackgroundPosition,  "background-position");
__LIT(KHStrBackgroundRepeat,  "background-repeat");
__LIT(KHStrBackgroundPattern,  "background-pattern");
__LIT(KHStrSolid,"solid");
__LIT(KHStrVerticalHatch,"vertical-hatch");
__LIT(KHStrForwardDiagonalHatch,"forward-diagnoal-hatch");
__LIT(KHStrHorizontalHatch,"horizontal-hatch");
__LIT(KHStrRearwardDiagonalHatch,"rearward-diagnoal-hatch");
__LIT(KHStrSquareCrossHatch,"square-cross-hatch");
__LIT(KHStrDiamondCrossHatch,"diamond-cross-hatch");
__LIT(KHStrRepeat,  "repeat");
__LIT(KHStrNoRepeat,  "no-repeat");
__LIT(KHStrRepeatX,  "repeat-x");
__LIT(KHStrRepeatY,  "repeat-y");
__LIT(KHStrStretch,  "stretch");
__LIT(KHStrBorder, "border");
__LIT(KHStrBorderWidth, "border-width");
__LIT(KHStrBorderColor, "border-color");
__LIT(KHStrBorderLeft, "border-left");
__LIT(KHStrBorderRight, "border-right");
__LIT(KHStrBorderTop, "border-top");
__LIT(KHStrBorderBottom, "border-bottom");
__LIT(KHStrWidth, "width");
__LIT(KHStrHeight, "height");
__LIT(KHStrMaxWidth, "max-width");
__LIT(KHStrMargin, "margin");
__LIT(KHStrMarginTop, "margin-top");
__LIT(KHStrMarginBottom, "margin-bottom");
__LIT(KHStrMarginLeft, "margin-left");
__LIT(KHStrMarginRight, "margin-right");
__LIT(KHStrPadding, "padding");
__LIT(KHStrPaddingTop, "padding-top");
__LIT(KHStrPaddingBottom, "padding-bottom");
__LIT(KHStrPaddingLeft, "padding-left");
__LIT(KHStrPaddingRight, "padding-right");
__LIT(KHStrClear, "clear");
__LIT(KHStrFilter, "filter");
__LIT(KHStrOverflow, "overflow");
__LIT(KHStrVisibility, "visibility");
__LIT(KHStrLineWrap, "line-wrap");
__LIT(KHStrClip, "clip");
__LIT(KHStrAlt, "alt");
__LIT(KHStrRemark, "remark");
__LIT(KHStrError, "error");
__LIT(KHStrId, "id");
__LIT(KHStrName, "name");
__LIT(KHStrFocusing, "focusing");
__LIT(KHStrSize, "size");
__LIT(KHStrStyle, "style");
__LIT(KHStrRes, "res");
__LIT(KHStrInnerText, "innertext");
__LIT(KHStrInnerHtml, "innerhtml");
__LIT(KHStrClass, "class");
__LIT(KHStrFocusClass, "focus-class");
__LIT(KHStrFocusStyle, "focus-style");
__LIT(KHStrDownClass, "down-class");
__LIT(KHStrDownStyle, "down-style");
__LIT(KHStrScrollLoop, "scroll-loop");
__LIT(KHStrScrollStep, "scroll-step");
__LIT(KHStrScrollPos, "scroll-pos");
__LIT(KHStrScrollVisible, "scroll-visible");
__LIT(KHStrScrollOnSlide, "scroll-on-slide");
__LIT(KHStrSrc, "src");
__LIT(KHStrFocusSrc, "focus-src");
__LIT(KHStrFullPath, "fullpath");
__LIT(KHStrAnimation, "animation");
__LIT(KHStrValue, "value");
__LIT(KHStrMaxLength, "max-length");
__LIT(KHStrBufSize, "buf-size");
__LIT(KHStrType, "type");
__LIT(KHStrText, "text");
__LIT(KHStrCheckbox, "checkbox");
__LIT(KHStrPassword, "password");
__LIT(KHStrRadio, "radio");
__LIT(KHStrButton, "button");
__LIT(KHStrSubmit, "submit");
__LIT(KHStrFile, "file");
__LIT(KHStrImage, "image");
__LIT(KHStrRows, "rows");
__LIT(KHStrInputMode, "input-mode");
__LIT(KHStrReadOnly, "readonly");
__LIT(KHStrNumber, "number");
__LIT(KHStrAll, "all");
__LIT(KHStrAlpha, "alpha");
__LIT(KHStrSelected, "selected");
__LIT(KHStrSelectedIndex, "selected-index");
__LIT(KHStrTextArray, "text-array");
__LIT(KHStrValueArray, "value-array");
__LIT(KHStrOptionArray, "option-array");
__LIT(KHStrDrawLines, "draw-lines");
__LIT(KHStrLineColor, "line-color");
__LIT(KHStrHref, "href");
__LIT(KHStrBody, "body");
__LIT(KHStrHead,"head");
__LIT(KHStrScript,"script");
__LIT(KHStrB,"b");
__LIT(KHStrStrong,"strong");
__LIT(KHStrI,"i");
__LIT(KHStrU,"u");
__LIT(KHStrFont,"font");
__LIT(KHStrP,"p");
__LIT(KHStrBr, "br");
__LIT(KHStrHr, "hr");
__LIT(KHStrBlockQuote,"blockquote");
__LIT(KHStrLi,"li");
__LIT(KHStrTr,"tr");
__LIT(KHStrTable,"table");
__LIT(KHStrTd,"td");
__LIT(KHStrOption,"option");
__LIT(KHStrParam,"param");
__LIT(KHStrA, "a");
__LIT(KHStrTarget, "target");
__LIT(KHStrImg, "img");
__LIT(KHStrDiv, "div");
__LIT(KHStrSpan, "span");
__LIT(KHStrForm, "form");
__LIT(KHStrMethod, "method");
__LIT(KHStrAction, "action");
__LIT(KHStrEncType, "enctype");
__LIT(KHStrFormData, "form-data");
__LIT(KHStrInput, "input");
__LIT(KHStrSelect, "select");
__LIT(KHStrClassId, "classid");
__LIT(KHStrData, "data");
__LIT(KHStrEmbed, "embed");
__LIT(KHStrObject, "object");
__LIT(KHStrTextArea, "textarea");
__LIT(KHStrStyleDot, "style.");
__LIT(KHStrListItemCount, "list-item-count");
__LIT(KHStrListSelectedIndex, "list-selected-index");
__LIT(KHStrListTopIndex, "list-top-index");
__LIT(KHStrListItemHtml, "list-item-html");
__LIT(KHStrListEmptyHtml, "list-empty-html");
__LIT(KHStrSelector, "selector");
__LIT(KHStrLink, "link");
__LIT(KHStrHover, "hover");
__LIT(KHStrFocus, "focus");
__LIT(KHStrDown, "down");
__LIT(KHStrTabStop, "tabstop");
__LIT(KHStrDirectFocus, "direct-focus");

__LIT(KHStrCreateSlides, "create-slides");
__LIT(KHStrCreateList, "create-list");
__LIT(KHStrCreateGrid, "create-grid");
__LIT(KHStrAddOption, "add-option");
__LIT(KHStrRemoveOption, "remove-option");
__LIT(KHStrClearOptions, "clear-options");
__LIT(KHStrUpdate, "update");
__LIT(KHStrUpdateAll, "update-all");

#endif
