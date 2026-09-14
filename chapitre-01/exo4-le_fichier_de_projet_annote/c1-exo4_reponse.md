# Annotation ligne par ligne — `Kernel/Foundation/NKMemory/NKMemory.jenga`

> **Pourquoi ce fichier.** Le cours montre les `.jenga` de NKGui, NKCanvas, NKGuiIntegration, NKGuiDemo, NKImage, NKFont, NKAudio, NKMedia et NKNetwork. **Aucun chapitre ne montre NKMemory.jenga.** NKMemory est pourtant déjà connu :
> - il a été construit en **3ᵉ position** par `jenga build --target NKMath` (14 `.cpp`, 2,55 s) ;
> - le comptage lui a trouvé **19 fichiers de test**, le plus gros dossier de tests hors `Externals/`.
>
> **Le fichier** : 85 lignes, 3 commits (`d557314e` 05/05, `1f26ef96` 29/05, `f19260db` 14/06/2026).
> **Pour comprendre chaque ligne**, j'ai lu `config/modules.jenga`, `config/toolchain.jenga`, `Nkentseu.jenga`, et le code de Jenga 2.8.0 : `Core/Api.py`, `Core/Builder.py`, `Core/Loader.py`, `Commands/Build.py`, `Docs/GUIDE_COMPLET_JENGA.md`.

**Légende**

| Étiquette | Sens |
|---|---|
| **TYPE** | quel genre de projet (bibliothèque, exécutable, tests) |
| **SOURCES** | quels fichiers sont compilés ou inclus |
| **DÉP** | dépendances : autres modules ou bibliothèques système |
| **FILTRE** | condition « seulement si… » (plateforme, configuration, option) |
| **TESTS** | suite de tests |
| **RÉGLAGE** | option de compilation ou dossier de sortie |
| **DOC** | commentaire ou description, sans effet sur la construction |
| ❓ | **je ne comprends pas encore**, ou je ne l'ai pas vérifié |

---

## 1. Le fichier en résumé

| Rubrique | Ce que dit le fichier | Comment je l'ai vérifié |
|---|---|---|
| **TYPE** | Bibliothèque statique. Aucune ligne `kind(...)` : le type vient de `nkentseudependson(selfexport=...)` | `modules.jenga:41` et `:359`, `Api.py:1617-1625`, et la ligne « NKMemory StaticLib » du rapport workspace |
| **SOURCES** | 14 `.cpp` compilés, 19 `.h` listés, en-tête précompilé `pch/pch.h` | La construction de NKMath a affiché « Found 14 source file(s) » |
| **DÉP** | NKCore, NKPlatform, plus `pthread` (Linux), `log` (Android), `hilog_ndk.z` (HarmonyOS) | La construction a affiché « depends: NKCore, NKPlatform » |
| **FILTRE** | 12 blocs `filter` : 1 pour les dossiers UWP, 8 par plateforme, 2 par configuration, 1 pour les tests. S'y ajoutent 4 filtres Linux cachés dans le helper | Lecture du fichier et de `modules.jenga:482-506` |
| **TESTS** | Crée un 2ᵉ projet `NKMemory_Tests` avec 19 `.cpp`. **Leur compilation est désactivée par défaut** pour tout le workspace | `Api.py:937-994`, `Nkentseu.jenga:451-453` |

---

## 2. Annotation

### Lignes 1 à 15 — en-tête et description

```python
 1  #!/usr/bin/env python3
 2  # -*- coding: utf-8 -*-
 3  """
 4  NKMemory â€” Gestion mÃ©moire avec tracking (C++17)
 5  =================================================
 6  SystÃ¨me central de gestion mÃ©moire avec allocation/dÃ©sallocation
 7  sÃ©curisÃ©e, tracking des allocations et dÃ©tection de fuites.
 8
 9  Contenu (src/NKMemory/) :
10    NkMemory.h/cpp    => SystÃ¨me de gestion mÃ©moire (NkMemorySystem singleton)
11    NkAllocator.h/cpp => Allocateur bas niveau configurable
12    NkSharedPtr.h     => Pointeur partagÃ© avec tracking
13    NkUniquePtr.h     => Pointeur unique avec tracking
14    NkUtils.h/cpp     => Utilitaires mÃ©moire (Copy, Move, Compare, Set)
15  """
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 1 | DOC | Ligne « shebang » de Python : un `.jenga` **est un script Python** (cours, `00-avant-propos.md:333`). Jenga ne l'exécute pas comme un programme : il le lit puis le fait tourner avec `exec()` (`Loader.py:282`). ❓ À quoi sert alors cette ligne ? Peut-être seulement à l'éditeur. |
| 2 | DOC | Déclare l'encodage UTF-8. Jenga lit le fichier en `utf-8-sig` (`Loader.py:282`). |
| 3, 15 | DOC | Début et fin d'une chaîne de description (*docstring*), sans effet sur la construction. |
| 4 à 7 | DOC | **Texte abîmé.** « â€” » et « mÃ©moire » sont de l'UTF-8 **encodé deux fois** : le fichier contient 11 séquences d'octets `C3 83`. Sans effet sur la construction, puisque cette chaîne n'est jamais utilisée. Pour comparer, `NKContainers.jenga` a des accents corrects. ❓ Lequel des 3 commits a abîmé le texte ? |
| 9 à 14 | DOC | **Liste périmée.** Elle cite 5 fichiers alors que `src/NKMemory/` contient **14 `.cpp` et 19 `.h`**. Les fichiers cités existent (`NkSharedPtr.h` et `NkUniquePtr.h` sont bien là), mais `NkGc`, `NkHash`, `NkPoolAllocator`, `NkTracker`… manquent. C'est le même genre d'écart que le cours relève dans `NKFont.jenga` (chap. 6) et `NKNetwork.jenga` (chap. 9). |

### Lignes 17 et 18 — imports

```python
17  from Jenga import *
18  from jengaconfig import *
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 17 | RÉGLAGE | Importe les fonctions de Jenga : `project`, `files`, `filter`, `test`… (définies dans `Core/Api.py`). |
| 18 | RÉGLAGE | **Import sans effet.** `jengaconfig` est un module vide qui sert à l'éditeur, rangé dans `.jenga-typings` (`Loader.py:271-273`). Les vrais symboles, `nkentseudependson` et `TC_WINDOWS`, viennent des `useconfig("config/…")` de `Nkentseu.jenga:435-437`. |

### Lignes 20 à 23 — le projet

```python
20  with project("NKMemory"):
21      language("C++")
22      cppdialect("C++17")
23      location(".")
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 20 | TYPE | Ouvre le projet **NKMemory**. Ce nom sert partout : dans les dépendances de NKContainers et NKMath, dans le jeton `%{prj.name}`, et dans le nom `NKMemory_Tests`. **Le type n'est pas écrit ici** (voir ligne 27). |
| 21 | RÉGLAGE | Langage C++. |
| 22 | RÉGLAGE | Norme C++17 (`Api.py:1655`). ❓ Option exacte passée à clang : probablement `-std=c++17`, non vérifié. |
| 23 | SOURCES | `location(".")` : le dossier de base est celui du `.jenga`. Les chemins de `files()` et du PCH sont relatifs à lui (guide Jenga, lignes 1391-1397). |

### Lignes 25 à 29 — dépendances **et** type

```python
25      nkentseudependson(
26          ["NKCore", "NKPlatform"],
27          selfexport="NKMemory",
28          extra_includes=["src", "pch"],
29      )
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 25 | DÉP | **Fonction propre à Nkentseu**, pas une fonction de Jenga : elle est définie dans `config/modules.jenga:334`. En un seul appel, elle émet les chemins d'inclusion (`includedirs`), les dépendances (`dependson`) et les `defines`. |
| 26 | DÉP | Dépendances directes : **NKCore, NKPlatform**. On retrouve la même liste à 3 endroits : ici, dans le registre `modules.jenga:60` (`"NKMemory": _m("NKENTSEU_MEMORY", ["NKPlatform", "NKCore"])`), et dans l'affichage de la construction (« depends: NKCore, NKPlatform »). Le helper en déduit `dependson(["NKCore", "NKPlatform"])`, les chemins `%{NKCore.location}/src` et `%{NKPlatform.location}/src`, et les defines `NKENTSEU_CORE_STATIC_LIB` et `NKENTSEU_PLATFORM_STATIC_LIB`. |
| 27 | **TYPE** | **C'est cette ligne qui fixe le type.** Avec `selfexport`, le helper passe en mode bibliothèque et appelle `kindexport(STATIC_LIB, "NKENTSEU_MEMORY")` (`modules.jenga:359`, avec `_GLOBAL_KIND = STATIC_LIB` à la ligne 41). Résultat : type **StaticLib** et define `NKENTSEU_MEMORY_STATIC_LIB` (`Api.py:1624-1625`). En mode bibliothèque, NKCore et NKPlatform sont **déclarés mais pas liés** (`modules.jenga:395`) : ce sont les applications qui les lient. Changer `_GLOBAL_KIND` ferait passer tout Nkentseu en `.dll`. |
| 28 | SOURCES | Ajoute `src` et `pch` aux chemins d'inclusion. **C'est redondant** : en mode bibliothèque, le helper les ajoute déjà (`modules.jenga:379`), et le doublon est retiré (`_dedup`, ligne 417). |
| *(effet caché)* | FILTRE | Le helper appelle aussi `_emit_linux_backend_defines(True)` (`modules.jenga:435`). Cela ajoute **4 filtres Linux invisibles dans ce fichier** (xcb, wayland, headless, xlib), qui posent `NKENTSEU_FORCE_WINDOWING_*_ONLY` et des dossiers de sortie séparés par backend. La raison est expliquée dans `modules.jenga:438-481` : sans eux, deux dispositions mémoire de `NkWindow` se retrouvaient dans le même binaire et provoquaient un plantage. |

### Lignes 31 et 32 — en-tête précompilé (PCH)

```python
31      pchheader("pch/pch.h")
32      pchsource("pch/pch.cpp")
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 31 | SOURCES | En-tête **précompilé** : `pch/pch.h` inclut `stddef.h`, `stdint.h`, `stdlib.h`, `string.h`, `new` et `stdio.h`. Après la construction de NKMath, `Build\Obj` contenait bien 5 fichiers `.pch`, un par projet. |
| 32 | SOURCES | Le `.cpp` qui sert à fabriquer le `.pch`. ❓ Il ne fait pas partie des « 14 source file(s) » affichés. Est-il compilé à part ? |

### Lignes 34 à 37 — fichiers source

```python
34      files([
35          "src/NKMemory/**.cpp",
36          "src/NKMemory/**.h",
37      ])
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 34 à 37 | SOURCES | `**` parcourt tous les sous-dossiers de `src/NKMemory/`. |
| 35 | SOURCES | Les `.cpp` : **14 fichiers**, ceux que Jenga compile (« Found 14 source file(s) » pendant la construction de NKMath) : `NkAllocator`, `NkContainerAllocator`, `NkFunction`, `NkFunctionSIMD`, `NkGc`, `NkGlobalOperators`, `NkHash`, `NkMemory`, `NkMultiLevelAllocator`, `NkPoolAllocator`, `NkProfiler`, `NkTag`, `NkTracker`, `NkUtils`. |
| 36 | SOURCES | Les `.h` : **19 en-têtes listés mais pas compilés à part**. C'est la réponse concrète à la question « compte-t-on les en-têtes ? » du rapport de comptage. ❓ Pourquoi les lister, alors que `NKContainers.jenga:32-34` ne liste que les `.cpp` ? Pour les projets d'IDE générés ? Pour la recompilation incrémentale ? |

### Lignes 39 et 40 — dossiers de sortie

```python
39      objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
40      targetdir("%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}")
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 39 | RÉGLAGE | Dossier des fichiers objets. Jetons (guide Jenga, lignes 672-678) : `%{wks.location}` = racine du dépôt, `%{cfg.buildcfg}` = `Debug`, `%{cfg.system}` = `Windows`, `%{prj.name}` = `NKMemory`. Ici : `Build/Obj/Debug-Windows/NKMemory`, créé pendant la construction de NKMath. |
| 40 | RÉGLAGE | Dossier de la bibliothèque : `Build/Lib/Debug-Windows/NKMemory.lib`, 687 286 octets. Ce dossier est ignoré par git (`.gitignore:151`). |

### Lignes 42 à 44 — dossiers séparés pour UWP

```python
42      with filter("system:Windows && options:windows-runtime=uwp"):
43          objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}-uwp/%{prj.name}")
44          targetdir("%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}-uwp")
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 42 | FILTRE | Vrai si la cible est Windows **et** que l'option `windows-runtime` vaut `uwp`. Cette option est déclarée dans `Nkentseu.jenga:503-521`, avec `desktop` pour valeur par défaut. `&&` veut dire « et ». |
| 43, 44 | RÉGLAGE | **Remplace** les dossiers des lignes 39-40 par des dossiers suffixés `-uwp`, pour que les objets desktop et UWP ne se mélangent pas. Un réglage placé dans un filtre écrase la valeur générale (`Builder.py:1400-1402`). ❓ Les valeurs par défaut des options sont bien appliquées (`Build.py:96-117`), donc ce bloc devrait être faux sans `--options windows-runtime=uwp`. Mais je n'ai pas lu comment la paire « windows-runtime / desktop » devient le texte que teste `options:` (`Builder.py:95` et `:1199-1205`). |

### Lignes 46 à 69 — toolchain et bibliothèques système par plateforme

```python
46      with filter("system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne"):
47          usetoolchain(TC_WINDOWS)
48      with filter("system:UWP || system:Windows && options:windows-runtime=uwp"):
49          usetoolchain("xbox-clang")
50      with filter("system:Linux"):
51          links(["pthread"])
52      with filter("system:macOS"):
53          usetoolchain("clang-native")
54      with filter("system:Android"):
55          # Workaround: disable PCH on Android (NDK r27 + clang 18 + libc++)
56          pchheader("")
57          pchsource("")
58          usetoolchain("android-ndk")
59          links(["log"])
60      with filter("system:HarmonyOS"):
61          # PCH desactive (NDK OHOS clang, meme contrainte qu'Android)
62          pchheader("")
63          pchsource("")
64          usetoolchain("ohos-ndk")
65          links(["hilog_ndk.z"])
66      with filter("system:Web"):
67          usetoolchain("emscripten")
68      with filter("system:XboxSeries || system:XboxOne"):
69          usetoolchain("xbox-clang")
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 46 | FILTRE | Windows desktop : ni UWP, ni Xbox. `!` veut dire « non ». |
| 47 | RÉGLAGE | `TC_WINDOWS` vaut `"nk-windows-clang-mingw"` (`config/toolchain.jenga:14`). Cette toolchain clang est enregistrée par `nkentseutoolchain()` (`Nkentseu.jenga:447`) avec `--target=x86_64-w64-windows-gnu`, `WINVER=0x0601` et `_WIN32_WINNT=0x0601`. Dans un filtre, son existence n'est vérifiée qu'au moment de la construction (`Api.py:1977-1979`). ❓ **La construction a affiché « Toolchain: clang-mingw »**, et le rapport workspace ne liste pas `nk-windows-clang-mingw` parmi les 6 toolchains. Laquelle a vraiment compilé NKMemory ? |
| 48 | FILTRE | `&&` passe avant `||` (`Builder.py:1108-1124`). Il faut donc lire : « UWP » **ou** « (Windows **et** uwp) ». ❓ `system:UWP` ne semble jamais pouvoir être vrai : la liste `TargetOS` de Jenga n'a pas de UWP (`Api.py:67-85`), et `Nkentseu.jenga:501` le dit lui-même (« Jenga v2.0.1 n'expose pas TargetOS.UWP »). Pourquoi l'avoir écrit ? |
| 49 | RÉGLAGE | Toolchain `xbox-clang` pour UWP. ❓ Pourquoi une toolchain Xbox pour du UWP Windows ? Jenga traite UWP comme une variante Xbox (`Core/Builders/Xbox.py`, mode « UWP Dev Mode »), mais je ne comprends pas encore le lien. |
| 50, 51 | FILTRE / DÉP | Sous Linux, lien avec `pthread`, la bibliothèque des threads POSIX. ❓ Quel est l'effet sur une bibliothèque **statique**, qui n'est pas liée elle-même ? Le lien est-il transmis aux programmes qui utilisent NKMemory ? |
| 52, 53 | FILTRE | macOS : toolchain `clang-native`. |
| 54 | FILTRE | Android. |
| 55 | DOC | Commentaire : le PCH est désactivé à cause de NDK r27 + clang 18 + libc++. ❓ Nature exacte du problème non vérifiée. |
| 56, 57 | RÉGLAGE | Une chaîne **vide** dans un filtre **remplace** le PCH des lignes 31-32 (`Builder.py:1409-1414`). Résultat : pas d'en-tête précompilé sur Android. |
| 58 | RÉGLAGE | Toolchain `android-ndk`. |
| 59 | DÉP | Lien avec `log`, la bibliothèque de journalisation d'Android. ❓ Même question que pour `pthread`. |
| 60 à 65 | FILTRE | HarmonyOS : même schéma qu'Android (PCH désactivé, toolchain `ohos-ndk`, lien `hilog_ndk.z`). ❓ Le « `.z` » fait-il partie du nom de la bibliothèque (`libhilog_ndk.z.so`) ? |
| 66, 67 | FILTRE | Web : toolchain `emscripten`. |
| 68, 69 | FILTRE | Xbox Series ou Xbox One : toolchain `xbox-clang`. |

### Lignes 71 à 78 — Debug et Release

```python
71      with filter("config:Debug"):
72          defines(["_DEBUG", "DEBUG", "NKENTSEU_DEBUG"])
73          optimize("Off")
74          symbols(True)
75      with filter("config:Release"):
76          defines(["NDEBUG"])
77          optimize("Speed")
78          symbols(False)
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 71 | FILTRE | Configuration Debug : le nom de la configuration est comparé au motif (`Builder.py:1174-1177`). C'est la configuration par défaut, celle utilisée pour construire NKMath. |
| 72 | RÉGLAGE | Defines `_DEBUG`, `DEBUG`, `NKENTSEU_DEBUG`. **`NKENTSEU_DEBUG` est réellement lu** par le code de NKMemory : `NkTracker.h/.cpp`, `NkPoolAllocator.cpp`, `NkMultiLevelAllocator.cpp`, `NkTag.cpp`, `NkIntrusivePtr.h`. ❓ `NKMath.jenga:64` ne le pose pas, alors que NKMemory, NKContainers, NKCore et NKPlatform le posent. Pourquoi cette différence ? |
| 73 | RÉGLAGE | Sans optimisation. ❓ Option exacte : probablement `-O0`. |
| 74 | RÉGLAGE | Avec symboles de débogage. ❓ Option exacte : probablement `-g`. |
| 75 | FILTRE | Configuration Release (`jenga build --config Release`). |
| 76 | RÉGLAGE | `NDEBUG` désactive les `assert` standard. |
| 77 | RÉGLAGE | Optimisation pour la vitesse. ❓ `-O2` ou `-O3` ? |
| 78 | RÉGLAGE | Sans symboles de débogage. |

### Lignes 80 à 83 — tests

```python
80      # Tests unitaires/stress (desktop uniquement)
81      with filter("(system:Linux || system:macOS || (system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne)) && !system:Android && !system:iOS || system:Web"):
82          with test():
83              testfiles(["tests/**.cpp"])
```

| Ligne | Rubrique | Annotation |
|---|---|---|
| 80 | DOC | Le commentaire annonce « desktop uniquement »… mais la ligne 81 dit autre chose. |
| 81 | FILTRE / TESTS | Comme `&&` passe avant `||`, il faut lire : **[ (Linux ou macOS ou Windows desktop) et pas Android et pas iOS ] ou Web**. Les tests sont donc prévus sur Linux, macOS, Windows desktop **et Web**, ce qui contredit le commentaire. ❓ `!system:Android && !system:iOS` ne change rien, puisque la parenthèse ne contient ni Android ni iOS. Pourquoi les écrire ? ❓ Web est-il voulu, ou est-ce une erreur de parenthèses ? |
| 82 | **TESTS** / TYPE | `with test():` crée un **deuxième projet** nommé `NKMemory_Tests`, de type **TEST_SUITE** (`Api.py:937-947`). C'est la ligne « NKMemory_Tests TestSuite Yes » du rapport workspace. Ce projet dépend de NKMemory, `__Unitest__`, NKCore et NKPlatform. Il copie les chemins, les defines et les réglages filtrés du parent (dont la toolchain), se lie à NKMemory, et produit dans `Build/Tests/Debug-Windows` (`Api.py:953-994`). Le bloc doit être placé directement dans un projet (`Api.py:911-916`). |
| 83 | **TESTS** / SOURCES | `tests/**.cpp` : **19 fichiers**, 16 `test_*.cpp` (arena, buddy, pool, gc, stress…) et 3 `benchmark_*.cpp`. Comme l'appel est dans un filtre, ces fichiers ne sont pris que si le filtre est vrai (`Api.py:2948-2950`). |
| *(effet caché)* | **TESTS** | **Ces tests ne sont pas compilés par défaut.** `Nkentseu.jenga:451` appelle `dutc(enable=True)`, raccourci de *disable unit test compilation* (`Api.py:1453-1456`), et `:453` appelle `dute(enable=True)`, qui désactive leur exécution. D'après `jenga build --help`, `--force-tests` passe outre. C'est pour cela que `jenga build --target NKMath` n'a pas construit `NKMemory_Tests`. ❓ Quelle est la commande exacte pour les construire et les lancer : `jenga test` ? `jenga build --target NKMemory_Tests --force-tests` ? Non essayé. |

---

## 3. Ce que ce fichier apprend, en lien avec les exercices précédents

| Exercice précédent | Ce que NKMemory.jenga explique |
|---|---|
| **Rapport workspace** : « NKMemory StaticLib » et « NKMemory_Tests TestSuite » | Le type **n'est écrit nulle part en clair**. StaticLib vient de `selfexport` (ligne 27), TestSuite de `with test()` (ligne 82). |
| **Comptage** : « les en-têtes sont-ils comptés ? » | Le fichier liste les `.h` (ligne 36), mais Jenga ne compile que les 14 `.cpp` (ligne 35). |
| **Comptage** : « les tests sont-ils comptés ? » | Les 19 tests existent (ligne 83), mais leur compilation est désactivée par défaut par le workspace (`dutc`). |
| **Construction de NKMath** : 3ᵉ position, « depends: NKCore, NKPlatform » | C'est la liste de la ligne 26, recopiée à l'identique par Jenga. |
| **Construction de NKMath** : dossiers `Build/Obj` et `Build/Lib` | Ils viennent des lignes 39-40. Sous Linux, le helper les remplace par des dossiers séparés par backend. |
| **Cours** : descriptions de `.jenga` périmées (NKFont, NKNetwork) | Même défaut ici (lignes 9-14), avec en plus un texte mal encodé (lignes 4-7). |

---

## 4. Tout ce que je ne comprends pas encore (❓)

| N° | Ligne | Question | Piste pour y répondre |
|---:|---|---|---|
| 1 | 1 | À quoi sert le shebang, puisque Jenga lance le fichier avec `exec()` ? | Essayer d'exécuter `python NKMemory.jenga` directement |
| 2 | 4-7 | Quel commit a encodé la description deux fois ? | `git log -p -- Kernel/Foundation/NKMemory/NKMemory.jenga` |
| 3 | 22 | Option exacte pour C++17 ? | `jenga build --target NKMemory --verbose` (option `--verbose` listée dans `--help`) |
| 4 | 32 | `pch.cpp` est-il compilé à part des 14 sources ? | Même commande `--verbose` |
| 5 | 36 | Pourquoi lister les `.h` alors qu'ils ne sont pas compilés ? | Chercher ce que fait `project.files` avec les `.h` dans `Core/Builder.py` |
| 6 | 42-48 | Comment l'option par défaut `desktop` devient-elle le texte testé par `options:` ? | Lire `Commands/Build.py` après la ligne 119, puis `Builder.py:95` |
| 7 | 47 | Pourquoi « Toolchain: clang-mingw » s'affiche-t-il au lieu de `nk-windows-clang-mingw` ? Laquelle compile vraiment ? | `--verbose`, et comparer `Utils/Reporter.py:971` avec `Builder.py:1420-1423` |
| 8 | 48 | `system:UWP` peut-il être vrai un jour ? | `TargetOS` n'a pas de UWP : essayer `--platform UWP` |
| 9 | 49 | Pourquoi la toolchain `xbox-clang` pour UWP ? | Lire `Core/Builders/Xbox.py` (mode « uwp ») |
| 10 | 51, 59, 65 | Que devient `links(...)` sur une bibliothèque statique ? | Construire sous Linux une application qui dépend de NKMemory et regarder la ligne d'édition de liens |
| 11 | 55 | Quel bug de PCH avec NDK r27 + clang 18 ? | Chercher dans `BugReports/` et l'historique git |
| 12 | 65 | Le `.z` de `hilog_ndk.z` fait-il partie du nom ? | Documentation du NDK HarmonyOS |
| 13 | 72 | Pourquoi NKMath ne pose-t-il pas `NKENTSEU_DEBUG` ? | Chercher ce define dans `Kernel/Foundation/NKMath/src` |
| 14 | 73-77 | Options exactes de `Off`, `Speed` et `symbols` ? | `--verbose`, en Debug puis avec `--config Release` |
| 15 | 81 | `!Android && !iOS` inutiles, et Web voulu ou erreur de parenthèses ? | Comparer avec les autres modules : NKMath et NKContainers ont exactement la même ligne |
| 16 | 82-83 | Commande exacte pour construire et lancer `NKMemory_Tests` ? | `jenga test --help`, puis `jenga build --target NKMemory_Tests --force-tests` |