# Chapitre 4 — Le chemin d'une touche

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Ce que mon programme voit quand j'appuie sur la touche **A** :

```text
n    | ms         | NkKey          | NkScancode       | VK natif | fenetre
1    | 5640       | NK_A           | SC_A             | 0x51     | 1
2    | 6406       | NK_SPACE       | SC_SPACE         | 0x20     | 1
3    | 7187       | NK_W           | SC_W             | 0x5A     | 1
4    | 7968       | NK_ESCAPE      | SC_ESCAPE        | 0x1B     | 1
```

Quatre colonnes, et chacune est la trace d'une étape différente. Voici les cinq étapes, dans l'ordre, telles que je les ai trouvées en lisant le moteur.

## Étape 1 — Windows dépose le message : **qui reçoit**

**`NkEventSystem::PumpOS()`** — `Kernel/Runtime/NKWindow/src/NKWindow/Platform/Win32/NkWin32EventSystem.cpp:87-101`

```cpp
MSG msg = {};
while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) break;
    TranslateMessage(&msg);
    DispatchMessageW(&msg); // route vers NkWin32WndProc
}
```

Le pilote du clavier a déjà posé un `WM_KEYDOWN` dans la file du **thread** de ma fenêtre. `PeekMessageW` l'en retire, `TranslateMessage` fabrique au passage le `WM_CHAR` du caractère, et `DispatchMessageW` appelle la procédure de fenêtre. Rien du moteur n'existe encore : à ce stade, c'est un message Windows.

## Étape 2 — La procédure de fenêtre attrape le message : **qui trie**

**`NkWin32WndProc`** — même fichier, ligne 122, qui délègue à **`NkWin32_ProcessMessage`**, ligne 207.

C'est un `switch (msg)` géant : une branche par message. La touche tombe sur `case WM_KEYDOWN`.

## Étape 3 — La traduction : **qui traduit**

**`NkWin32_ProcessMessage`, `case WM_KEYDOWN`** — `NkWin32EventSystem.cpp:623-646`

```cpp
UINT sc = (lp >> 16) & 0xFF;          // le scancode est dans le lParam
bool isExt = (lp >> 24) & 1;
NkScancode nkSc = NkScancodeFromWin32(sc, isExt);   // Win32 -> USB HID
NkKey k = NkScancodeToKey(nkSc);                    // position -> NkKey
if (k == NkKey::NK_UNKNOWN)
    k = NkWin32_VkeyToNkKey((UINT)wp, isExt);       // repli : par le code virtuel
...
NkKeyPressEvent e(k, nkSc, mods, nativeKey, isExt, winId);
```

Trois choses naissent ici : le **scancode** (position physique, indépendante de la disposition), la **touche** `NkKey`, et l'objet `NkKeyPressEvent` lui-même. Les tables de conversion sont dans `NkKeycodeMap.h` (`NkScancodeFromWin32`, `ScancodeToNkKey`).

J'ai vu les **deux branches** en mesurant. En envoyant de vrais scancodes, la première marche :

```text
NK_A | SC_A | VK 0x51
```

En envoyant seulement un code virtuel (scancode à zéro), la première branche échoue et c'est le repli qui donne la touche :

```text
NK_A | SC_UNKNOWN | VK 0x41
```

La colonne `NkScancode` de mon programme est donc la trace directe de cette étape.

## Étape 4 — Le rangement : **qui range**

**`NkEventSystem::Enqueue`** puis **`DeliverOnPumpThread`** — `Kernel/Runtime/NKEvent/src/NKEvent/NkEventSystem.cpp:202` et `:229`

```cpp
evt.SetWindowId(winId);                 // c'est ici que ma colonne "fenetre" est remplie
...
DispatchToCallbacks(&evt, winId);       // les abonnes sont appeles TOUT DE SUITE
++mTotalEventCount;
if (mAutoUpdateInputState) UpdateInputState(&evt);   // l'etat clavier est mis a jour
if (mQueueMode) {
    NkEventPtr clone(evt.Clone());
    NkEventPriority prio = NkGetEventPriority(evt.GetType());  // une touche = HIGH
    NkScopedSpinLock lock(mQueueMutex);
    mEventQueue.Push(traits::NkMove(clone), prio);
}
```

Quatre choses, dans cet ordre : l'événement est **estampillé** de l'identifiant de sa fenêtre, les **abonnés** sont appelés immédiatement (c'est par là que les actions nommées sont alimentées, `NkWESystem.cpp:142`), l'**état clavier** est mis à jour — ce que lira `IsKeyPressed` — et enfin une **copie** est poussée dans la file à deux priorités, en `HIGH` pour une touche.

## Étape 5 — La distribution : **qui distribue**

**`NkEventSystem::PollEvent()`** — `NkEventSystem.cpp:405-450`

```cpp
DrainForeignEvents();
{   NkScopedSpinLock lock(mQueueMutex);
    auto ev = mEventQueue.Pop();
    if (ev) { mCurrentEvent = traits::NkMove(ev); return mCurrentEvent.Get(); } }

// Queue vide — pomper l'OS pour remplir
PumpOS();
RefreshAxes();
// Retenter apres le pump
```

C'est le point que je n'avais pas compris avant de lire : **c'est `PollEvent` qui appelle `PumpOS`**, pas l'inverse. Ma boucle ne pompe rien elle-même ; quand la file est vide, `PollEvent` va chercher les messages Windows, ce qui relance les étapes 1 à 4, puis retente le dépilage. La boucle fait donc le tour complet toute seule.

Et `mCurrentEvent` garde la propriété : c'est ce qui rend le pointeur invalide au prochain appel.

## Étape 6 — Ma ligne

```cpp
while (NkEvent *ev = NkEvents().PollEvent()) {
    if (auto *k = ev->As<NkKeyPressEvent>()) { ... }
}
```

Entre l'appui et cette ligne : **six fonctions, trois fichiers**, et environ 1 ms.

## Le chemin en une phrase par étape

| # | Qui | Où | Ce qu'il fait |
|---|---|---|---|
| 1 | `PumpOS` | `NkWin32EventSystem.cpp:87` | **reçoit** : retire le message Windows de la file du thread |
| 2 | `NkWin32WndProc` → `NkWin32_ProcessMessage` | même fichier, `:122` et `:207` | **trie** : une branche par message |
| 3 | `case WM_KEYDOWN` | même fichier, `:623-646` | **traduit** : lParam → scancode USB → `NkKey`, et fabrique l'événement |
| 4 | `Enqueue` / `DeliverOnPumpThread` | `NkEventSystem.cpp:202` et `:229` | **range** : estampille, appelle les abonnés, met à jour l'état, pousse une copie en file HIGH |
| 5 | `PollEvent` | `NkEventSystem.cpp:405` | **distribue** : dépile, garde la propriété, et pompe l'OS si la file est vide |
| 6 | ma boucle | `Applications/CheminTouche/src/main.cpp` | **utilise** : `ev->As<NkKeyPressEvent>()` |
