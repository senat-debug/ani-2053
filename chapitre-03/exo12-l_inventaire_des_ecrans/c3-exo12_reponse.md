# Chapitre 3 — Exercice 12 : l'inventaire des écrans

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

**Ma machine n'a qu'un écran.** Je le dis tout de suite : la deuxième moitié de l'exercice — déplacer la fenêtre d'un écran à l'autre et vérifier que les valeurs suivent — je ne peux pas la faire. Ce que je peux faire, c'est l'inventaire complet du seul écran, le déplacement de la fenêtre dessus, et la lecture du backend pour dire ce qui suivrait et ce qui ne suivrait pas avec un second écran.

## L'inventaire

```text
===== A l'ouverture =====
fenetre : position 632 , 340   taille 638 x 352   echelle 1.25
GetMonitorCount() = 1, EnumerateMonitors() en rend 1
  ecran 0 "\\.\DISPLAY1"  [principal]
      taille logique 1920 x 1080 | physique 1920 x 1080
      position 0 , 0 | echelle 1.25 (120 x 120 dpi) | 60 Hz   <== PORTE LA FENETRE
  GetCurrentMonitor() : index=0 "\\.\DISPLAY1" position 0 , 0 echelle 1.25
  GetDisplaySize()     : 1920 x 1080
  GetDisplayPosition() : 0 , 0
```

`NkDisplayInfo` donne tout ce que l'exercice demande : nom, taille, position, échelle, DPI, fréquence, et le drapeau `isPrimary`.

## Le déplacement

J'ai déplacé la fenêtre à trois endroits, et relevé à chaque fois :

| Position demandée | Position relue | Écran porteur | Échelle |
|---|---|---|---|
| ouverture (centrée) | 632 , 340 | DISPLAY1 | 1.25 |
| 40 , 40 | 40 , 40 | DISPLAY1 | 1.25 |
| 1200 , 500 | 1200 , 500 | DISPLAY1 | 1.25 |
| 2200 , 300 | **2200 , 300** | DISPLAY1 | 1.25 |

La position suit exactement. Deux choses à noter :

- **2200 est en dehors de l'écran** (qui fait 1920 de large), et Windows accepte : la fenêtre part hors champ, invisible. L'écran porteur reste DISPLAY1 parce que le backend demande le moniteur *le plus proche* :

```cpp
// Platform/Win32/NkWin32Window.cpp:838-842
NkDisplayInfo NkWindow::GetCurrentMonitor() const {
    HMONITOR hmon = mData.mHwnd ? MonitorFromWindow(mData.mHwnd, MONITOR_DEFAULTTONEAREST)
                                : MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
    return Win32FillDisplayInfo(hmon, 0);
}
```

- **Un seul déplacement produit plusieurs `NkWindowMoveEvent`** : mon journal contient six blocs « Déplacement reçu » pour un seul `SetPosition`, tous à la même position. Il faut donc filtrer si le traitement coûte cher.

## Trois écarts que j'ai trouvés en lisant le backend

**1. `GetCurrentMonitor()` ne dit jamais quel écran c'est.** Regardez la dernière ligne du code ci-dessus : `Win32FillDisplayInfo(hmon, 0)`. L'index est écrit **en dur à 0**. Avec deux écrans, une fenêtre posée sur le second rendrait quand même `index = 0`. C'est pour ça que mon programme ne compare pas les index, mais la **position et la taille** :

```cpp
static bool MemeEcran(const NkDisplayInfo &a, const NkDisplayInfo &b) {
	return a.posX == b.posX && a.posY == b.posY && a.width == b.width && a.height == b.height;
}
```

**2. `GetDisplayPosition()` est écrit en dur.**

```cpp
// Platform/Win32/NkWin32Window.cpp:770-772
NkVec2u NkWindow::GetDisplayPosition() const {
    return {0, 0};
}
```

Avec un écran, c'est vrai. Avec un second écran à droite, sa position serait 1920,0, et cette fonction continuerait de dire 0,0. Elle ne suivrait donc **pas** la fenêtre. `GetDisplaySize()` a le même défaut : il lit `GetSystemMetrics(SM_CXSCREEN)` (ligne 766-768), c'est-à-dire **l'écran principal**, pas celui de la fenêtre.

C'est `EnumerateMonitors()` et `GetCurrentMonitor()` qu'il faut employer, pas ce couple-là.

**3. Le champ « taille logique » contient des pixels physiques.** `NkDisplayInfo::width` est documenté « Résolution horizontale en pixels logiques » (`NkSystemEvent.h:160`). Mesure : il rend **1920 × 1080**, exactement comme `physWidth`. Et le bureau logique, à 125 %, fait 1536 × 864 — c'est ce que Windows déclare à un programme qui ignore le DPI :

```text
\\.\DISPLAY1 principal=True bounds={X=0,Y=0,Width=1536,Height=864}
```

La raison est dans le remplissage : `width` vient de `mi.rcMonitor` (`NkWin32Window.cpp:791-792`), et comme le moteur déclare le processus conscient du DPI par moniteur, ce rectangle est en pixels physiques. Les deux champs disent donc la même chose, et le nom du premier est trompeur. C'est le même constat qu'à l'exercice 4.

## Ce qu'il faudrait pour finir l'exercice

Un second écran, avec une échelle différente du premier (par exemple 100 % contre 125 %). On vérifierait alors trois choses :

1. `EnumerateMonitors()` rend bien deux entrées avec des `posX` différents — d'après le code, oui : la position vient de `rcMonitor`, propre à chaque moniteur ;
2. `GetCurrentMonitor()` change de `posX` et de `dpiScale` quand la fenêtre passe de l'un à l'autre — d'après le code, oui, mais son `index` restera 0 ;
3. `GetDpiScale()` de la fenêtre change aussi — il lit `GetDpiForWindow` (ligne 762-764), donc oui, et un `NkWindowDpiEvent` existe pour prévenir.
