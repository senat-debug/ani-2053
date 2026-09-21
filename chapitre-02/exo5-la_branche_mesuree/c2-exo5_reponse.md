# Exercice 5 — Ce que coûtent une branche et trois commits

Je travaille dans mon dépôt d'essai `essai-git`. Pour mesurer la place prise, j'ai compté les octets du dossier `.git` avec `du -sb .git`, avant et après chaque étape.

## 1. Avant

```bash
du -sb .git
```

```text
34412	.git
```

## 2. Créer la branche

```bash
git switch -c essai-branche
du -sb .git
```

```text
Switched to a new branch 'essai-branche'
34812	.git
```

Le dépôt a pris **400 octets**. J'ai comparé la liste des fichiers de `.git` avant et après pour voir d'où ils viennent :

| Fichier dans `.git` | Octets | Ce que c'est |
|---|---:|---|
| `refs/heads/essai-branche` | 41 | **la branche elle-même** |
| `logs/refs/heads/essai-branche` et `logs/HEAD` | 350 | le journal des déplacements (celui que lit `git reflog`) |
| `HEAD` | +9 | il contient maintenant `ref: refs/heads/essai-branche` |

La branche, c'est un fichier de 41 octets qui contient une seule ligne :

```text
2ce579a0f5ac436480e85e8e20994cd81ff9dc6f
```

C'est l'empreinte du commit sur lequel j'étais, plus un retour à la ligne. Aucun fichier du projet n'a été copié.

## 3. Trois commits sur la branche

J'ai modifié une ligne de `jeu.cpp` à chaque fois :

```bash
git add jeu.cpp
git commit -m "Donne 5 vies au joueur au lieu de 3"
git add jeu.cpp
git commit -m "Passe la partie de 10 a 20 tours"
git add jeu.cpp
git commit -m "Donne 15 points par tour au lieu de 10"
du -sb .git
```

```text
37575	.git
```

Les trois commits ont pris **2 763 octets**. Ils ont créé 9 objets, 3 par commit :

| Objet | Nombre | Taille brute | Taille sur le disque | Ce qu'il contient |
|---|---:|---:|---:|---|
| `blob` | 3 | 342 octets chacun | 221 à 222 octets | le nouveau contenu **complet** de `jeu.cpp` |
| `tree` | 3 | 146 octets chacun | 149 à 150 octets | la liste des 4 fichiers du projet |
| `commit` | 3 | 261 à 267 octets | 179 à 183 octets | l'arbre, le parent, l'auteur, le message |
| **Total** | **9** | | **1 658 octets** | |

Les 1 105 octets restants viennent des journaux (`logs/`), qui gagnent une ligne à chaque commit.

## 4. Bilan

| Étape | Taille de `.git` | Gain |
|---|---:|---:|
| Avant | 34 412 octets | — |
| Après la branche | 34 812 octets | + 400 |
| Après les 3 commits | 37 575 octets | + 2 763 |
| **Total** | | **+ 3 163 octets (3 Ko)** |

## 5. Explication

**Une branche ne coûte presque rien.** C'est un nom qui pointe sur un commit , et Git le range dans un fichier de 41 octets. Il ne copie aucun fichier du projet. Créer une branche sur Nkentseu coûterait exactement la même chose que sur mon petit dépôt.

**Un commit coûte seulement ce qui a changé.** Chaque commit garde un instantané complet, mais il ne recopie pas les fichiers qui n'ont pas bougé. Mon arbre liste 4 fichiers : `README.md`, `notes.txt` et `liste.txt` pointent vers les **mêmes objets** qu'avant. Seul `jeu.cpp` a un nouvel objet à chaque commit. C'est le partage des objets identiques du §2.11.2.

**Git compresse ce qu'il écrit.** `jeu.cpp` fait 342 octets, mais son objet n'en prend que 222 sur le disque.

Il reste une chose qui peut surprendre : pour changer **une seule ligne**, Git stocke une **copie entière** de `jeu.cpp` (342 octets bruts). C'est le modèle des instantanés, et non des différences (§2.2). Sur un petit fichier, ça ne coûte presque rien. Et plus tard, `git gc` range les objets dans un fichier « pack » et ne garde que les différences entre versions proches, ce qui réduit encore la place.