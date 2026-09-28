# Chapitre 3 — Exercice 4 : le facteur d'échelle

Windows 11, backend Win32, cible de rendu **NKCanvas** (`NkRenderWindow`, OpenGL). Jenga 2.8.0, Debug. Le 2026-09-28.

Mon écran est à **125 %**, donc je n'ai pas eu besoin de changer le réglage : `GetDpiScale()` renvoie 1.25.

## Le relevé

```text
Demande dans la configuration : 1280 x 720
Ecran : 1920 x 1080, 1 moniteur(s)

a l'ouverture           | fenetre 1278 x 712 | cible 1278 x 712 | echelle 1.25 | ecart +0 x +0
apres SetSize(1280,720) | fenetre 1280 x 720 | cible 1280 x 720 | echelle 1.25 | ecart +0 x +0
apres SetSize(800,450)  | fenetre  800 x 450 | cible  800 x 450 | echelle 1.25 | ecart +0 x +0
apres Maximize()        | fenetre 1920 x 991 | cible 1920 x 991 | echelle 1.25 | ecart +0 x +0
apres Restore()         | fenetre  800 x 450 | cible  800 x 450 | echelle 1.25 | ecart +0 x +0
```

Et dans la barre de titre, les trois valeurs côte à côte :

```text
fenetre 800x450 | cible 800x450 | echelle 1.25
```

## Résultat : l'écart est toujours de zéro

Je m'attendais, avec un facteur de 1,25, à voir la cible de rendu annoncer 25 % de plus que la fenêtre. **Ce n'est jamais le cas** : les deux tailles sont identiques à chaque mesure, y compris agrandie.

Ce n'est pas de la chance. La cible ne fait que **retransmettre** la question :

```cpp
// Kernel/Runtime/NKCanvas/src/NKCanvas/Renderer/Targets/NkRenderWindow.cpp:225-227
math::NkVec2u NkRenderWindow::GetSize() const {
    return mWindow ? mWindow->GetSize() : math::NkVec2u{0, 0};
}
```

Et la fenêtre, sur Win32, répond déjà en **pixels physiques** :

```cpp
// Platform/Win32/NkWin32Window.cpp:736-740
NkVec2u NkWindow::GetSize() const {
    RECT rc = {};
    if (mData.mHwnd) GetClientRect(mData.mHwnd, &rc);
    NkVec2u size = {(uint32)(rc.right - rc.left), (uint32)(rc.bottom - rc.top)};
```

`GetClientRect` rend des pixels réels parce que le moteur déclare le processus **conscient du DPI par moniteur** dès la première fenêtre :

```cpp
// Platform/Win32/NkWin32Window.cpp:70-77
// Windows fait rendre l'app en résolution LOGIQUE puis l'UPSCALE vers le physique
// -> tout est flou (icônes ET texte). Avec, on rend en pixels PHYSIQUES = net.
pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
```

Même chose côté macOS : `QueryContentSizePx` (`Platform/Cocoa/NkCocoaWindow.mm:118-126`) multiplie la taille en points par `backingScaleFactor` **avant** de la rendre. Les deux backends que j'ai lus répondent donc en pixels.

Le facteur, lui, est bien réel : `GetDpiScale()` vaut `GetDpiForWindow(hwnd) / 96` (`NkWin32Window.cpp:762-764`), soit 120/96 = 1.25.

## Où le piège du chapitre mord quand même

Le §3.3.2 dit « la fenêtre parle en points, la cible de rendu parle en pixels ». Dans Nkentseu, les deux parlent en pixels. Mais l'échelle se voit ailleurs, et je l'ai mesurée :

**À l'ouverture, je n'obtiens pas ce que j'ai demandé.** J'ai écrit `1280 × 720`, la fenêtre s'ouvre à **1278 × 712** : 2 px et 8 px perdus. Et `SetSize(1280,720)`, juste après, donne exactement 1280 × 720. L'écart vient donc du **chemin de création**, pas de `SetSize`. Le calcul du cadre à la création passe par `AdjustWindowRectEx` (`NkWin32Window.cpp:477`), qui raisonne sur le DPI du système et non sur celui de la fenêtre — c'est mon explication la plus probable, je ne l'ai pas prouvée ligne à ligne.

**Ce que la configuration signifie.** `cfg.width = 1280` ne veut pas dire « 1280 points » mais « 1280 pixels ». Sur mon écran à 125 %, la fenêtre occupe donc deux tiers de la largeur (1280 sur 1920), pas 1280 × 1,25.

**L'écran aussi est en pixels.** `GetDisplaySize()` renvoie 1920 × 1080 (`GetSystemMetrics(SM_CXSCREEN)`, ligne 766-768), la taille physique ; un bureau à 125 % ne fait « que » 1536 × 864 en points.

## Ce que je retiens de la règle du chapitre

La règle « pour dessiner, demandez la taille à la cible de rendu » reste la bonne habitude, même si aujourd'hui les deux appels donnent le même nombre. Elle coûte zéro, et elle protège de trois cas qui arrivent :
- un backend qui rendrait des points (ce n'est le cas ni de Win32 ni de Cocoa, mais rien ne le garantit pour les 12 autres) ;
- une cible qui **n'est pas** la fenêtre : une texture de rendu, un aperçu, une capture, où la taille n'a plus rien à voir ;
- le cas mesuré ci-dessus, où la fenêtre n'a pas la taille qu'on lui a demandée.