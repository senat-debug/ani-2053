# Exercice 8 — Six façons de défaire

J'ai tout fait dans mon dépôt `LUDO`, sur une nouvelle branche `exercice-defaire`. Je l'ai poussée tout de suite sur GitHub, parce que la situation 4 demande un commit déjà poussé.

```bash
git switch -c exercice-defaire
git push -u origin exercice-defaire
```

## 1. Une modification non voulue

Dans `src/main.cpp`, j'ai changé `return 0;` en `return 1;` « par erreur ».

```text
 M src/main.cpp
```

Dans le `git diff`, j'ai aussi vu une ligne `\ No newline at end of file`. Rien à voir avec mon erreur : le fichier ne se termine simplement pas par un retour à la ligne.

Pour annuler :

```bash
git restore src/main.cpp
```

`git status` ne montre plus rien et `return 0;` est revenu. Mais la modification est perdue pour de bon, puisqu'elle n'avait jamais été commitée.

## 2. Un add de trop

J'ai traduit le titre du README. J'avais aussi un fichier `brouillon.txt` avec des notes perso, et j'ai fait `git add .`. Évidemment, il a tout pris :

```text
Changes to be committed:
	modified:   README.md
	new file:   brouillon.txt
```

```bash
git restore --staged brouillon.txt
```

```text
Changes to be committed:
	modified:   README.md

Untracked files:
	brouillon.txt
```

Le brouillon est sorti de l'index mais il existe toujours sur le disque. J'ai commité le README seul (`c283ca6`).

## 3. Un commit de trop

Cette fois, j'ai commité le brouillon avec le message `wip` (`d4f55da`). Pour le défaire :

```bash
git reset --soft HEAD~1
```

Le commit `wip` a disparu de `git log`, mais `brouillon.txt` est toujours dans l'index (`new file: brouillon.txt`). `--soft` défait seulement le commit, il garde tout le reste. Ensuite, j'ai retiré le brouillon de l'index et je l'ai supprimé.

## 4. Un commit poussé à annuler

J'ai poussé le commit du README :

```text
   5843010..c283ca6  exercice-defaire -> exercice-defaire
```

Il est déjà sur GitHub, donc pas de `reset` : ça réécrirait l'historique. J'utilise `revert` :

```bash
git revert HEAD
```

```text
[exercice-defaire 74cf000] Revert "Traduit le titre du README en francais"
 Date: Mon Sep 21 16:52:10 2026 +0100
 1 file changed, 1 insertion(+), 1 deletion(-)
```

`git log` montre maintenant les deux commits : celui du titre, puis celui qui l'annule. Le titre est redevenu `# Ludo Game in C++`, et j'ai pu pousser normalement, sans `--force`.

## 5. Un travail en cours à mettre de côté

J'ai commencé à ajouter `// TODO : afficher le vainqueur` dans `main.cpp`. En même temps, l'avertissement `LF will be replaced by CRLF` est réapparu. Au début, je le prenais pour une erreur ; c'est juste Git qui prévient pour les fins de ligne Windows.

```bash
git stash
```

```text
Saved working directory and index state WIP on exercice-defaire: 74cf000 Revert "Traduit le titre du README en francais"
```

`git status` est vide, et `git stash list` montre `stash@{0}`. Pour récupérer mon travail :

```bash
git stash pop
```

Git a réaffiché tout le `git status` (`modified: src/main.cpp`), puis `Dropped refs/stash@{0}`. Ma ligne `// TODO` était revenue.

## 6. Un commit « perdu »

J'ai fait un commit (`71b2afa`, « Traduit le titre License du README »), puis je l'ai « perdu » exprès :

```bash
git reset --hard HEAD~1
```

```text
HEAD is now at 74cf000 Revert "Traduit le titre du README en francais"
```

Il n'est plus dans `git log`, et le README est revenu à `## License`. Je l'ai cherché dans le reflog :

```bash
git reflog
```

```text
74cf000 HEAD@{0}: reset: moving to HEAD~1
71b2afa HEAD@{1}: commit: Traduit le titre License du README
74cf000 HEAD@{2}: reset: moving to HEAD
74cf000 HEAD@{3}: revert: Revert "Traduit le titre du README en francais"
```

Mon commit est en `HEAD@{1}`. La ligne `HEAD@{2}: reset: moving to HEAD` m'a surpris, parce que je n'avais tapé aucun reset à ce moment-là. En fait, elle vient du `git stash` de la situation 5 : il remet les fichiers à l'état du dernier commit, et le reflog l'enregistre comme un reset.

```bash
git reset --hard HEAD@{1}
```

```text
HEAD is now at 71b2afa Traduit le titre License du README
```

Le commit est revenu, et la ligne 72 affiche de nouveau `## Licence`.

## Bilan

- `git restore` : défait une modification, et elle est perdue.
- `git restore --staged` : défait un `add`, et garde la modification.
- `git reset --soft HEAD~1` : défait un commit, et garde les fichiers.
- `git revert` : annule un commit déjà poussé en ajoutant un commit inverse.
- `git stash` / `git stash pop` : range le travail en cours, puis le ressort.
- `git reflog` : retrouve un commit qu'on croyait perdu.

Seule la première détruit vraiment quelque chose. Même après un `reset --hard`, le reflog m'a permis de tout retrouver.