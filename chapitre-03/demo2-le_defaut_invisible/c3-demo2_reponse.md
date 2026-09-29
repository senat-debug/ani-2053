# Chapitre 3 — Démo 2 : le défaut invisible

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Mon écran est à 125 %. Le 2026-09-29.

## Le programme : une interface, deux façons de la dessiner

Une barre en haut, trois boutons ancrés à droite, un panneau en bas à droite, deux lignes de texte. Les tailles sont écrites une seule fois, en unités de design :

```cpp
static const float kBarre   = 40.f;
static const float kBouton  = 90.f;
static const float kMarge   = 12.f;
static const float kPanneau = 220.f;
static const unsigned kTexte = 16;
```

Toute la différence entre la version fausse et la version corrigée tient à **une variable** :

```cpp
// combien de pixels vaut une unite de design ?
const float s = corrige ? densite : 1.f;
```

- **version fausse** : `s = 1`, les constantes sont prises pour des pixels ;
- **version corrigée** : `s = densité`, les constantes sont converties en pixels au moment de dessiner.

## Comment j'ai simulé la machine à forte densité

Je n'ai qu'un écran. Un écran dense, c'est **le même panneau physique avec plus de pixels** : je simule donc une fenêtre qui a `densite` fois plus de pixels pour la même surface apparente.

```cpp
const float simul = (densiteSimulee > 0.f) ? densiteSimulee : 1.f;
cfg.width  = (uint32)(760 * simul);
cfg.height = (uint32)(420 * simul);
```

```bash
DefautInvisible.exe                     # ma machine
DefautInvisible.exe densite 2           # ecran dense, code inchange
DefautInvisible.exe densite 2 corrige   # ecran dense, code corrige
```

C'est une simulation, et je le dis : les pixels sont vrais, la densité physique ne l'est pas. Mais le défaut reproduit est exactement celui du chapitre, parce que le programme se trompe sur **le nombre de pixels que vaut une unité de dessin**.

## Les trois captures

| # | Commande | Fenêtre | Ce qu'on voit |
|---|---|---|---|
| 1 | `DefautInvisible.exe` | cible **758 × 412** | l'interface est juste : la barre occupe le dixième de la hauteur, les boutons se lisent, le panneau est proportionné |
| 2 | `densite 2` | cible **1518 × 832** | **le défaut** : la barre est une bande fine, les boutons sont minuscules dans un coin, le texte est illisible, le panneau flotte, perdu |
| 3 | `densite 2 corrige` | cible **1518 × 832** | tout est revenu à sa taille : mêmes proportions qu'en 1, sur deux fois plus de pixels |

Les titres des fenêtres, relevés au moment des captures :

```text
FAUSSE   | densite 1.25 | cible 758x412
FAUSSE   | densite 2.00 | cible 1518x832
CORRIGEE | densite 2.00 | cible 1518x832
```

**Le point important** : la capture 2 tourne avec **exactement le même code** que la capture 1. Rien n'a changé dans le programme, seul l'écran a changé. C'est bien un défaut qu'on ne peut pas voir sur sa propre machine.

## La correction : changer l'endroit où la taille est demandée

Version fausse — la taille est prise là où elle a été **écrite**, c'est-à-dire dans la configuration, en unités de design :

```cpp
NkRectangleShape barre({L, kBarre});        // kBarre est un nombre de "points"
barre.SetPosition({0.f, 0.f});
```

Version corrigée — la taille est demandée là où elle est **vraie**, c'est-à-dire à la cible de rendu, et l'unité de design est convertie :

```cpp
const float L = (float)target.GetSize().x;  // pixels reels
const float H = (float)target.GetSize().y;
NkRectangleShape barre({L, kBarre * s});    // s = densite
```

Trois lignes changent, et elles disent toutes la même chose : **ne jamais dessiner avec un nombre écrit dans le code sans lui donner son unité**.

C'est la règle du §3.3.2, mot pour mot : « pour dessiner, demandez la taille à la cible de rendu, pas à la fenêtre. La cible connaît les pixels réels. »

## l'exercice 4 avait déjà montré, et qui complète celui-ci

À l'exercice 4, j'avais mesuré que `NkWindow::GetSize()` et `NkRenderWindow::GetSize()` rendent **le même nombre** dans ce moteur : la cible ne fait que retransmettre la question (`NkRenderWindow.cpp:225-227`), et la fenêtre répond déjà en pixels physiques parce que le processus est déclaré conscient du DPI (`NkWin32Window.cpp:70-77`).

Le piège n'est donc pas « avoir demandé au mauvais objet » — les deux répondent pareil ici. Le piège est **plus bas** : c'est d'avoir écrit `40` dans le code en croyant que c'était 40 pixels partout. Sur un écran deux fois plus dense, ces 40 pixels font deux fois moins de millimètres, et l'interface rétrécit sans que rien ne plante, sans message d'erreur, sans que l'auteur puisse le voir.
