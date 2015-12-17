1.Absolute value, the unit is pixel. e.g.
```
font-size:10
```

2.Size name. e.g.
```
font-size:medium
```
Following table shows the supported font size name and their real size in different screen dimension.
| Name | 178x208 | 240x320 | 360x640 |
|:-----|:--------|:--------|:--------|
| xx-small | 9       | 10      | 16      |
| x-small | 11      | 12      | 20      |
| small | 13      | 14      | 22      |
| medium | 15      | 16      | 24      |
| large | 17      | 18      | 28      |
| x-large | 19      | 22      | 32      |
| xx-large | 24      | 26      | 38      |


3.Relative value, real size=medium size + relative value. e.g.
```
 font-size: +5
```
The real font size will be 16+5=21, on 240\*320 screen.

The default font size of HtmlControl is 'medium'.