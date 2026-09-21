# Exercice 2 — `git status` à chaque étape



Au départ, le dépôt est propre : `git status` affiche `nothing to commit, working tree clean`.

## 1. Après la modification

J'ai ajouté une ligne à `notes.txt` :

```bash
printf 'Mes notes de cours.\nChapitre 2 : les trois endroits.\n' > notes.txt
git status
```

```text
On branch main
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
	modified:   notes.txt

no changes added to commit (use "git add" and/or "git commit -a")
```

## 2. Après le `add`

```bash
git add notes.txt
git status
```

```text
On branch main
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	modified:   notes.txt
```

## 3. Après le `commit`

```bash
git commit -m "Complete les notes du chapitre 2"
git status
```

```text
[main 5d601a4] Complete les notes du chapitre 2
 1 file changed, 1 insertion(+)

On branch main
nothing to commit, working tree clean
```

## Ce qui change entre les trois

| Étape | Titre affiché par `git status` | Où est ma modification |
|---|---|---|
| Après la modification | `Changes not staged for commit` | dans le **répertoire de travail** seulement |
| Après `add` | `Changes to be committed` | dans l'**index** : elle ira dans le prochain commit |
| Après `commit` | `nothing to commit, working tree clean` | dans le **dépôt** : c'est le commit `5d601a4` |

Le fichier ne change pas entre les trois étapes. C'est **l'endroit où se trouve la modification** qui change : `git add` la fait passer du répertoire de travail à l'index, puis `git commit` la fait passer de l'index au dépôt. Ce sont les trois endroits du §2.3.

Git propose aussi la commande pour revenir en arrière à chaque étape :
- après la modification : `git restore` annule la modification ;
- après le `add` : `git restore --staged` retire le fichier de l'index, mais garde la modification.

