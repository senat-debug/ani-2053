# Comptage des sources — dépôt Nkentseu vs. le cours (rapport final)

> **Dépôt** : `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu` — HEAD `dc2154ed` (2026-09-12) + 27 modifications non commitées.
> **Date du comptage** : 2026-09-14.
> **Outils** : Windows PowerShell 5.1.22621, git 2.53.0, GNU `wc` 8.32 (Git Bash). Aucun outil de métrique (`cloc`, `tokei`…).
> **Où taper les commandes** : VS Code › *Terminal › Nouveau terminal*, qui s'ouvre à la racine du dépôt. Les commandes `powershell` se tapent dans le terminal PowerShell, les commandes `bash` dans le terminal *Git Bash* (menu ⌄ à côté du `+`).
> **Fichier source** = en-têtes (`.h .hpp .hh .hxx .inl .inc .ipp .tpp`) + implémentations (`.c .cc .cpp .cxx .m .mm`).
> **Ligne** = ligne brute (lignes vides et commentaires compris), comme `wc -l`.

## 1. Réponse courte

| | Chapitre 1 | Mon comptage au commit du cours | Mon comptage aujourd'hui |
|---|---:|---:|---:|
| NKGui : fichiers source | 13 | **13** | 15 |
| NKGui : lignes | ≈ 7 780 | **7 780** | 9 104 |
| `NkGuiWidgets.cpp` : lignes | 4 973 | **4 973** | 5 192 |
| Dépôt entier | *aucun chiffre* | — | 2 703 fichiers · 1 248 120 lignes (hors `Externals/`) |

- Le chapitre ne donne **aucun total pour le dépôt**, seulement des chiffres sur NKGui.
- Au commit du cours, je retrouve ses chiffres **à la ligne près**. Il a compté **les en-têtes et les `.cpp` de `src/`, lignes vides comprises**. NKGui n'avait **ni dossier Build ni tests**.
- Aujourd'hui, l'écart (+2 fichiers, +1 324 lignes) vient **du code ajouté après la rédaction**, pas de la méthode.

---

## 2. Préparation (à lancer une fois, dans le même terminal)

Cette commande définit les extensions, une fonction `Compte` qui affiche *fichiers / en-têtes / .c-.cpp / lignes / non vides*, et deux listes : `$tout` (tout le disque sauf `.git`) et `$projet` (sans `Externals`).

```powershell
# P — préparation
$ext = '.h','.hpp','.hh','.hxx','.inl','.inc','.ipp','.tpp','.c','.cc','.cpp','.cxx','.m','.mm'; $impl = '.c','.cc','.cpp','.cxx','.m','.mm'; function Compte($f) { $f = @($f); $l = ($f | ForEach-Object { [IO.File]::ReadAllLines($_.FullName).Count } | Measure-Object -Sum).Sum; $nv = ($f | ForEach-Object { @([IO.File]::ReadAllLines($_.FullName) | Where-Object { $_.Trim() }).Count } | Measure-Object -Sum).Sum; "{0} fichiers | {1} en-tetes | {2} .c/.cpp | {3} lignes | {4} non vides" -f $f.Count, @($f | Where-Object { $impl -notcontains $_.Extension.ToLower() }).Count, @($f | Where-Object { $impl -contains $_.Extension.ToLower() }).Count, [int]$l, [int]$nv }; $tout = Get-ChildItem -Recurse -File | Where-Object { $ext -contains $_.Extension.ToLower() -and $_.FullName -notmatch '\\\.git\\' }; $projet = $tout | Where-Object { $_.FullName -notmatch '\\Externals\\' }
```

Comment les lignes sont comptées : `[IO.File]::ReadAllLines()` lit le fichier et `.Count` donne le nombre de lignes. Les lignes non vides sont filtrées par `Where-Object { $_.Trim() }`.

---

## 3. Comptage du dépôt entier

### 3.1 Totaux

```powershell
# M1 — tout le disque (projet + Externals)
Compte $tout
# M2 — bibliothèques tierces
Compte ($tout | Where-Object { $_.FullName -match '\\Externals\\' })
# M3 — code du projet (hors Externals)
Compte $projet
# M4 — fichiers suivis par git
$suivis = git -c core.quotepath=off ls-files | Where-Object { $ext -contains [IO.Path]::GetExtension($_).ToLower() }; Compte ($suivis | ForEach-Object { Get-Item -LiteralPath $_ })
# M5 — fichiers présents sur le disque mais non suivis par git
$chemins = @{}; $suivis | ForEach-Object { $chemins[$_.Replace('/','\')] = 1 }; Compte ($tout | Where-Object { -not $chemins.ContainsKey($_.FullName.Substring($PWD.Path.Length + 1)) })
```

| ID | Périmètre | Fichiers | En-têtes | .c/.cpp | Lignes | Non vides |
|---|---|---:|---:|---:|---:|---:|
| M1 | Tout le disque | 5 221 | 2 894 | 2 327 | 2 972 716 | 2 648 263 |
| M2 | `Externals/` | 2 518 | 1 406 | 1 112 | 1 724 596 | 1 510 099 |
| M3 | **Projet (hors `Externals/`)** | **2 703** | 1 488 | 1 215 | **1 248 120** | 1 138 164 |
| M4 | Suivis par git | 3 320 | 1 934 | 1 386 | 1 598 162 | 1 445 625 |
| M5 | Non suivis | 1 901 | 960 | 941 | 1 374 554 | 1 202 638 |

- M4 + M5 = M1. Les 1 901 fichiers non suivis sont tous dans `Externals/` : NKAssimp, Vulkan-Headers, GLSlang, Glad, SPIRV-Cross, ImGui…
- `Externals/` représente **58 %** des lignes sur le disque.

### 3.2 En-têtes

```powershell
# M6 — projet, en-têtes seuls
Compte ($projet | Where-Object { $impl -notcontains $_.Extension.ToLower() })
# M7 — projet, .c/.cpp seuls
Compte ($projet | Where-Object { $impl -contains $_.Extension.ToLower() })
```

| ID | Périmètre | Fichiers | Lignes | Non vides |
|---|---|---:|---:|---:|
| M6 | Projet, en-têtes seuls | 1 488 | 613 196 | 558 256 |
| M7 | Projet, `.c/.cpp` seuls | 1 215 | 634 924 | 579 908 |

Les en-têtes font **55 % des fichiers** et **49 % des lignes** du projet. Oublier les en-têtes divise donc le total par deux.

### 3.3 Tests

```powershell
# M8 — tests du projet (règle large : tests\, _Tests, test*.cpp, Unitest)
$regleTests = '(?i)(\\tests?\\|_tests?\\|_tests?\.|\\unittests?\\|test[^\\]*\.(c|cc|cpp|h|hpp)$|\\__Unitest__\\|\\Unitest\\)'; Compte ($projet | Where-Object { $_.FullName -match $regleTests })
# M9 — projet hors tests
Compte ($projet | Where-Object { $_.FullName -notmatch $regleTests })
# M10 — projet hors tests, .c/.cpp seuls
Compte ($projet | Where-Object { $_.FullName -notmatch $regleTests -and $impl -contains $_.Extension.ToLower() })
# M11 — tests, règle simple (dossiers tests\ seulement)
Compte ($projet | Where-Object { $_.FullName -match '\\tests?\\' })
# M12 — tests dans Externals
Compte ($tout | Where-Object { $_.FullName -match '\\Externals\\' -and $_.FullName -match $regleTests })
```

| ID | Périmètre | Fichiers | En-têtes | .c/.cpp | Lignes | Non vides |
|---|---|---:|---:|---:|---:|---:|
| M8 | Tests du projet (règle large) | 106 | 6 | 100 | 18 888 | 16 446 |
| M9 | Projet hors tests | 2 597 | 1 482 | 1 115 | 1 229 232 | 1 121 718 |
| M10 | Projet hors tests, `.c/.cpp` seuls | 1 115 | 0 | 1 115 | 617 269 | 564 552 |
| M11 | Tests, règle simple `\tests\` | 94 | 4 | 90 | 13 509 | 11 705 |
| M12 | Tests dans `Externals/` | 339 | 53 | 286 | 78 960 | 68 026 |

Les tests pèsent **1,5 %** des lignes du projet. La règle choisie change le résultat : 106 ou 94 fichiers selon la règle.

### 3.4 Dossier Build

```powershell
# D1 — tous les dossiers de build (nom exact, sensible à la casse)
Get-ChildItem -Recurse -Directory -Force -ErrorAction SilentlyContinue | Where-Object { $_.Name -cmatch '^(Build|build|bin|obj|out|Intermediate)$' -and $_.FullName -notmatch '\\\.git(\\|$)' } | ForEach-Object { $_.FullName.Substring($PWD.Path.Length + 1) }
# D2 — ce que .gitignore exclut
Select-String -Path .gitignore -Pattern '^\s*/?\[?[Bb]\]?uild','^\s*\[Bb\]in','^\s*\[Oo\]bj' | Select-Object -First 8 | ForEach-Object { "$($_.LineNumber): $($_.Line)" }
# M13 — sources situées dans un dossier de build (sensible à la casse)
Compte ($tout | Where-Object { $_.FullName -cmatch '\\(Build|build|bin|obj|out|Intermediate)\\' })
# M14 — la même chose avec -match (piège : insensible à la casse)
Compte ($tout | Where-Object { $_.FullName -match '\\(build|bin|obj|out|intermediate)\\' })
```

| ID | Résultat |
|---|---|
| D1 | Un seul dossier : `Externals\Libs\NKSPIRVCross\build`. **Aucun `Build/` à la racine.** |
| D2 | `.gitignore` ligne 79 `Build/`, ligne 80 `build/`, ligne 152 `[Bb]in/`, ligne 153 `[Oo]bj/` |
| M13 | 3 fichiers, 1 673 lignes (restes CMake de SPIRV-Cross) |
| M14 | 13 fichiers, 5 476 lignes : `-match` ignore la casse et attrape les dossiers `Obj/` d'Assimp, qui sont du **vrai code** |

Le dossier Build ne pèse rien ici : il n'existe pas sur cette copie, et git l'ignore de toute façon.

### 3.5 Répartition et données embarquées

```powershell
# M15 — par dossier de premier niveau
$projet | Group-Object { $_.FullName.Substring($PWD.Path.Length + 1).Split('\')[0] } | Sort-Object Count -Descending | ForEach-Object { "{0,-14} {1}" -f $_.Name, (Compte $_.Group) }
# M16 — les 8 plus gros fichiers
$projet | ForEach-Object { [pscustomobject]@{ Lignes = [IO.File]::ReadAllLines($_.FullName).Count; Fichier = $_.FullName.Substring($PWD.Path.Length + 1) } } | Sort-Object Lignes -Descending | Select-Object -First 8 | Format-Table -AutoSize
# M17 — données générées embarquées (polices, modèle de ciel)
$data = $projet | Where-Object { $_.Name -match '_data\.h$|Embedded\.cpp$|ArHosekSkyModelData' }; $data | ForEach-Object { "{0,7}  {1}" -f [IO.File]::ReadAllLines($_.FullName).Count, $_.FullName.Substring($PWD.Path.Length + 1) }; Compte $data
```

**M15**

| Dossier | Fichiers | Lignes |
|---|---:|---:|
| `Kernel/` | 1 625 | 777 557 |
| `Applications/` | 867 | 388 705 |
| `Engine/` | 185 | 47 916 |
| `scripts/` | 2 | 28 241 |
| `Integrations/` | 9 | 2 920 |
| `Tutoriels3D/` | 6 | 1 379 |
| `Sandbox/` | 4 | 601 |
| `Resources/` | 3 | 423 |
| `preview.c` + `main.cpp` (racine) | 2 | 378 |

**M16** : les plus gros fichiers sont surtout des données. `ArHosekSkyModelData_Spectral.h` fait 33 770 lignes, `NkFontEmbedded.cpp` 20 550, `NkDemo3D.cpp` 20 415, `NotoSans_data.h` 17 846…

**M17** : **8 fichiers, 113 082 lignes** de données générées, soit **9,2 %** du projet hors tests. `Inter_data.h` est présent en double, dans `scripts/Embedded/` et dans `NKFont/Embedded/`.

---

## 4. Les chiffres du chapitre

```powershell
# A3 — quand le cours a-t-il été écrit ?
git log --format="%h %ad %s" --date=short -- Documentation/cours/md/01-le-decor.md
# A4 — les chiffres du chapitre 1
Select-String -Path Documentation\cours\md\01-le-decor.md -Pattern '7.780','4.973' -Encoding UTF8 | ForEach-Object { "$($_.LineNumber): $($_.Line)" }
# A5 — le chiffre du chapitre 2 (NKCanvas)
Select-String -Path Documentation\cours\md\02-nkcanvas.md -Pattern '23.400' -Encoding UTF8 | ForEach-Object { "$($_.LineNumber): $($_.Line)" }
```

| ID | Résultat |
|---|---|
| A3 | `1a2d9551 2026-08-05 Documentation : cours complet NkCanvas & NKGui`. Aucune modification des chiffres depuis. |
| A4 | ligne 308 : « 7 780 lignes réparties sur 13 fichiers source » · ligne 310 : « seul 4 973 lignes » |
| A5 | ligne 28 : « Environ 23 400 lignes réparties sur 101 fichiers » |

---

## 5. NKGui : le chapitre vs. mon comptage

### 5.1 Commandes

```powershell
# B1 — NKGui au commit du cours, fichier par fichier (lu dans git, sans toucher au dossier)
git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui/src | ForEach-Object { "{0,6}  {1}" -f @(git show "1a2d9551:$_").Count, $_ }
# B2 — total au commit
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui/src; "$(@($n).Count) fichiers, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_").Count } | Measure-Object -Sum).Sum) lignes"
# B3 — au commit, sans les en-têtes
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui/src | Where-Object { $_ -like '*.cpp' }; "$(@($n).Count) fichiers, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_").Count } | Measure-Object -Sum).Sum) lignes"
# B4 — au commit, sans les lignes vides
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui/src; "$((@($n) | ForEach-Object { @(git show "1a2d9551:$_" | Where-Object { $_.Trim() }).Count } | Measure-Object -Sum).Sum) lignes non vides"
# B5 — y avait-il des tests ou un Build dans NKGui ? (tout ce qui n'est pas dans src/)
git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui | Where-Object { $_ -notlike '*/src/*' }
# B6 — historique de NkGuiWidgets.cpp
git log --format="%h %ad" --date=short -- Kernel/Runtime/NKGui/src/NKGui/Widgets/NkGuiWidgets.cpp | Select-Object -First 5 | ForEach-Object { $h = $_.Split(' ')[0]; "$_  $(@(git show "${h}:Kernel/Runtime/NKGui/src/NKGui/Widgets/NkGuiWidgets.cpp").Count) lignes" }
# B7 — NKGui aujourd'hui, fichier par fichier
Get-ChildItem Kernel\Runtime\NKGui\src -Recurse -File | ForEach-Object { "{0,6}  {1}" -f [IO.File]::ReadAllLines($_.FullName).Count, $_.Name }
# B8 — total aujourd'hui
$f = Get-ChildItem Kernel\Runtime\NKGui\src -Recurse -File -Include *.h,*.cpp; "$($f.Count) fichiers, $(($f | ForEach-Object { [IO.File]::ReadAllLines($_.FullName).Count } | Measure-Object -Sum).Sum) lignes"
# B9 — ce qui a changé depuis le cours
git diff --shortstat 1a2d9551 HEAD -- Kernel/Runtime/NKGui/src
```

Contre-épreuve avec un autre outil, `wc -l` dans Git Bash :

```bash
# E1 — NKGui au commit du cours
git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKGui/src | while read -r f; do git show "1a2d9551:$f" | wc -l; done | awk '{s+=$1; n++} END {print n" fichiers, "s" lignes"}'
# E2 — NKGui aujourd'hui
git ls-files -- Kernel/Runtime/NKGui/src | tr '\n' '\0' | xargs -0 cat | wc -l
```

### 5.2 Résultats

| Fichier (`src/NKGui/`) | Chapitre | Au commit (B1) | Aujourd'hui (B7) | Écart |
|---|---:|---:|---:|---:|
| `Core/NkGuiContext.cpp` | 573 | 573 | 729 | +156 |
| `Core/NkGuiContext.h` | 551 | 551 | 648 | +97 |
| `Core/NkGuiDrawList.cpp` | 342 | 342 | 498 | +156 |
| `Core/NkGuiDrawList.h` | 101 | 101 | 127 | +26 |
| `Core/NkGuiFont.cpp` | 158 | 158 | 158 | 0 |
| `Core/NkGuiFont.h` | 81 | 81 | 88 | +7 |
| `Core/NkGuiInput.h` | 145 | 145 | 190 | +45 |
| `Core/NkGuiTypes.h` | 333 | 333 | 342 | +9 |
| `NKGui.h` | — | 35 | 35 | 0 |
| `NkGuiApi.h` | — | 80 | 80 | 0 |
| `NkGuiExport.h` | — | 3 | 35 | +32 |
| `Widgets/NkGuiWidgets.cpp` | **4 973** | **4 973** | **5 192** | +219 |
| `Widgets/NkGuiWidgets.h` | 405 | 405 | 480 | +75 |
| `Core/NkGuiIcons.cpp` | — | *absent* | 282 | +282 |
| `Core/NkGuiIcons.h` | — | *absent* | 220 | +220 |
| **Total** | **≈ 7 780 · 13 fichiers** | **7 780 · 13 fichiers** (B2) | **9 104 · 15 fichiers** (B8) | **+1 324** |

| ID | Résultat |
|---|---|
| B3 | Sans en-têtes : 4 fichiers, 6 046 lignes |
| B4 | Sans lignes vides : 7 207 lignes |
| B5 | Hors `src/`, seulement `ARCHITECTURE.md`, `NKGui.jenga`, `README.md`, `ROADMAP.md` : **pas de tests, pas de Build** |
| B6 | `NkGuiWidgets.cpp` : 4 971 (23/07) → **4 973 (30/07)** → 5 095 (17/08) → 5 103 (18/08) → 5 192 (18/08) |
| B9 | 14 fichiers modifiés, +1 414 / −90 lignes, soit **+1 324** nettes |
| E1 | `13 fichiers, 7780 lignes` : identique à B2 |
| E2 | `9104` : identique à B8 |

---

## 6. Les trois questions

| Question | Pour le chiffre du chapitre (NKGui) | Pour le dépôt entier |
|---|---|---|
| **Les en-têtes sont-ils comptés ?** | **Oui** : 9 des 13 fichiers sont des `.h`. Sans eux, 6 046 lignes au lieu de 7 780 (B3). | Ils font 49 % des lignes : 613 196 sur 1 248 120 (M6). |
| **Le dossier Build est-il compté ?** | **Non**, et NKGui n'en a pas (B5). | Pas de `Build/` à la racine (D1), ignoré par git (D2). Seulement 3 restes CMake dans `Externals/` (M13). |
| **Les fichiers de test sont-ils comptés ?** | **Sans objet** : NKGui n'a aucun test (B5). | 106 fichiers, 18 888 lignes, soit 1,5 % (M8). |
| *En plus : les lignes vides ?* | **Oui**. Sans elles, 7 207 lignes (B4). | 109 956 lignes vides, soit 8,8 % (M3). |
| *En plus : `Externals/` ?* | Sans objet. | Inclure `Externals/` multiplie le total par **2,4** (M1 / M3). |

---

## 7. Pour comparaison : NKCanvas (chapitre 2)

```powershell
# C1 — code de src/ au commit du cours
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKCanvas/src | Where-Object { $_ -match '\.(h|inl|cpp|mm)$' }; "$(@($n).Count) fichiers, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_").Count } | Measure-Object -Sum).Sum) lignes"
# C2 — code de tout le dossier (src + pch) au commit
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKCanvas | Where-Object { $_ -match '\.(h|inl|cpp|mm)$' }; "$(@($n).Count) fichiers, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_").Count } | Measure-Object -Sum).Sum) lignes"
# C3 — tout le dossier, tous types de fichiers, au commit
$n = git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKCanvas; "$(@($n).Count) fichiers, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_").Count } | Measure-Object -Sum).Sum) lignes, $((@($n) | ForEach-Object { @(git show "1a2d9551:$_" | Where-Object { $_.Trim() }).Count } | Measure-Object -Sum).Sum) non vides"
# C4 — les fichiers qui ne sont pas du code
git ls-tree -r --name-only 1a2d9551 -- Kernel/Runtime/NKCanvas | Where-Object { $_ -notmatch '\.(h|inl|cpp|mm)$' }
# C5 — code de src/ aujourd'hui
$f = Get-ChildItem Kernel\Runtime\NKCanvas\src -Recurse -File -Include *.h,*.inl,*.cpp,*.mm; "$($f.Count) fichiers, $(($f | ForEach-Object { [IO.File]::ReadAllLines($_.FullName).Count } | Measure-Object -Sum).Sum) lignes"
```

| ID | Règle | Fichiers | Lignes |
|---|---|---:|---:|
| — | **Chapitre 2** | **101** | **≈ 23 400** |
| C1 | Code de `src/`, au commit | 95 | 24 381 |
| C2 | Code avec `pch/`, au commit | 97 | 24 393 |
| C3 | Tout le dossier, au commit | **101** | 26 065 (22 658 non vides) |
| C5 | Code de `src/`, aujourd'hui | 101 | 27 285 |

- **101 fichiers** ne se retrouve qu'en comptant tout le dossier, avec les 4 fichiers non-code de C4 (`NKCanvas.jenga`, `ROADMAP.md`, `USAGE.md`, `Backend/Software/ROADMAP.md`). Ce n'est donc pas un compte de *fichiers source*.
- **≈ 23 400 lignes** ne se retrouve avec **aucune** règle. Au plus près : 22 658 (−742) ou 24 381 (+981). Le chiffre vient d'une note (`Documentation/notes_nkcanvas_nkgui.md`) relevée sur une autre copie du dépôt (`D:\Projets\2026\Nkentseu`).
- Les 101 fichiers de C5 sont une coïncidence : 95 fichiers au commit, plus 6 fichiers `App/` ajoutés depuis.

---

## 8. Conclusion : pourquoi les chiffres diffèrent

1. **Le chapitre et moi ne mesurons pas la même chose.** Il mesure un module (NKGui), et je mesure le dépôt : **9 104 lignes sur 1 229 232**, soit 0,7 % du projet hors tests.
2. **Sur le même périmètre et à la même date, les chiffres sont identiques** : 13 fichiers, 7 780 lignes, 4 973 pour `NkGuiWidgets.cpp`. Deux outils différents le confirment (PowerShell et `wc -l`).
3. **L'écart d'aujourd'hui est temporel.** Depuis le cours, 14 fichiers ont été modifiés ou créés, dont les 2 nouveaux `NkGuiIcons`, pour **+1 324 lignes nettes** (B9). L'essentiel date des 17–18/08, 12–13 jours après le cours (B6).
4. **Pour le dépôt entier, le résultat dépend surtout de la règle choisie** : de 617 269 lignes (M10 : `.c/.cpp`, hors tests, hors `Externals`) à 2 972 716 (M1 : tout le disque), soit un facteur **4,8**. Les en-têtes et `Externals/` pèsent le plus. Les tests (1,5 %) et le Build (≈ 0) pèsent peu.

## 9. Limites

- Tests repérés par le **chemin**, pas par le contenu : deux règles donnent 106 ou 94 fichiers (M8, M11).
- Dépôt compté **dans son état actuel**, 27 modifications non commitées comprises. Seuls NKGui et NKCanvas ont été recomptés au commit du cours.
- Lignes **brutes** : les commentaires sont comptés, contrairement à `cloc`.
- Un fichier sans saut de ligne final compte pour une ligne de plus qu'avec `wc -l`. Aucun écart n'a été observé sur NKGui (E1 = B2, E2 = B8).