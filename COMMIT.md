## Format des commits

### Grammaire BNF

```
<commit> ::= ["!"] <type> "(" <target> [":" <specifier>] ")" [":" <description>] [";" <commit>]

<type_error> ::=
  | "compilation"
  | "runtime"
  | "segfault"
  | "memory"
  | "undefined"
  | "error"
  | "warning"
  | "assert"
  | "exception"
  | "panic"

<type> ::=
  | "add"
  | "fix"
  | "refactor"
  | "docs"
  | "test"
  | "style"
  | "perf"
  | "chore"
  | <type_error>

<target> ::=
  | <file>
  | <feature>
  | <directory>

<file> ::= r"[^/\0]+"

<directory> ::= r"[^/\0]+(/[^/\0]+)*/"

<feature> ::= <identifier>

<specifier> ::=
  | <line_number>
  | <function_name>

<line_number> ::= r"[1-9][0-9]*"

<function_name> ::= <identifier>

<identifier> ::= r"[a-zA-Z_][a-zA-Z0-9_]*"

<description> ::= r"[^;]+"
```

### Explication des éléments

- **`!`** *(optionnel)* : Indique que le code ne compile pas ou produit une erreur. Si l'erreur est connue, elle peut être précisée comme un élément de type `type_error`.
- **`type_error`** *(optionnel)* : Indique le type d'erreur rencontrée.
  - `compilation` : Erreur de compilation.
  - `runtime` : Erreur d'exécution.
  - `segfault` : Erreur de segmentation.
  - `memory` : Problème de mémoire.
  - `undefined` : Comportement indéfini.
  - `error` : Autre erreur.
  - `warning` : Avertissement.
  - `assert` : Échec d'une assertion.
  - `exception` : Exception levée.
  - `panic` : État critique du programme.
- **`<type>`** : Indique la nature de la modification.
  - `add` : Ajout de code ou de fichiers.
  - `fix` : Correction de bug.
  - `refactor` : Réorganisation du code sans changement fonctionnel.
  - `docs` : Ajout ou modification de documentation.
  - `test` : Ajout ou modification de tests.
  - `style` : Changement de formatage sans impact sur le code.
  - `perf` : Amélioration des performances.
  - `chore` : Maintenance du projet sans impact direct sur le code.
  - `rough` : Pour les fichiers ou le code temporaire, d'expérimentation ou de brouillon.
- **`<target>`** : Spécifie le fichier ou la fonctionnalité concernée.
- **`<specifier>`** *(optionnel)* : Précise une ligne ou une fonction ciblée.
- **`<description>`** *(optionnel)* : Fournit un message explicatif.
- **Les commits multiples** peuvent être séparés par `; `.

### Exemples

- **Ajout d'un fichier de documentation**
  ```sh
  git commit -m "add(README.md) : Ajout du fichier README"
  ```

- **Correction d'un bug sur une ligne spécifique**
  ```sh
  git commit -m "fix(main.c:42) : Correction du segfault ligne 42"
  ```

- **Ajout d'une nouvelle fonction**
  ```sh
  git commit -m "add(utils.c:parse_input) : Ajout de la fonction parse_input"
  ```

- **Refactorisation d'une fonction**
  ```sh
  git commit -m "refactor(server.c:handle_request) : Simplification de la logique"
  ```

- **Modification du style (indentation, espaces, etc.)**
  ```sh
  git commit -m "style(config.py) : Correction des indentations"
  ```

- **Amélioration des performances d'une boucle**
  ```sh
  git commit -m "perf(matrix.c:multiply) : Optimisation de la multiplication des matrices"
  ```

- **Ajout de tests unitaires**
  ```sh
  git commit -m "test(math_utils.c) : Ajout des tests pour les fonctions mathématiques"
  ```

- **Modification de plusieurs fichiers dans un seul commit**
  ```sh
  git commit -m "add(server.c:init_server) : Ajout de la fonction init_server; fix(client.c:connect) : Correction de la gestion des erreurs"
  ```

- **Ajout d'une nouvelle fonctionnalité produisant une erreur**
  ```sh
  git commit -m "!add(auth.c:login) : Ajout de la fonction de connexion; error(auth.c:login) : Erreur de segmentation lors de la connexion"
  ```

- **Ajout d'une nouvelle fonctionnalité avec un avertissement**
  ```sh
  git commit -m "add(auth.c:login) : Ajout de la fonction de connexion; warning(auth.c:login) : Avertissement sur l'utilisation de variables non initialisées"
  ```

- **Modification d'une fonctionalité avec une erreur inconnue (ou obvieuse)**
  ```sh
  git commit -m "!fix(auth.c:login) : Correction de la fonction de connexion"
  ```