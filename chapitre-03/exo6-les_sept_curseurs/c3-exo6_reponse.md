# Chapitre 3 — Exercice 6 : les sept curseurs

Windows 11, backend Win32, Jenga 2.8.0, Debug. Écran à 125 %. Le 2026-09-28.

Une seule fenêtre de 1120 × 300, découpée en **sept bandes verticales** que je calcule moi-même : `zone = x / (largeur / 7)`. NKWindow ne propose rien pour subdiviser une fenêtre, donc c'est à mon code de savoir dans quel rectangle est la souris.

## Comment je vérifie ce que je vois

Je ne me fie pas à mon œil. À chaque `NkMouseMoveEvent`, avant de demander quoi que ce soit, je lis la forme **réellement affichée** par le système avec `GetCursorInfo`, et je compare son `HCURSOR` aux sept curseurs standards chargés une fois au départ (`LoadCursorW(IDC_ARROW)`, `IDC_IBEAM`, `IDC_HAND`, `IDC_SIZENS`, `IDC_SIZEWE`, `IDC_SIZENWSE`, `IDC_SIZENESW`). La forme lue au début d'un mouvement est le résultat de la demande faite au mouvement précédent.

J'ai ensuite promené la souris de zone en zone, six petits déplacements par zone : **86 mouvements reçus, 45 appels à `SetCursor`**.

## A) Une forme par zone, posée à chaque mouvement

| Zone | Forme demandée | Forme obtenue |
|---|---|---|
| 1 | `Arrow` | **Arrow** |
| 2 | `TextInput` | **TextInput** |
| 3 | `Hand` | **Hand** |
| 4 | `ResizeNS` | **ResizeNS** |
| 5 | `ResizeWE` | **ResizeWE** |
| 6 | `ResizeNWSE` | **ResizeNWSE** |
| 7 | `ResizeNESW` | **ResizeNESW** |

**Aucun écart.** Les sept formes répondent, et le curseur change bien à chaque passage de frontière.

C'est logique quand on lit le code : la conversion est une simple table, et les sept valeurs de `NkCursorType` ont toutes leur équivalent Win32 :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Core/NkWindowCursor.cpp:14-40
void NkWindow::SetCursor(NkCursorType cursor) {
    LPCWSTR idc = IDC_ARROW;
    switch (cursor) {
        case NkCursorType::TextInput:  idc = IDC_IBEAM;      break;
        case NkCursorType::Hand:       idc = IDC_HAND;       break;
        case NkCursorType::ResizeNS:   idc = IDC_SIZENS;     break;
        case NkCursorType::ResizeWE:   idc = IDC_SIZEWE;     break;
        case NkCursorType::ResizeNWSE: idc = IDC_SIZENWSE;   break;
        case NkCursorType::ResizeNESW: idc = IDC_SIZENESW;   break;
        default:                       idc = IDC_ARROW;      break;
    }
    HCURSOR hc = ::LoadCursorW(nullptr, idc);
```

Là où un curseur n'est posé par aucun code, c'est **sur les autres plateformes**, et le fichier le dit en toutes lettres :

```cpp
// NkWindowCursor.cpp:54-58
#else
void NkWindow::SetCursor(NkCursorType /*cursor*/) {
    // Plateformes sans curseur souris (Android, iOS, Web tactile, headless).
}
#endif
```

Sur Android, iOS et le web tactile, mes sept appels ne font **rien** — et c'est le comportement voulu du §3.9 : ne rien faire plutôt qu'échouer.

## B) Un seul appel, au démarrage

Je passe en mode « appel unique » avec la touche `U` : un `SetCursor(Hand)` et plus rien ensuite. Je repasse sur les sept zones.

| Zone | Forme demandée | Forme obtenue |
|---|---|---|
| 1 à 7 | `Hand`, une seule fois au démarrage | **Hand** partout |

**La main reste**, dans les sept zones, sans clignoter et sans revenir à la flèche.

Ce n'est pas ce que je m'attendais à voir. Le chapitre dit (§3.4.1) que « le curseur est persistant, et il faut le redemander à chaque image », et l'en-tête du moteur répète la même chose :

```cpp
// Core/NkWindow.h:280-282
// Persistant : à rappeler chaque frame avec le curseur voulu (sinon, sur
// certaines plateformes, le système le réinitialise à la flèche).
void SetCursor(NkCursorType cursor);
```

Sur Win32, ce rappel n'est pas nécessaire, et le backend explique pourquoi. `SetCursor` mémorise la forme, et c'est le système qui vient la redemander à chaque mouvement :

```cpp
// Core/NkWindowCursor.cpp:41
mData.mClientCursor = hc; // mémorisé pour WM_SETCURSOR
```

```cpp
// Platform/Win32/NkWin32EventSystem.cpp:688-693
case WM_SETCURSOR:
    if (owner && LOWORD(lp) == HTCLIENT && owner->mData.mClientCursor) {
        ::SetCursor(owner->mData.mClientCursor);
        result = TRUE; // on a géré → empêche le reset par DefWindowProc
    }
```

Windows envoie `WM_SETCURSOR` dès que la souris bouge au-dessus de la zone client. Sans ce `return TRUE`, `DefWindowProc` remettrait le curseur de la classe de fenêtre, c'est-à-dire la flèche — c'est exactement le défaut décrit par le chapitre. Le moteur l'a déjà traité pour moi.

## Un détail du code qui compte

`SetCursor` n'applique la forme tout de suite **que si la souris est au-dessus de ma fenêtre** :

```cpp
// Core/NkWindowCursor.cpp:42-51
// N'applique IMMEDIATEMENT que si le curseur survole NOTRE fenetre :
// appelee chaque frame, ::SetCursor depuis l'ARRIERE-PLAN ecrasait le
// curseur de la fenetre au premier plan en continu (clignotement
// constate par Rihen). WM_SETCURSOR fait foi dans notre client.
POINT ptC;
if (!::GetCursorPos(&ptC)) return;
HWND underC = ::WindowFromPoint(ptC);
if (underC && ::GetAncestor(underC, GA_ROOT) == mData.mHwnd)
    ::SetCursor(hc);
```

Autrement dit : si j'appelle `SetCursor` pendant que la souris est ailleurs, seule la mémoire est mise à jour ; la forme apparaîtra au retour de la souris. C'est la correction d'un vrai défaut — une application qui posait son curseur à chaque image faisait clignoter celui des autres fenêtres.
