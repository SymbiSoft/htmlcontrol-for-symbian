1. Hex value, e.g.
```
color: #FF0000
background-color:#FF0000
```

2. Color names, e.g.
```
color: red
background-color: black
```

3. S60 skin color, e.g.
```
color: skin#5
```
The expression is skin#id1,id2,id3. id1, id2 compose a TAknsItemID value, id3 is the color index. They are all in hex.
  * If id1,id2,id3 are both exists, then id1 is iMajor, id2 is iMinor, id3 is the color index.
  * If id3 is omitted, then id1 is iMinor, id2 is the color index, iMajor will be EAknsMajorSkin.
  * If id2,id3 are both omitted, then id1 is the color index, iMajor will be EAknsMajorSkin, iMinor will be EAknsMinorQsnTextColors.

Check aknsconstants.hrh for more information.

4. UIQ skin color, e.g.
```
color: skin#101F61AA,2
```
The expression is skin#SkinUid, TextDrawerIndex.

Check QSkinIds.h for more information.