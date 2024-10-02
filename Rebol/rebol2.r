REBOL [title: "Calculator"]
view layout [ 
     across 
     f: field 225x50 font-size 25  return 
     style b btn 50x50 [append f/text face/text show f] 
     b "1"  b "2"  b "3"  b " + "  return 
     b "4"  b "5"  b "6"  b " - "  return 
     b "7"  b "8"  b "9"  b " * "  return 
     b "0"  b "."  b " / "  b "=" [ 
         attempt [f/text: form do f/text show f] 
     ] 
]
