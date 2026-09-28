# Chapitre 3 — Exercice 11 : deux fenêtres

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

Deux fenêtres côte à côte, A et B. J'ai cliqué 3 fois dans A, puis 2 fois dans B.

## Le routage des clics

```text
Fenetre A : id=1
Fenetre B : id=2

clic 1   recu par la fenetre A  (id=1) en 346 , 263
clic 2   recu par la fenetre A  (id=1) en 361 , 263
clic 3   recu par la fenetre A  (id=1) en 376 , 263
clic 4   recu par la fenetre B  (id=2) en 497 , 268
clic 5   recu par la fenetre B  (id=2) en 512 , 268
fermeture demandee par la fenetre B
fermeture demandee par la fenetre A

clics : 5 au total (A : 3, B : 2)
```

Et les titres, mis à jour à chaque clic :

```text
titre A : 'Fenetre A - 3 clic(s)'
titre B : 'Fenetre B - 2 clic(s)'
```

Le mécanisme tient en une ligne : **chaque événement porte l'identifiant de sa fenêtre**.

```cpp
const uint64 id = ev->GetWindowId();          // NkEvent.h:489
const char *nom = (id == A.GetId()) ? "A" : (id == B.GetId() ? "B" : "?");
```

Les coordonnées sont **relatives à la fenêtre qui a reçu le clic** : les clics 4 et 5 valent 497 et 512, dans une zone client de 518 px de large, alors que B est à 680 px du bord de l'écran. Sans l'identifiant, ces nombres ne voudraient rien dire.

La fermeture suit la même règle : j'ai fermé B, le programme a continué ; j'ai fermé A, il s'est arrêté avec le code 0. Une boucle `while (window.IsOpen())` écrite pour une seule fenêtre aurait tué le programme à la première fermeture.

## Dessiner dans les deux : ça marche

Je m'attendais à un problème, il n'y en a pas eu :

```text
cible de rendu A valide : oui
cible de rendu B valide : oui
meme renderer pour les deux : non (un par fenetre)

images dessinees : A = 613, B = 552
```

Une `NkRenderWindow` par fenêtre, chacune avec son `Clear` et son `Display`, et les deux fenêtres affichent bien leur propre couleur. Les compteurs d'images diffèrent (613 contre 552) parce que B a été fermée avant A.

## Ce qui me manquerait vraiment

Ce n'est pas la deuxième cible de rendu. C'est ce qu'il y a **dedans**.

**1. Les ressources ne se partagent pas.** Chaque fenêtre a son propre renderer — je l'ai vérifié, les deux pointeurs sont différents. Or toutes les ressources se chargent *pour un renderer donné* :

```cpp
// NKCanvas/Renderer/Resources/NkFont.h:80
bool LoadFromFile(NkIRenderer2D &renderer, const char *path);
// NKCanvas/Renderer/Resources/NkTexture.h:56
bool LoadFromFile(NkIRenderer2D &renderer, const char *path);
```

Une police chargée avec `*cibleA.GetRenderer()` appartient au contexte de A. Pour écrire le même texte dans B, il faut **la charger une seconde fois**, avec le renderer de B. Deux fois la mémoire, deux fois le temps de chargement, et deux objets à garder en vie. C'est exactement l'avertissement du §3.8 : « une texture chargée pour l'une n'est pas forcément utilisable par l'autre ».

**2. Le redimensionnement, par fenêtre.** Mon programme ne le gère pas. Il faudrait recevoir `NkWindowResizeEvent`, regarder son `GetWindowId()`, et appeler `OnResize` sur **la bonne** cible. Avec une seule fenêtre, on peut se tromper sans le voir ; avec deux, on redimensionne la mauvaise swapchain.

**3. Une vraie boucle.** La mienne dessine les deux fenêtres à chaque tour, à fond de CPU. Il faudrait au minimum ne redessiner que la fenêtre concernée, et savoir laquelle a le focus (`NkWindowFocusGainedEvent` / `NkWindowFocusLostEvent`).

**4. Un état par fenêtre.** Tout ce que j'ai écrit en dur (la couleur, le rectangle) devrait devenir une structure par fenêtre : sa cible, ses ressources, son contenu, sa taille. Sinon le code se remplit de `if (id == A.GetId())`, comme mon compteur de clics.

En résumé : le moteur me donne le routage des événements (`NkWindowId`) et une cible de rendu par fenêtre. Ce qui manque, c'est **le partage des ressources entre contextes** — et c'est une contrainte de la carte graphique, pas un oubli du moteur.