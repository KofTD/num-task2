// Code printout in the style of `enscript`: landscape A4, two columns,
// header with file name (left) and "Page i of n" per file (right), no date.
// Black and white only: keywords, strings and preprocessor lines are bold,
// comments are italic. Font: JetBrains Mono.
//
// Usage:  typst compile print-code.typ code.pdf --input files=a.cpp,b.cpp,c.hpp
// Optional: --input size=7.5pt   --input cols=2   --input landscape=false

#let files = sys.inputs.at("files", default: "").split(",").filter(f => f != "")
#let cols = int(sys.inputs.at("cols", default: "2"))
#let landscape = sys.inputs.at("landscape", default: "true") == "true"
#let size = eval(sys.inputs.at("size", default: "7.5pt"))

#let lang-of(path) = {
  let ext = path.split(".").last()
  (cpp: "cpp", cc: "cpp", cxx: "cpp", hpp: "cpp", hh: "cpp", h: "cpp",
   c: "c", py: "py", rs: "rust", zig: "zig").at(ext, default: "txt")
}

// Black-only highlighting theme: style comes from bold / italic, not colour.
#let theme = bytes(```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>name</key><string>mono</string>
<key>settings</key><array>
<dict><key>settings</key><dict>
  <key>foreground</key><string>#000000</string>
  <key>background</key><string>#FFFFFF</string>
</dict></dict>
<dict><key>scope</key><string>comment</string>
  <key>settings</key><dict><key>fontStyle</key><string>italic</string></dict></dict>
<dict><key>scope</key><string>keyword.control, storage, meta.preprocessor, string, support.type, constant.language</string>
  <key>settings</key><dict><key>fontStyle</key><string>bold</string></dict></dict>
</array></dict></plist>
```.text)

#set page(
  paper: "a4",
  flipped: landscape,
  columns: cols,
  margin: (left: 10mm, right: 10mm, top: 17mm, bottom: 9mm),
  header-ascent: 6pt,
  header: context {
    let here-page = here().page()
    let starts = query(<file-start>)
    let mine = starts.filter(m => m.location().page() <= here-page)
    if mine.len() == 0 { return }
    let cur = mine.last()
    let first = cur.location().page()
    let next = starts.filter(m => m.location().page() > first)
    let last = if next.len() > 0 { next.first().location().page() - 1 } else {
      counter(page).final().first()
    }
    set text(font: ("JetBrains Mono", "DejaVu Sans Mono"), size: 8.5pt, weight: "bold")
    grid(columns: (1fr, auto),
      cur.value,
      [Page #(here-page - first + 1) of #(last - first + 1)])
  },
)
#set columns(gutter: 8mm)
#set text(font: ("JetBrains Mono", "DejaVu Sans Mono"), size: size, fill: black)
#set raw(theme: theme, tab-size: 4)
#set par(leading: 0.69em, justify: false)

// "  1: code" — number right-aligned in 4 monospace cells, then a colon.
#show raw.line: it => {
  box(width: 4 * 0.6em, align(right, str(it.number)))
  [:]
  h(0.6em)
  it.body
}

#for (i, path) in files.enumerate() {
  if i > 0 { pagebreak() }
  [#metadata(path) <file-start>]
  raw(read(path).trim("\n", at: end), lang: lang-of(path), block: true)
}
