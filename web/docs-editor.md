# Pseudo++

## Despre Proiect

Pseudo++ este un interpretor și editor pentru limbajul pseudocod, facut pentru 
elevi si profesorii de informatică din România.

Codul sursă este disponibil pe [GitHub](https://github.com/BujorelActimel/pseudo).

### Filozofie

Cred că programarea se învață făcând, nu privind. Ciclul e simplu:

1. Citești problema
2. Te gândești la o soluție
3. O scrii
4. O testezi
5. Nu merge? Repeti de la 2.

Acest proces nu se poate face cu pixul pe hârtie. Ai nevoie să vezi ce face codul tău, să experimentezi, să greșești și să încerci din nou.

### Privat și Offline

Pseudo++ funcționează **complet în browser**, fără server sau conexiune la internet după încărcarea inițială:

- **Zero analytics** — nu colectăm date
- **Zero telemetrie** — nimic nu părăsește browserul
- **Offline-ready** — odată încărcat, funcționează fără internet
- **Self-hosted** — poate fi hostat pe orice server static

### Self-Hosting

Pentru a hosta propria instanță, descarcati arhiva `pseudo-web.zip` din sectiunea `release` 
de la repoul de pe [github](https://github.com/BujorelActimel/pseudo) si porniti orice 
server HTTP static din directorul(folderul) in care ati dezarhivat arhiva.

---

## Interfața

Editorul este împărțit în două panouri principale:

- **Editor** (stânga) — Aici scrieți codul pseudocod
- **Consolă** (dreapta) — Aici apare output-ul și introduceți input-ul

Puteți redimensiona panourile trăgând de separatorul dintre ele.

### Tab-uri pentru Fișiere

Editorul suportă mai multe fișiere prin sistem de tab-uri:

| Acțiune | Cum |
|---------|-----|
| Fișier nou | Click pe butonul `+` |
| Redenumire | Dublu-click pe tab |
| Închidere | Click pe `×` din tab |

> **Tip:** Fișierele sunt salvate automat în browser (localStorage) și vor fi restaurate la următoarea vizită.

## Execuție

### Butoane de Control

| Buton | Funcție | Descriere |
|-------|---------|-----------|
| Play (verde) | **Run** | Execută programul normal |
| Bug (galben) | **Debug** | Execută cu indicatori pas-cu-pas |
| Stop (roșu) | **Stop** | Oprește execuția curentă |

### Modul Debug

*Work in progress.*

Momentan, în modul debug, în consolă apar mesaje care indică linia curentă executată.

## Consola

### Output

Rezultatele comenzii `scrie` apar în consolă. Fiecare apel `scrie` generează o linie nouă.

### Input

Când programul ajunge la o comandă `citeste`:

1. Apare un prompt `>` în consolă
2. Tastați valoarea dorită
3. Apăsați `Enter` pentru a trimite valoarea

> **Note:** Dacă programul pare blocat, verificați dacă așteaptă input (căutați promptul `>`).

### Curățare

Butonul `×` din header-ul consolei șterge tot conținutul afișat.

## Scurtături de Tastatură

### Editare

| Scurtătură | Acțiune |
|------------|---------|
| `Ctrl` + `Z` | Anulează (undo) |
| `Ctrl` + `Shift` + `Z` | Refă (redo) |
| `Ctrl` + `X` | Taie selecția |
| `Ctrl` + `C` | Copiază selecția |
| `Ctrl` + `V` | Lipește |
| `Ctrl` + `A` | Selectează tot |

### Linii

| Scurtătură | Acțiune |
|------------|---------|
| `Ctrl` + `/` | Comentează/decomentează linia |
| `Alt` + `↑` | Mută linia în sus |
| `Alt` + `↓` | Mută linia în jos |
| `Ctrl` + `Shift` + `K` | Șterge linia |
| `Ctrl` + `Enter` | Inserează linie dedesubt |

### Căutare

| Scurtătură | Acțiune |
|------------|---------|
| `Ctrl` + `F` | Caută |
| `Ctrl` + `H` | Caută și înlocuiește |
| `F3` / `Enter` | Următoarea apariție |
| `Shift` + `F3` | Apariția anterioară |
| `Ctrl` + `D` | Selectează următoarea apariție a cuvântului |

### Navigare

| Scurtătură | Acțiune |
|------------|---------|
| `Ctrl` + `G` | Mergi la linia... |
| `Ctrl` + `Home` | Mergi la începutul fișierului |
| `Ctrl` + `End` | Mergi la sfârșitul fișierului |
| `Ctrl` + `←` / `→` | Sari peste cuvânt |

## Persistența Datelor

Editorul salvează automat:

- **Fișierele deschise** — conținutul și numele
- **Tab-ul activ** — care fișier era selectat
- **Dimensiunea panourilor** — proporția editor/consolă

Datele sunt stocate în `localStorage` și persistă între sesiuni. Pentru a reseta:

1. Deschideți DevTools (`F12`)
2. Tab-ul Application → Local Storage
3. Ștergeți intrarea `pseudo_editor_state`

---

## Detalii Tehnice

Pseudo++ este format din două componente principale:

1. **Interpretorul** — scris în C, rulează direct în browser
2. **Editorul** — bazat pe [Monaco Editor](https://microsoft.github.io/monaco-editor/) (același editor ca în VS Code)

Totul rulează local în browser, codul tău nu este trimis nicăieri.

Pentru mai multe detalii tehnice, vezi [repository-ul GitHub](https://github.com/BujorelActimel/pseudo).
