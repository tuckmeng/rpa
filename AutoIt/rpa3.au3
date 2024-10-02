#include <MsgBoxConstants.au3>
#include <StringConstants.au3>

Local $aResult = StringRegExp("You used 36 of 279 pages.", '([0-9]{1,3})(?: pages)', $STR_REGEXPARRAYMATCH)
If Not @error Then
    MsgBox($MB_OK, "SRE Example 6 Result", $aResult[0])
EndIf
