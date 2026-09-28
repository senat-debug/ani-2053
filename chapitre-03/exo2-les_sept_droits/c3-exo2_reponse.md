# Chapitre 3 — Exercice 2 : les sept droits

Windows 11, backend **Win32**, Jenga 2.8.0, Debug, clang. Le 2026-09-28.

Plutôt que de juger à l'œil, je lis les styles que le système a vraiment posés, avec `GetWindowLongPtrW` sur le `HWND` que le moteur expose (`NkWindow::mData.mHwnd`, `Core/NkWindow.h:351`). Je regarde aussi le menu système (`SC_CLOSE`, `SC_MOVE`) et la hauteur de la zone non-cliente, c'est-à-dire le bandeau de titre.

## Le relevé brut

```text
droit | POPUP | CAPTION | THICKFRAME | MINIMIZEBOX | MAXIMIZEBOX | Fermer | Deplacer | bandeau
frame          | oui   | oui     | oui        | oui         | oui         | actif  | actif    | 0 px
resizable      | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px
minimizable    | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px
movable        | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px
closable       | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px
maximizable    | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px
canFullscreen  | non   | oui     | oui        | oui         | oui         | actif  | actif    | 47 px

SetSize(700,400) sur la fenetre sans resizable : 700 x 400
Maximize() sur la fenetre sans maximizable : IsMaximized=oui
```

Les six dernières lignes sont **identiques** à celles d'une fenêtre normale. Une seule ligne diffère : `frame`.

## Le tableau demandé

| Droit désactivé | Effet attendu | Effet observé |
|---|---|---|
| **frame** | ni bordure ni barre de titre | **honoré** : `WS_POPUP` posé, bandeau de titre à **0 px** au lieu de 47 |
| **resizable** | on ne peut plus redimensionner | **ignoré** : `WS_THICKFRAME` toujours là, poignées actives, et `SetSize(700,400)` passe sans rien dire |
| **minimizable** | bouton « réduire » absent ou grisé | **ignoré** : `WS_MINIMIZEBOX` présent, bouton actif |
| **movable** | fenêtre impossible à déplacer | **ignoré** : entrée « Déplacer » du menu système **active** |
| **closable** | bouton de fermeture inopérant | **ignoré** : entrée « Fermer » **active**, et mon `WM_CLOSE` a bien fermé la fenêtre |
| **maximizable** | bouton « agrandir » absent ou grisé | **ignoré** : `WS_MAXIMIZEBOX` présent, et `Maximize()` donne `IsMaximized = oui` |
| **canFullscreen** | plein écran refusé | **ignoré** : rien ne change dans la fenêtre |

## Pourquoi : ce que le backend lit vraiment

Tout se joue dans une seule expression, à la création de la fenêtre Win32 :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Platform/Win32/NkWin32Window.cpp:431-437
mData.mDwStyle =
    config.frame ? WS_OVERLAPPEDWINDOW
                 : (WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
// frame=false : on garde WS_THICKFRAME/CAPTION (snap + min/max + resize
// natif via BeginResize) mais on SUPPRIME visuellement la zone non-cliente
// (titre + bordure OS) via WM_NCCALCSIZE -> seule notre deco s'affiche.
mData.mBorderless = !config.frame;
```

**`config.frame` est le seul des sept droits qui apparaisse dans tout le backend Win32.** Les six autres ne sont lus nulle part : le style vaut `WS_OVERLAPPEDWINDOW`, qui contient déjà `WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU`. Le résultat est donc le même quoi que je mette dans la configuration.

En cherchant `.<droit>` dans tout le module `NKWindow` :

| Droit | Qui le lit, dans le moteur |
|---|---|
| `frame` | Win32 (`NkWin32Window.cpp:432`), Cocoa (`NkCocoaWindow.mm:269`), et les autres backends |
| `resizable` | Cocoa `:271`, Wayland `:1065`, `:1108`, `:1453`, XCB `:424`, XLib `:375` — **jamais Win32** |
| `minimizable` | Cocoa `:274`, et c'est tout |
| `movable` | **personne** |
| `closable` | **personne** |
| `maximizable` | **personne** |
| `canFullscreen` | **personne** |

Quatre droits sur sept ne sont lus par **aucun** backend, sur aucune plateforme. Ce ne sont pas des réglages en panne : ce sont des champs déclarés et jamais branchés.

Deux détails qui le confirment :

- **`closable`** : sur `WM_CLOSE`, le backend met un `NkWindowCloseEvent` dans la file et bloque le traitement par défaut (`Platform/Win32/NkWin32EventSystem.cpp:253-257`), sans jamais regarder `closable`. C'est mon programme qui décide de fermer ou non. Autrement dit, le droit existe déjà, mais il est **dans l'application**, pas dans la configuration.
- **`movable` et `closable`** sont bien *écrits* quelque part : `Kernel/Runtime/NKUI/NkUIMultiViewport.cpp:398-399` fait `cfg.movable = true; cfg.closable = true;`. Quelqu'un les remplit, personne ne les lit.

## Ce que ça change pour moi

Sur Windows, il ne faut pas compter sur ces six champs. Ce qui marche vraiment aujourd'hui :

- **frame** : le seul droit honoré, et il suffit pour une fenêtre sans bordure (exercice 10) ;
- **fermeture** : ne pas appeler `window.Close()` sur `NkWindowCloseEvent`, ça donne une fenêtre qu'on ne peut pas fermer ;
- **taille** : `minWidth`/`minHeight` bornent la fenêtre, et eux sont appliqués.

La question du chapitre — « rien ne garantit que le backend les honore tous » — a donc une réponse mesurée : sur Win32, **un sur sept**.