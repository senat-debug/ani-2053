# Chapitre 3 — Exercice 3 : les bornes

Windows 11, backend Win32, Jenga 2.8.0, Debug, clang. Écran à **125 %** (`GetDpiScale()` = 1.25). Le 2026-09-28.

Deux fenêtres dans le même programme : la première avec `minWidth = 400` et `minHeight = 300`, la seconde avec le minimum retiré (`1 × 1`). Sur chacune, j'essaie de descendre en dessous de trois façons : par `SetSize` du moteur, par `SetWindowPos` de Windows, et en demandant à la fenêtre elle-même ce qu'elle déclare au système (`WM_GETMINMAXINFO`).

## Le relevé

```text
Defauts de NkWindowConfig : minWidth=160 minHeight=90
Facteur d echelle de l ecran (GetDpiScale) : 1.25
Minimum impose par Windows (SM_CXMIN x SM_CYMIN) : 166 x 47

FENETRE 1 : minWidth=400 minHeight=300
  bornes declarees au systeme (WM_GETMINMAXINFO) : 400 x 300
  a l'ouverture                      client  418 x 253  | exterieur  436 x 300
  apres SetSize(1,1)                 client  382 x 253  | exterieur  400 x 300
  apres SetSize(200,150)             client  382 x 253  | exterieur  400 x 300
  apres SetWindowPos 1x1 (systeme)   client  382 x 253  | exterieur  400 x 300

FENETRE 2 : minWidth=1 minHeight=1 (minimum retire)
  bornes declarees au systeme (WM_GETMINMAXINFO) : 1 x 1
  a l'ouverture                      client  420 x 240  | exterieur  438 x 287
  apres SetSize(1,1)                 client  148 x 1    | exterieur  166 x 48
  apres SetSize(200,150)             client  200 x 150  | exterieur  218 x 197
  apres SetWindowPos 1x1 (systeme)   client  148 x 0    | exterieur  166 x 47
```

## Avec la borne : elle tient, et elle tient sur l'extérieur

Quoi que je demande, la fenêtre 1 s'arrête à **400 × 300 en extérieur**, jamais en dessous. `SetSize(1,1)` et `SetSize(200,150)` donnent exactement le même résultat.

Le point qui m'a surpris : **la borne porte sur la fenêtre entière, pas sur la zone de dessin**. À 400 × 300 extérieur, il ne reste que **382 × 253** de zone client. La différence, c'est le cadre : 18 px de bordures en largeur, 47 px de barre de titre en hauteur (à 125 %).

La borne s'applique aussi **à l'ouverture** : j'ai demandé 420 × 240, ce qui fait 287 px de haut avec le cadre, donc moins que les 300 demandés. La fenêtre s'est ouverte à 436 × 300, c'est-à-dire **plus grande que ce que j'avais écrit**. La hauteur de la configuration a été écrasée par la borne.
*(La largeur ouverte fait 418 de client au lieu de 420 : 2 px perdus, probablement un arrondi lié au 125 %. Je ne l'ai pas expliqué.)*

## Où c'est appliqué dans le moteur

Le moteur ne vérifie rien lui-même. Il **répond** à la question que Windows lui pose avant chaque changement de taille :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Platform/Win32/NkWin32EventSystem.cpp:677-682
case WM_GETMINMAXINFO:
    if (owner) {
        auto *mm = reinterpret_cast<MINMAXINFO *>(lp);
        mm->ptMinTrackSize.x = (LONG)owner->GetConfig().minWidth;
        mm->ptMinTrackSize.y = (LONG)owner->GetConfig().minHeight;
    }
    break;
```

`ptMinTrackSize` est la taille minimale de **suivi** : c'est exactement la borne que Windows applique quand l'utilisateur tire un bord, et ma mesure montre qu'elle s'applique aussi à `SetWindowPos`. C'est aussi ce que me renvoie mon sondage `WM_GETMINMAXINFO` : 400 × 300 pour la fenêtre 1, 1 × 1 pour la fenêtre 2.

Et `SetSize` ne borne rien de son côté :

```cpp
// Platform/Win32/NkWin32Window.cpp:849-856
void NkWindow::SetSize(uint32 w, uint32 h) {
    mConfig.width = w;
    mConfig.height = h;
    RECT rc = {0, 0, (LONG)w, (LONG)h};
    AdjustWindowRectEx(&rc, mData.mDwStyle, FALSE, mData.mDwExStyle);
    SetWindowPos(mData.mHwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
}
```

Preuve par la fenêtre 2 : `SetSize(1,1)` y donne un client de **148 × 1**. Si le moteur bornait, il aurait ramené à ses défauts 160 × 90.

## Sans la borne : ce que le système accepte quand même

La fenêtre 2 descend jusqu'à :

| Mesure | Valeur |
|---|---|
| Plus petit extérieur atteint | **166 × 47 px** |
| Zone client correspondante | **148 × 0 px** |
| `GetSystemMetrics(SM_CXMIN, SM_CYMIN)` | **166 × 47 px** |

Le plancher mesuré est **exactement** la métrique que Windows publie : `SM_CXMIN` × `SM_CYMIN`. Ce n'est donc pas un hasard ni un arrondi, c'est la limite du système. Elle vient de la barre de titre : une fenêtre à `WS_CAPTION` ne peut pas être plus étroite que ses trois boutons, et pas plus basse que sa barre. Elle dépend de l'échelle : à 125 %, 166 × 47 ; à 100 %, elle serait plus petite.

La petite différence entre 48 et 47 px de haut : avec `SetSize(1,1)` je demande 1 px de client, donc 48 extérieur ; avec `SetWindowPos 1x1` je demande 1 px d'extérieur, et Windows remonte à son plancher de 47.

## Ce que ça confirme du chapitre

Le §3.2.1 dit que les bornes minimales « ne sont pas de la décoration », et qu'un calcul qui divise par la largeur tombe sur une division par zéro. C'est mesuré : **la fenêtre sans borne atteint une zone client de hauteur 0**. Un rendu qui calcule `largeur / hauteur` s'y arrête net.

Les défauts `160 × 90` ne laissent jamais arriver là : ils imposent 160 × 90 en extérieur, donc environ 142 × 43 de client, petit mais jamais nul.