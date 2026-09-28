# Chapitre 4 — Le saut, l'état nu et la détection de front

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

Trois personnages, la même physique, la même touche : 3 appuis brefs sur l'espace, puis un maintien d'environ 1,5 s.

## Les chiffres

```text
trames                      : 575
trames avec espace enfonce  : 105

methode                    | sauts      | trames en l'air
A. etat nu                 | 105        | 302
B. etat + front            | 4          | 156
C. evenement               | 4          | 156
```

**105 sauts pour 4 appuis.** Le compte n'a rien de mystérieux : c'est exactement le nombre de trames pendant lesquelles la touche était enfoncée.

## A. L'état nu, et le défaut qu'il produit

```cpp
if (espace) nu.Sauter();
```

Une ligne, et elle est fausse. `IsKeyPressed` répond « la touche est enfoncée **maintenant** » : tant que le doigt reste dessus, elle répond oui à **chaque trame**. Mon personnage reçoit donc une impulsion 105 fois, dont une centaine en plein vol.

À l'écran, ça ne ressemble pas à un saut : le carré rouge **monte en continu** tant que l'espace est tenu, et il reste en l'air presque deux fois plus longtemps que les autres (302 trames contre 156). C'est le saut infini, le défaut classique des premiers jeux qu'on écrit.

Un appui bref de 70 ms suffit déjà à déclencher 4 ou 5 sauts, parce qu'à 60 images par seconde, 70 ms font 4 trames.

## B. La détection de front

```cpp
if (espace && !espaceAvant) front.Sauter();
espaceAvant = espace;
```

Deux lignes, une variable. Je ne saute plus quand la touche **est** enfoncée, mais quand elle **vient de l'être** : le passage de relâché à enfoncé, le front montant.

**4 sauts pour 4 appuis.** Le maintien de 1,5 s ne compte que pour un seul : la touche n'est passée qu'une fois de relâché à enfoncé.

La variable `espaceAvant` est le prix à payer : elle mémorise l'état de la trame précédente. C'est un état à maintenir, à réinitialiser quand la fenêtre perd le focus, et à dupliquer pour chaque touche qui en a besoin.

## C. Par événement

```cpp
if (auto *k = ev->As<NkKeyPressEvent>()) {
    if (k->GetKey() == NkKey::NK_SPACE) evenement.Sauter();
}
// NkKeyRepeatEvent volontairement ignore : une pression = un saut.
```

**4 sauts, exactement comme B**, et 156 trames en l'air : les deux carrés bougent de façon identique, image par image.

La différence est dans le code : **je n'ai aucune variable à tenir**. Le front, c'est l'événement lui-même. `NkKeyPressEvent` n'existe qu'au moment de l'appui ; il ne se répète pas tout seul. La seule précaution est de ne **pas** traiter `NkKeyRepeatEvent` : si je l'avais ajouté, le maintien aurait redonné des sauts, comme à l'exercice précédent où les répétitions faisaient avancer le carré.

## Comparaison

| | A. état nu | B. état + front | C. événement |
|---|---|---|---|
| Sauts pour 4 appuis | **105** | 4 | 4 |
| État à mémoriser | aucun | `espaceAvant` | aucun |
| Lignes de code | 1 | 2 + une variable | 3 |
| Risque d'oubli | saut infini | oublier de mettre à jour `espaceAvant` | traiter la répétition par erreur |
| Dépend de la cadence d'images | **oui** | non | non |

