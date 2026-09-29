# Chapitre 4 — Le même geste, aux trois hauteurs

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-29.

Un seul geste : la barre d'espace. Trois appuis brefs, un maintien, puis deux appuis sur **G** (l'autre source de la même action). 11,1 s en tout.

## Ce que chaque hauteur a vu

```text
1. EVENEMENT : 4 appui(s), 19 repetition(s), 4 relachement(s)
   -> des instants dates, avec la touche et les modificateurs
2. ETAT      : 775 trame(s) avec la touche enfoncee, 4 front(s) montant(s)
   -> un booleen par trame, aucune histoire, aucune repetition
3. ACTION    : 6 declenchement(s), derniere source Key:NK_G
   -> un nom, et la source n'est connue que si on la demande
```

Le même geste, trois nombres différents, et surtout **trois natures différentes** :

| | Ce qu'elle voit | Ce qu'elle ne voit pas |
|---|---|---|
| **Événement** | des instants datés, la touche exacte, les modificateurs, la différence appui / répétition / relâchement | ce qui se passe entre deux événements |
| **État** | « c'est enfoncé, maintenant », une fois par trame | le moment exact, les répétitions, l'ordre des touches |
| **Action** | un **nom** et rien d'autre ; la source est là si on la demande (`Key:NK_G`) | le maintien : 6 déclenchements pour 6 appuis, aucune répétition |

Les 6 déclenchements de l'action pour 4 appuis sur espace et 2 sur G le disent bien : **deux touches, un seul nom**, et le code du jeu n'a pas changé d'une ligne.

## Mes cinq choix

| Usage | Hauteur | Pourquoi, en une phrase |
|---|---|---|
| **Ouvrir un menu** | événement | Un menu s'ouvre une fois, au moment précis de l'appui, et surtout pas tant que la touche reste enfoncée. |
| **Déplacer un personnage** | état | Le déplacement dure, donc il se lit à chaque trame et se multiplie par le temps écoulé. |
| **Sauter** | **discutable** — action nommée (ou événement) | Un saut est un instant, mais lequel des deux dépend de ce que le jeu promet au joueur. |
| **Raccourci clavier** | événement | Il faut la touche **et** les modificateurs au moment exact (`HasCtrl()`), et c'est l'événement qui les transporte. |
| **Viser** | **discutable** — état (manette) ou événement de mouvement brut (souris) | La bonne hauteur dépend de l'appareil, pas de l'usage. |

## Les deux qui se discutent

### Sauter : action nommée ou événement ?

**Pour l'action nommée** : le saut est typiquement ce qu'un joueur veut remapper, et il doit marcher aussi au bouton A d'une manette. Une action donne les deux gratuitement, et mon relevé le montre : 6 déclenchements depuis deux touches différentes, sans un `if` de plus.

**Pour l'événement** : un jeu de plateforme sérieux a besoin de choses que l'action ne donne pas — le **moment exact** de l'appui pour le « tampon de saut » (l'appui juste avant d'atterrir doit compter), la **durée** de l'appui pour un saut plus ou moins haut, et le **relâchement** pour couper l'élan. L'action, elle, ne rend que « ça vient de partir » : j'ai mesuré qu'elle ne répète pas et qu'elle ne prévient pas du relâchement.

**Ce qui fait pencher** : si le saut est simple et doit être remappable → action. Si la hauteur dépend de la durée d'appui, ou s'il y a un tampon de saut → événement, avec l'action seulement pour savoir **quelle** touche écouter.

### Viser : état ou événement ?

**À la manette**, viser c'est un stick tenu dans une direction : c'est un **état** continu, lu à chaque trame et multiplié par le temps écoulé. Rien d'autre n'a de sens : il n'y a pas d'« événement de visée ».

**À la souris**, il n'existe pas d'état à interroger : la souris n'a pas de position « tenue », elle a des **déplacements**. Ce sont des événements de mouvement, et pour viser il faut le **déplacement brut**, sans accélération ni bornes d'écran (QCM 7).

**Ce qui fait pencher** : l'appareil. Et c'est exactement le cas où l'action nommée ne sauve pas : un axe de stick et un delta de souris n'ont ni la même unité ni la même nature, et il faut deux chemins qui se rejoignent seulement au moment d'appliquer la rotation.

*(Le raccourci clavier se discute un peu aussi : si l'utilisateur peut le redéfinir, on passe par une action. Mais comme un raccourci s'affiche dans un menu — « Ctrl+S » — il faut de toute façon la touche et ses modificateurs, donc l'événement reste dessous.)*

## Deux choix vérifiés pour de vrai

### 1. Ouvrir un menu → événement

```text
a la hauteur EVENEMENT : le menu s'ouvre 4 fois
a la hauteur ETAT      : le menu s'ouvre 775 fois
```

Pour **4 appuis**. À la hauteur état, le menu s'ouvrirait 775 fois, c'est-à-dire à chaque trame où la touche est tenue : il clignoterait, ou se rouvrirait indéfiniment. Le choix n'est pas une question de goût, c'est un facteur **190**.

### 2. Déplacer un personnage → état

```text
a la hauteur EVENEMENT : x = 138 (par a-coups, au rythme du systeme)
a la hauteur ETAT      : x = 4650 (regulier, au rythme de la boucle)
```

À la hauteur événement, le personnage avance au **rythme de répétition du clavier** : un pas, un délai d'une demi-seconde, puis des pas saccadés que le joueur n'a pas choisis. Les 19 répétitions mesurées, c'est Windows qui les décide, pas moi.

À la hauteur état, il avance à chaque trame : 775 pas réguliers, sans délai au départ. C'est fluide — à condition de multiplier par le temps écoulé, sinon la vitesse dépend de la machine.