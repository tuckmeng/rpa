#include <GUIConstantsEx.au3>
#include <WindowsConstants.au3>
#include <StaticConstants.au3>

; Create the GUI window
GUICreate("Digital Clock", 250, 100, -1, -1, $WS_OVERLAPPEDWINDOW)

; Create a label to show the time
Local $hLabel = GUICtrlCreateLabel("", 10, 10, 230, 50)
GUICtrlSetFont($hLabel, 32, 800, 0, "Arial")
GUICtrlSetColor($hLabel, 0xFFFFFF) ; Set text color to white
GUICtrlSetBkColor($hLabel, 0x000000) ; Set background color to black

; Show the GUI
GUISetState()

; Update the clock every second
While 1
    ; Get the current time in HH:MM:SS format
    Local $sTime = StringFormat("%02d:%02d:%02d", @HOUR, @MIN, @SEC)

    ; Update the label with the current time
    GUICtrlSetData($hLabel, $sTime)

    ; Check for messages (like closing the window)
    If GUIGetMsg() = $GUI_EVENT_CLOSE Then ExitLoop
    
    ; Sleep for 1 second
    Sleep(1000)
WEnd

; Clean up
GUIDelete()
