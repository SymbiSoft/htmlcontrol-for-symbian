HtmlControl support local image resource and system skin.

## 1. Image source expression. ##

### 1.1. Mbm file. ###

a) given a bitmap file name and bitmap index. e.g.
```
<img src='pic.mbm#1'>
```
b) given a bitmap file name, bitmap index and mask index. e.g.
```
<img src='pic.mbm#1,2'>
```
c) given a bitmap file name only. In this case, the first bitmap will be used for drawing, the second bitmap will be used for the mask, if it is exists. e.g.
```
<img src='pic.mbm'>
```
Note: All numbers are in hex. e.g.
```
<img src='pic.mbm#A,B'>
```

There are 3 predfined bitmap file names:

1) avkon.mbm(S60 only). e.g.
```
<img src='avkon.mbm#4078,4079'>
<img src='avkon.mbm#407A,407B'>
```
2) qikon.mbm(UIQ only).
3) **.** represents the application bitmap file. e.g.
```
<img src='*#1,2'>
```

### 1.2. General image file, in gif/jpg/png/bmp format. ###
e.g.
```
<img src='pic.gif'>
```
if the image is animated gif, it will play animation by default. You can also disable the animation, e.g.
```
<img src='pic.gif' animation='false'>
```

### 1.3. Svg. Svg is not supported directly, you can use mif only. ###
> e.g.
```
<img src='test.mif' width='40' height='40'>
```

### 1.4. Application icon. ###
e.g.
```
<img src='appicon#00d33043' width='32' height='32'>
```
It will be the icon of an application with UID 00d33043.

### 1.5. S60 skin background. ###
e.g.
```
background-image: skin#10005a26,1000
background-image: skin#1000
```
The expression is 'skin#id1,id2'. id1 and id2 compose a TAknsItemID value, and they are all in hex.
  * If id1 and id2 are both exists, then id1 is iMajor and id2 is iMinor.
  * If id2 is omitted, then id1 is iMinor, iMajor will be EAknsMajorSkin.
  * If id1 and id2 are both omitted, then id1 will be EAknsMajorSkin, id2 will be EAknsMinorQsnBgScreen.
Check aknsconstants.hrh for more information.

### 1.6. S60 skin frame. ###
e.g.
```
background-image: frame#10005a26,1400
background-image: frame#1400
```
The expression is 'frame#id1,id2'. Please refer to 1.5 Skin for the details of id1 and id2.

In aknscontants.hrh, you can find the following definition which define a fram,
```
// General list highlight frame
  EAknsMinorQsnFrList                 = 0x1400,
  EAknsMinorQsnFrListCornerTl         = 0x1401,
  EAknsMinorQsnFrListCornerTr         = 0x1402,
  EAknsMinorQsnFrListCornerBl         = 0x1403,
  EAknsMinorQsnFrListCornerBr         = 0x1404,
  EAknsMinorQsnFrListSideT            = 0x1405,
  EAknsMinorQsnFrListSideB            = 0x1406,
  EAknsMinorQsnFrListSideL            = 0x1407,
  EAknsMinorQsnFrListSideR            = 0x1408,
  EAknsMinorQsnFrListCenter           = 0x1409,
```
So frame#1400 will be the 'general list highlight frame'.

### 1.7 S60 skin icon. ###
e.g.
```
<img src='icon#10005a26,4210' width='32' height='32'>
```
The express is 'icon#id1,id2'.Please refer to 1.5 for the details of id1 and id2.

### 1.8. UIQ skin background. ###
> e.g.
```
<img src='skin#101F61AA,4,4' width='16' height='16'>
background-image: skin#101F61AA,0
```
The expression is 'skin#SkinUid,PatchIndex,MultiBitmapIndex. All numbers are in hex.

Check QSkinIds.h for more information.

### 1.9 UIQ skin icon. ###
e.g.
```
<img src='icon#10201234' width='16' height'16'>
```
The expression is 'icon#uri\_icon'. Check CQikContent in UIQ SDK for more information.

### 1.10. Gradient background. ###
e.g.
```
background-image: gradient#4199F5,286AE9,1
```
The expression is 'gradient#starColor,endColor,orientation'. For orientation, 0 is vertical and 1 is horizontal. It can be omitted, default is 0.


## 2. Image location. ##
The default image search path is application private path, on all availabe volum. e.g.
```
<img src='images/pic.gif'>
```
For Symibian 9.0 downwards, the control will try to locate the image on `!:/system/apps/<APPNAME>/images/pic.gif`, ! represented all availabe volum.
For Symibian 9.0 onwards, the control will try to locate the image on `!:/private/<SECUREID>/images/pic.gif`, ! represented all availabe volum.

You can also add custom search path, e.g.
```
HtmlCtlLib::AddSearchPathL(_L("c:\\data\\others\\"));
Note: the path must end of backlash.
```

Full path is also supportted, e.g.
```
<img src='file:///c/data/images/11.gif'>
```

## 3. Image cache policy. ##
Images are cached in memory if their reference count is greate than zero. Once a image's reference count is zero, it will be unloaded immediatelly.

If you want a manually cache a image, try to do a trick by using global stylesheet. e.g.
```
_LIT(KStyle, ".cached1 { background-image: pic.gif }");
HtmlCtlLib::AddGlobalStyleSheet(KStyle);
```

## 4. Failed-to-load images. ##
By default, images failed to load will display empty. You can change this behavior by setting img:error style. e.g.
```
_LIT(KStyle, "img:error { background-image: default/image_error.mbm}");
iControl->AddStyleSheetL(KStyle);
```
Thus all images failed to load in iControl will display image\_error.mbm.