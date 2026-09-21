# Exercice 4 — Deux sujets, deux commits avec `git add -p`

Je travaille dans mon dépôt d'essai `essai-git`. J'y ai d'abord commité un petit fichier `jeu.cpp` de 19 lignes (commit `7969279`).

## 1. Deux modifications sans rapport

J'ai changé deux choses dans `jeu.cpp` :
- **ligne 1** : je corrige une faute dans le commentaire, « esai » devient « essai » ;
- **ligne 16** : je change une valeur du jeu, la vitesse passe de `250.f` à `180.f`.

```bash
git diff
```

```text
@@ -1,4 +1,4 @@
-// jeu.cpp : un petit jeu d'esai pour le chapitre 2
+// jeu.cpp : un petit jeu d'essai pour le chapitre 2
 #include <cstdio>

 int main()
@@ -13,7 +13,7 @@ int main()

     printf("score : %d, vies : %d\n", score, vies);

-    const float vitesse = 250.f;
+    const float vitesse = 180.f;
     printf("vitesse : %.0f\n", vitesse);
     return 0;
 }
```

Git découpe le diff en deux **morceaux** (les lignes `@@`), un par modification. C'est ce découpage que `git add -p` utilise.

## 2. Premier commit : seulement la faute

```bash
git add -p jeu.cpp
```

Git me montre chaque morceau et me demande s'il faut l'ajouter :

```text
(1/2) Stage this hunk [y,n,q,a,d,k,K,j,J,g,/,e,p,P,?]? y
(2/2) Stage this hunk [y,n,q,a,d,K,J,g,/,e,p,P,?]? n
```

J'ai répondu `y` (oui) pour la faute, et `n` (non) pour la vitesse.

```bash
git status
```

```text
Changes to be committed:
	modified:   jeu.cpp

Changes not staged for commit:
	modified:   jeu.cpp
```

`jeu.cpp` apparaît **deux fois**. C'est normal : une partie du fichier est dans l'index (la faute), et l'autre partie est encore seulement dans le répertoire de travail (la vitesse). Pour vérifier ce qui va partir, j'ai tapé `git diff --cached` : il ne montre que la ligne 1.

```bash
git commit -m "Corrige la faute dans l'en-tete de jeu.cpp"
```

## 3. Deuxième commit : la vitesse

```bash
git add -p jeu.cpp
```

```text
(1/1) Stage this hunk [y,n,q,a,d,e,p,P,?]? y
```

Il ne reste plus qu'un morceau, celui de la vitesse.

```bash
git commit -m "Baisse la vitesse du jeu de 250 a 180"
```

Après ce commit, `git status` ne montre plus rien : tout est commité.

## 4. Vérification dans l'historique

```bash
git log --oneline -3
```

```text
2ce579a Baisse la vitesse du jeu de 250 a 180
1cc5820 Corrige la faute dans l'en-tete de jeu.cpp
7969279 Ajoute le petit jeu d'essai
```

J'ai ouvert chaque commit avec `git show` :

```bash
git show 1cc5820
```

```text
@@ -1,4 +1,4 @@
-// jeu.cpp : un petit jeu d'esai pour le chapitre 2
+// jeu.cpp : un petit jeu d'essai pour le chapitre 2
```

```bash
git show 2ce579a
```

```text
@@ -13,7 +13,7 @@ int main()
-    const float vitesse = 250.f;
+    const float vitesse = 180.f;
```

Chaque commit ne contient que **son** sujet : `1 file changed, 1 insertion(+), 1 deletion(-)` pour les deux. Si j'avais fait `git add jeu.cpp`, les deux modifications seraient parties dans un seul commit, et il serait impossible d'annuler le changement de vitesse sans annuler aussi la correction de la faute.

**Attention** : `git add -p` ne sépare les modifications que si elles sont assez éloignées. Si elles ne sont séparées que par 6 lignes ou moins, Git en fait un seul morceau. Il faut alors taper `s` pour le couper en deux, ou `e` pour choisir les lignes à la main.
