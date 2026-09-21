# Exercice 3 — Juger trois messages de commit

> **Consigne** : prendre trois commits du dépôt du moteur et juger leurs messages. Dit-il ce qu'il fait ? Pourquoi ? Porte-t-il un seul sujet ? Récrire le plus faible.
> **Fait le** 2026-09-21, sur le dépôt Nkentseu (`origin/main`, 1 843 commits).

J'ai choisi trois commits très différents, pour comparer. Pour chacun, j'ai lu le message avec `git show --stat <commit>` et j'ai vérifié dans le diff si le message disait vrai. Je les juge avec les règles du §2.5.2 : un sujet court à l'impératif, une ligne vide, un corps qui dit pourquoi, un seul sujet par commit.

## Commit 1 — `3b79729b` (27 août 2026)

```text
NKRenderer : Present() avant EndFrame() sur les 3 sites inverses, et le commentaire qui enseignait l inverse
```

Suivi d'un corps de 30 lignes : les 3 fichiers concernés, pourquoi l'ordre était faux, comment Vulkan perd la frame sans erreur, et ce qui a été mesuré ou non.

- **Ce qu'il fait** : oui. On sait qu'il remet `Present()` avant `EndFrame()` à 3 endroits, et qu'il corrige un commentaire. Le diff le confirme : 3 fichiers, 17 lignes ajoutées.
- **Pourquoi** : oui, très bien. Le corps explique la cause, et même pourquoi le défaut était resté caché (OpenGL et DX11 ne le montrent pas).
- **Un seul sujet** : oui. Le commentaire corrigé enseignait justement le mauvais ordre, c'est donc le même sujet.
- **Défaut** : le sujet fait 108 caractères. C'est trop long pour une ligne de sujet.

## Commit 2 — `9b31cc8f` (23 août 2026)

```text
Fusion : ConquerorLab jouable pour le stagiaire, et NK3DModeler lineaire
```

C'est un commit de fusion (2 parents) qui fait entrer 19 commits de deux chantiers.

- **Ce qu'il fait** : oui, en gros. Le corps liste ce qui arrive, chantier par chantier.
- **Pourquoi** : à moitié. Pour ConquerorLab, oui : « le kit du stagiaire était bloqué ». Pour NK3DModeler, il y a des chiffres (31 322 ms → 77,7 ms), mais pas la raison de faire ce travail maintenant.
- **Un seul sujet** : non. Il y a **deux** sujets sans rapport, et le « et » du sujet le montre. Le livre dit : si le message contient « et aussi », il faut faire deux commits. Ici, il fallait deux fusions.

## Commit 3 — `fb72e2ed` (10 mai 2026)

```text
vulkan renderer bug fix
```

Pas de corps.

- **Ce qu'il fait** : non. « bug fix » ne dit ni quel défaut ni où. Le diff ne touche en fait qu'un fichier, `shadow.vert.vk.glsl`, sur 8 lignes.
- **Pourquoi** : non, rien.
- **Un seul sujet** : oui. C'est le seul point positif.
- **En plus**, les commits `601d30cb`, `bc4c6f06` et `fb72e2ed` portent le même message. Dans `git log --oneline`, on ne peut pas les distinguer.

Le pire, c'est que le pourquoi existe. L'auteur l'a écrit en commentaire dans le shader, et pas dans le message.

## Bilan

| Commit | Ce qu'il fait ? | Pourquoi ? | Un seul sujet ? |
|---|---|---|---|
| `3b79729b` | oui | oui | oui (sujet trop long) |
| `9b31cc8f` | oui | à moitié | **non** |
| `fb72e2ed` | **non** | **non** | oui |

Le plus faible est `fb72e2ed` : il ne répond à aucune des deux premières questions.

## Ma réécriture de `fb72e2ed`

```text
Corriger les ombres Vulkan, absentes sur la moitie des pixels

La matrice lightVP suit la convention OpenGL : la profondeur qu'elle
donne va de -1 a 1. Vulkan attend une profondeur entre 0 et 1, et
coupait donc la moitie proche de chaque cascade d'ombre. L'atlas
d'ombres etait a moitie vide, et le rendu PBR ne mettait pas d'ombre
sur la moitie des pixels.

Le shader shadow.vert.vk.glsl ramene maintenant la profondeur dans
[0,1] avec z' = 0.5*z + 0.5*w. Seul le shader Vulkan change.
```

Ce que j'ai changé :
- un **sujet court à l'impératif**, qui dit quel défaut est corrigé ;
- une **ligne vide**, puis un **corps qui dit pourquoi** : la cause (deux conventions de profondeur différentes) et ce que l'utilisateur voyait (des ombres à moitié absentes) ;
- le **comment** tient en une ligne, parce que le détail est dans le diff.

## Ce que je retiens

Le même auteur a écrit « bug fix » en mai et 30 lignes d'explication en août. Un bon message ne demande pas plus de travail : ici, l'explication était déjà dans le commentaire du code, il suffisait de la mettre dans le message. C'est le message qu'on lit dans `git log`, pas le commentaire.
