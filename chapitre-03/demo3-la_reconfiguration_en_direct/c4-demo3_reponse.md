# Chapitre 4 — Les touches changées à chaud

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-29.

## 1. Comment la configuration devient modifiable à chaud

Deux chemins, un seul point d'entrée.

**Le fichier surveillé.** Toutes les 30 trames, le programme compare la date de dernière écriture de `touches2.cfg` à celle qu'il a mémorisée :

```cpp
if ((trames % 30) == 0) {
    const ULONGLONG d = DateFichier();   // GetFileAttributesExA -> ftLastWriteTime
    if (d != 0 && d != date) { ... relire, valider, appliquer ou refuser ... }
}
```

Si la date a changé, il relit, **valide**, et n'applique que si tout est bon. Pas de redémarrage, pas de pause : la partie continue pendant le rechargement.

**Le réglage dans le programme.** `F2` demande les trois touches l'une après l'autre, puis réécrit le fichier — ce qui met à jour la date, donc le surveillant ne se déclenche pas en double.

Et `F3` cache la configuration dans le titre : c'est ce qui m'a permis de jouer à l'aveugle.

Preuve que ça marche à chaud, dans le journal :

```text
config de depart : Key:NK_LEFT / Key:NK_RIGHT / Key:NK_SPACE

trame 330    RECHARGEE : Key:NK_I / Key:NK_A / Key:NK_K
```

Le programme a changé de touches à la trame 330, sans être relancé.

## 2. L'essai à l'aveugle

**Je n'avais personne sous la main.** J'ai donc fait jouer ce rôle à un script : il tire trois touches **au hasard** parmi huit candidates, écrit le fichier, et n'affiche rien. J'ai mis la configuration en mode caché (F3) avant, et je suis parti à la découverte comme un joueur qui s'assoit devant une machine réglée par quelqu'un d'autre.

**Ce que j'ai fait** : j'ai appuyé sur chaque touche candidate 600 ms, et j'ai regardé le carré, en mesurant sa position sur une capture d'écran.

```text
  touche A : x 429 -> 651  (delta 222)
  touche E : x 651 -> 651  (delta 0)
  touche I : x 651 -> 429  (delta -222)
  touche O : x 429 -> 429  (delta 0)
  touche K : x 429 -> 429  (delta 0)
  touche M, U, P : rien
```

Deux touches trouvées en huit essais : **A = droite, I = gauche**. Les six autres ne faisaient rien — mais l'une d'elles devait être « Valider », qui ne produit rien tant qu'on n'est pas sur la cible.

**La deuxième passe** : j'ai amené le carré dans la zone verte, puis essayé les six restantes une par une :

```text
carre place en 667 (la zone va de ~630 a ~770)
  touche E : rien
  touche O : rien
  touche K -> LE SCORE MONTE : 1 pt
```

**A, I, K.** Le journal, lu après coup, confirme : `Gauche=Key:NK_I / Droite=Key:NK_A / Valider=Key:NK_K`.

**Ce que ça m'a fait** : trois choses que je n'aurais pas devinées en lisant du code.

- La première minute est **désagréable** : on appuie et rien ne se passe, et on ne sait pas si c'est la touche qui est fausse ou le programme qui est cassé.
- Les touches de déplacement se trouvent vite, parce que l'effet est **immédiat et visible**. « Valider » a été bien plus long : son effet est **conditionnel**, et tant qu'on n'est pas au bon endroit, une bonne touche ressemble exactement à une mauvaise.
- Mon instinct disait que A serait à gauche, parce qu'il est à gauche sur le clavier. C'était l'inverse. Une configuration libre autorise donc des réglages **corrects mais déroutants**.

Conclusion pratique : un jeu doit **afficher** sa configuration quelque part, et une action sans retour visible (comme « Valider ») a besoin d'un indice — un message « appuyez sur K », ou un contour qui s'allume quand on est sur la cible.

## 3. Ce qui a cassé, ou failli casser

Les trois états impossibles, essayés pour de vrai en écrivant le fichier pendant que le programme tournait :

| Config envoyée à chaud | Réponse du programme |
|---|---|
| `Gauche=Key:NK_A`, `Droite=Key:NK_A` | `REFUS : "Gauche" et "Droite" ont la meme touche (Key:NK_A)` |
| ligne `Valider` absente | `REFUS : action "Valider" sans touche` |
| `Valider=Key:NK_LSUPER` (touche Windows) | `REFUS : "Valider" utilise NK_LSUPER, reservee par le systeme` |
| config correcte ensuite | `RECHARGEE : Key:NK_O / Key:NK_P / Key:NK_M` |

Journal :

```text
trame 18870  REFUSEE : "Gauche" et "Droite" ont la meme touche (Key:NK_A) -> on garde Key:NK_I / Key:NK_A / Key:NK_K
trame 18960  REFUSEE : action "Valider" sans touche -> on garde ...
trame 19050  REFUSEE : "Valider" utilise NK_LSUPER, reservee par le systeme -> on garde ...
trame 19140  RECHARGEE : Key:NK_O / Key:NK_P / Key:NK_M

points : 1 | rechargements : 2 | refus : 3
```

**Le vrai sujet : l'état impossible.** Les trois cas ne se valent pas.

- **La touche en double** rend le jeu incohérent mais pas bloqué : le carré partirait à gauche et à droite en même temps, donc il ne bougerait plus. L'utilisateur croit son clavier cassé.
- **L'action sans touche** est pire : le jeu se lance, se joue, et **une action devient inatteignable**. Sans « Valider », on ne peut plus marquer un point ni, dans un vrai jeu, ouvrir le menu qui permettrait de corriger le réglage. C'est le cas sans issue.
- **La touche réservée** est le plus sournois : rien ne casse dans mon programme, mais la touche Windows ouvre le menu Démarrer par-dessus le jeu, et `Alt+F4` fermerait tout. Le programme reçoit bien l'événement — c'est **le système qui agit en plus**, et je ne peux pas l'en empêcher.

**Comment je les empêche** : je valide la configuration **avant** de l'appliquer, jamais après, et en cas de refus **je garde l'ancienne** plutôt que d'appliquer à moitié. Les trois règles tiennent dans une fonction de vingt lignes, `Valider()`, appelée aux deux endroits — le rechargement du fichier **et** le réglage par F2. Si elle n'était appelée qu'à un seul endroit, l'autre chemin laisserait passer l'état impossible.
