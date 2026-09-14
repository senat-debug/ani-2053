# Debug ou Release : même projet, deux binaires, et les lignes qui l'expliquent

> **La consigne** : construire le même projet dans les deux configurations, comparer la taille du binaire produit et le temps de construction, puis retrouver dans le `.jenga` les lignes qui expliquent ces quatre nombres.
> **Le projet** : **MonEssai**, le nôtre, construit depuis les exercices précédents. C'est un exécutable console qui n'affiche rien, appelle `CPUFeatures::Get()` et dépend de NKPlatform.
> **Le dépôt** : `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu`, le 2026-09-14. Jenga 2.8.0, cible Windows x86_64, toolchain `clang-mingw`.
> **Les commandes** se tapent à la racine du dépôt, dans le terminal *Git Bash* de VS Code.

---

## 1. Les quatre nombres

| | **Debug** | **Release** | Écart |
|---|---:|---:|---:|
| **Taille de `MonEssai.exe`** | **277 053 octets** | **260 585 octets** | −16 468 octets (−5,9 %) |
| **Temps de construction** (Jenga, moyenne) | **2,26 s** | **2,25 s** | aucun |
| *Temps mesuré au chronomètre (moyenne)* | *7,9 s* | *7,9 s* | *aucun* |

Les deux programmes se lancent et **n'affichent rien** (code de retour 0).

**En une phrase :** le binaire Release n'est que **6 % plus petit** et ne se construit **pas plus vite**. `MonEssai.jenga` ne contient aucune ligne propre à Debug ou à Release. Seule sa dépendance NKPlatform change de réglages, et rien ne retire les informations de débogage de l'exécutable final.

---

## 2. Comment j'ai mesuré

### 2.1 Les commandes

Chaque configuration est construite **depuis zéro** avec `rebuild` (nettoyage puis construction), en alternant Debug et Release :

```bash
jenga rebuild --config Debug   --target MonEssai
jenga rebuild --config Release --target MonEssai
```

Chaque commande est chronométrée comme dans le rapport de chronométrage :

```bash
t0=$(date +%s%3N); jenga rebuild --config Release --target MonEssai > R.log 2>&1; t1=$(date +%s%3N); echo "$((t1-t0)) ms"
```

Tailles et contenu des binaires :

```bash
stat -c '%10s  %n' Build/Bin/Debug-Windows/MonEssai/MonEssai.exe Build/Bin/Release-Windows/MonEssai/MonEssai.exe
objdump -h Build/Bin/Release-Windows/MonEssai/MonEssai.exe
```

`objdump -h` liste les **sections** d'un binaire. Les sections `.text` contiennent le code machine, les sections `.debug_*` les informations de débogage. `objdump` est fourni par MSYS2 : `C:\msys64\ucrt64\bin\objdump.exe`.

### 2.2 Les six constructions

| Essai | Configuration | Temps mesuré | Temps Jenga | NKPlatform | MonEssai | Compilés |
|---|---|---:|---:|---:|---:|---:|
| D1 | Debug | ~~15 254 ms~~ | ~~4,67 s~~ | ~~4,29 s~~ | ~~0,38 s~~ | 8 |
| R1 | Release | 7 740 ms | 2,20 s | 1,95 s | 0,25 s | 8 |
| D2 | Debug | 7 737 ms | 2,11 s | 1,84 s | 0,27 s | 8 |
| R2 | Release | 7 965 ms | 2,28 s | 1,99 s | 0,28 s | 8 |
| D3 | Debug | 8 159 ms | 2,40 s | 2,14 s | 0,27 s | 8 |
| R3 | Release | 8 115 ms | 2,28 s | 2,03 s | 0,25 s | 8 |
| **Moyenne Debug** (D2, D3) | | **7,9 s** | **2,26 s** | **1,99 s** | **0,27 s** | |
| **Moyenne Release** (R1-R3) | | **7,9 s** | **2,25 s** | **1,99 s** | **0,26 s** | |

**D1 est écarté** : c'était la première commande de la série, deux fois plus lente que toutes les autres. C'est probablement un démarrage « à froid », avec des fichiers pas encore en mémoire. Les tailles, elles, sont identiques d'un essai à l'autre, au octet près.

Rappel du rapport de chronométrage : dans ce dépôt, **chaque construction recompile tout**, à cause de l'espace dans le chemin. Ces temps sont donc bien, dans les deux configurations, ceux d'une construction complète.

### 2.3 Ce qu'il y a dans les binaires

| Fichier | Debug | Release |
|---|---:|---:|
| `MonEssai.exe` | 277 053 | 260 585 |
| … dont sections `.debug_*` | **174 990** (63 %) | **163 281** (63 %) |
| … dont code `.text` | 33 760 | 31 664 |
| `NKPlatform.lib` | **44 182** | **12 162** |
| … dont sections `.debug_*` | **20 518** | **504** |
| … dont code `.text` | 4 711 | 2 542 |
| `src_main.obj` (MonEssai) | **3 530** | **3 530** |
| … dont sections `.debug_*` | 1 834 | 1 834 |
| … dont code `.text` | 39 | 39 |

*(tailles en octets)*

Trois observations :
1. **`main.obj` est strictement identique** dans les deux configurations. MonEssai lui-même est compilé exactement de la même façon.
2. **`NKPlatform.lib` est 3,6 fois plus petit en Release** : ses informations de débogage disparaissent presque entièrement (20 518 → 504 octets) et son code rétrécit de 46 %.
3. **L'exécutable Release contient encore 163 Ko d'informations de débogage**, soit 63 % de sa taille. Elles ne viennent ni de `main.obj` (1,8 Ko) ni de NKPlatform (0,5 Ko en Release). Elles viennent donc d'autres objets que la toolchain MinGW ajoute à l'édition de liens.

---

## 3. Les lignes qui expliquent les quatre nombres

### 3.1 Pourquoi il y a deux binaires, côte à côte

**`Nkentseu.jenga`, ligne 457** — le workspace déclare les deux configurations :

```python
    configurations(["Debug", "Release"])
```

**`MonEssai.jenga`, lignes 29-30** — le nom de la configuration entre dans le chemin de sortie :

```python
    objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
    targetdir("%{wks.location}/Build/Bin/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
```

`%{cfg.buildcfg}` vaut `Debug` ou `Release`. Les deux exécutables vivent donc dans deux dossiers, `Build/Bin/Debug-Windows/MonEssai/` et `Build/Bin/Release-Windows/MonEssai/`, et **coexistent sur le disque**. C'est exactement ce que demande l'exercice pratique du chapitre « Le bureau » (`10-le-bureau.tex`, lignes 237-238). `NKPlatform.jenga` fait de même, lignes 33-34.

### 3.2 Taille : ce qui change (NKPlatform)

**`Kernel/Foundation/NKPlatform/NKPlatform.jenga`, lignes 61-68 :**

```python
61      with filter("config:Debug"):
62          defines(["_DEBUG", "DEBUG", "NKENTSEU_DEBUG"])
63          optimize("Off")
64          symbols(True)
65      with filter("config:Release"):
66          defines(["NDEBUG", "NKENTSEU_RELEASE"])
67          optimize("Speed")
68          symbols(False)
```

Jenga traduit ces mots en options de clang (`Core/Builders/Windows.py:646-655`) :

| Ligne du `.jenga` | Option de clang | Effet mesuré sur `NKPlatform.lib` |
|---|---|---|
| 64 `symbols(True)` → 68 `symbols(False)` | `-g` → rien | informations de débogage : **20 518 → 504 octets** |
| 63 `optimize("Off")` → 67 `optimize("Speed")` | `-O0` → `-O2` | code `.text` : **4 711 → 2 542 octets** (−46 %) |
| 62 → 66 `defines(...)` | `-D_DEBUG …` → `-DNDEBUG …` | `NKENTSEU_DEBUG` est lu par `NkPlatformConfig` et `NkFoundationLog` ; son poids exact n'est pas isolé ❓ |

MonEssai n'utilise qu'une partie de `NKPlatform.lib`, les objets nécessaires à `CPUFeatures::Get()`. Dans l'exécutable, le passage en Release retire donc **11 709 octets** de débogage et **2 096 octets** de code, et non 32 Ko.

### 3.3 Taille : ce qui ne change pas (MonEssai)

**`MonEssai.jenga` n'a aucune ligne `filter("config:...")`, aucun `optimize`, aucun `symbols`** (relisez les lignes 15 à 33). MonEssai prend donc **les valeurs par défaut de Jenga** dans les deux configurations, définies dans `Core/Api.py:350-351` :

```python
    optimize: Optimization = Optimization.OFF
    symbols: bool = True
```

`main.cpp` est donc compilé en `-O0 -g` **en Debug comme en Release**. C'est pourquoi `src_main.obj` fait 3 530 octets dans les deux cas, avec les mêmes 1 834 octets de débogage. **Le mot « Release » ne change rien à un projet qui ne dit pas ce que Release veut dire.**

### 3.4 Taille : ce que personne ne retire

**`config/toolchain.jenga`, ligne 55** — les options d'édition de liens de la toolchain Windows :

```python
            ldflags(["--target=x86_64-w64-windows-gnu"])
```

Aucune option ne retire les informations de débogage à l'édition de liens (`-s`, par exemple). La ligne d'édition de liens réelle, affichée par `--verbose`, le confirme : il n'y a rien de plus.

```text
clang++.EXE -o …\Build\Bin\Release-Windows\MonEssai\MonEssai.exe …\src_main.obj
    -L…\Build\Lib\Release-Windows -Wl,--start-group …\NKPlatform.lib -Wl,--end-group
    --target=x86_64-w64-windows-gnu
```

Les **~161 Ko d'informations de débogage** apportés par les objets de la toolchain restent donc dans les deux exécutables. Sur 277 Ko, le gain de NKPlatform (≈ 14 Ko) pèse peu : d'où les **6 %** seulement.

Il reste 2 511 octets d'écart qui ne sont dans aucune section listée par `objdump -h` : probablement la table des symboles et l'alignement du fichier ❓.

### 3.5 Temps : deux coûts qui s'annulent

Les mêmes lignes 63-64 et 67-68 de `NKPlatform.jenga` jouent en **sens contraire** sur le temps :

| | Debug (lignes 63-64) | Release (lignes 67-68) |
|---|---|---|
| Optimisation | `-O0` : le compilateur n'optimise pas, **il va vite** | `-O2` : il optimise, **cela coûte du temps** |
| Débogage | `-g` : il écrit 20 Ko d'informations, **cela coûte du temps** | rien : **il gagne ce temps** |
| **NKPlatform, temps mesuré** | **1,99 s** | **1,99 s** |

Sur 7 petits fichiers, les deux coûts se compensent. L'en-tête précompilé est lui aussi construit avec les mêmes options que les sources (`Windows.py:110-116` et `:147-153`), donc il se compense de la même façon.

Pour **MonEssai**, aucune ligne ne change : `main.cpp` est compilé à l'identique, en **0,27 s** et **0,26 s**.

Le temps « au mur », environ 7,9 s dans les deux cas, ajoute un coût fixe qui ne dépend pas de la configuration : démarrer Jenga, charger le workspace de 274 projets, nettoyer (`rebuild`), et afficher le bilan.

### 3.6 Récapitulatif

| Nombre | Lignes qui l'expliquent |
|---|---|
| **277 053 octets** (Debug) | `NKPlatform.jenga:63-64` (`-O0`, `-g`) + `MonEssai.jenga`, qui n'a aucune ligne de configuration (`Api.py:350-351` → `-O0 -g`) + `toolchain.jenga:55` (rien n'est retiré) |
| **260 585 octets** (Release) | `NKPlatform.jenga:67-68` (`-O2`, pas de `-g`) retire ≈ 14 Ko ; mais `MonEssai.jenga` reste en `-O0 -g` et `toolchain.jenga:55` garde ≈ 161 Ko de débogage |
| **2,26 s** (Debug) | `NKPlatform.jenga:63-64` : compilation rapide (`-O0`) mais écriture du débogage (`-g`) |
| **2,25 s** (Release) | `NKPlatform.jenga:67-68` : optimisation (`-O2`) mais pas de débogage ; les deux coûts se compensent |

Et pour que les deux binaires existent côte à côte : `Nkentseu.jenga:457` et `MonEssai.jenga:29-30`.

---

## 4. Une surprise en chemin : `rebuild --target` nettoie plus que la cible

La première reconstruction Debug a annoncé **88 fichiers supprimés** ; les suivantes, 10. En regroupant les lignes `Removed` du journal par projet :

| Supprimé par D1 (`rebuild --config Debug --target MonEssai`) | Nombre |
|---|---:|
| Objets de NKContainers | 43 |
| Objets de NKMemory | 14 |
| Objets de NKMath | 12 |
| Objets de NKPlatform | 7 |
| Objets de NKCore | 5 |
| Bibliothèques `NKContainers.lib`, `NKCore.lib`, `NKMath.lib`, `NKMemory.lib`, `NKPlatform.lib` | 5 |
| Objet et exécutable de MonEssai | 2 |

**`--target MonEssai` n'a pas limité le nettoyage** : les fichiers Debug de NKCore, NKMemory, NKContainers et NKMath ont été effacés alors que MonEssai n'en dépend pas. Le chapitre 13 prévient que `clean` ne nettoie que la configuration visée ; on voit ici qu'à l'intérieur de cette configuration, il nettoie **tous** les projets. Ces fichiers se reconstruisent sans problème, mais la prochaine construction de NKMath en Debug repartira de zéro.

---

## 5. Ce que cela éclaire dans nos exercices précédents

| Exercice précédent | Ce que cette mesure explique |
|---|---|
| **Annotation de NKMemory, question ❓ n°14** : quelles options derrière `Off`, `Speed` et `symbols` ? | Réponse, dans le code de Jenga (`Windows.py:646-655`) : `-O0`, `-O2` et `-g`. Les tailles mesurées concordent : −46 % de code avec `-O2`, −98 % de débogage sans `-g`. |
| **MonEssai, question d'étudiant** : pourquoi un `main` vide fait-il 131 Ko ? | Parce que l'essentiel d'un exécutable MinGW construit ainsi, ce sont des informations de débogage : 63 % ici, **même en Release**. |
| **Annotation de NKMemory** : « en Debug on veut suivre le programme, en Release la vitesse » | Vrai pour les modules du moteur, qui ont leurs lignes `config:`. **Faux pour MonEssai**, qui ne les a pas : son Release est un Debug rangé ailleurs. |
| **Rapport de chronométrage** : chaque construction recompile tout | Les temps comparés ici sont donc deux constructions complètes. Sans ce défaut, la comparaison demanderait de toute façon un `rebuild`. |
| **Cours, chapitre « Le bureau »** : « une mesure sans sa configuration n'est pas une mesure » | Confirmé : NKPlatform pèse 44 Ko ou 12 Ko selon la configuration. |

---


## 6. État du dépôt

- **Aucun fichier source ni `.jenga` n'a été modifié** pour cet exercice. Seuls les fichiers produits ont changé : `Build/Bin/*-Windows/MonEssai/`, `Build/Lib/*-Windows/NKPlatform.lib` et les objets, que git ignore.
- **Les fichiers Debug de NKCore, NKMemory, NKContainers et NKMath ont été effacés** par le premier `rebuild` (voir §4). `jenga build --target NKMath` les reconstruira.
- Les deux exécutables sont en place et se lancent sans rien afficher :

```text
Build/Bin/Debug-Windows/MonEssai/MonEssai.exe     277 053 octets
Build/Bin/Release-Windows/MonEssai/MonEssai.exe   260 585 octets
```