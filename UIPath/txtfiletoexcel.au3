#include <Excel.au3>
#include <File.au3>

; Set the file paths
Local $txtFilePath = "C:\path\to\your\file.txt" ; Path to your text file
Local $excelFilePath = "C:\path\to\your\output.xlsx" ; Output path for Excel file

; Check if the text file exists
If Not FileExists($txtFilePath) Then
    MsgBox(0, "Error", "Text file does not exist.")
    Exit
EndIf

; Read the contents of the text file
Local $fileContents = FileRead($txtFilePath)

; Create a new Excel application
Local $oExcel = _Excel_Open()
If @error Then
    MsgBox(0, "Error", "Could not open Excel.")
    Exit
EndIf

; Create a new workbook
Local $oWorkbook = _Excel_BookNew($oExcel)
If @error Then
    MsgBox(0, "Error", "Could not create a new workbook.")
    _Excel_Close($oExcel)
    Exit
EndIf

; Split the file contents by line
Local $lines = StringSplit($fileContents, @CRLF)

; Write data to Excel
For $i = 1 To $lines[0]
    _Excel_RangeWrite($oWorkbook, $lines[$i], Default, "A" & $i) ; Write each line to column A
Next

; Save the workbook
_Excel_BookSaveAs($oWorkbook, $excelFilePath, 51) ; 51 = xlOpenXMLWorkbook (xlsx format)

; Close the workbook and Excel application
_Excel_BookClose($oWorkbook)
_Excel_Close($oExcel)

; Notify the user of completion
MsgBox(0, "Success", "Data has been written to Excel successfully.")

