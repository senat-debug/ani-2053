# Chapitre 4 — Le rejeu

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-29.

## L'enregistrement : une minute jouée

```text
ENREGISTREMENT
ticks joues        : 3600 (60.0 s reelles)
changements ecrits : 266 lignes d'entree
x final            : 698.000
score              : 1
empreinte          : 17993787940421706839
```

Le fichier produit, `partie.rec`, fait **270 lignes** pour une minute de jeu :

```text
graine 20260929
289 1
309 0
334 1
375 0
401 4
...
3599 4
x 698.000
score 1
empreinte 17993787940421706839
```

Chaque ligne du milieu dit : « au tick 401, l'entrée devient 4 » (4 = valider). Je n'enregistre **pas** les positions, pas les images, pas les événements bruts — seulement **les changements d'entrée**, datés en ticks. D'où les 266 lignes au lieu de 3 600.

## Le rejeu : la même minute, sans toucher au clavier

```text
REJEU
ticks rejoues      : 3600 (59.4 s reelles)
x final            : 698.000 (attendu 698.000) -> IDENTIQUE
score              : 1 (attendu 1) -> IDENTIQUE
empreinte          : 17993787940421706839 (attendue 17993787940421706839) -> IDENTIQUE
```

L'**empreinte** est le point important : elle est calculée à **chaque** tick, sur la position, le score et la cible, et mélangée façon FNV. Deux parties qui finiraient au même endroit par des chemins différents auraient des empreintes différentes. Ici, les 3 600 ticks se sont déroulés exactement pareil.

## Les trois conditions qui rendent ça possible

**1. Un pas de temps fixe.** Le jeu n'avance jamais « d'un peu », il avance d'un tick de 1/60 s. Le temps réel ne sert qu'à décider **combien** de ticks exécuter :

```cpp
reste += (double)(maintenant - derniere) * kTicksParSeconde / 1000.0;
while (reste >= 1.0 && tick < kDuree) { reste -= 1.0; ...; ++tick; }
```

Avec un pas variable (`x += vitesse * dt`), le rejeu serait faux dès la première image plus lente.

**2. Un hasard reproductible.** La cible ne vient pas de `rand()` mais d'un générateur alimenté par une **graine enregistrée** dans le fichier. Au rejeu, la même graine donne les mêmes cibles.

**3. Une seule fonction qui fait avancer le jeu.** `Jeu::Avancer(masque, hasard)` ne lit ni l'horloge, ni le clavier, ni l'écran : tout ce dont elle a besoin lui est **passé en paramètre**. C'est ce qui permet de la nourrir depuis le clavier ou depuis un fichier sans changer une ligne.

Concrètement, la seule différence entre les deux modes est là :

```cpp
if (rejouer) masque = entrees[tick];
else         masque = (état du clavier lu à ce tick);
```

## Ce que ça permet pour les tests

J'ai rajouté un mode `vite` qui rejoue **sans horloge, sans dessin** :

```text
REJEU ACCELERE
ticks rejoues      : 3600 en 0 ms
x final            : 698.000 (attendu 698.000) -> IDENTIQUE
score              : 1 (attendu 1) -> IDENTIQUE
empreinte          : 17993787940421706839 (attendue 17993787940421706839) -> IDENTIQUE
```

**Une minute de jeu vérifiée en moins d'une milliseconde.** C'est ça, le vrai intérêt :

- **Un défaut se rejoue au lieu de se raconter.** Un joueur qui traverse un mur envoie son `partie.rec` de 270 lignes : le défaut se reproduit chez moi, à l'identique, autant de fois que je veux. Plus de « ça m'est arrivé une fois ».
- **Un test de non-régression pour presque rien.** Le test tient en trois lignes : rejouer le fichier, comparer l'empreinte à celle enregistrée. Aucune image à comparer, aucun pixel, aucune capture — un seul entier de 64 bits.
- **La vitesse change tout.** Mille enregistrements d'une minute se rejouent en une seconde. On peut donc les lancer à **chaque commit** : c'est exactement le genre de test que le chapitre 1 disait désactivé dans le workspace, et qui ne coûterait rien ici.
- **Ça marche sans écran.** Le mode accéléré n'appelle ni `Clear` ni `Display` : il tournerait sur le backend `Noop` (§3.7), donc sur une machine d'intégration continue sans affichage.
- **Ça détecte les défauts de déterminisme eux-mêmes.** Si un jour quelqu'un remplace le pas fixe par un `dt` variable, ou appelle `rand()` dans la logique, le rejeu **diverge** et l'empreinte le dit tout de suite. Le test protège la propriété autant que le jeu.

## Ce qui casserait le rejeu

Trois choses, et je les ai évitées dans mon code :
- lire l'horloge **dans** la logique (un `GetTickCount64()` dans `Avancer` et tout est perdu) ;
- dépendre de la cadence d'images (pas variable) ;
- une source de hasard non enregistrée — y compris l'ordre d'un conteneur trié sur des adresses mémoire.

Et une quatrième que je n'ai pas testée : le **multithread**. Si deux fils touchent l'état du jeu, l'ordre change d'une exécution à l'autre, et le rejeu ne vaut plus rien.