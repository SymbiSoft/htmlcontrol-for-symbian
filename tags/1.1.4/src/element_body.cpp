#include "controlimpl.h"
#include "element_body.h"

CHtmlElementBody::CHtmlElementBody(CHtmlControl* aOwner)
:CHtmlElementDiv(aOwner)
{
	iTagName.Set(KHStrBody);
	iFlags.Set(EBody);
}
