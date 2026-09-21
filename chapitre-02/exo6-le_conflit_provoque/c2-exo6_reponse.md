# Exercice 6 — Le refus, le conflit, la résolution

J'ai utilisé deux clones du dépôt `https://github.com/senat-debug/LUDO.git` :
- **clone A** : `C:\Users\ngatc\Desktop\LUDO`, mon dossier habituel ;
- **clone B** : `C:\Users\ngatc\Desktop\LUDO-bis`, un second clone fait pour l'exercice.

Pour ne pas toucher à `main`, j'ai travaillé sur une branche `exercice-conflit`, créée et poussée depuis A. Les deux clones partent donc du même commit, `5843010`.

```bash
git switch -c exercice-conflit
git push -u origin exercice-conflit
```

```text
To https://github.com/senat-debug/LUDO.git
 * [new branch]      exercice-conflit -> exercice-conflit
branch 'exercice-conflit' set up to track 'origin/exercice-conflit'.
```

## 1. La même ligne, modifiée des deux côtés

La ligne choisie est la ligne 1 de `README.md` : `# Ludo Game in C++`.

**Clone A** : je la traduis, je commite et je pousse.

```bash
git add README.md
git commit -m "Traduit le titre du README en francais"
git push
```

```text
[exercice-conflit e29ec3a] Traduit le titre du README en francais
 1 file changed, 1 insertion(+), 1 deletion(-)
To https://github.com/senat-debug/LUDO.git
   5843010..e29ec3a  exercice-conflit -> exercice-conflit
```

Le push passe : GitHub n'avait rien de nouveau.

**Clone B** : je change la même ligne autrement, et je commite.

```bash
git add README.md
git commit -m "Precise le titre du README"
```

```text
[exercice-conflit c14d6ae] Precise le titre du README
 1 file changed, 1 insertion(+), 1 deletion(-)
```

## 2. Le refus

```bash
git push
```

```text
To https://github.com/senat-debug/LUDO.git
 ! [rejected]        exercice-conflit -> exercice-conflit (fetch first)
error: failed to push some refs to 'https://github.com/senat-debug/LUDO.git'
hint: Updates were rejected because the remote contains work that you do not
hint: have locally. This is usually caused by another repository pushing to
hint: the same ref. If you want to integrate the remote changes, use
hint: 'git pull' before pushing again.
```

GitHub refuse, parce qu'il a le commit `e29ec3a` de A, et que B ne l'a pas. S'il acceptait, le commit de A serait écrasé. Git me dit quoi faire : `git pull`.

## 3. Le conflit

```bash
git pull
```

```text
From https://github.com/senat-debug/LUDO
   5843010..e29ec3a  exercice-conflit -> origin/exercice-conflit
Auto-merging README.md
CONFLICT (content): Merge conflict in README.md
Automatic merge failed; fix conflicts and then commit the result.
```

`pull` a récupéré le commit de A, puis a essayé de fusionner. Il n'a pas pu, parce que les deux commits changent **la même ligne**.

```bash
git status
```

```text
On branch exercice-conflit
Your branch and 'origin/exercice-conflit' have diverged,
and have 1 and 1 different commits each, respectively.

You have unmerged paths.
  (fix conflicts and run "git commit")
  (use "git merge --abort" to abort the merge)

Unmerged paths:
  (use "git add <file>..." to mark resolution)
	both modified:   README.md
```

Dans `README.md`, Git a mis les deux versions entre des marqueurs :

```text
<<<<<<< HEAD
# Ludo : le jeu de plateau en C++
=======
# Jeu de Ludo en C++
>>>>>>> e29ec3a87bf0466151efbfdc738ba014eb179f45
```

- entre `<<<<<<< HEAD` et `=======` : ma version, celle du clone B ;
- entre `=======` et `>>>>>>>` : la version qui arrive, celle du commit `e29ec3a` de A.

## 4. La résolution

J'ai choisi une des deux versions, sans mélanger au hasard : je garde la plus précise. J'ai effacé les trois marqueurs, et il reste une seule ligne :

```text
# Ludo : le jeu de plateau en C++
```

Puis :

```bash
git add README.md
git commit --no-edit
```

```text
[exercice-conflit 852facc] Merge branch 'exercice-conflit' of https://github.com/senat-debug/LUDO into exercice-conflit
```

`git add` dit à Git « ce fichier est réglé ». `git commit` termine la fusion, et `--no-edit` garde le message de fusion proposé par Git.

```bash
git push
```

```text
To https://github.com/senat-debug/LUDO.git
   e29ec3a..852facc  exercice-conflit -> exercice-conflit
```

Cette fois, le push passe : B contient maintenant le commit de A.

## 5. Le graphe

```bash
git log --oneline --graph -5
```

```text
*   852facc Merge branch 'exercice-conflit' of https://github.com/senat-debug/LUDO into exercice-conflit
|\
| * e29ec3a Traduit le titre du README en francais
* | c14d6ae Precise le titre du README
|/
* 5843010 ajouts des fichiers
* 1aa0cab Initial commit
```

On voit les deux commits faits en parallèle depuis `5843010`, et le commit de fusion `852facc`, qui a **deux parents**.

Le refus affiche `(fetch first)` parce que B n'avait encore jamais vu le commit de A. Si j'avais fait `git fetch` avant de pousser, sans fusionner, Git aurait affiché `(non-fast-forward)`. La cause est la même : le serveur a des commits que je n'ai pas.