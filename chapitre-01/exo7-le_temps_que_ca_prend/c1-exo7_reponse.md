# Deux constructions d'affilée : pourquoi la seconde n'est pas plus rapide

> **La consigne** : chronométrer une construction complète, puis une seconde lancée juste après sans rien modifier, et expliquer l'écart entre les deux.
> **Le dépôt** : `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu`, le 2026-09-14. Jenga 2.8.0, configuration Debug, cible Windows x86_64, toolchain `clang-mingw`, machine à 8 cœurs logiques.
> **Les commandes** se tapent à la racine du dépôt, dans le terminal *Git Bash* de VS Code.

---

## 1. En bref

| Construction | Commande | Temps mesuré | Temps affiché par Jenga | Fichiers compilés |
|---|---|---:|---:|---:|
| **1ʳᵉ, complète** | `jenga rebuild --target NKMath` | 23,1 s | 17,05 s | 81 |
| **2ᵉ, juste après** | `jenga build --target NKMath` | 21,4 s | 17,13 s | **81** |
| 3ᵉ, pour vérifier | `jenga build --target NKMath` | 21,7 s | 17,71 s | **81** |

**L'écart est quasi nul.** La seconde construction recompile les 81 fichiers, exactement comme la première. Le chapitre « La construction » du cours Jenga annonce pourtant qu'une seconde construction sans modification ne recompile rien.

**La cause, démontrée : l'espace dans le chemin du dépôt** (`gap l2`, `chap 1`). Pour savoir si un fichier est à jour, Jenga relit la liste des en-têtes qu'il utilise, dans un fichier `.d`. Il découpe mal les chemins qui contiennent un espace. Il juge alors **tous** les en-têtes « introuvables », et recompile tout, à chaque fois.

**La contre-épreuve le confirme.** Le même petit projet, construit deux fois :
- dans un dossier **sans** espace, la seconde construction compile **0 fichier** (`All files up to date`, 0,02 s au lieu de 0,21 s) ;
- dans un dossier **avec** espace, la seconde construction recompile (1 fichier, 0,23 s).

---

## 2. Ce que j'ai chronométré, et pourquoi pas tout le workspace

Une construction « complète » de tout le workspace, c'est **274 projets**, dont des démos Vulkan, Android, XR ou IA. Elle prendrait des heures et échouerait sur les SDK absents de cette machine. La mesure ne dirait alors plus rien de l'écart entre deux constructions.

J'ai donc chronométré **la chaîne NKMath**, déjà construite dans nos exercices : 5 projets (NKPlatform, NKCore, NKMemory, NKContainers, NKMath) et 81 fichiers `.cpp`. Elle est assez grosse pour que la différence se voie, et assez petite pour être relancée plusieurs fois.

- **Construction complète** = `jenga rebuild --target NKMath`. `rebuild` nettoie d'abord : 86 fichiers supprimés, objets, PCH et bibliothèques. C'est donc bien une construction depuis zéro.
- **Seconde construction** = `jenga build --target NKMath`, lancée immédiatement après, sans toucher à aucun fichier.

J'ai d'abord vérifié qu'**aucun autre processus** (`python`, `clang`, `ld`, `NKCode`) ne tournait pendant les mesures.

---

## 3. Les mesures

### 3.1 Les commandes

Chaque commande est chronométrée « au mur », de son lancement à sa fin, avec l'horloge du système en millisecondes :

```bash
t0=$(date +%s%3N); jenga info > /dev/null 2>&1; t1=$(date +%s%3N); echo "info : $((t1-t0)) ms"
t0=$(date +%s%3N); jenga rebuild --target NKMath > R1.log 2>&1; t1=$(date +%s%3N); echo "R1 : $((t1-t0)) ms"
t0=$(date +%s%3N); jenga build --target NKMath > R2.log 2>&1; t1=$(date +%s%3N); echo "R2 : $((t1-t0)) ms"
t0=$(date +%s%3N); jenga build --target NKMath > R3.log 2>&1; t1=$(date +%s%3N); echo "R3 : $((t1-t0)) ms"
```

Pour compter les fichiers compilés dans un journal :

```bash
grep -c 'Compiled:' R2.log
```

### 3.2 Les résultats (série de 17:33 à 17:35)

| Essai | Commande | Code | Temps mesuré | Temps Jenga | Compilés | Supprimés |
|---|---|---:|---:|---:|---:|---:|
| — | `jenga info` (chargement seul) | 0 | 2 903 ms | — | 0 | 0 |
| R1 | `jenga rebuild --target NKMath` | 0 | 23 068 ms | 17,05 s | 81 | 86 |
| R2 | `jenga build --target NKMath` | 0 | 21 363 ms | 17,13 s | 81 | 0 |
| R3 | `jenga build --target NKMath` | 0 | 21 657 ms | 17,71 s | 81 | 0 |
| M1 | `jenga build --target MonEssai` | 0 | 6 735 ms | 2,47 s | 8 | 0 |
| M2 | `jenga build --target MonEssai` | 0 | 6 238 ms | 2,20 s | 8 | 0 |

Temps de chaque projet, tel qu'affiché par Jenga :

| Projet | Fichiers | R1 | R2 | R3 |
|---|---:|---:|---:|---:|
| NKPlatform | 7 | 2,28 s | 2,07 s | 1,97 s |
| NKCore | 5 | 1,81 s | 1,59 s | 1,60 s |
| NKMemory | 14 | 2,66 s | 2,41 s | 2,49 s |
| NKContainers | 43 | 4,49 s | 5,31 s | 5,70 s |
| NKMath | 12 | 5,80 s | 5,76 s | 5,95 s |
| **Total** | **81** | **17,05 s** | **17,13 s** | **17,71 s** |

Un premier relevé, fait quelques minutes plus tôt, donnait la même chose : **18,09 s** pour `rebuild`, **18,01 s** pour le `build` suivant, et 81 fichiers compilés les deux fois. Dans le journal de R2, aucune ligne ne parle de cache ou de fichier « à jour ».

---

## 4. L'écart attendu

Le chapitre `Documentation/jenga/tex/chapitres/09-la-construction.tex` décrit ce qui **devrait** se passer.

- **Quatre raisons de recompiler un fichier** (lignes 65-67) : l'objet n'existe pas ; la source est plus récente que lui ; le fichier de dépendances `.d` manque ; un en-tête dont il dépend est plus récent.
- **Pourquoi la première construction est plus lente** (lignes 76-77) : il n'y a pas encore de fichier `.d`, donc tout est recompilé.

Le code de Jenga (`Core/Builder.py:1549-1591`) ajoute un cinquième critère : la **signature de compilation** (`.jenga_sig`). C'est une empreinte des options de compilation (defines, chemins d'en-têtes, toolchain…).

Juste après une construction réussie, aucun de ces critères ne devrait déclencher de recompilation. On attendait donc **une seconde construction très courte** : pas de compilation, et pas d'édition de liens non plus, puisque Jenga la saute quand la cible est plus récente que ses entrées (`_CibleDejaAJour`, `Builder.py:373-405`).

---

## 5. L'enquête : pourquoi tout est recompilé

### 5.1 Ce qui change sur le disque entre les deux constructions

Entre R1 et R2, j'ai relevé la date de chaque fichier de `Build/Obj` pour les 5 projets, et l'empreinte `md5` de chaque `.jenga_sig`, avant et après :

| Fichiers | Résultat |
|---|---|
| 81 objets `.obj` | **81 réécrits** (date changée) |
| 5 en-têtes précompilés `.pch` | **5 reconstruits** |
| 81 signatures `.jenga_sig` | **81 contenus différents** |

Deux suspects se dégagent : le PCH, puisque toutes les sources dépendent de lui, et la signature, puisque son contenu change.

### 5.2 Fausse piste n°1 : le PCH

Si seul le PCH était en cause, un fichier **sans PCH** ne serait pas recompilé. Or `main.cpp` de MonEssai n'utilise pas de PCH, et il est recompilé dans M1 **et** dans M2 (`Compiled: main.cpp` dans les deux journaux). Le PCH n'est donc pas la cause première.

### 5.3 Fausse piste n°2 : la signature de compilation

La signature est l'empreinte `sha256` d'un texte JSON qui décrit la compilation (`Builder.py:1615-1675`). Ce texte contient notamment **l'action** en cours. Entre `rebuild` (R1) et `build` (R2), l'action change, ce qui explique à lui seul que les 81 `.jenga_sig` diffèrent. Mais cela n'explique pas R2 → R3, deux `build` identiques qui recompilent tout.

Pour trancher sans deviner, j'ai **enregistré le texte JSON de chaque signature** pendant deux constructions de MonEssai. Je n'ai pas modifié Jenga : un petit script enveloppe `json.dumps` le temps de l'exécution, puis un second script compare les deux enregistrements.

```bash
python sigprobe.py sig1.jsonl build --target MonEssai --no-daemon
python sigprobe.py sig2.jsonl build --target MonEssai --no-daemon
python sigdiff.py sig1.jsonl sig2.jsonl
```

Résultat :

```text
signatures run1=8 run2=8
identiques=8 differentes=0
```

**Les 8 signatures sont identiques d'une construction à l'autre**, et pourtant les 8 fichiers sont recompilés. La décision de recompiler est donc prise **plus tôt** dans `_NeedsCompileSource`, sur l'un des quatre critères du chapitre.

### 5.4 La vraie cause : l'espace dans le chemin

Voici le fichier `.d` que le compilateur a écrit pour `main.cpp` (`cat -A` rend visibles les fins de ligne) :

```text
C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu\Build\Obj\Debug-Windows\MonEssai\src_main.obj: \^M$
  C:/Users/ngatc/Desktop/gap\ l2/chap\ 1/Nkentseu/Applications/MonEssai/src/main.cpp \^M$
  C:/Users/ngatc/Desktop/gap\ l2/chap\ 1/Nkentseu/Kernel/Foundation/NKPlatform/src/NKPlatform/NkCPUFeatures.h \^M$
  ...
```

Le compilateur fait bien son travail : dans ce format, un espace dans un chemin s'écrit `\ `. Le problème est dans la façon dont Jenga relit ce fichier (`Builder.py:1522-1536`) :

```python
content = content.replace("\\\r\n", " ").replace("\\\n", " ")
tokens = content.split()                                     # 1. decoupe sur TOUS les espaces
...
cleaned = token.strip().rstrip("\\")
cleaned = cleaned.replace("\\ ", " ").replace("\\:", ":")   # 2. trop tard : il n'y a plus d'espace
```

L'ordre est inversé. Le texte est **d'abord** découpé sur tous les espaces, **puis** Jenga cherche les `\ ` à remettre en espaces. Mais le découpage les a déjà cassés. Le chemin `C:/Users/ngatc/Desktop/gap\ l2/chap\ 1/Nkentseu/…/main.cpp` devient trois morceaux, dont aucun n'existe.

J'ai lancé **le vrai code de Jenga** (`Builder._ParseDependencyFile`) sur trois fichiers `.d` du dépôt :

```text
=== Build\Obj\Debug-Windows\MonEssai\src_main.obj.d ===
chemins extraits par Jenga : 21   introuvables : 21
  INTROUVABLE  C:\Users\ngatc\Desktop\gap
  INTROUVABLE  C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu\l2\chap
  INTROUVABLE  C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu\1\Nkentseu\Applications\MonEssai\src\main.cpp

=== Build\Obj\Debug-Windows\NKPlatform\NKPlatform.pch.d ===
chemins extraits par Jenga : 3   introuvables : 3

=== Build\Obj\Debug-Windows\NKPlatform\src_NKPlatform_NkEnv.obj.d ===
chemins extraits par Jenga : 24   introuvables : 24
```

**100 % des chemins sont déclarés introuvables.** Or `_NeedsCompileSource` recompile dès qu'un seul en-tête manque (`Builder.py:1577-1580`) :

```python
for dep in deps:
    if not dep.exists():
        return True      # un en-tete « introuvable » : on recompile
```

Le PCH subit le même sort. Sa vérification (`PchIsFresh`, `Builder.py:1498-1505`) relit son propre `.d` avec la même fonction, trouve des chemins « introuvables », et le reconstruit à chaque fois. Or le code de Jenga contient un commentaire (`Builders/Windows.py:85-95`) qui promettait justement d'éviter de refaire le PCH à chaque construction. Ici, cette protection ne peut pas fonctionner.

### 5.5 La contre-épreuve

Pour être sûr que l'espace est **la** cause, j'ai construit deux fois le même projet minimal : un `main.cpp` qui n'affiche rien et inclut un en-tête local `outil.h`. La première copie est dans un dossier sans espace, la seconde dans un dossier avec espace. Même `.jenga`, même toolchain.

```bash
cd "…/scratchpad/SansEspace"   && jenga build --no-daemon && jenga build --no-daemon
cd "…/scratchpad/Avec Espace"  && jenga build --no-daemon && jenga build --no-daemon
```

| Dossier | Construction | Temps mesuré | Temps Jenga | Compilés | Message |
|---|---|---:|---:|---:|---|
| `SansEspace` | 1ʳᵉ | 1 897 ms | 0,21 s | 1 | — |
| `SansEspace` | 2ᵉ | 1 683 ms | **0,02 s** | **0** | **`✓ All files up to date`** |
| `Avec Espace` | 1ʳᵉ | 1 850 ms | 0,21 s | 1 | — |
| `Avec Espace` | 2ᵉ | 1 828 ms | 0,23 s | **1** | *(recompilé)* |

Dans le `.d` du dossier avec espace, on retrouve la même écriture `Avec\ Espace/main.cpp`. **Seul l'espace distingue les deux dossiers**, et c'est exactement lui qui fait disparaître le gain de la seconde construction.

---

## 6. L'écart, expliqué

### 6.1 Dans Nkentseu, tel qu'il est installé sur cette machine

| Poste | 1ʳᵉ (rebuild) | 2ᵉ (build) | Pourquoi |
|---|---:|---:|---|
| Chargement du workspace par Jenga | ≈ 3 s | ≈ 3 s | Lire `Nkentseu.jenga` et ses 274 projets. `jenga info` seul prend 2,9 s. |
| Nettoyage | ≈ 1,7 s (probable) | 0 | Seul `rebuild` supprime les 86 fichiers : c'est l'écart entre 23,1 et 21,4 s. |
| Construction des 5 projets | 17,05 s | 17,13 s | **Identique** : 5 PCH et 81 `.cpp` recompilés, 5 éditions de liens, dans les deux cas. |
| **Total mesuré** | **23,1 s** | **21,4 s** | L'écart d'1,7 s vient du nettoyage, pas de la compilation. |

Les petites variations d'un essai à l'autre (17,05 / 17,13 / 17,71 s) relèvent du bruit : une compilation n'a jamais exactement la même durée.

**Explication, en une phrase : il n'y a pas d'écart, parce que Jenga ne sait pas relire ses fichiers `.d` quand le chemin du dépôt contient un espace. Chaque construction se comporte comme une toute première construction.**

### 6.2 Là où l'écart existe

Dans un dossier sans espace, la seconde construction ne compile rien : 0,21 s devient **0,02 s**, et `All files up to date` s'affiche. Transposé à NKMath, on s'attendrait à ce qu'une seconde construction se réduise au chargement du workspace, soit quelques secondes au lieu d'une vingtaine. **C'est une estimation** : je ne l'ai pas mesurée sur Nkentseu, puisque cela demanderait de déplacer le dépôt.

---

## 7. Ce que cela éclaire dans nos exercices précédents

| Ce qu'on avait vu | L'explication |
|---|---|
| **Construction de NKMath** : à 16:06, puis 16:09, les 81 fichiers recompilés les deux fois (20,59 s puis 16,64 s) | Même cause : aucun `.d` n'est lisible. |
| **Annotation de NKMemory, question d'étudiant** : « pourquoi la 2ᵉ construction a-t-elle tout recompilé ? » (marquée ❓) | Question résolue : ni la toolchain ni `.jenga_sig`, mais l'espace dans le chemin. |
| **Annotation de NKMemory** : « les 86 fichiers `.d` doivent servir à savoir quoi recompiler » | L'hypothèse était juste, mais ici ces fichiers sont illisibles pour Jenga. |
| **Expérience `dependson` / `links`** : NKPlatform recompilé à chaque essai | Même cause. |
| **Comptage des sources** : le chemin du dépôt est `…\gap l2\chap 1\Nkentseu` | C'est ce chemin, et pas le code de Nkentseu, qui supprime la construction incrémentale. |
| **Cours, chapitre 9** : « la décision se prend sur les dates » | Vrai, mais on n'arrive même pas à comparer les dates : Jenga s'arrête avant, sur des en-têtes « introuvables ». |

---

## 8. Ce qu'on pourrait faire (rien n'a été appliqué)

| Option | Effet attendu | Coût |
|---|---|---|
| **Placer le dépôt dans un chemin sans espace**, par exemple `C:\Users\ngatc\Desktop\gap_l2\chap1\Nkentseu` | La construction incrémentale fonctionnerait, comme dans la contre-épreuve. | Déplacer le dossier, puis relancer une construction complète. À décider par vous. |
| **Signaler le défaut aux mainteneurs de Jenga** : `Core/Builder.py`, `_ParseDependencyFile` | Corrige le problème pour tout le monde. | Il faudrait protéger les `\ ` **avant** de découper le texte. C'est une piste, non testée. |
| **Ne rien changer** | Chaque construction coûte le prix d'une construction complète. | À savoir avant toute mesure de performance. |

Je n'ai **pas modifié Jenga** : c'est un outil installé à part, et le corriger ici fausserait la comparaison avec les autres machines.

---

## 9. Mes questions d'étudiant

| Ma question | Mon hypothèse | Comment je pourrais vérifier |
|---|---|---|
| Combien de temps prendrait la seconde construction de NKMath si le dépôt était dans un chemin sans espace ? | Environ 3 s (chargement), puis `All files up to date` pour les 5 projets. | Cloner le dépôt dans `C:\dev\Nkentseu` et refaire R1 puis R2 |
| Les autres machines de l'équipe ont-elles le même problème ? | Seulement si leur chemin contient un espace. `D:\Projets\2026\Nkentseu\Nkentseu`, cité dans les notes du cours, n'en a pas. | Comparer avec un journal de construction de l'intégration continue (`.github/`) |
| La bibliothèque est-elle quand même re-liée à chaque fois ? | Oui : des objets tout neufs sont plus récents que la cible, donc `_CibleDejaAJour` répond « non ». | Compter les lignes `Linking...` : 5 dans R2 |
| Pourquoi un espace dans le nom d'un dossier casse-t-il un outil en 2026 ? | Le format `.d` vient de Make, où l'espace sépare les fichiers. Il faut l'échapper, et donc aussi le lire correctement. | Chercher d'autres `.split()` sur des chemins dans `Jenga/Core` |
| Les fins de ligne Windows (`\r\n`) des `.d` jouent-elles un rôle ? | Non : Jenga les traite (`replace("\\\r\n", " ")`), et dans le dossier sans espace tout fonctionne. | Déjà démontré par la contre-épreuve |

---

## 10. Limites et état du dépôt

- **Périmètre** : la chaîne NKMath (5 projets), pas les 274 projets du workspace. La cause trouvée touche pourtant **tout** fichier dont le `.d` contient le chemin du dépôt, c'est-à-dire tous.
- **Contre-épreuve** : les deux dossiers ont été construits **en même temps**. Les temps « au mur » sont donc légèrement pollués ; le nombre de fichiers compilés et le message `All files up to date`, eux, ne le sont pas.
- **Temps de nettoyage** : l'attribution d'environ 1,7 s au nettoyage est une déduction (23,1 − 21,4 s), pas une mesure séparée.
- **Aucun fichier du dépôt n'a été modifié pour cet exercice.** Seuls les fichiers produits dans `Build/` ont été régénérés, et git les ignore. Les scripts `sigprobe.py` et `sigdiff.py`, ainsi que les deux projets de contre-épreuve, sont dans un dossier temporaire, hors du dépôt.&