
```
RESOURCE TBUF r_qtn_caption	{buf = qtn_caption_string;}
```

Example 1:
```
_LIT(KHtml, "<b><text res='"MAKESTR(R_QTN_CAPTION)"'></b>");
```
TEXT tag is a extended tag.

Example 2:
```
_LIT(KHtml, "<input type='button' *value='"MAKESTR(R_QTN_CAPTION)"'>");
```
Adding star before property name will make the property value load from resource.