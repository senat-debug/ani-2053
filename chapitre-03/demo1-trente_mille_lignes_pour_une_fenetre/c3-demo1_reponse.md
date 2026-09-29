 # Chapitre 3 — Trente mille lignes pour une fenêtre

Module `Kernel/Runtime/NKWindow`, dépôt Nkentseu, mesuré le 2026-09-29 sur ma machine (Windows 11, Git Bash).

## 1. Fichiers et lignes

Les commandes, lancées à la racine du dépôt. Je ne compte que les fichiers **suivis par git** (pas de `Build/`, pas de fichier temporaire) :

```bash
git ls-files "Kernel/Runtime/NKWindow/*" | grep -Ei '\.(h|hpp|inl|c|cpp|mm)$' | wc -l
git ls-files "Kernel/Runtime/NKWindow/*" | grep -Ei '\.(h|hpp|inl|c|cpp|mm)$' | xargs wc -l | tail -1
```

```text
119
  33647 total
```

| Extension | Fichiers | Lignes |
|---|---:|---:|
| `.h` | 74 | 11 921 |
| `.cpp` | 35 | 18 532 |
| `.mm` (Objective-C++) | 8 | 2 926 |
| **Total** | **119** | **33 647** |

Le chapitre annonce 109 fichiers et 30 453 lignes. L'écart n'est pas une erreur de comptage : le module a **grossi depuis la mesure du livre** — +10 fichiers, +3 194 lignes. Même méthode, dépôt plus récent.

## 2. Les backends de plateforme

```bash
ls Kernel/Runtime/NKWindow/src/NKWindow/Platform
```

```text
Android  Cocoa  Common  Emscripten  HarmonyOS  Linux  Noop
UIKit    UWP    Wayland  Win32      XCB        XLib   Xbox
```

**14 dossiers**, exactement le chiffre du chapitre. Leur poids, mesuré avec une boucle sur les `.cpp`, `.h` et `.mm` de chaque dossier :

| Backend | Fichiers | Lignes | |
|---|---:|---:|---|
| Wayland | 9 | 5 894 | Linux moderne |
| Win32 | 6 | 3 384 | Windows bureau |
| Android | 7 | 2 841 | mobile |
| Emscripten | 7 | 2 424 | web |
| XCB | 5 | 2 372 | Linux, protocole X moderne |
| XLib | 5 | 2 128 | Linux, protocole X historique |
| HarmonyOS | 6 | 1 693 | mobile |
| UIKit | 6 | 1 350 | iOS |
| Cocoa | 5 | 1 340 | macOS |
| Xbox | 5 | 802 | console |
| Noop | 5 | 538 | **aucune fenêtre** |
| UWP | 5 | 523 | Windows applications |
| Linux | 1 | 415 | code commun aux trois backends Linux |
| Common | 1 | 59 | partagé |

Deux remarques que la liste rend visibles, et que le chapitre annonce :

- **Linux en a trois** (XLib, XCB, Wayland), plus un dossier `Linux` commun : 10 437 lignes à lui seul, soit près d'un tiers du module ;
- **`Noop` existe** : 538 lignes pour une fenêtre qui n'existe pas, et c'est ce qui permet de faire tourner un test ou un banc sur une machine sans écran.

## 3. Le même appel, dans deux backends

J'ai choisi **`SetFullscreen(bool)`**, déclaré une seule fois dans l'interface publique :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Core/NkWindow.h:161
void SetFullscreen(bool fullscreen);
```

### Win32

```cpp
// Platform/Win32/NkWin32Window.cpp:1346-1362
void NkWindow::SetFullscreen(bool fs) {
    mConfig.fullscreen = fs;

    if (fs) {
        SetWindowLongW(mData.mHwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        SetWindowPos(mData.mHwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                     SWP_FRAMECHANGED);

        // Mettre à jour la taille
        mConfig.width  = GetSystemMetrics(SM_CXSCREEN);
        mConfig.height = GetSystemMetrics(SM_CYSCREEN);
    } else {
        SetWindowLongW(mData.mHwnd, GWL_STYLE, (LONG)mData.mDwStyle);
        SetWindowPos(mData.mHwnd, nullptr, mConfig.x, mConfig.y, (int)mConfig.width, (int)mConfig.height,
                     SWP_FRAMECHANGED | SWP_NOZORDER);
    }
}
```

### XLib

```cpp
// Platform/XLib/NkXLibWindow.cpp:1081-1110
void NkWindow::SetFullscreen(bool fullscreen) {
    if (!mData.mDisplay || !mData.mXid) {
        return;
    }
    mConfig.fullscreen = fullscreen;

    Atom wmState = XInternAtom(mData.mDisplay, "_NET_WM_STATE", False);
    Atom wmFs    = XInternAtom(mData.mDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    XEvent ev = {};
    ev.type = ClientMessage;
    ev.xclient.window       = mData.mXid;
    ev.xclient.message_type = wmState;
    ev.xclient.format       = 32;
    ev.xclient.data.l[0] = fullscreen ? 1 : 0;
    ev.xclient.data.l[1] = static_cast<long>(wmFs);
    XSendEvent(mData.mDisplay, DefaultRootWindow(mData.mDisplay), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &ev);
    XFlush(mData.mDisplay);

    // Mettre à jour la taille
    if (fullscreen) {
        mConfig.width  = DisplayWidth(mData.mDisplay, mData.mScreen);
        mConfig.height = DisplayHeight(mData.mDisplay, mData.mScreen);
    } else {
        XWindowAttributes attrs;
        XGetWindowAttributes(mData.mDisplay, mData.mXid, &attrs);
        mConfig.width  = attrs.width;
        mConfig.height = attrs.height;
    }
}
```

## Ce qui est identique

- **La signature** : `void NkWindow::SetFullscreen(bool)`. Mon programme écrit la même ligne des deux côtés.
- **La mémoire de l'état** : les deux écrivent `mConfig.fullscreen`, et les deux remettent à jour `mConfig.width` et `mConfig.height`.
- **La garde** : les deux vérifient que la fenêtre native existe avant d'agir (`mHwnd` d'un côté, `mDisplay` et `mXid` de l'autre).
- **Le contrat** : après l'appel, `GetSize()` rend la nouvelle taille. C'est ce que l'application voit.

## Ce qui change

| | Win32 | XLib |
|---|---|---|
| À qui on parle | à **la fenêtre**, directement | au **gestionnaire de fenêtres**, par la racine de l'écran |
| Par quel moyen | deux appels d'API (`SetWindowLongW`, `SetWindowPos`) | un **message client** X11 envoyé avec `XSendEvent` puis `XFlush` |
| Nature de l'opération | un ordre : le style change, la fenêtre est repositionnée | une **demande** : le gestionnaire peut l'honorer, la retarder, ou l'ignorer |
| Ce qu'il faut savoir | la constante de style à restaurer (`mData.mDwStyle`) | deux **atomes** à résoudre au préalable (`_NET_WM_STATE`, `_NET_WM_STATE_FULLSCREEN`) |
| Taille de l'écran | `GetSystemMetrics(SM_CXSCREEN)` | `DisplayWidth(display, screen)` |
| Retour à la fenêtre | on repose le style et la position mémorisés | on relit l'état réel avec `XGetWindowAttributes` |
| Synchronicité | l'effet est immédiat au retour de l'appel | l'effet arrive **plus tard**, quand le gestionnaire réagit |
