REBOL [Title: "Web Page Editor"]  ; required header

; Create a GUI window:

view layout [

    ; Here's a text entry field containing a generic URL address for
    ; the page to be edited.  The label "page-to-read" is assigned to
    ; this widget:

    page-to-read: field 600 "ftp://user:pass@website.com/path/page.html"

    ; Here's a multi-line text field to hold and edit the HTML
    ; downloaded from the above URL.  The label "the-html" is assigned
    ; to it:

    the-html: area 600x440

    ; The "across" words lays out the next buttons on the same line:

    across

    ; Here's a button to download and display the HTML at the URL given
    ; above:

    btn "Download HTML Page" [

        ; When the button is clicked, read the HTML at the URL above,
        ; insert it into the multi-line text area (by setting the text
        ; property of that field to the downloaded text), and update the
        ; display:

        the-html/text: read (to-url page-to-read/text)
        show the-html

    ]

    ; Here's another button to read and display text from a local file:

    btn "Load Local HTML File" [

        ; When the button is clicked, read the HTML from a file selected
        ; by the user, insert it into the multi-line text area, and update
        ; the display:

        the-html/text: read (to-file request-file)
        show the-html
    ]

    ; Here's another button.  When clicked, the edited contents of the
    ; multi-line text area are saved back to the URL:

    btn "Save Changes to Web Site" [
        write (to-url page-to-read/text) the-html/text
    ]

    ; Here's another button to write the edited contents of the multi-
    ; line text area to a local file selected by the user:

    btn "Save Changes to Local File" [
        write (to-file request-file/save) the-html/text
    ]
]
