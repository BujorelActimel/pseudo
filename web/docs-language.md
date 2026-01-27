# Limbajul Pseudocod

Referință completă pentru sintaxa limbajului Pseudocod.

## Variabile

### Atribuire

Folosiți operatorul săgeată `<-` pentru a atribui valori:

```pseudo
x <- 10
nume <- "Ana"
pi <- 3.14159
```

### Tipuri de Date

| Tip | Exemple | Descriere |
|-----|---------|-----------|
| Întreg | `42`, `-7`, `0` | Numere întregi |
| Real | `3.14`, `-0.5` | Numere cu zecimale |
| Șir de caractere | `"text"`, `'text'` | Text între ghilimele |

### Reguli pentru Nume

- Trebuie să înceapă cu o literă sau `_`
- Pot conține litere, cifre și `_`
- Sunt case-sensitive (`x` ≠ `X`)

```pseudo
numar <- 10       # valid
_temp <- 5        # valid
numar2 <- 20      # valid
2numar <- 30      # INVALID
```

## Operatori

### Aritmetici

| Operator | Descriere | Exemplu |
|----------|-----------|---------|
| `+` | Adunare | `5 + 3` → `8` |
| `-` | Scădere | `5 - 3` → `2` |
| `*` | Înmulțire | `5 * 3` → `15` |
| `/` | Împărțire | `10 / 4` → `2.5` |
| `%` | Modulo | `10 % 3` → `1` |
| `[x]` | Parte întreagă | `[3.7]` → `3` |
| `√` | Radical | `√16` → `4` |

```pseudo
a <- 10
b <- 3

suma <- a + b          # 13
diferenta <- a - b     # 7
produs <- a * b        # 30
cat <- a / b           # 3.333...
rest <- a % b          # 1
intreg <- [a / b]      # 3
rad <- √16             # 4
```

### Comparație

| Operator | Descriere |
|----------|-----------|
| `=` | Egal |
| `!=` | Diferit |
| `<` | Mai mic |
| `>` | Mai mare |
| `<=` | Mai mic sau egal |
| `>=` | Mai mare sau egal |

```pseudo
x <- 5
y <- 10

daca x < y atunci
    scrie "x este mai mic decat y"
sf
```

### Logici

| Operator | Descriere |
|----------|-----------|
| `si` / `SI` | ȘI logic |
| `sau` / `SAU` | SAU logic |
| `not` / `NOT` | Negație |

```pseudo
hp_jucator <- 80
hp_inamic <- 0

daca hp_jucator > 0 si hp_inamic > 0 atunci
    scrie "Fight!"
sf

daca hp_jucator <= 0 sau hp_inamic <= 0 atunci
    scrie "Game Over!"

    daca not hp_jucator <= 0 atunci
        scrie "You Win!"
    altfel
        scrie "You Lost!"
    sf
sf
```

### Interschimbare (swap)

Operatorul `<->` (sau `<-->`) schimbă valorile a două variabile:

```pseudo
a <- 5
b <- 10

a <-> b

scrie a    # 10
scrie b    # 5
```

## Intrare / Ieșire

### Afișare — `scrie`

```pseudo
scrie "Salut, lume!"

x <- 42
scrie "Valoarea: ", x

scrie "Suma: ", (a + b)
```

Argumentele multiple sunt separate prin virgulă.

### Citire — `citeste`

```pseudo
scrie "Numele tau:"
citeste nume
scrie "Salut, ", nume

# Mai multe variabile
scrie "Doua numere:"
citeste a, b
```

> **Tip:** La citire multiplă, fiecare variabilă se introduce separat.

## Structuri Condiționale

### Daca-Atunci

```pseudo
daca conditie atunci
    # instructiuni
sf
```

Exemplu:

```pseudo
nota <- 7

daca nota >= 5 atunci
    scrie "Promovat!"
sf
```

### Daca-Atunci-Altfel

```pseudo
daca conditie atunci
    # ramura adevarat
altfel
    # ramura fals
sf
```

Exemplu:

```pseudo
varsta <- 16

daca varsta >= 18 atunci
    scrie "Major"
altfel
    scrie "Minor"
sf
```

### Condiții Imbricate

```pseudo
nota <- 8

daca nota >= 9 atunci
    scrie "Excelent"
altfel
    daca nota >= 7 atunci
        scrie "Bine"
    altfel
        daca nota >= 5 atunci
            scrie "Suficient"
        altfel
            scrie "Insuficient"
        sf
    sf
sf
```

## Structuri Repetitive

### Bucla Pentru

Iterează pe un interval cu pas opțional:

```pseudo
pentru var <- start, stop executa
    # instructiuni
sf

pentru var <- start, stop, pas executa
    # instructiuni
sf
```

Exemple:

```pseudo
# De la 1 la 5
pentru i <- 1, 5 executa
    scrie i
sf

# Numere pare
pentru i <- 2, 10, 2 executa
    scrie i
sf

# Numărătoare inversă
pentru i <- 10, 1, -1 executa
    scrie i
sf
```

### Bucla Cat Timp (while)

Execută cât timp condiția e adevărată. Verifică condiția **înainte** de fiecare iterație:

```pseudo
cat timp conditie executa
    # instructiuni
sf
```

Exemplu:

```pseudo
x <- 5

cat timp x > 0 executa
    scrie x
    x <- x - 1
sf
```

> **Note:** Poate să nu execute deloc dacă condiția e falsă de la început.

### Bucle Do-While

Pseudocodul oferă două variante de buclă do-while care execută corpul **cel puțin o dată**:

#### Varianta 1: Repeta...Pana Cand

Se oprește când condiția devine **adevărată**:

```pseudo
repeta
    # instructiuni
pana cand conditie
```

```pseudo
repeta
    scrie "Numar pozitiv:"
    citeste n
pana cand n > 0
# Se oprește când n > 0 (condiție adevărată)
```

#### Varianta 2: Executa...Cat Timp

Continuă cât timp condiția e **adevărată**:

```pseudo
executa
    # instructiuni
cat timp conditie
```

```pseudo
executa
    scrie "Numar pozitiv:"
    citeste n
cat timp n <= 0
# Continuă cât timp n <= 0 (condiție adevărată)
```

#### Comparație

| | `repeta...pana cand` | `executa...cat timp` |
|--|----------------------|----------------------|
| Se oprește când | condiția e **adevărată** | condiția e **falsă** |
| Continuă când | condiția e **falsă** | condiția e **adevărată** |

> **Tip:** Cele două forme sunt echivalente! Pentru a converti una în cealaltă, pune `not` înaintea condiției:
> - `pana cand x > 0` ⟺ `cat timp not (x > 0)`
> - `cat timp x <= 0` ⟺ `pana cand not (x <= 0)`

## Comentarii

Folosiți `#` pentru comentarii:

```pseudo
# Acesta este un comentariu
x <- 10  # comentariu inline

# Comentariile sunt ignorate
# la execuție
```

## Referință Rapidă

### Cuvinte Cheie

| Cuvânt | Scop |
|--------|------|
| `daca` | Început condiție |
| `atunci` | Bloc then |
| `altfel` | Bloc else |
| `sf` | Sfârșit bloc |
| `pentru` | Buclă for |
| `cat timp` | Buclă while |
| `executa` | Început corp buclă |
| `repeta` | Buclă do-while |
| `pana cand` | Condiție until |
| `citeste` | Citire |
| `scrie` | Scriere |

### Toți Operatorii

| Operator | Descriere |
|----------|-----------|
| `<-` | Atribuire |
| `<->` sau `<-->` | Interschimbare |
| `+` `-` `*` `/` `%` | Aritmetici |
| `[x]` | Parte întreagă |
| `√` | Radical |
| `=` `!=` `<` `>` `<=` `>=` | Comparație |
| `si` `sau` `not` | Logici |
