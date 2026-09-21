# Exercice 1 — Le dépôt d'essai

> **Consigne** : créer un dépôt vide, ajouter trois fichiers en trois commits, afficher l'historique en une ligne par commit, puis afficher le graphe.
> **Fait le** 2026-09-19, dans `C:\Users\ngatc\Desktop\essai-git`, avec Git 2.53.0 (Git Bash).

## 1. Créer le dépôt vide

```bash
mkdir essai-git
cd essai-git
git init -b main
```

```text
Initialized empty Git repository in C:/Users/ngatc/Desktop/essai-git/.git/
```

Git a créé le dossier caché `.git` : c'est lui, le dépôt. `-b main` donne le nom `main` à la branche.

## 2. Trois fichiers, trois commits

Pour chaque fichier, j'ai fait la même chose : le créer, `git add`, puis `git commit`.

```bash
printf 'Depot d.essai pour le chapitre 2.\n' > README.md
git add README.md
git commit -m "Ajoute le README"

printf 'Mes notes de cours.\n' > notes.txt
git add notes.txt
git commit -m "Ajoute les notes"

printf 'a\nb\nc\n' > liste.txt
git add liste.txt
git commit -m "Ajoute la liste"
```

```text
[main (root-commit) 2e6aad0] Ajoute le README
[main 1c66c79] Ajoute les notes
[main 57473dc] Ajoute la liste
```

`git add` met le fichier dans l'index, et `git commit` enregistre ce qu'il y a dans l'index. Le premier commit est marqué `root-commit` parce qu'il n'a pas de parent.

## 3. L'historique, une ligne par commit

```bash
git log --oneline
```

```text
57473dc Ajoute la liste
1c66c79 Ajoute les notes
2e6aad0 Ajoute le README
```

J'ai bien trois commits, du plus récent au plus ancien.

## 4. Le graphe

```bash
git log --oneline --graph --all
```

```text
* 57473dc (HEAD -> main) Ajoute la liste
* 1c66c79 Ajoute les notes
* 2e6aad0 Ajoute le README
```

Le graphe est une ligne droite, parce que je n'ai qu'une seule branche et aucune fusion. `(HEAD -> main)` veut dire que je suis sur `main` et que `main` pointe sur le dernier commit.

