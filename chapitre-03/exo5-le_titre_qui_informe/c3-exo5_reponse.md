# Chapitre 3 — Exercice 5 : le titre qui informe

Windows 11, backend Win32, Jenga 2.8.0, Debug. Écran à 125 %. Le 2026-09-28.

Le titre doit porter trois choses : le nom du document, un astérisque s'il est modifié, et la taille de la fenêtre. La difficulté n'est pas de les écrire, c'est de choisir **quand** les écrire.

## Comment je décide du bon moment

Je garde deux états :
- `etat` : ce qui est vrai maintenant ;
- `affiche` : ce qui est **réellement** écrit dans la barre de titre.

À chaque tour de boucle, je les compare. S'ils sont égaux, je ne fais rien. Trois événements seulement peuvent les faire diverger :

| Événement | Ce qu'il change |
|---|---|
| `NkWindowResizeEvent` | `largeur`, `hauteur` (il porte déjà les valeurs : `GetWidth()`, `GetHeight()`) |
| `NkTextInputEvent` ou `NkKeyPressEvent` | `modifie = true` |
| `NkKeyPressEvent` avec `NK_S` et `HasCtrl()` | `modifie = false` — l'enregistrement |

## Ce que ça donne

J'ai piloté le programme de l'extérieur : une frappe, deux redimensionnements, un Ctrl+S, puis la fermeture.

```text
titre au depart      : brouillon.txt - 898 x 492
apres une frappe     : brouillon.txt * - 898 x 492
apres redim 700x420  : brouillon.txt * - 857 x 478
apres redim 1100x600 : brouillon.txt * - 1357 x 703
apres Ctrl+S         : brouillon.txt - 1357 x 703
```

Et le journal du programme :

```text
Titres reellement poses dans la barre :
  1. tour 1       brouillon.txt - 898 x 492
  2. tour 1249    brouillon.txt * - 898 x 492
  3. tour 1655    brouillon.txt * - 857 x 478
  4. tour 2060    brouillon.txt * - 1357 x 703
  5. tour 2667    brouillon.txt - 1357 x 703

Tours de boucle : 3096
Appels a SetTitle : 5
Soit 1 appel pour 619 tours
```

**5 appels pour 3 096 tours.** À chaque image, ça aurait fait 3 096 appels, dont 3 091 pour réécrire exactement le même texte.

## Pourquoi ça compte

`SetTitle` n'est pas une écriture dans une variable :

```cpp
// Platform/Win32/NkWin32Window.cpp:724-730
void NkWindow::SetTitle(const NkString &t) {
    mConfig.title = t;
    if (mData.mHwnd) {
        SetWindowTextW(mData.mHwnd, NkUtf8ToWide(t).CStr());
    }
}
```

Chaque appel : une conversion UTF-8 → UTF-16 (donc une allocation), puis `SetWindowTextW`, qui envoie `WM_SETTEXT` à la fenêtre et fait **redessiner la barre de titre** par le système. À 60 images par seconde, c'est 60 repeints par seconde d'un texte qui ne bouge pas, plus la barre des tâches qui suit.

C'est exactement ce que dit le §3.3.3 : « `SetTitle` sert plus qu'on ne croit : c'est l'endroit le moins coûteux pour montrer un état ». Le moins coûteux, à condition de ne l'appeler que quand l'état change.

## Deux choses vues au passage

**La fenêtre ne s'ouvre pas à la taille demandée.** J'ai écrit 900 × 500, elle s'ouvre à 898 × 492 : les mêmes 2 px et 8 px perdus qu'à l'exercice 4, sur le chemin de création.

**Mes redimensionnements n'ont pas donné les tailles demandées.** J'ai envoyé `SetWindowPos(700, 420)` et la fenêtre annonce 857 × 478. Ce n'est pas un défaut du moteur : mon script de pilotage n'est pas conscient du DPI, donc Windows a converti ma demande en pixels physiques : 700 × 1,25 = 875 d'extérieur, moins 18 px de bordures = 857 de client. Idem en hauteur : 420 × 1,25 = 525, moins 47 px de barre de titre = 478. C'est le facteur d'échelle de l'exercice 4, vu de l'autre côté.

**Le `Sleep(1)`** n'est là que parce que ce programme ne dessine rien : sans lui, la boucle tourne à vide et occupe un cœur entier. Dans un vrai programme, c'est le `Display()` de la cible de rendu qui cadence.