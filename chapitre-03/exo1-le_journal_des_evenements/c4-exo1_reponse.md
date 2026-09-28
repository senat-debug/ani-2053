# Chapitre 4 — Le journal des événements

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Une fenêtre, `dropEnabled = true`, et chaque événement reçu écrit dans un fichier avec l'heure, son type, ses familles et un détail. Séance de 13,1 s : 3 s d'attente, 2 s de souris, 2 s de frappe, deux redimensionnements, une tentative de dépôt.

## À quoi ressemble le journal

```text
ms         | type                         | familles               | detail
0          | NK_WINDOW_CREATE             | WINDOW                 |
0          | NK_WINDOW_FOCUS_GAINED       | WINDOW                 |
0          | NK_WINDOW_SHOWN              | WINDOW                 |
0          | NK_WINDOW_RESTORE            | WINDOW                 |
0          | NK_WINDOW_PAINT              | WINDOW                 |
0          | NK_WINDOW_RESIZE             | WINDOW                 | 818 x 412
3047       | NK_MOUSE_ENTER               | INPUT|MOUSE            |
3047       | NK_MOUSE_MOVE                | INPUT|MOUSE            | x=778 y=410
3063       | NK_MOUSE_MOVE                | INPUT|MOUSE            | x=797 y=403
6188       | NK_KEY_PRESSED               | INPUT|KEYBOARD         | touche=55 ctrl=0
6188       | NK_TEXT_INPUT                | INPUT|KEYBOARD         | codepoint=97
6188       | NK_KEY_RELEASED              | INPUT|KEYBOARD         |
9454       | NK_WINDOW_RESIZE             | WINDOW                 | 857 x 428
10079      | NK_WINDOW_RESIZE             | WINDOW                 | 1007 x 478
```

La famille n'est pas le type : elle est un **masque de bits**, donc un événement peut appartenir à plusieurs familles à la fois. Une frappe est `INPUT|KEYBOARD`, un mouvement est `INPUT|MOUSE`, un redimensionnement est seulement `WINDOW`. Le filtrage se fait sur ce masque (`NkEvent.h:79-93`).

## Les comptes

```text
duree de la seance : 13.1 s
evenements recus   : 179
moyenne            : 13.6 par seconde

par seconde :
  seconde 0  : 9     <- ouverture de la fenetre
  seconde 1  : 0
  seconde 2  : 0
  seconde 3  : 47    <- souris
  seconde 4  : 48    <- souris
  seconde 5  : 0
  seconde 6  : 27    <- frappe
  seconde 7  : 30    <- frappe
  seconde 8  : 6
  seconde 9  : 4     <- redimensionnement
  seconde 10 : 4
  seconde 13 : 4     <- fermeture
  pointe : 48 evenements en une seconde
```

**Une seconde d'usage normal, c'est de 0 à environ 50 événements.** Zéro quand on ne touche à rien : le moteur n'invente rien, aucun événement n'arrive tout seul. Une cinquantaine quand la souris bouge sans arrêt.

## La répartition par type

| Type | Nombre | Part |
|---|---:|---:|
| `NK_MOUSE_MOVE` | 88 | **49 %** |
| `NK_KEY_PRESSED` | 21 | 12 % |
| `NK_KEY_RELEASED` | 21 | 12 % |
| `NK_TEXT_INPUT` | 21 | 12 % |
| `NK_MOUSE_ENTER` / `NK_MOUSE_LEAVE` | 5 / 4 | 5 % |
| `NK_WINDOW_*` (create, paint, resize, move, focus, restore, shown, close, destroy) | 19 | 10 % |

Trois choses que ce tableau m'apprend :

**La souris écrase tout.** À elle seule, elle fait la moitié des événements. C'est pour ça que `NK_MOUSE_MOVE` est classé en priorité `NORMAL` dans la file, donc jetable si elle déborde, alors que les clics et les frappes sont `HIGH` (`NkEventSystem.h:95-130`).

**Une touche, c'est trois événements.** 21 `KEY_PRESSED`, 21 `TEXT_INPUT`, 21 `KEY_RELEASED` : exactement trois par frappe, et jamais un de plus. Ce n'est pas une répétition : `KEY_PRESSED` dit *quelle touche*, `TEXT_INPUT` dit *quel caractère* (le codepoint 97 = « a »). Les deux sont nécessaires, et un programme qui confond les deux se trompe dès qu'on change de disposition clavier.

**Le système coalesce les mouvements.** Mon script a envoyé environ 125 positions de souris en 2 s, et le programme n'a reçu que **88** mouvements. Windows fusionne les déplacements trop rapprochés : inutile d'espérer un événement par micro-déplacement.

