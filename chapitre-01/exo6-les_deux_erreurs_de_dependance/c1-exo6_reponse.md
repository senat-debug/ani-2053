# `dependson` ou `links` : une ligne retirée, deux résultats très différents

> **La consigne** : dans mon projet (MonEssai), retirer d'abord `dependson`, reconstruire et noter le message. Le remettre, retirer `links`, reconstruire et noter le message. Rendre les deux messages et ce qui les distingue.
> **Le dépôt** : `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu`, le 2026-09-14. Jenga 2.8.0, configuration Debug, cible Windows x86_64, compilateur `C:\msys64\ucrt64\bin\clang++.EXE`.
> **La commande**, toujours la même, tapée à la racine du dépôt dans le terminal de VS Code :
>
> ```bash
> jenga build --target MonEssai --verbose
> ```
>
> `--verbose` fait afficher la ligne d'édition de liens. C'est elle qui explique tout.

---

## 1. En bref

| Essai | Ce que dit `MonEssai.jenga` | Résultat | Message |
|---|---|---|---|
| **Départ** | `dependson` + `links` + chemin d'en-têtes |  `Projects Built: 2/2` | aucun |
| **Expérience 1** | `dependson` **retiré**, `links` gardé |  échec à l'**édition de liens** | `ld: cannot find -lNKPlatform: No such file or directory` |
| **Expérience 2** | `dependson` remis, `links` **retiré** |  `Projects Built: 2/2` | **aucun message d'erreur** |

**Ce qui les distingue, en deux phrases.** Sans `dependson`, NKPlatform n'est plus construit et plus personne ne dit à l'éditeur de liens **où** trouver la bibliothèque : `links` ne transmet qu'un nom, et l'éditeur de liens échoue. Sans `links`, rien ne se passe, car c'est `dependson` qui ajoute **lui-même** `NKPlatform.lib` à l'édition de liens : `links(["NKPlatform"])` était donc redondant.

---

## 2. Préparer le terrain : MonEssai doit vraiment utiliser une bibliothèque

Dans l'exercice précédent, MonEssai était un `main` vide, **sans aucune dépendance**. Il n'y avait donc ni `dependson` ni `links` à retirer. J'ai d'abord fait en sorte qu'il utilise vraiment NKPlatform, tout en n'affichant toujours rien.

### 2.1 Le nouveau `main.cpp`

```cpp
// MonEssai — n'affiche rien, mais utilise NKPlatform pour de vrai :
//  - l'en-tete doit etre TROUVE a la compilation (chemin d'inclusion) ;
//  - CPUFeatures::Get() est defini dans NkCPUFeatures.cpp (NKENTSEU_NO_INLINE),
//    donc son symbole doit etre TROUVE a l'edition de liens (NKPlatform.lib).
#include "NKPlatform/NkCPUFeatures.h"

int main() {
	const nkentseu::platform::CPUFeatures &cpu = nkentseu::platform::CPUFeatures::Get();
	(void)cpu;
	return 0;
}
```

Pourquoi `CPUFeatures::Get()` ? Pour trois raisons, que j'ai vérifiées dans le code :
- elle est **définie dans un `.cpp`** (`NkCPUFeatures.cpp:203`) et marquée `NKENTSEU_NO_INLINE`. Son code est donc dans `NKPlatform.lib` : sans la bibliothèque, l'édition de liens ne peut pas la trouver ;
- elle **n'affiche rien** : elle détecte le processeur une seule fois et renvoie le résultat ;
- NKPlatform n'a **aucune dépendance**, donc une seule bibliothèque est en jeu.

### 2.2 Le `.jenga` : les mots de Jenga, sans `nkentseudependson`

J'ai écrit `dependson` et `links` **en clair**. `nkentseudependson` les aurait posés lui-même, en ajoutant aussi les chemins d'en-têtes, et aurait caché ce qu'on veut observer.

```python
with project("MonEssai"):
    consoleapp()
    language("C++")
    cppdialect("C++17")
    location(".")

    files(["src/**.cpp"])
    includedirs(["src"])

    dependson(["NKPlatform"])
    links(["NKPlatform"])

    objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
    targetdir("%{wks.location}/Build/Bin/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")

    with filter("system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne"):
        usetoolchain(TC_WINDOWS)
```

---

## 3. Première surprise : `dependson` ne transmet pas les chemins d'en-têtes

Le chapitre « Les dépendances » (`Documentation/jenga/tex/chapitres/07-les-dependances.tex`, lignes 15-21) affirme que `dependson` fait deux choses : il impose un **ordre**, et il fait **hériter** les chemins d'inclusion de la dépendance. J'ai donc construit ce `.jenga` sans rien ajouter d'autre.

**Message 0 — `dependson` + `links`, sans chemin d'en-têtes vers NKPlatform :**

```text
Build Order (2 projects):
  1. NKPlatform [STATIC_LIB] →
  2. MonEssai [CONSOLE_APP] (depends: NKPlatform)
...
✓ Built: Build\Lib\Debug-Windows\NKPlatform.lib
...
║                                 Compilation Error: main.cpp                                  ║
║ C:\...\Applications\MonEssai\src\main.cpp:5:10: fatal error: 'NKPlatform/NkCPUFeatures.h'   ║
║ file not found                                                                               ║
║     5 | #include "NKPlatform/NkCPUFeatures.h"                                                ║
║       |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~                                                ║
...
Projects Built:  1/2
Status:         ✗ FAILURE
```

**L'ordre est bien respecté**, puisque NKPlatform est construit en premier. **Mais le chemin d'en-têtes n'est pas transmis.** Le code de Jenga le confirme : il n'existe aucune notion d'en-têtes « publics », et `externalincludedirs()` ou `sysincludedirs()` ne sont que des synonymes de `includedirs()` (`Core/Api.py:1709-1718`). Je n'ai trouvé aucun endroit où `Builder.py` recopierait les `includeDirs` d'une dépendance.

J'ai donc ajouté le chemin à la main, sous la même forme que le cours utilise pour NKGlad :

```python
    # Chemin d'inclusion de NKPlatform ecrit a la main : dependson ne le
    # transmet PAS (constate : « 'NKPlatform/NkCPUFeatures.h' file not found »).
    includedirs(["src", "%{NKPlatform.location}/src"])
```

**État de départ — tout fonctionne :**

```text
Build Order (2 projects):
  1. NKPlatform [STATIC_LIB] →
  2. MonEssai [CONSOLE_APP] (depends: NKPlatform)
✓   [1/1] Compiled: main.cpp
[Link:Clang:MonEssai] C:\msys64\ucrt64\bin\clang++.EXE -o ...\Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
    ...\Build\Obj\Debug-Windows\MonEssai\src_main.obj
    -LC:\...\Nkentseu\Build\Lib\Debug-Windows
    -Wl,--start-group C:\...\Nkentseu\Build\Lib\Debug-Windows\NKPlatform.lib -Wl,--end-group
    --target=x86_64-w64-windows-gnu
✓ Built: Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
Projects Built:  2/2
Status:         ✓ SUCCESS
```

L'exécutable se lance : **0 octet affiché, code de retour 0**. Retenez bien cette ligne d'édition de liens : `links(["NKPlatform"])` n'y apparaît pas comme `-lNKPlatform`, mais comme le **chemin complet** du fichier `.lib`, précédé d'un `-L` vers `Build\Lib\Debug-Windows`.

---

## 4. Expérience 1 — retirer `dependson`

```python
    # EXPERIENCE 1 : dependson retire, links garde
    links(["NKPlatform"])
```

**Message 1 :**

```text
Build Order (1 projects):
  1. MonEssai [CONSOLE_APP]
ℹ Found 1 source file(s)
✓   [1/1] Compiled: main.cpp
[Link:Clang:MonEssai] C:\msys64\ucrt64\bin\clang++.EXE -o ...\Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
    ...\Build\Obj\Debug-Windows\MonEssai\src_main.obj
    -Wl,--start-group -lNKPlatform -Wl,--end-group --target=x86_64-w64-windows-gnu
ℹ Linking...
╔══════════════════════════════════════════════════════════════════════════════════════════════╗
║                                Compilation Error: Link Failed                                ║
╠══════════════════════════════════════════════════════════════════════════════════════════════╣
║ C:/msys64/ucrt64/bin/ld: cannot find -lNKPlatform: No such file or directory                 ║
║ clang++: error: linker command failed with exit code 1 (use -v to see invocation)            ║
╚══════════════════════════════════════════════════════════════════════════════════════════════╝
│  ✗ Build Failed                                                                 Time: 0.23s  │
Projects Built:  0/1
Failed:         1
Errors:         1
Status:         ✗ FAILURE
```

Code de retour de `jenga` : **1**.

Ce que j'observe :
1. **NKPlatform disparaît du « Build Order »** : un seul projet est construit. Sans `dependson`, Jenga ne sait plus que MonEssai en a besoin.
2. **La compilation passe**, grâce au chemin d'en-têtes ajouté à la main.
3. **L'édition de liens reçoit un simple nom** : `-lNKPlatform`, **sans aucun `-L`**. C'est exactement ce que dit le chapitre : `links(["X11"])` devient `-lX11`, « et rien de plus ».
4. **C'est `ld`, l'éditeur de liens, qui échoue**, pas Jenga : Jenga ne vérifie pas que la bibliothèque existe. Pourtant `NKPlatform.lib` est bien sur le disque, dans `Build\Lib\Debug-Windows`… mais personne ne dit à `ld` d'y regarder.

---

## 5. Expérience 2 — remettre `dependson`, retirer `links`

```python
    # EXPERIENCE 2 : dependson remis, links retire
    dependson(["NKPlatform"])
```

J'ai d'abord **supprimé l'ancien `MonEssai.exe`**, pour être sûr que l'édition de liens soit vraiment refaite et que le succès ne vienne pas d'un reste de l'essai précédent.

**Message 2 :**

```text
Build Order (2 projects):
  1. NKPlatform [STATIC_LIB] →
  2. MonEssai [CONSOLE_APP] (depends: NKPlatform)
✓ Built: Build\Lib\Debug-Windows\NKPlatform.lib
✓   [1/1] Compiled: main.cpp
[Link:Clang:MonEssai] C:\msys64\ucrt64\bin\clang++.EXE -o ...\Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
    ...\Build\Obj\Debug-Windows\MonEssai\src_main.obj
    -LC:\...\Nkentseu\Build\Lib\Debug-Windows
    -Wl,--start-group C:\...\Nkentseu\Build\Lib\Debug-Windows\NKPlatform.lib -Wl,--end-group
    --target=x86_64-w64-windows-gnu
✓ Built: Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
BUILD COMPLETED
Projects Built:  2/2
Status:         ✓ SUCCESS
```

Code de retour : **0**. L'exécutable se lance : **0 octet affiché, code 0**.

**Pas un seul message d'erreur**, et la ligne d'édition de liens est **identique** à celle de l'état de départ, où `links` était présent. Le code de Jenga explique pourquoi (`Core/Builder.py:2332-2368`). Pour chaque projet listé dans `dependson` qui est une bibliothèque, Jenga :
- ajoute son dossier de sortie aux dossiers de recherche (le `-L`) ;
- ajoute le chemin complet de son fichier `.lib` à l'édition de liens, s'il n'y est pas déjà.

`links(["NKPlatform"])` ne servait donc à rien : `dependson` faisait déjà le travail.

---

## 6. Les deux messages côte à côte

| | Expérience 1 : sans `dependson` | Expérience 2 : sans `links` |
|---|---|---|
| **Build Order** | 1 projet : MonEssai seul | 2 projets : NKPlatform, puis MonEssai |
| **NKPlatform construit ?** | Non | Oui, en premier |
| **Compilation de `main.cpp`** |  réussie |  réussie |
| **Ligne d'édition de liens** | `-lNKPlatform`, **sans `-L`** | `-L…\Build\Lib\Debug-Windows` + **chemin complet** de `NKPlatform.lib` |
| **Qui signale le problème ?** | `ld`, l'éditeur de liens de MinGW | personne |
| **Message** | `ld: cannot find -lNKPlatform: No such file or directory` puis `clang++: error: linker command failed with exit code 1` | aucun, `BUILD COMPLETED` |
| **Bilan Jenga** | `Projects Built: 0/1`, `Status: ✗ FAILURE`, code 1 | `Projects Built: 2/2`, `Status: ✓ SUCCESS`, code 0 |
| **Exécutable** | non produit | produit, s'exécute sans rien afficher |

### Ce qui les distingue

1. **Un échec contre un succès.** Retirer `dependson` casse la construction ; retirer `links` ne change rien du tout.
2. **Le moment de l'échec.** Dans l'expérience 1, tout va bien jusqu'à la toute dernière étape : la compilation réussit, c'est l'**édition de liens** qui échoue.
3. **Qui parle.** Le message ne vient pas de Jenga, mais de `ld`. Il ne mentionne ni `MonEssai.jenga`, ni `dependson` : il faut savoir le relier soi-même à la ligne retirée.
4. **Ce que porte chaque mot, dans Jenga 2.8.0 :**
   - `dependson` porte l'**ordre** de construction **et** la **bibliothèque à lier** (dossier et fichier) ;
   - `links` ne porte qu'un **nom**, transmis tel quel à l'éditeur de liens. Pour un projet du workspace, il est redondant avec `dependson`. Il reste indispensable pour les bibliothèques **du système** (`user32`, `pthread`…), que Jenga ne construit pas ;
   - les **chemins d'en-têtes** ne sont portés par aucun des deux : il faut `includedirs`.

---

## 7. Ce que l'expérience confirme ou contredit dans le chapitre

| Le chapitre « Les dépendances » dit… | Ce que j'ai observé |
|---|---|
| `dependson` impose un **ordre** (ligne 19) |  Confirmé : 2 projets avec `dependson`, 1 seul sans. |
| `dependson` fait **hériter** les chemins d'inclusion (lignes 20-21) |  **Pas dans Jenga 2.8.0** : message 0, « file not found », alors que `dependson` était présent. Aucun code de Jenga ne transmet les `includeDirs`. |
| `links(["X11"])` devient `-lX11`, « et rien de plus » (ligne 70) |  Confirmé : `-lNKPlatform` dans l'expérience 1. |
| Jenga ne vérifie pas que la bibliothèque existe, c'est l'éditeur de liens qui échoue (lignes 70-72) |  Confirmé : le message vient de `ld`. |
| La démonstration attendue : sans `dependson`, l'échec serait à la **compilation**, sur l'en-tête introuvable (lignes 236-239) |  **Pas reproductible tel quel.** Sans chemin d'en-têtes écrit à la main, l'échec est à la compilation **même avec** `dependson` (message 0). Avec le chemin écrit à la main, retirer `dependson` fait échouer l'**édition de liens** (message 1). |

**Ce que j'en retiens.** Il existe bien deux échecs de nature différente, un à la compilation et un à l'édition de liens, mais pour d'autres raisons que celles du chapitre :
- l'échec à la **compilation** vient d'un `includedirs` manquant ;
- l'échec à l'**édition de liens** vient d'un `dependson` manquant.

C'est aussi pour cela que Nkentseu a écrit `nkentseudependson` : cette fonction ajoute **elle-même** les chemins `%{<module>.location}/src` de chaque dépendance (`config/modules.jenga:379-392`). Dans le dépôt, le message 0 ne peut donc pas se produire.

---

## 8. Ce que cela éclaire dans nos exercices précédents

| Exercice précédent | Ce que cette expérience explique |
|---|---|
| **Annotation de NKMemory.jenga**, ligne 27 : « en mode bibliothèque, NKCore et NKPlatform sont déclarés mais pas liés » | Ce n'est pas grave : c'est l'**application finale** qui les lie, parce que Jenga transforme chaque `dependson` en fichier `.lib` au moment de l'édition de liens. |
| **NKMemory, question  n°10** : que devient `links(["pthread"])` sur une bibliothèque statique ? | Début de réponse : `Builder.py:2370-2413` transmet les bibliothèques **système** des dépendances statiques à l'exécutable final, en fin de ligne. |
| **Construction de NKMath** : « Build Order (5 projects) » | La ligne `Build Order` est bien le reflet des `dependson` : un `dependson` retiré, un projet de moins. |
| **MonEssai, exercice précédent** : « Build Order (1 projects) » | Avec une vraie dépendance, on passe à 2 projets. |
| **Question d'étudiant** : « pourquoi tout est-il recompilé à chaque construction ? » | Même constat ici : les 7 `.cpp` de NKPlatform sont recompilés à chaque essai. La question reste ouverte . |

---

## 9. Mes questions d'étudiant

| Ma question | Mon hypothèse | Comment je pourrais vérifier |
|---|---|---|
| Dans l'expérience 1, `-lNKPlatform` aurait-il trouvé le fichier si j'avais ajouté le `-L` à la main ? | Pas sûr : sous MinGW, `-lNKPlatform` cherche plutôt un fichier `libNKPlatform.a`, et le fichier s'appelle `NKPlatform.lib`.  | Ajouter `libdirs(["%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}"])` sans `dependson`, et reconstruire |
| Garder `links(["NKPlatform"])` en plus de `dependson` est-il gênant ? | Non : Jenga retire les doublons (`seen`, `Builder.py:2348-2358`). La ligne d'édition de liens était identique dans les deux cas. | Déjà observé : départ et expérience 2 donnent la même ligne |
| Le chapitre a-t-il été écrit pour une autre version de Jenga, qui transmettait les en-têtes ? | Possible : le cours de Jenga et Jenga 2.8.0 ont pu évoluer séparément. | `git log` sur `Documentation/jenga/tex/chapitres/07-les-dependances.tex` et sur `Jenga/Core/Builder.py` |
| Si `NKPlatform.lib` n'existait pas sur le disque, le message 1 serait-il différent ? | Non : sans `-L`, `ld` ne regarde de toute façon pas dans `Build\Lib`. | Renommer `NKPlatform.lib` et refaire l'expérience 1 |
| Comment retrouver la ligne fautive quand le message de `ld` ne cite pas le `.jenga` ? | En lisant la ligne `[Link:…]` de `--verbose` : `-lNKPlatform` sans `-L` montre que Jenga n'a pas résolu le nom en projet. | Comparer les lignes `[Link:…]` des deux essais, comme au §6 |

---

## 10. État final du dépôt

J'ai **remis MonEssai dans son état de départ**, avec `dependson` et `links` : c'est la version qui se construit et s'exécute correctement.

```python
    files(["src/**.cpp"])
    # Chemin d'inclusion de NKPlatform ecrit a la main : dependson ne le
    # transmet PAS (constate : « 'NKPlatform/NkCPUFeatures.h' file not found »).
    includedirs(["src", "%{NKPlatform.location}/src"])

    dependson(["NKPlatform"])
    links(["NKPlatform"])
```

| Fichier | État |
|---|---|
| `Applications/MonEssai/MonEssai.jenga` | réécrit : `dependson` et `links` explicites, et chemin d'en-têtes de NKPlatform |
| `Applications/MonEssai/src/main.cpp` | appelle maintenant `CPUFeatures::Get()`, et n'affiche toujours rien |
| `Nkentseu.jenga` | inchangé depuis l'exercice précédent (5 lignes `include`) |
| git | rien n'est commité : `Nkentseu.jenga` modifié, `Applications/MonEssai/` non suivi. `Build/` est ignoré. |

