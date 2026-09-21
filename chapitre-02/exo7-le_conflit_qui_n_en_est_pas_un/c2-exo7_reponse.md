# Exercice 7 — Deux endroits éloignés : Git fusionne tout seul

Je travaille le dépôt, `C:\Users\ngatc\Desktop\LUDO`. Chaque personne a sa **branche**, et les deux partent du même commit de `main`, `5843010`. Les deux modifient `README.md`, mais à **69 lignes d'écart**.

## 1. Personne A : la ligne 3

```bash
git switch -c personne-a
```

Je remplace `## Overview` par `## Présentation` (ligne 3), puis :

```bash
git add README.md
git commit -m "Traduit le titre Overview du README"
```

```text
[personne-a 8e09992] Traduit le titre Overview du README
 1 file changed, 1 insertion(+), 1 deletion(-)
```

## 2. Personne B : la ligne 72

B repart de `main`, donc il ne voit pas le travail de A.

```bash
git switch main
git switch -c personne-b
```

Je remplace `## License` par `## Licence` (ligne 72), puis :

```bash
git add README.md
git commit -m "Traduit le titre License du README"
```

```text
[personne-b 3323ed3] Traduit le titre License du README
 1 file changed, 1 insertion(+), 1 deletion(-)
```

## 3. La fusion

Je me place sur la branche de A et je fusionne celle de B :

```bash
git switch personne-a
git merge personne-b
```

```text
Auto-merging README.md
Merge made by the 'ort' strategy.
 README.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

Il n'y a **aucun conflit**, et Git ne m'a rien demandé. Il a assemblé les deux modifications du même fichier et créé tout seul le commit de fusion.

## 4. Vérification

Le fichier contient bien les deux modifications, sans aucun marqueur `<<<<<<<` :

```text
ligne 3  : ## Présentation
ligne 72 : ## Licence
```

```bash
git log --oneline --graph -4
```

```text
*   3ef6d0d Merge branch 'personne-b' into personne-a
|\
| * 3323ed3 Traduit le titre License du README
* | 8e09992 Traduit le titre Overview du README
|/
* 5843010 ajouts des fichiers
```

Le commit de fusion `3ef6d0d` a deux parents : `8e09992` (A) et `3323ed3` (B).

## 5. Pourquoi Git n'a rien demandé

Git ne compare pas des fichiers entiers, il compare des **morceaux de lignes**. Pour chaque branche, il regarde ce qui a changé depuis l'ancêtre commun `5843010` :
- A a changé la ligne 3 ;
- B a changé la ligne 72.

Les deux morceaux ne se touchent pas, donc Git applique les deux. Il n'y a conflit que quand les deux côtés changent **les mêmes lignes**, ou des lignes collées l'une à l'autre (§2.8.1).

**Remarque** : dans un vrai terminal, `git merge` ouvre l'éditeur pour qu'on relise le message du commit de fusion. Ce n'est pas une question sur le contenu : on enregistre et on ferme. Avec `git merge --no-edit`, l'éditeur ne s'ouvre pas.