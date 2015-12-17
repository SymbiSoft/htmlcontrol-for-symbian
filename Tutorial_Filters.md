## 1. Corner filter ##
e.g.
```
<div style='width:100;height:100;border:1;background-color:#ffff00;filter:corner(10,10)'></div>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164013.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164013.jpg)

If the corner size is 9999, the result will be a circle. e.g.
```
<div style='width:100;height:100;border:1;background-color:#ffff00;filter:corner(9999,9999)'></div>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_163940.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_163940.jpg)

## 2. Alpha filter ##
e.g.
```
<div style='width:100;height:100;border:1;background-color:#000000;filter:alpha(opacity=50)'></div>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164144.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164144.jpg)

opacity is a number from 0 to 100.

Alpha filter can also be applied to image, e.g.
```
<img src='images/win.png'><br>
<img src='images/win.png' style='filter:alpha(opacity=50)'>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_173843.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_173843.jpg)

## 3. Fade filter ##
e.g.
```
<body style='filter:fade(enabled=true)'>
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164428.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_164428.jpg)

## 4. scale9Grid filter ##
If you are familar with Flash technology, you will know what scale9Grid is. To put it simply, scale9Grid is a rectangular region that defines the nine scaling regions for the image. You may google 'scale9Grid' for details.

e.g. consider the following image:

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/win.png](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/win.png)

When the image is scaled or stretched, the area within the rectangle scale normally, but the area outside of the rectangle scale according to the scale9Grid rules.

```
<div style='width:100%;height:50;background-image:images/win.png; background-position:stretch; filter:scale9grid(5,20,119,90)'></div><br>
<div style='width:100%;height:50;background-image:images/win.png; background-position:stretch'></div>
```

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_172602.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-24_172602.jpg)