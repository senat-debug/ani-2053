# Chapitre 3 — Exercice 1 : la fenêtre nue

J'ai créé `Applications/FenetreNue` dans le dépôt Nkentseu, déclaré dans `Nkentseu.jenga`, et construit avec Jenga 2.8.0 (Debug, Windows, clang).

## Le programme

```cpp
#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

using namespace nkentseu;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Fenetre nue";

	NkWindow window(cfg);
	if (!window.IsOpen()) {
		return -1;
	}

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>())
				window.Close();
		}
	}
	return 0;
}
```



## Ce que j'ai retiré, et pourquoi ça tient

Le livre montre 27 lignes. J'en ai enlevé trois choses, et j'ai vérifié à chaque fois que ça construit et que ça tourne :

- **Le bloc `NKENTSEU_DEFINE_APP_DATA`** (7 lignes). Il construit un `NkAppData` avec `appName` et `appVersion`. Sans lui, les valeurs par défaut s'appliquent : `"NkApp"` et `"1.0.0"` (`Core/NkWESystem.h:30`). Le programme se construit et tourne pareil.
- **`cfg.width` et `cfg.height`** : les défauts sont déjà 1280 × 720 (§3.2.1).
- **Le `logger.Error`** avant le `return -1` : le code de retour suffit.

## La ligne que le chapitre ne donne pas

Le programme du §3.1 a une boucle vide, avec un commentaire qui renvoie au chapitre 4. Tel quel, **il ne se termine jamais** : sur Windows, `WM_CLOSE` ne ferme pas la fenêtre, il met un `NkWindowCloseEvent` dans la file et bloque le traitement par défaut (`Platform/Win32/NkWin32EventSystem.cpp:253-257`, `suppressDefaultProc = true`). Si personne ne lit la file, `IsOpen()` reste vrai pour toujours.

Il a donc fallu ajouter les lignes 16 à 18. Le commentaire d'usage en tête de `Core/NkWindow.h:12-16` donne exactement ce squelette :

```cpp
while (window.IsOpen()) {
    nkentseu::NkEvents().PollEvents();
    /* render */
}
```

## Ce que ça a coûté

| Mesure | Valeur |
|---|---|
| Projets construits | 13 (NKWindow et sa chaîne) |
| Temps de construction | 51,5 s à froid, dont 3,1 s pour mon `main.cpp` |
| Taille de l'exécutable | 4 255 356 octets (4,1 Mo), en Debug |
| Exécution | fenêtre « Fenetre nue » ouverte, fermée par le bouton du système, **code de retour 0** |

18 lignes utiles, 13 projets et 4 Mo de binaire pour une fenêtre vide : c'est le prix de la portabilité dont parle le chapitre (30 453 lignes et 14 backends dans NKWindow).



`windowedapp()` au lieu de `consoleapp()` : sans ça, une console noire s'ouvre à côté de la fenêtre. Les bibliothèques Windows sont les mêmes que celles de `NKWindow.jenga:78-82` : NKWindow est une bibliothèque **statique**, donc c'est l'application qui doit les relier.