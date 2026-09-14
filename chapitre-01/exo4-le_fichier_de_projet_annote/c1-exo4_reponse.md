# NKMemory.jenga, lu ligne par ligne

> **Pourquoi ce fichier ?** Je cherchais un `.jenga` que le cours ne montre pas. Les chapitres présentent ceux de NKGui, NKCanvas, NKGuiIntegration, NKGuiDemo, NKImage, NKFont, NKAudio, NKMedia et NKNetwork, mais **jamais celui de NKMemory**. Pourtant, ce module, on l'a déjà croisé deux fois :
> - quand j'ai construit NKMath, Jenga l'a compilé en **3ᵉ position** (14 fichiers `.cpp`, 2,55 secondes) ;
> - quand j'ai compté les sources, c'est lui qui avait **le plus de fichiers de test** hors `Externals/` : 19.
>
> Le fichier fait 85 lignes et n'a été modifié que trois fois (5 mai, 29 mai et 14 juin 2026). Pour comprendre ce qu'il fait vraiment, le lire ne suffisait pas. J'ai ouvert les fichiers de configuration de Nkentseu (`config/modules.jenga`, `config/toolchain.jenga`, `Nkentseu.jenga`) et le code de Jenga 2.8.0. Les références entre parenthèses, comme `(Api.py:1617)`, indiquent où j'ai trouvé chaque information, pour que vous puissiez aller vérifier.

### Comment lire les annotations

Chaque ligne reçoit une étiquette qui dit à quoi elle sert :

| Étiquette | Ce qu'elle veut dire |
|---|---|
| **TYPE** | le genre de projet : bibliothèque, programme, tests |
| **SOURCES** | les fichiers compilés ou inclus |
| **DÉP** | ce dont le module a besoin : d'autres modules ou des bibliothèques du système |
| **FILTRE** | une condition : « seulement sous Linux », « seulement en Debug »… |
| **TESTS** | les tests du module |
| **RÉGLAGE** | une option de compilation ou un dossier de sortie |
| **DOC** | un commentaire, sans effet sur la construction |
| ❓ | **je ne comprends pas encore**, ou je n'ai pas pu le vérifier |

---

## 1. Ce fichier en cinq phrases

- **C'est une bibliothèque statique**, mais aucune ligne ne le dit en clair. C'est un appel de fonction, à la ligne 27, qui le décide *(modules.jenga:41 et :359, Api.py:1617-1625)*.
- **Il compile 14 fichiers `.cpp`.** Il liste aussi 19 en-têtes `.h`, mais ceux-là ne sont pas compilés à part. Jenga l'a confirmé en affichant « Found 14 source file(s) » pendant la construction de NKMath.
- **Il dépend de NKCore et de NKPlatform**, plus d'une bibliothèque système selon la plateforme : `pthread` sous Linux, `log` sous Android, `hilog_ndk.z` sous HarmonyOS.
- **Il pose 12 conditions (`filter`)** : une pour ranger à part les fichiers UWP, huit pour s'adapter à chaque plateforme, deux pour Debug et Release, une pour les tests. Quatre autres conditions, pour Linux, sont cachées dans une fonction qu'il appelle *(modules.jenga:482-506)*.
- **Il déclare 19 tests**, qui forment un second projet nommé `NKMemory_Tests`… mais le workspace désactive leur compilation par défaut *(Nkentseu.jenga:451-453)*.

---

## 2. Le fichier, bloc par bloc

### Lignes 1 à 15 — la carte d'identité

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

**En clair :** rien ici ne sert à construire. Le fichier se présente comme un script Python, puis décrit le module. Et cette description a deux défauts : ses accents sont abîmés, et elle n'est plus à jour.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 1 | DOC | Cette ligne dit « je suis un script Python ». C'est vrai : un `.jenga` **est** du Python (le cours le dit dans l'avant-propos). Mais Jenga ne le lance pas comme un programme : il lit le fichier et exécute son contenu *(Loader.py:282)*. ❓ Alors à quoi sert cette ligne ? Peut-être seulement à aider l'éditeur. |
| 2 | DOC | Annonce que le fichier est en UTF-8. Jenga le lit bien ainsi. |
| 3 et 15 | DOC | Les triples guillemets ouvrent et ferment la description du module. |
| 4 à 7 | DOC | **Les accents sont abîmés.** « mÃ©moire » au lieu de « mémoire » : le texte a été converti en UTF-8 **deux fois** (on trouve 11 fois la séquence d'octets `C3 83` dans le fichier). Ce n'est pas grave pour la construction, car personne ne lit ce texte. `NKContainers.jenga`, lui, a des accents corrects. ❓ Laquelle des trois modifications a causé ce problème ? |
| 9 à 14 | DOC | **La liste est périmée.** Elle cite 5 fichiers, alors que le dossier en contient **14 `.cpp` et 19 `.h`**. Les fichiers cités existent bien, mais d'autres comme `NkGc`, `NkHash`, `NkPoolAllocator` ou `NkTracker` n'y figurent pas. Le cours relève le même genre d'oubli dans `NKFont.jenga` (chap. 6) et `NKNetwork.jenga` (chap. 9). |

### Lignes 17 et 18 — on sort les outils

```python
17  from Jenga import *
18  from jengaconfig import *
```

**En clair :** avant d'écrire quoi que ce soit, le fichier charge le vocabulaire de Jenga. La deuxième ligne est surprenante : elle ne charge rien.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 17 | RÉGLAGE | Rend disponibles les mots de Jenga : `project`, `files`, `filter`, `test`… |
| 18 | RÉGLAGE | **Cette ligne ne sert qu'à l'éditeur.** `jengaconfig` est un module vide, présent pour que l'éditeur ne signale pas d'erreur *(Loader.py:271-273)*. Les vraies fonctions de Nkentseu, comme `nkentseudependson`, sont chargées par le fichier racine *(Nkentseu.jenga:435-437)*. |

### Lignes 20 à 23 — le projet s'ouvre

```python
20  with project("NKMemory"):
21      language("C++")
22      cppdialect("C++17")
23      location(".")
```

**En clair :** on ouvre le projet, on choisit le langage et la version du C++, et on dit où se trouvent les fichiers. Ce qui manque est plus intéressant : **nulle part on ne dit que c'est une bibliothèque**.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 20 | TYPE | Le nom **NKMemory** servira partout : dans les dépendances de NKContainers et de NKMath, dans les dossiers de sortie, et dans le nom du projet de tests `NKMemory_Tests`. Le type, lui, n'est pas écrit ici : il faut attendre la ligne 27. |
| 21 | RÉGLAGE | Le module est écrit en C++. |
| 22 | RÉGLAGE | Il utilise la norme C++17. ❓ Je suppose que Jenga passe `-std=c++17` au compilateur, mais je ne l'ai pas vu. |
| 23 | SOURCES | `"."` veut dire « le dossier où se trouve ce fichier ». Tous les chemins qui suivent partent de là. |

### Lignes 25 à 29 — le cœur du fichier

```python
25      nkentseudependson(
26          ["NKCore", "NKPlatform"],
27          selfexport="NKMemory",
28          extra_includes=["src", "pch"],
29      )
```

**En clair :** en apparence, c'est une simple liste de dépendances. En réalité, cet appel fait **quatre choses d'un coup** : il déclare les dépendances, ajoute les dossiers d'en-têtes à connaître, décide que NKMemory est une bibliothèque statique, et ajoute des réglages pour Linux qu'on ne voit pas ici. C'est le bloc qui m'a demandé le plus de recherche.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 25 | DÉP | `nkentseudependson` **n'est pas une fonction de Jenga**. Elle a été écrite pour Nkentseu, dans `config/modules.jenga` (ligne 334), pour éviter de répéter les mêmes réglages dans chaque module. |
| 26 | DÉP | NKMemory a besoin de **NKCore et NKPlatform**. Cette liste existe à trois endroits qui concordent : ici, dans le registre des modules *(modules.jenga:60)*, et dans ce que Jenga a affiché en construisant NKMath (« depends: NKCore, NKPlatform »). À partir de ces deux noms, la fonction trouve seule où sont leurs en-têtes et ajoute les réglages nécessaires. |
| 27 | **TYPE** | **C'est ici que NKMemory devient une bibliothèque statique.** En écrivant `selfexport="NKMemory"`, on dit « ce projet est une bibliothèque qui s'appelle NKMemory ». La fonction applique alors le réglage choisi pour tout Nkentseu : bibliothèque **statique**, un fichier `.lib` *(modules.jenga:41 et :359)*. Autre conséquence : NKMemory **annonce** qu'il a besoin de NKCore et NKPlatform, mais ne les **assemble** pas avec lui. Ce travail revient à l'application finale *(modules.jenga:395)*. Si on changeait ce réglage global, tous les modules deviendraient des `.dll`. |
| 28 | SOURCES | Demande d'ajouter les dossiers `src` et `pch` à la liste des en-têtes. **C'est inutile** : la fonction les ajoute déjà d'elle-même pour une bibliothèque, puis retire les doublons *(modules.jenga:379 et :417)*. |
| *(caché)* | FILTRE | La fonction ajoute aussi **quatre conditions pour Linux** qu'on ne voit pas dans ce fichier *(modules.jenga:435)*. Sous Linux, on peut afficher les fenêtres de plusieurs façons (XLib, XCB, Wayland, ou sans fenêtre). Selon ce choix, la fonction pose le bon réglage et range les fichiers produits dans un dossier séparé. Le commentaire de `modules.jenga` (lignes 438-481) raconte pourquoi : un jour, deux versions incompatibles de la même classe se sont retrouvées dans le même programme, et l'application plantait au démarrage. |

### Lignes 31 et 32 — gagner du temps de compilation

```python
31      pchheader("pch/pch.h")
32      pchsource("pch/pch.cpp")
```

**En clair :** un **en-tête précompilé** (PCH), c'est un groupe d'en-têtes que le compilateur analyse une seule fois puis réutilise pour tous les `.cpp`, au lieu de les relire à chaque fichier.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 31 | SOURCES | `pch.h` regroupe six en-têtes standards : `stddef.h`, `stdint.h`, `stdlib.h`, `string.h`, `new` et `stdio.h`. Après la construction de NKMath, j'ai bien trouvé 5 fichiers `.pch` dans `Build\Obj`, un par module. |
| 32 | SOURCES | `pch.cpp` sert à fabriquer ce fichier précompilé. ❓ Il ne fait pas partie des « 14 source file(s) » annoncés par Jenga. Est-il compilé à part ? |

### Lignes 34 à 37 — les fichiers à compiler

```python
34      files([
35          "src/NKMemory/**.cpp",
36          "src/NKMemory/**.h",
37      ])
```

**En clair :** « prends tous les `.cpp` et tous les `.h` du dossier `src/NKMemory`, sous-dossiers compris ». Les deux étoiles `**` veulent dire « cherche aussi dans les sous-dossiers ».

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 35 | SOURCES | Les **14 `.cpp`** sont ceux que Jenga a compilés : `NkAllocator`, `NkContainerAllocator`, `NkFunction`, `NkFunctionSIMD`, `NkGc`, `NkGlobalOperators`, `NkHash`, `NkMemory`, `NkMultiLevelAllocator`, `NkPoolAllocator`, `NkProfiler`, `NkTag`, `NkTracker` et `NkUtils`. |
| 36 | SOURCES | Les **19 `.h`** sont listés, mais **jamais compilés seuls** : ils sont lus au travers des `.cpp` qui les incluent. Voilà une réponse concrète à la question « compte-t-on les en-têtes ? » de l'exercice de comptage. ❓ Pourquoi les lister alors, quand `NKContainers.jenga` se contente des `.cpp` ? Pour générer un projet d'IDE ? Pour savoir quoi recompiler quand un `.h` change ? |

### Lignes 39 et 40 — où ranger le résultat

```python
39      objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
40      targetdir("%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}")
```

**En clair :** les morceaux entre `%{…}` sont remplacés au moment de construire. `%{wks.location}` devient la racine du dépôt, `%{cfg.buildcfg}` devient `Debug`, `%{cfg.system}` devient `Windows` et `%{prj.name}` devient `NKMemory` (guide Jenga, lignes 672-678).

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 39 | RÉGLAGE | Les fichiers intermédiaires vont dans `Build/Obj/Debug-Windows/NKMemory`. Ce dossier est apparu quand j'ai construit NKMath. |
| 40 | RÉGLAGE | La bibliothèque finale va dans `Build/Lib/Debug-Windows/NKMemory.lib` (687 286 octets). git ignore ce dossier *(.gitignore:151)*. |

### Lignes 42 à 44 — un coin à part pour UWP

```python
42      with filter("system:Windows && options:windows-runtime=uwp"):
43          objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}-uwp/%{prj.name}")
44          targetdir("%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}-uwp")
```

**En clair :** UWP est une autre façon de faire des applications Windows. Si on construit pour UWP, les fichiers vont dans des dossiers qui finissent par `-uwp`, pour ne pas se mélanger avec ceux du Windows classique.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 42 | FILTRE | La condition se lit « Windows **et** option UWP ». `&&` veut dire « et ». L'option `windows-runtime` est déclarée dans `Nkentseu.jenga` (lignes 503-521), et vaut `desktop` si on ne précise rien. |
| 43 et 44 | RÉGLAGE | Ces deux lignes **remplacent** les dossiers des lignes 39-40, mais seulement si la condition est vraie *(Builder.py:1400-1402)*. ❓ Par défaut, cette condition devrait donc être fausse. Mais je n'ai pas lu en détail comment Jenga compare la valeur `desktop` au texte `windows-runtime=uwp`. |

### Lignes 46 à 69 — s'adapter à chaque plateforme

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

**En clair :** pour chaque plateforme, le fichier choisit une **toolchain**, c'est-à-dire le compilateur et les outils qui vont avec. Parfois il ajoute une bibliothèque du système. Sur ma machine Windows, seul le premier bloc me concerne.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 46 | FILTRE | « Windows classique » : Windows, **mais pas** UWP, **ni** Xbox. `!` veut dire « non ». |
| 47 | RÉGLAGE | `TC_WINDOWS` vaut `"nk-windows-clang-mingw"` *(config/toolchain.jenga:14)* : une toolchain clang que Nkentseu déclare lui-même, avec ses propres réglages pour Windows. ❓ **Quelque chose ne colle pas** : pendant la construction, Jenga a affiché « Toolchain: clang-mingw », et ce nom `nk-windows-clang-mingw` n'apparaît pas dans la liste des 6 toolchains du rapport workspace. Laquelle a vraiment compilé NKMemory ? |
| 48 | FILTRE | Jenga évalue les « et » avant les « ou » *(Builder.py:1108-1124)*. La condition se lit donc « UWP », **ou bien** « Windows et option UWP ». ❓ La première moitié me paraît impossible : Jenga ne connaît aucun système nommé UWP *(Api.py:67-85)*, et `Nkentseu.jenga` le reconnaît lui-même (ligne 501). Pourquoi l'avoir écrite ? |
| 49 | RÉGLAGE | Pour UWP, on prend la toolchain Xbox. ❓ Pourquoi la Xbox pour une application Windows ? Jenga semble traiter UWP comme un mode de la Xbox *(Core/Builders/Xbox.py)*, mais je ne saisis pas encore pourquoi. |
| 50 et 51 | FILTRE / DÉP | Sous Linux, NKMemory a besoin de `pthread`, la bibliothèque des threads. ❓ Mais une bibliothèque statique n'est pas assemblée elle-même : que devient cette demande ? Est-elle transmise aux programmes qui utilisent NKMemory ? |
| 52 et 53 | FILTRE | Sous macOS, on utilise le clang du système. |
| 54 et 55 | FILTRE / DOC | Sous Android, un commentaire prévient que le PCH pose problème avec les outils Android du moment. ❓ Je n'ai pas cherché quel était ce problème. |
| 56 et 57 | RÉGLAGE | Mettre un nom **vide** désactive le PCH des lignes 31-32, mais seulement pour Android *(Builder.py:1409-1414)*. |
| 58 et 59 | RÉGLAGE / DÉP | Toolchain Android, et bibliothèque `log` pour écrire dans le journal d'Android. ❓ Même question que pour `pthread`. |
| 60 à 65 | FILTRE | HarmonyOS suit le même schéma qu'Android. ❓ Le `.z` de `hilog_ndk.z` fait-il vraiment partie du nom de la bibliothèque ? |
| 66 et 67 | FILTRE | Pour le Web, on compile avec Emscripten. |
| 68 et 69 | FILTRE | Pour les Xbox, toolchain Xbox. |

### Lignes 71 à 78 — Debug ou Release

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

**En clair :** en **Debug**, on veut pouvoir suivre le programme pas à pas : pas d'optimisation, des informations de débogage, et des vérifications en plus. En **Release**, on veut la vitesse.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 71 | FILTRE | Seulement en Debug, la configuration utilisée par défaut quand j'ai construit NKMath. |
| 72 | RÉGLAGE | Trois mots-clés sont définis pour le code. `NKENTSEU_DEBUG` **sert vraiment** : on le retrouve dans `NkTracker`, `NkPoolAllocator`, `NkMultiLevelAllocator`, `NkTag` et `NkIntrusivePtr.h`, sans doute pour activer le suivi des allocations. ❓ Curieusement, NKMath ne le définit pas, alors que NKMemory, NKContainers, NKCore et NKPlatform le font. Oubli ou choix ? |
| 73 et 74 | RÉGLAGE | Pas d'optimisation, mais des symboles de débogage. ❓ Je suppose que cela donne `-O0` et `-g` pour clang, sans l'avoir vérifié. |
| 75 et 76 | FILTRE / RÉGLAGE | En Release, `NDEBUG` désactive les `assert` standard. |
| 77 et 78 | RÉGLAGE | Optimisation pour la vitesse, sans symboles. ❓ « Vitesse » veut-il dire `-O2` ou `-O3` ? |

### Lignes 80 à 83 — les tests

```python
80      # Tests unitaires/stress (desktop uniquement)
81      with filter("(system:Linux || system:macOS || (system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne)) && !system:Android && !system:iOS || system:Web"):
82          with test():
83              testfiles(["tests/**.cpp"])
```

**En clair :** « sur les ordinateurs de bureau, crée un projet de tests avec tout ce qui se trouve dans `tests/` ». C'est du moins ce que dit le commentaire. En lisant la condition de près, j'ai eu une surprise.

| Ligne | Étiquette | Ce que j'en comprends |
|---|---|---|
| 80 | DOC | Le commentaire promet « desktop uniquement ». |
| 81 | FILTRE / TESTS | **La condition ne dit pas la même chose.** Comme Jenga évalue les « et » avant les « ou », elle se lit : **(Linux ou macOS ou Windows classique, et pas Android, et pas iOS) — ou bien Web**. Les tests sont donc aussi prévus pour le **Web**, ce qui n'est pas du « desktop ». ❓ Et `pas Android, pas iOS` ne change rien, puisque la parenthèse ne contient ni l'un ni l'autre. Pourquoi l'écrire ? Le Web est-il voulu, ou manque-t-il une parenthèse ? |
| 82 | **TESTS** / TYPE | `with test()` crée un **second projet**, `NKMemory_Tests`, de type suite de tests *(Api.py:937-947)*. C'est lui qui apparaît dans le rapport workspace. Il reprend les réglages de NKMemory, dépend de NKMemory, de NKCore, de NKPlatform et de `__Unitest__`, le framework de test de Jenga, et range son programme dans `Build/Tests/Debug-Windows` *(Api.py:953-994)*. |
| 83 | **TESTS** / SOURCES | Le dossier `tests/` contient **19 fichiers** : 16 tests (`test_allocator_pool.cpp`, `test_gc.cpp`, `test_memory_stress.cpp`…) et 3 mesures de performance (`benchmark_*.cpp`). |
| *(caché)* | **TESTS** | **Ces tests ne sont pas compilés par défaut.** Le fichier racine `Nkentseu.jenga` désactive la compilation des tests (`dutc`, ligne 451) et leur exécution (`dute`, ligne 453) pour tout le workspace. C'est pour cela que la construction de NKMath n'a pas touché à `NKMemory_Tests`. D'après `jenga build --help`, l'option `--force-tests` passe outre. ❓ Je n'ai pas essayé. Est-ce `jenga test`, ou `jenga build --target NKMemory_Tests --force-tests` ? |

---

## 3. Ce que ce fichier éclaire dans nos exercices précédents

| Ce qu'on avait vu | Ce que NKMemory.jenga explique |
|---|---|
| Le rapport workspace affichait « NKMemory StaticLib » et « NKMemory_Tests TestSuite » | Aucun des deux types n'est écrit en clair. StaticLib vient de `selfexport` (ligne 27), TestSuite de `with test()` (ligne 82). |
| Au comptage, on se demandait s'il fallait compter les en-têtes | Le fichier liste les `.h`, mais Jenga ne compile que les `.cpp`. |
| Au comptage, on se demandait s'il fallait compter les tests | Les 19 tests existent bien, mais le workspace ne les compile pas par défaut. |
| En construisant NKMath, Jenga affichait « depends: NKCore, NKPlatform » pour NKMemory | C'est mot pour mot la liste de la ligne 26. |
| Le dossier `Build/` est apparu avec la construction | Son chemin vient des lignes 39-40. |
| Le cours signalait des descriptions de `.jenga` périmées (NKFont, NKNetwork) | Même défaut ici, avec en prime des accents abîmés. |

---

## 4. Ce que je ne comprends pas encore

J'ai relevé **16 points** que je n'ai pas encore élucidés. Pour chacun, voici ce que je ferais pour trouver la réponse.

| N° | Ligne | Ma question | Ce que je ferais pour trouver |
|---:|---|---|---|
| 1 | 1 | Si Jenga exécute le fichier lui-même, à quoi sert la ligne `#!/usr/bin/env python3` ? | Lancer `python NKMemory.jenga` et voir ce qui se passe |
| 2 | 4-7 | Qui a abîmé les accents, et quand ? | Parcourir les trois modifications avec `git log -p -- Kernel/Foundation/NKMemory/NKMemory.jenga` |
| 3 | 22 | Quelle option exacte correspond au C++17 ? | Construire avec `jenga build --target NKMemory --verbose` et lire les commandes affichées |
| 4 | 32 | `pch.cpp` est-il compilé à part des 14 sources ? | Même commande `--verbose` |
| 5 | 36 | Pourquoi lister les `.h` s'ils ne sont pas compilés ? | Chercher ce que Jenga fait des `.h` dans `Core/Builder.py` |
| 6 | 42-48 | Comment la valeur par défaut `desktop` est-elle comparée au texte `windows-runtime=uwp` ? | Lire la suite de `Commands/Build.py` (après la ligne 119) |
| 7 | 47 | Pourquoi Jenga affiche-t-il `clang-mingw` au lieu de `nk-windows-clang-mingw` ? Laquelle compile vraiment ? | `--verbose`, puis comparer `Utils/Reporter.py:971` et `Builder.py:1420-1423` |
| 8 | 48 | La condition `system:UWP` peut-elle être vraie un jour ? | Essayer `jenga build --target NKMemory --platform UWP` |
| 9 | 49 | Pourquoi une toolchain Xbox pour UWP ? | Lire le mode UWP dans `Core/Builders/Xbox.py` |
| 10 | 51, 59, 65 | Que devient `links(...)` pour une bibliothèque statique ? | Construire sous Linux une petite application qui utilise NKMemory et regarder la commande d'assemblage |
| 11 | 55 | Quel problème de PCH avec les outils Android ? | Chercher dans `BugReports/` et dans l'historique git |
| 12 | 65 | Le `.z` de `hilog_ndk.z` fait-il partie du nom ? | Consulter la documentation du NDK HarmonyOS |
| 13 | 72 | Pourquoi NKMath ne définit-il pas `NKENTSEU_DEBUG` ? | Chercher ce mot-clé dans le code de NKMath |
| 14 | 73-77 | Quelles options exactes pour Debug et Release ? | `--verbose`, une fois en Debug et une fois avec `--config Release` |
| 15 | 81 | Le Web est-il voulu dans les tests, ou manque-t-il une parenthèse ? | NKMath et NKContainers ont exactement la même condition : chercher qui l'a écrite en premier |
| 16 | 82-83 | Comment construire et lancer `NKMemory_Tests` ? | `jenga test --help`, puis essayer `jenga build --target NKMemory_Tests --force-tests` |

---

## 6. Ce que je retiens

Je pensais qu'un fichier de 85 lignes se lirait en dix minutes. En réalité, **NKMemory.jenga ne se comprend pas tout seul**. Son type est décidé par une fonction écrite ailleurs. Quatre de ses conditions sont invisibles. Ses tests sont désactivés depuis le fichier racine. Et même sa propre description n'est plus à jour.

La leçon que j'en tire : pour savoir ce que fait vraiment un module, **ce que Jenga affiche en construisant** (« Found 14 source file(s) », « depends: NKCore, NKPlatform ») est plus fiable que les commentaires. Il me reste 16 questions ouvertes, mais pour chacune je sais maintenant où chercher.