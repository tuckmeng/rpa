; Check command line parameters
If $CmdLine[0] < 2 Then
    ConsoleWrite("Usage: " & @ScriptName & " <window_title_substring> <commands>" & @CRLF)
    Exit 1
EndIf

; Get the window title substring and commands from command line arguments
Local $windowTitleSubstring = $CmdLine[1]
Local $commands = $CmdLine[2]

; Function to find the window by title substring
Func FindWindowByTitle($titleSubstring)
    Local $hWnd = WinGetHandle($titleSubstring)
    If @error Then
        Return 0 ; No window found
    EndIf
    Return $hWnd
EndFunc

; Function to simulate mouse actions
Func SimulateMouseAction($command, $hWnd)
    ; Activate the window
    WinActivate($hWnd)
    Sleep(100) ; Wait a moment for the window to activate

    ; Process the command
    If StringLeft($command, 2) = "m " Then
        ; Move command
        Local $coords = StringSplit(StringTrimLeft($command, 2), " ")
        If $coords[0] = 2 Then
            Local $x = Int($coords[1])
            Local $y = Int($coords[2])
            MouseMove($x, $y, 0)
        Else
            ConsoleWrite("Invalid move command: " & $command & @CRLF)
            Return
        EndIf
    ElseIf $command = "c" Then
        ; Left click
        MouseClick("left")
    ElseIf $command = "dc" Then
        ; Double click
        MouseClick("left", "", "", 2)
    ElseIf $command = "rc" Then
        ; Right click
        MouseClick("right")
    Else
        ConsoleWrite("Unknown command: " & $command & @CRLF)
    EndIf
EndFunc

; Split commands and execute them
Local $commandList = StringSplit($commands, ",")
Local $hWnd = FindWindowByTitle($windowTitleSubstring)

If $hWnd = 0 Then
    ConsoleWrite("Window not found containing title substring: " & $windowTitleSubstring & @CRLF)
    Exit 1
EndIf

For $i = 1 To $commandList[0]
    SimulateMouseAction($commandList[$i], $hWnd)
Next

Exit 0
