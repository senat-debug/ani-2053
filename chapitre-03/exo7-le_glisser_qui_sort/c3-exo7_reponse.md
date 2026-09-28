# Chapitre 3 — Exercice 7 : le glisser qui sort

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Le même geste, fait deux fois : bouton gauche enfoncé au centre de la fenêtre, puis la souris part vers la droite par pas de 40 px jusqu'à sortir largement, puis relâchement **dehors**. La première fois sans capture, la seconde avec.

Le programme classe lui-même chaque position reçue : « dedans » ou « dehors », en la comparant à `GetSize()`.

## Le relevé

```text
A) SANS capture
  mouvements recus DANS la fenetre  : 6
  mouvements recus HORS la fenetre  : 0
  derniere position recue           : 649 , 192
  GetCapture() == notre fenetre     : non
  relachement recu                  : NON

B) AVEC capture
  mouvements recus DANS la fenetre  : 7
  mouvements recus HORS la fenetre  : 6
  derniere position recue           : 949 , 192
  GetCapture() == notre fenetre     : oui
  relachement recu                  : oui (souris dehors)
```

## Ce que ça change pour l'utilisateur

| | Sans capture | Avec capture |
|---|---|---|
| Pendant le glisser | l'objet **se bloque au bord** de la fenêtre et n'avance plus | l'objet **continue de suivre** la souris, même loin dehors |
| Au relâchement dehors | le programme **ne l'apprend jamais** | il le reçoit, à la position exacte du relâchement |
| Après le geste | le programme croit encore que le bouton est enfoncé | le glisser est terminé proprement |

Le point le plus visible pour l'utilisateur n'est pas le blocage, c'est **l'après**. Sans capture, mon programme n'a jamais reçu le relâchement : son drapeau `enGlisser` reste vrai. L'utilisateur, lui, a lâché le bouton. Quand il revient dans la fenêtre, **l'objet se remet à suivre la souris sans qu'aucun bouton soit appuyé**, et il faut recliquer pour s'en débarrasser. C'est le défaut classique du curseur « collé ».

Avec la capture, le geste se termine là où l'utilisateur l'a terminé. Il peut même tirer une poignée bien au-delà de la fenêtre : la dernière position reçue est 949 en X, alors que la zone client n'en fait que ~700. Si j'étais sorti par la gauche ou par le haut, j'aurais reçu des coordonnées **négatives** — `NkMouseMoveEvent` les porte en `int32` (`NkMouseEvent.h:264, 285`), donc elles passent sans être tronquées.

## Pourquoi, dans le moteur

La capture est un simple relais vers Windows :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Platform/Win32/NkWin32Window.cpp:1376-1381
void NkWindow::CaptureMouse(bool cap) {
    if (cap)
        SetCapture(mData.mHwnd);
    else
        ReleaseCapture();
}
```

`SetCapture` dit au système : « tous les messages de souris viennent chez moi, où que soit le curseur, jusqu'à nouvel ordre ». Sans lui, Windows envoie les messages à la fenêtre **sous** le curseur ; dès que la souris sort, c'est une autre fenêtre — ou le bureau — qui les reçoit, y compris le `WM_LBUTTONUP`. Mon `GetCapture() == notre fenetre` le confirme : `non` dans le cas A, `oui` dans le cas B.

## À ne pas confondre avec le confinement

Le §3.4.2 met les deux côte à côte, et ce n'est pas la même chose :

```cpp
// NkWin32Window.cpp:1383-1395
void NkWindow::ClipMouseToClient(bool clip) {
    ...
    ::ClipCursor(&clipR);
```

- **Capture** : la souris **peut** sortir, mais c'est moi qui continue de recevoir ses mouvements. C'est pour les glissers.
- **Confinement** (`ClipMouseToClient`) : la souris **ne peut pas** sortir du tout, `ClipCursor` l'enferme dans le rectangle client. C'est pour les jeux à la première personne et la stratégie.

## La règle

Capturer au `NkMouseButtonPressEvent`, relâcher au `NkMouseButtonReleaseEvent` — c'est ce que fait mon programme. Et surtout **ne jamais oublier le relâchement** : tant qu'une fenêtre garde la capture, elle reçoit toute la souris du bureau, et l'utilisateur ne peut plus cliquer ailleurs.

