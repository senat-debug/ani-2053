# Chapitre 3 — Exercice 10 : la fenêtre sans bordure

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Écran à 125 %. Le 2026-09-28.

`cfg.frame = false`, et tout ce que la barre du système faisait, c'est à moi de le refaire.

## Ce que j'ai vérifié

| Ce que je devais faire | Comment je l'ai vérifié | Résultat |
|---|---|---|
| Fenêtre sans bordure | `GetWindowRect` − `GetClientRect` | **0 px** de zone non-cliente (47 px avec une barre système) |
| Un titre | capture d'écran | affiché à gauche de ma barre |
| Trois boutons | capture, survol du bouton fermer | `_`, `[]`, `X` en haut à droite, le `X` passe en rouge au survol |
| Le déplacement à la souris | compteur dans le programme | `BeginDragMove` appelé 2 fois sur 2 clics dans la barre |
| Le double-clic qui agrandit | `IsZoomed` avant/après | `False` → **`True`**, taille 720×416 → 1550×830 (en points) |
| Le double-clic qui restaure | `IsZoomed` + position | `True` → **`False`**, retour exact à 720×416 à la même position |
| Le bouton fermer | code de retour du programme | fermeture propre, **code 0** |

## Ce que j'ai dû écrire

| | Lignes de `main.cpp` |
|---|---:|
| Exercice 1 — fenêtre avec la barre du système | **22** |
| Exercice 10 — la même fenêtre, barre faite à la main | **182** |

Huit fois plus de code, et je n'ai pourtant fait que le strict minimum. Le découpage :

- **la barre et les boutons** : rectangles dessinés à chaque image, plus un test `BoutonSous(x,y)` pour savoir lequel est survolé ou cliqué ;
- **le déplacement** : je ne le calcule pas moi-même. Au clic dans la barre, j'appelle `window.BeginDragMove()` (`NkWindow.h:153`) et c'est le système qui déplace la fenêtre — la vitesse, l'accrochage aux bords, tout reste natif ;
- **le redimensionnement** : `BordSous(x,y)` détecte les 6 px près de chaque bord, puis `window.BeginResize(edge)` (`NkWindow.h:160`) passe la main au système, avec les huit coins et côtés de `NkResizeEdge` ;
- **le curseur** : c'est l'exercice 6 qui sert ici. Sans barre système, plus personne ne met `ResizeWE` sur les bords : je le pose moi-même à chaque mouvement ;
- **le double-clic** : `NkMouseDoubleClickEvent` est détecté par l'OS, je n'ai qu'à tester s'il est dans ma barre ;
- **le glyphe du bouton du milieu** dépend de `IsMaximized()` — c'est exactement l'usage que le §3.3.1 annonce pour cette méthode : « savoir quel glyphe afficher sur le bouton du milieu ».

C'est possible parce que le backend garde les styles de redimensionnement même sans bordure, et masque seulement la zone non-cliente :

```cpp
// Platform/Win32/NkWin32Window.cpp:431-437
mData.mDwStyle = config.frame ? WS_OVERLAPPEDWINDOW
                              : (WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
// frame=false : on garde WS_THICKFRAME/CAPTION (snap + min/max + resize
// natif via BeginResize) mais on SUPPRIME visuellement la zone non-cliente
// (titre + bordure OS) via WM_NCCALCSIZE -> seule notre deco s'affiche.
mData.mBorderless = !config.frame;
```

## Le temps que ça m'a pris

**12 minutes et 3 secondes**, de 16:39:09 à 16:51:12. Ce temps comprend :

- lire dans l'en-tête les méthodes dont j'avais besoin (`BeginDragMove`, `BeginResize`, `IsMaximized`, `NkMouseDoubleClickEvent`) ;
- écrire les 182 lignes ;
- **deux erreurs de compilation** : `NkCursorType` que j'ai écrit sans le préfixe `NkWindow::`, et `NkFont` ambigu entre `nkentseu::NkFont` et `nkentseu::renderer::NkFont` — le dépôt documente déjà ce piège dans ConquerorProto ;
- trois constructions (49 s chacune, 17 projets) et les essais à la souris.

Pour une barre vraiment finie, il faudrait compter bien plus : je n'ai fait ni le menu système au clic droit, ni les infobulles, ni les coins arrondis, ni le comportement au changement de thème.

