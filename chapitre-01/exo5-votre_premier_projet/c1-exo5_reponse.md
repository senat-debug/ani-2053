# MonEssai : de zéro à un exécutable qui ne fait rien

> **La consigne** : créer `Applications/MonEssai` avec son `.jenga` et un `main.cpp` qui n'affiche rien, le déclarer au workspace, vérifier qu'il apparaît dans `jenga info`, puis le construire.
> **Le dépôt** : `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu`, le 2026-09-14. Jenga 2.8.0, configuration Debug, cible Windows x86_64.
> **Les commandes** se tapent à la racine du dépôt, dans le terminal de VS Code : *Terminal › Nouveau terminal*. Choisissez *Git Bash* pour celles marquées `bash`.

## 1. En bref

| Étape | Résultat |
|---|---|
| Créer le dossier et les 2 fichiers | `MonEssai.jenga` (882 octets) et `src/main.cpp` (127 octets) |
| Déclarer au workspace | 5 lignes ajoutées à la fin de `Nkentseu.jenga` |
| Vérifier dans `jenga info` | ✅ ligne 304 : `MonEssai  ConsoleApp  C++  No  Yes`. Le workspace passe de **273 à 274 projets** |
| Construire | ✅ `BUILD COMPLETED`, 1 projet, 1 fichier compilé, 0,50 s |
| Vérifier qu'il n'affiche rien | ✅ 0 octet affiché, code de retour 0 |

Avant de commencer, j'ai vérifié que `Applications/MonEssai` n'existait pas et que `Nkentseu.jenga` n'avait aucune modification en cours (`git status` vide). Ainsi, le seul changement du workspace est le mien.

---

## 2. Étape 1 — créer le dossier et les fichiers

```text
Applications/MonEssai/
├── MonEssai.jenga
└── src/
    └── main.cpp
```

J'ai suivi la même organisation que les autres applications : un `.jenga` à la racine du dossier, le code dans `src/`.

### 2.1 `src/main.cpp`

```cpp
// MonEssai — point d'entree minimal : ne fait rien, n'affiche rien.
// Code de retour 0 = succes.
int main() {
	return 0;
}
```

Aucun `#include` n'est nécessaire, puisque rien n'est affiché. `return 0` signale au système que le programme s'est bien terminé.

### 2.2 `MonEssai.jenga`

Mon modèle est `Applications/NkNavCoreDemo/NkNavCoreDemo.jenga`, une vraie application console du dépôt. J'en ai retiré tout ce dont un programme vide n'a pas besoin.

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""MonEssai — application d'essai minimale : un main() qui n'affiche rien.

Sert a verifier la chaine complete : declarer un projet, le voir dans
`jenga info`, puis le construire. Aucune dependance Nkentseu.
Modele : Applications/NkNavCoreDemo/NkNavCoreDemo.jenga.
"""

from Jenga import *
from jengaconfig import *

with project("MonEssai"):
    consoleapp()
    language("C++")
    cppdialect("C++17")
    location(".")

    files(["src/**.cpp"])

    nkentseudependson(
        [],
        extra_includes=["src"],
    )

    objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
    targetdir("%{wks.location}/Build/Bin/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")

    with filter("system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne"):
        usetoolchain(TC_WINDOWS)
```

Mes choix, et pourquoi :

| Ligne | Mon choix | Pourquoi |
|---|---|---|
| `consoleapp()` | Application **console** | Le programme n'ouvre pas de fenêtre. Les exemples du cours (`MonApp`, `NkAtelier`) utilisent `windowedapp()` parce qu'ils affichent une interface. Ici, contrairement à NKMemory, **le type est écrit en clair** (`Api.py:1572`). |
| `files(["src/**.cpp"])` | Tous les `.cpp` de `src/` | Il n'y en a qu'un, mais un deuxième fichier serait pris sans toucher au `.jenga`. |
| `nkentseudependson([], …)` | **Aucune dépendance**, mais la fonction est quand même appelée | Un `main` vide n'a besoin d'aucun module. J'appelle la fonction pour rester dans la convention du dépôt : elle ajoute `src` aux en-têtes, et sous Linux elle pose les réglages de fenêtrage et range les fichiers par backend, comme pour tous les projets (`modules.jenga:435`). Sans `selfexport`, elle fonctionne en mode **application**. |
| `targetdir(".../Build/Bin/...")` | `Bin`, pas `Lib` | Un exécutable va dans `Build/Bin/<config>-<système>/<projet>/`, comme `MonApp` dans le cours (`04-application.md:1031`). Les bibliothèques comme NKMemory vont dans `Build/Lib`. |
| `usetoolchain(TC_WINDOWS)` dans le filtre Windows | La toolchain de Nkentseu | C'est ce que font NkNavCoreDemo, NKMemory et les autres projets sous Windows. |
| *Pas de* `pchheader`, `links`, `defines` | Rien de superflu | Pas d'en-tête précompilé pour un seul fichier de 5 lignes, pas de bibliothèque système, pas de réglage Debug/Release particulier. |

---

## 3. Étape 2 — déclarer l'application au workspace

Un `.jenga` isolé n'est pas vu par Jenga : il faut l'**inclure** depuis le fichier racine. Le cours le rappelle (`10-projet-final.md:171`). J'ai ajouté la déclaration après la dernière application, `UnkenyEditor`, dans la même forme que les autres.

Commande pour voir la modification :

```powershell
git diff -- Nkentseu.jenga
```

Résultat :

```diff
@@ -1726,3 +1726,8 @@ with workspace("Nkentseu", location="."):
     # contient pas son outil.
     with include("Applications/UnkenyEditor/UnkenyEditor.jenga"):
         pass
+
+    # MonEssai — application d'essai minimale (main vide), pour verifier
+    # la chaine declaration -> jenga info -> jenga build.
+    with include("Applications/MonEssai/MonEssai.jenga"):
+        pass
```

`git diff --stat` confirme : **1 fichier modifié, 5 lignes ajoutées**, aucune supprimée.

---

## 4. Étape 3 — vérifier dans `jenga info`

```bash
jenga info
```

Pour ne garder que ce qui m'intéresse, j'ai enregistré la sortie dans un fichier, retiré les couleurs, puis cherché MonEssai :

```bash
jenga info > info.log 2>&1
sed -e 's/\x1b\[[0-9;]*m//g' info.log > info.txt
grep -n "MonEssai" info.txt
```

Résultat :

```text
304:MonEssai                     ConsoleApp    C++        No     Yes
```

**MonEssai apparaît bien**, avec le bon type (`ConsoleApp`), le bon langage, et `No` dans la colonne Test (ce n'est pas une suite de tests).

Comparaison avec le rapport workspace du début :

| Type | Avant | Après |
|---|---:|---:|
| ConsoleApp | 98 | **99** |
| StaticLib | 60 | 60 |
| TestSuite | 60 | 60 |
| WindowedApp | 55 | 55 |
| **Total** | **273** | **274** |

Le reste n'a pas bougé : même fichier racine (`Nkentseu.jenga`), même projet de démarrage (`Sandbox`), mêmes 6 toolchains. Seul avertissement affiché, celui qu'on connaît déjà : `[NKCode] ATTENTION : aucun wheel Jenga trouve…`.

---

## 5. Étape 4 — construire

```bash
jenga build --target MonEssai
```

Ce que Jenga a affiché (couleurs retirées) :

```text
Configuration: Debug
Target:        Windows x86_64
Toolchain:     clang-mingw
Build Order (1 projects):
  1. MonEssai [CONSOLE_APP]
ℹ Found 1 source file(s)
✓   [1/1] Compiled: main.cpp
ℹ Linking...
✓ Built: Build\Bin\Debug-Windows\MonEssai\MonEssai.exe
│  ✓ Build Successful                                     Time: 0.50s  │
BUILD COMPLETED
Projects Built:  1/1
Time:           0.50s
Status:         ✓ SUCCESS
```

Code de retour de la commande : **0**. Durée mesurée à la montre : 4 secondes (de 17:06:12 à 17:06:16), chargement du workspace compris.

---

## 6. Étape 5 — vérifier qu'il n'affiche vraiment rien

```bash
OUT=$(./Build/Bin/Debug-Windows/MonEssai/MonEssai.exe 2>&1); CODE=$?
echo "code retour: $CODE"
echo "octets affiches (stdout+stderr): $(printf '%s' "$OUT" | wc -c)"
```

Résultat :

```text
code retour: 0
octets affiches (stdout+stderr): 0
```

Le programme n'écrit **rien**, ni sur la sortie normale ni sur la sortie d'erreur, et se termine avec succès.

Fichiers produits :

```powershell
Get-ChildItem Build\Bin\Debug-Windows\MonEssai, Build\Obj\Debug-Windows\MonEssai -Recurse -File
git check-ignore -v Build/Bin/Debug-Windows/MonEssai/MonEssai.exe
```

| Fichier | Taille | Rôle |
|---|---:|---|
| `Build/Bin/Debug-Windows/MonEssai/MonEssai.exe` | 131 420 octets | le programme |
| `Build/Obj/Debug-Windows/MonEssai/src_main.obj` | 1 702 octets | `main.cpp` compilé |
| `Build/Obj/Debug-Windows/MonEssai/src_main.obj.d` | 182 octets | liste des fichiers dont `main.cpp` dépend |
| `Build/Obj/Debug-Windows/MonEssai/src_main.obj.jenga_sig` | 66 octets | signature utilisée par Jenga |

git ignore tout cela (`.gitignore:151  [Bb]uild/`) : les fichiers produits ne se retrouveront pas dans un commit.

---

## 7. Ce que cet exercice relie aux précédents

| Ce qu'on avait vu | Ce que MonEssai montre |
|---|---|
| **NKMath** : « Build Order (5 projects) » | MonEssai n'a aucune dépendance, donc « Build Order (**1** projects) ». Jenga ne construit que ce qui est nécessaire. |
| **NKMemory.jenga** : le type était caché dans `selfexport` | Ici, le type est écrit en clair avec `consoleapp()`. Sans `selfexport`, `nkentseudependson` fonctionne en mode application. |
| **NKMemory** : sortie dans `Build/Lib` | Une application sort dans `Build/Bin/…/MonEssai/`, un dossier par programme. |
| **Question d'étudiant** : « à quoi servent les `.d` ? » | Un seul `.d` ici, de 182 octets. C'est un bon fichier pour commencer à regarder ce qu'il contient. |
| **Rapport workspace** : 273 projets | 274 : on voit l'effet direct d'une ligne `include(...)`. |
| **Comptage des sources** : 2 703 fichiers dans le projet | +1 fichier (`main.cpp`, 5 lignes). |
| **Construction de NKMath** : « Toolchain: clang-mingw » | Même affichage, alors que MonEssai demande aussi `TC_WINDOWS` (`nk-windows-clang-mingw`). Le mystère n°7 de l'annotation de NKMemory reste entier. |

---

## 8. Mes questions d'étudiant

| Ma question | Mon hypothèse | Comment vérifier |
|---|---|---|
| Pourquoi un `main` de 5 lignes donne-t-il un `.exe` de **131 Ko** ? | La bibliothèque d'exécution C/C++ de MinGW et les symboles de débogage (Debug) sont inclus dans l'exécutable. | Construire en Release (`--config Release`) et comparer la taille |
| Si je retire la ligne `include`, MonEssai disparaît-il de `jenga info` sans que je supprime le dossier ? | Oui : Jenga ne voit que ce qui est inclus depuis `Nkentseu.jenga`. | Commenter les 2 lignes et relancer `jenga info` : on devrait retrouver 273 |
| L'appel `nkentseudependson([])` est-il vraiment utile pour un programme sans dépendance ? | Sous Windows, presque rien ; sous Linux, il range les fichiers par backend. | Le retirer sur une copie et comparer sous Linux |
| Pourquoi « Found 1 source file(s) » alors que le `.jenga` n'est pas un fichier source ? | Seuls les fichiers de `files()` sont des sources ; le `.jenga` décrit la construction. | Ajouter un 2ᵉ `.cpp` dans `src/` et voir « Found 2 » |
| Faut-il valider (commit) MonEssai dans le dépôt ? | C'est un essai. Le laisser dans `Nkentseu.jenga` ajoute un projet pour tout le monde. | À décider avec l'équipe ; rien n'est commité pour l'instant |
| Que se passe-t-il si je mets une faute dans le nom du `.jenga` inclus ? | `jenga info` échoue au chargement. `Nkentseu.jenga:1704-1708` raconte exactement ce cas pour GemCrush (« External file not found »). | Écrire `MonEsai.jenga` dans l'include et relancer `jenga info` |

---

## 9. État du dépôt et comment annuler

L'essai laisse **deux changements non commités** :

```text
 M Nkentseu.jenga          (5 lignes ajoutées)
?? Applications/MonEssai/  (2 fichiers)
```

Je n'ai rien commité. Pour tout annuler, depuis la racine du dépôt :

```powershell
git checkout -- Nkentseu.jenga
```

```powershell
Remove-Item -Recurse Applications\MonEssai, Build\Bin\Debug-Windows\MonEssai, Build\Obj\Debug-Windows\MonEssai
```

`git checkout -- Nkentseu.jenga` ne retire que mes 5 lignes, puisque le fichier n'avait aucune autre modification avant l'essai.