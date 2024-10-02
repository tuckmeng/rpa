REBOL [title: "Days Between"]
view layout [
    btn 200 "Start" [face/text: s: request-date]
    btn 200 "End" [
        face/text: e: request-date
        f/text: e - s
        show f
    ]
    text "Days Between:"
    f: field
]
