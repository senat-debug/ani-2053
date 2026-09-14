# Cinq façons de construire — et pourquoi un en-tête coûte si cher


## Les mesures

| # | Cas | Commande | Temps mesuré | Temps Jenga | Fichiers compilés | Bibliothèques reliées |
|---:|---|---|---:|---:|---:|---:|
| 1 | **Complète à froid** (dossier `Build` supprimé) | `jenga build --target NKCanvas` | **76,6 s** | 74,4 s | **197** | 16 |
| 2 | **Complète à chaud** (juste après, rien modifié) | `jenga build --target NKCanvas` | **6,6 s** | 4,6 s | **0** | 0 |
| 3 | **Un seul module** (juste après le cas 2) | `jenga build --target NKMath` | **14,6 s** | 12,6 s | **81** | 5 |
| 3′ | … relancé à l'identique | `jenga build --target NKMath` | 5,2 s | 1,2 s | 0 | 0 |
| 4 | **Un fichier source modifié** (`NkAngle.cpp`) | `jenga build --target NKCanvas` | **7,0 s** | 4,7 s | **1** | 6 |
| 5 | **Un en-tête modifié** (`NkAngle.h`) | `jenga build --target NKCanvas` | **47,7 s** | 45,6 s | **79** | 6 |

La modification est la même dans les cas 4 et 5 : une ligne de commentaire ajoutée à la fin du fichier. Le « temps mesuré » ajoute environ 2 à 4 s de chargement du workspace au temps affiché par Jenga.

**Remettre le fichier en état coûte autant que le modifier.** `git checkout` lui donne une date neuve : 1 fichier recompilé pour le source, 79 pour l'en-tête (46,3 s).

## Ce qu'on voit, cas par cas

- **À froid** : tout est compilé et relié. C'est le prix de référence.
- **À chaud** : les 16 projets sont « à jour ». Il reste quelques secondes pour charger le workspace et vérifier les dates.
- **Un seul module** : surprise, NKMath recompile ses 5 projets (81 fichiers) alors qu'ils venaient d'être construits pour NKCanvas. En comparant les signatures de compilation, la cause apparaît : le champ `context.options` contient **`target:nkcanvas`** d'un côté et **`target:nkmath`** de l'autre. **Changer de cible invalide les objets partagés.** Relancé à l'identique, NKMath ne compile plus rien. Et revenir ensuite à NKCanvas recompile de nouveau les 81 fichiers.
- **Un fichier source** : seul `NkAngle.cpp` est recompilé. NKMath est ré-archivé, puis les 5 bibliothèques qui en dépendent (NKFont, NKEvent, NKImage, NKWindow, NKCanvas) sont reliées de nouveau, **sans rien compiler**.

## Le cas intéressant : un seul en-tête

### Ce qui s'est passé

Modifier `NkAngle.h` a recompilé **79 fichiers dans 6 projets**, soit **10 fois le coût** du cas 4 et **61 % d'une construction à froid**, pour une ligne de commentaire :

| Projet | `.cpp` recompilés | PCH reconstruit ? |
|---|---:|---|
| NKMath | 9 sur 12 | non |
| NKFont | 8 | non |
| NKEvent | 10 | non |
| NKImage | 13 | non |
| NKWindow | 8 | non |
| NKCanvas | 31 sur 31 | non |
| NKPlatform, NKCore, NKMemory, NKContainers, NKThreading, NKLogger, NKTime, NKFileSystem, NKStream, NKGlad | 0 | — |

**J'avais prévu ce nombre avant de lancer la construction**, en cherchant `NkAngle.h` dans les fichiers `.d` : 9 + 8 + 10 + 13 + 8 + 31 = **79**. La prévision et la mesure tombent juste.

### Pourquoi

1. **Un en-tête n'est jamais compilé seul.** Jenga ne compile que les `.cpp`. Il faut donc qu'il sache quels `.cpp` utilisent l'en-tête.
2. **Ce sont les fichiers `.d` qui le lui disent.** À chaque compilation, clang écrit dans `NkXxx.obj.d` la liste de **tous** les fichiers qu'il a ouverts, **y compris indirectement**. Les 31 `.cpp` de NKCanvas n'incluent sans doute pas `NkAngle.h` eux-mêmes : ils le reçoivent au travers d'autres en-têtes. Mais il figure dans leurs `.d`.
3. **La décision se prend sur une date.** Pour chaque objet, Jenga compare la date de chaque fichier de son `.d` à celle de l'objet (`Jenga/Core/Builder.py:1577-1582`). `NkAngle.h` vient d'être modifié, il est plus récent que les 79 objets, donc les 79 sont recompilés. Jenga ne regarde pas *ce qui* a changé : un commentaire suffit.
4. **L'effet traverse les modules.** `NkAngle.h` est un en-tête **public** de NKMath. Les 5 modules en aval l'incluent, donc leur code est recompilé alors qu'il n'a pas changé d'une ligne.
5. **Puis vient la cascade de ré-archivage.** Chaque bibliothèque dont une dépendance a été reconstruite est reliée de nouveau (`Builder.py:2431-2433`).
6. **Ça aurait pu être pire.** Si l'en-tête avait fait partie d'un en-tête précompilé, **tous** les `.cpp` du projet auraient été recompilés. Ce n'est pas le cas ici : aucun `.pch.d` ne contient `NkAngle.h`. Et pour un en-tête de NKPlatform comme `NkArchDetect.h`, les `.d` annoncent **193 `.cpp` sur 197**, c'est-à-dire presque une construction complète. *Prévision seulement, non mesurée.*

**Ce que j'en retiens** : le coût d'une modification ne dépend pas de sa taille, mais du **nombre de fichiers qui incluent ce qu'on touche**. Un `.cpp` ne concerne que lui-même ; un en-tête public concerne tous ceux qui l'incluent, jusque dans les autres modules. Moins un en-tête public en montre, moins il coûte à modifier.

## À savoir

- **Les signatures de compilation contiennent les options de la ligne de commande** : `target:…`, `no-daemon`, `action:build`… Changer de cible, passer de `build` à `test`, ou lancer Jenga avec ou sans `--no-daemon` suffit à tout recompiler. C'est pourquoi toutes les mesures ont été faites avec les mêmes options.
- **Dans le dépôt d'origine**, dont le chemin contient `gap l2` et `chap 1`, les cas 2 à 5 recompileraient tout, comme le cas 1 (voir le rapport de chronométrage).
- **Dans le clone, `.vscode/settings.json` et `pyrightconfig.json` apparaissent modifiés dans `git status`**, alors qu'il était propre juste après sa création. Seules des commandes Jenga ont tourné depuis : ce sont très probablement elles qui les réécrivent. `NkAngle.cpp` et `NkAngle.h` sont revenus à leur état d'origine.