# Chapitre 4 — Un seul chemin de fermeture

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

La règle que je me suis donnée : **un seul `window.Close()` dans tout le fichier**, et il est dans le traitement de `NkWindowCloseEvent`. Rien d'autre ne ferme la fenêtre.

Pour pouvoir essayer les trois déclencheurs dans une seule séance, je **refuse** les deux premières demandes et j'accepte la troisième. C'est exactement ce que fait un éditeur qui demande « voulez-vous enregistrer ? » : l'événement arrive, et c'est le programme qui décide.

## L'essai

```text
--- 1) le bouton du systeme (la croix) : WM_CLOSE ---
   fenetre encore la : True
--- 2) le raccourci du gestionnaire de fenetres : Alt+F4 ---
   fenetre encore la : True
--- quelques touches et clics qui ne doivent RIEN fermer ---
   fenetre encore la : True
--- 3) ma propre touche : Echap ---
   a quitte : True | code : 0
```

Et le journal du programme :

```text
demande 1 : NkWindowCloseEvent (forced=false) venue de systeme
            -> refusee, la fenetre reste ouverte
demande 2 : NkWindowCloseEvent (forced=false) venue de systeme
            -> refusee, la fenetre reste ouverte
touche Echap : je depose un NkWindowCloseEvent dans la file
demande 3 : NkWindowCloseEvent (forced=false) venue de Echap (depose par le programme)
            -> acceptee, Close() appele (1 fois au total)

demandes de fermeture recues : 3
appels a Close()             : 1
autres evenements recus      : 29 (aucun n'a ferme la fenetre)
```

## Les trois chemins, et pourquoi ils n'en font qu'un

| Déclencheur | Ce qui se passe | Événement reçu |
|---|---|---|
| **Bouton du système** (la croix) | Windows envoie `WM_CLOSE` à la fenêtre | `NkWindowCloseEvent` |
| **Alt+F4** | Windows le traduit en `WM_SYSCOMMAND SC_CLOSE`, qui finit en `WM_CLOSE` | `NkWindowCloseEvent` |
| **Ma touche Échap** | mon code dépose lui-même l'événement dans la file | `NkWindowCloseEvent` |

Les deux premiers se rejoignent **dans le backend**, qui n'a qu'un seul endroit pour ça :

```cpp
// Platform/Win32/NkWin32EventSystem.cpp:253-257
case WM_CLOSE: {
    NkWindowCloseEvent evt(false, winId);
    EnqueueForWindow(evt);
    suppressDefaultProc = true;
    break;
}
```

`suppressDefaultProc = true` est le point important : sans lui, `DefWindowProc` détruirait la fenêtre tout seul et mon programme n'aurait rien à décider. C'est ce qui rend le refus possible.

Le troisième, je le fabrique moi-même, et **sans une ligne de code Windows** :

```cpp
NkWindowCloseEvent demande(false, window.GetId());
NkEvents().Enqueue_Public(demande, window.GetId());
```

`Enqueue_Public` (`NkEventSystem.h:415`) est le pont prévu pour déposer un événement dans la file. Ma touche ne ferme donc rien : elle **demande**, comme le système. Si j'avais écrit `window.Close()` dans le traitement d'Échap, j'aurais eu deux chemins — et le jour où j'ajoute une confirmation d'enregistrement, j'en oublierais un.

## Ce que ça prouve

- **3 demandes, 1 fermeture** : le compteur le dit. Le programme a bien décidé, deux fois « non », une fois « oui ».
- **29 autres événements** (frappes, clics, mouvements, focus) n'ont rien fermé.
- Le programme s'est terminé par **le code 0** : la boucle `while (window.IsOpen())` s'est arrêtée parce que `Close()` a été appelé, pas parce que le système a tué la fenêtre.

