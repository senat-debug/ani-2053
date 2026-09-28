# Chapitre 4 — Clavier et manette, une seule logique

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

## La logique, écrite une seule fois

Tout le pilotage du carré tient en deux lignes, et elles ne nomment **aucun appareil** :

```cpp
if (axeH != 0.f) ++tramesQuiBougent;
x += axeH * 7.f;
```

`axeH` est un nombre entre −1 et +1, rempli par un **axe nommé** du moteur. Les cinq lignes qui suivent sont la seule partie du programme où des touches et des sticks apparaissent :

```cpp
axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_RIGHT), +1.f));
axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_LEFT),  -1.f));
axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_D),     +1.f));
axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_A),     -1.f));
axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::GamepadAxis(NkGamepadAxis::NK_GP_AXIS_LX), +1.f));
```

Cinq sources, **un seul nom**. Ajouter une manette, un pavé directionnel ou un écran tactile, c'est une ligne de plus ici, et zéro ligne ailleurs. C'est exactement ce que le QCM 10 appelle le premier bénéfice : le code des règles ne connaît plus le clavier.

Le moteur rafraîchit les axes tout seul : `RefreshAxes()` est appelé à l'intérieur de `PollEvent` (`NkEventSystem.cpp:385` et `:438`), avec le commentaire « un axe se relit une fois par tour, il ne s'attend pas ». Je n'ai donc rien à appeler dans ma boucle.

## Le débranchement met le jeu en pause

```cpp
if (auto *d = ev->As<NkGamepadDisconnectEvent>()) {
    enPause = true;
    window.SetTitle("PAUSE - manette debranchee (Entree pour reprendre)");
}
```

Et la boucle n'intègre plus rien tant que `enPause` est vrai. Le relevé :

```text
manettes vues au demarrage : 0

trame 335   manette 0 DEBRANCHEE -> PAUSE (x = 385)
trame 457   reprise demandee au clavier (x = 385)

trames                 : 562
trames ou le carre bouge : 85
trames en pause          : 122
debranchements recus     : 1
rebranchements recus     : 0
derniere source vue      : clavier
x final                  : 595
```

**x = 385 au débranchement, x = 385 à la reprise.** Entre les deux, j'ai tenu la flèche droite pendant une seconde entière, soit 122 trames : le carré n'a pas bougé d'un pixel. Le jeu n'a pas continué tout seul. L'écran passe au rouge sombre et le carré devient gris pendant la pause, puis tout repart après Entrée (x final 595).

## Ce que je n'ai pas pu vérifier

**Aucune manette n'est branchée sur cette machine** : le programme le dit lui-même, `manettes vues au demarrage : 0`, et la « dernière source vue » reste `clavier`.

Donc :

- la ligne `GamepadAxis(NK_GP_AXIS_LX)` est écrite et compile, mais **je ne l'ai jamais vue produire une valeur** ;
- le débranchement, je l'ai **simulé** : la touche X dépose un vrai `NkGamepadDisconnectEvent` dans la file avec `Enqueue_Public`, exactement comme le ferait le backend.

```cpp
NkGamepadDisconnectEvent faux(0, window.GetId());
NkEvents().Enqueue_Public(faux, window.GetId());
```

Ce que cela prouve : **mon traitement de la pause est bon**, puisqu'il réagit à l'événement réel du moteur. Ce que cela ne prouve pas : que le backend émette bien cet événement quand on arrache une vraie manette. Pour ça, il me faut une manette.

Ce que je peux dire du code : ces événements sont classés en priorité `HIGH` dans la file (`NkEventSystem.h:123-125`, `NK_GAMEPAD_CONNECT` et `NK_GAMEPAD_DISCONNECT`), donc ils ne sont jamais jetés quand la file déborde — ce qui est la moindre des choses pour un débranchement.

## Un détail de l'axe nommé

Le gestionnaire d'axe est appelé **une fois par commande et par trame**, même quand la valeur vaut 0. Comme cinq commandes portent le même nom, je reçois cinq appels par trame, et je garde celui qui pousse le plus fort :

```cpp
if ((valeur < 0 ? -valeur : valeur) > (axeH < 0 ? -axeH : axeH)) axeH = valeur;
```

Sans cette précaution, le dernier appel écraserait les autres : une commande à 0 effacerait la flèche tenue. Et je remets `axeH` à zéro **au début** de chaque trame, sinon le carré continuerait d'avancer après le relâchement — ce serait le défaut de l'exercice précédent, en pire.