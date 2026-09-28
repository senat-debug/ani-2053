# Chapitre 4 — Les touches choisies par le joueur

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

Trois actions : **Gauche**, **Droite**, **Valider**. Le jeu : amener le carré sur la zone verte, puis valider.

## Le fichier

Au premier lancement, il n'existe pas, et le programme l'écrit :

```text
# touches.cfg — une action par ligne : Action=Appareil:Entree
Gauche=Key:NK_LEFT
Droite=Key:NK_RIGHT
Valider=Key:NK_SPACE
```

Je n'ai inventé aucun format : c'est celui du moteur. `NkInputCode::ToString()` écrit « appareil:entrée », et `FromString()` le relit :

```cpp
// NkEventDispatcher.h:465-474
// La forme est « appareil:entree », par exemple :
//   Key:SPACE      Key:LEFT       Key:NK_ESCAPE
//   Mouse:LEFT     Wheel:V        Wheel:H
//   Gamepad:SOUTH  GamepadAxis:LEFT_X
// Sans les deux points, on suppose une touche : « SPACE » vaut « Key:SPACE ».
```

Deux conséquences que j'aime bien : le joueur peut **éditer le fichier à la main**, et le jour où je veux lier une action à un bouton de manette, il n'y a **rien à changer** dans mon chargeur — `Gamepad:SOUTH` se relit tout seul.

## Changer les touches dans le programme

`F2` lance le réglage : le titre demande chaque action l'une après l'autre, et la touche suivante est prise comme réponse. À la fin, le fichier est réécrit.

```text
titre pendant le reglage : Appuyez sur la touche pour : Gauche
titre apres reglage      : 0 point(s) | Gauche=Key:NK_Q  Droite=Key:NK_D  Valider=Key:NK_ENTER | F2 pour changer
```

Le journal du programme :

```text
Gauche   reglee sur Key:NK_Q
Droite   reglee sur Key:NK_D
Valider  reglee sur Key:NK_ENTER
config ecrite dans touches.cfg
```

Et le fichier sur le disque, après :

```text
Gauche=Key:NK_Q
Droite=Key:NK_D
Valider=Key:NK_ENTER
```

## La vérification : une partie entière avec ses touches

J'ai **fermé** le programme et je l'ai relancé. Au démarrage, sans rien faire :

```text
3 action(s) relue(s) depuis touches.cfg
  Gauche   -> Key:NK_Q
  Droite   -> Key:NK_D
  Valider  -> Key:NK_ENTER

titre au demarrage : 0 point(s) | Gauche=Key:NK_Q  Droite=Key:NK_D  Valider=Key:NK_ENTER | F2 pour changer
```

Puis j'ai joué : **D** pour aller à droite, **Entrée** pour valider.

```text
trame 492   point 1 (valide avec Key:NK_ENTER)
```

Le titre est passé à « 1 point(s) ». Ensuite j'ai appuyé sur **la flèche droite et sur l'espace**, les touches d'origine : le score n'a pas bougé, et le carré non plus. Les anciennes touches ne sont plus branchées sur rien.

## Ce que le code du jeu connaît des touches

Rien de figé. Les seules lignes qui nomment une touche sont les **valeurs par défaut**, utilisées uniquement quand le fichier n'existe pas :

```cpp
c.codes[0] = NkInputCode::Key(NkKey::NK_LEFT);
c.codes[1] = NkInputCode::Key(NkKey::NK_RIGHT);
c.codes[2] = NkInputCode::Key(NkKey::NK_SPACE);
```

Partout ailleurs, le jeu compare au code **lu dans la configuration** :

```cpp
if ((uint32)k->GetKey() == conf.codes[2].code) { /* valider */ }
...
if (clavier.IsKeyPressed((NkKey)conf.codes[1].code)) x += 6.f;
```

C'est le QCM 11 en pratique : ce qui varie d'un utilisateur à l'autre vit dans un fichier, pas dans le code.

