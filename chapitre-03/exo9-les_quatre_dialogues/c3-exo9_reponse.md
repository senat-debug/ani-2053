# Chapitre 3 — Exercice 9 : les quatre dialogues

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Le programme ouvre les quatre boîtes l'une après l'autre. Un script extérieur les ferme **sans rien choisir** dès qu'elles apparaissent, en leur envoyant `WM_CLOSE` — c'est le geste de l'utilisateur qui clique sur la croix.

## Les quatre boîtes, telles qu'elles sont apparues

```text
je ferme sans rien choisir : 'Ouvrir une image'
je ferme sans rien choisir : 'Enregistrer la planche'
je ferme sans rien choisir : 'Rechercher un dossier'
je ferme sans rien choisir : 'Erreur'
programme encore vivant : True
code de retour : 0
```

## Ce que le programme a reçu

```text
1) OpenFileDialog
OpenFileDialog     confirmed=false path="" (0 octets)
                   -> annule : je ne touche pas a path, je continue

2) SaveFileDialog
SaveFileDialog     confirmed=false path="" (0 octets)
                   -> annule : je ne touche pas a path, je continue

3) OpenFolderDialog
OpenFolderDialog   confirmed=false path="" (0 octets)
                   -> annule : je ne touche pas a path, je continue

4) OpenMessageBox (ne rend aucun resultat)
                   -> revenu de la boite, le programme continue

Les quatre dialogues sont passes, la fenetre est toujours ouverte
Tours de boucle apres les dialogues : 206624
Sortie normale, code 0
```

| Dialogue | Annulation | Ce que je reçois |
|---|---|---|
| `OpenFileDialog` | croix / Échap | `confirmed = false`, `path` vide |
| `SaveFileDialog` | croix / Échap | `confirmed = false`, `path` vide |
| `OpenFolderDialog` | croix / Annuler | `confirmed = false`, `path` vide |
| `OpenMessageBox` | croix | **rien** : la fonction ne rend aucun résultat |

**Aucune des quatre n'a fait planter le programme** : il a continué sa boucle 206 624 tours après les dialogues, puis s'est fermé normalement avec le code 0.

## Le traitement correct de l'annulation

Une seule règle, et elle tient en une ligne : **tester `confirmed` avant de regarder `path`**.

```cpp
if (!r.confirmed) return;          // annule : on ne fait rien
if (r.path.Size() == 0) return;    // ceinture : chemin vide malgre confirmed
// ici seulement, on peut se servir de r.path
```

Le danger n'est pas le plantage de la boîte elle-même : c'est **la ligne d'après**. Un programme qui écrit directement `chargerImage(r.path)` reçoit une chaîne vide, et c'est son propre code de chargement qui va échouer, dix lignes plus loin, avec un message qui ne parle plus de dialogue. Le chapitre le dit : « tester le premier n'est pas une politesse : un utilisateur qui annule est le cas le plus fréquent » (§3.6.1).

Le backend, lui, est déjà propre : il ne met `confirmed` à vrai que si le système a dit oui.

```cpp
// Core/NkDialogs.cpp:99
r.confirmed = (GetOpenFileNameA(&ofn) == TRUE);
```

```cpp
// Core/NkDialogs.cpp:111-118
LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
if (pidl) {
    if (SHGetPathFromIDListA(pidl, path)) {
        r.confirmed = true;
        r.path = path;
    }
    CoTaskMemFree(pidl);
}
```

Sur annulation, `SHBrowseForFolderA` rend `NULL`, on n'entre pas dans le `if`, et le `NkDialogResult` reste à sa valeur de construction : `confirmed = false`, `path` vide. La mémoire du système est quand même libérée (`CoTaskMemFree`).

## Deux écarts que j'ai constatés

**Le titre du dialogue de dossier n'est pas le mien.** J'ai demandé « Choisir un dossier », la fenêtre s'appelle **« Rechercher un dossier »**. Ce n'est pas un défaut du moteur mais de l'API choisie : dans `BROWSEINFO`, `lpszTitle` est le texte affiché **au-dessus de l'arborescence**, pas la barre de titre, qui reste celle du système.

```cpp
// Core/NkDialogs.cpp:109-110
bi.lpszTitle = title.Empty() ? "Selectionner un dossier" : title.CStr();
bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
```

Les deux autres, eux, ont bien affiché mes titres (« Ouvrir une image », « Enregistrer la planche »), parce que `OPENFILENAME.lpstrTitle` est bien la barre de titre.

**La boîte de message ne répond à aucune question.** Elle est déclarée `void` (`NkDialogs.h:65`) et n'a qu'un seul bouton :

```cpp
// Core/NkDialogs.cpp:142-143
void NkDialogs::OpenMessageBox(const NkString &message, const NkString &title, int type) {
    UINT flags = MB_OK;
```

Donc « annuler » et « OK » sont la même chose pour mon programme : il ne peut pas les distinguer. Pour poser une vraie question (Oui / Non / Annuler), cette API ne suffit pas.

## Ce que ça change ailleurs

Les quatre fonctions que j'ai appelées sont **bloquantes** : le programme s'arrête à la ligne tant que la boîte est ouverte. C'est visible dans mon relevé : les tours de boucle ne commencent qu'après la quatrième boîte.

Sur le web et sur certaines plateformes mobiles, ce blocage figerait la page, et il faut les versions `…Async` avec fonction de rappel (`NkDialogs.h:81-87`, §3.6.1). Le traitement de l'annulation y est le même : le rappel reçoit un `NkDialogResult`, et on teste `confirmed` avant tout.