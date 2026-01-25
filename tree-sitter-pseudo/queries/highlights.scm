; Keywords
[
  "citeste"
  "scrie"
  "daca"
  "atunci"
  "altfel"
  "sf"
  "pentru"
  "executa"
  "cat"
  "timp"
  "repeta"
  "pana"
  "cand"
] @keyword

; Logical operators
[
  "SAU"
  "sau"
  "SI"
  "si"
  "NOT"
  "not"
] @keyword.operator

; Operators
[
  "<-"
  "<->"
  "<-->"
  "+"
  "-"
  "*"
  "/"
  "%"
  "="
  "!="
  "<"
  "<="
  ">"
  ">="
  "√"
] @operator

; Punctuation
[
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

[
  ","
  ";"
] @punctuation.delimiter

; Literals
(number) @number
(string) @string

; Identifiers
(identifier) @variable

; Assignment target
(assign
  name: (identifier) @variable.parameter)

; Swap targets
(swap
  left: (identifier) @variable.parameter
  right: (identifier) @variable.parameter)

; For loop variable
(for
  var: (identifier) @variable.parameter)

; Read targets
(read
  names: (name_list
    (identifier) @variable.parameter))

; Comments
(comment) @comment
