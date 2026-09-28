# Chapitre 4 — Garder le pointeur d'un événement

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Je garde **exprès** le pointeur rendu par `PollEvent` d'une trame à l'autre, alors que l'en-tête l'interdit :

```cpp
// NkEventSystem.h:393-396
// CORRECTION 2 : PollEvent() retourne un pointeur valide uniquement
// jusqu'au PROCHAIN appel de PollEvent(). Ne jamais stocker ce pointeur
// entre frames — utiliser PollEventCopy() si une durée de vie propre est
// requise (ex: file de travail asynchrone, traitement différé).
```

Ma trame dure 60 ms pour que la file ait le temps de se remplir : la plupart des trames reçoivent **2 ou 3 événements**, donc `PollEvent` est bien rappelé après ma prise.

## Ce que j'ai observé

```text
trame 52  |  2 ev. | garde @00000142ff76df18 pris comme [NK_MOUSE_MOVE x=764 y=245] | il montre MAINTENANT [NK_MOUSE_MOVE x=764 y=245]
trame 53  |  1 ev. | garde @00000142ff76df18 pris comme [NK_MOUSE_MOVE x=764 y=245] | il montre MAINTENANT [NK_MOUSE_MOVE x=764 y=245]
...
trame 67  |  0 ev. | garde @00000142ff76df18 pris comme [NK_MOUSE_MOVE x=764 y=245] | il montre MAINTENANT [NK_MOUSE_MOVE x=764 y=245]
```

**Rien.** Le programme ne plante pas, les valeurs ne changent pas, et sur 100 trames le contenu lu reste identique à ce qu'il était à la prise — y compris dans les trames à 2 ou 3 événements, et y compris après avoir tapé six touches pour faire passer des événements d'un **autre type**.

C'est le pire résultat possible, et c'est le plus instructif : **le défaut ne se voit pas**.

## Pourquoi ça ne se voit pas, et pourquoi c'est faux quand même

Le mécanisme est à trois lignes du code :

```cpp
// NkEventSystem.cpp:420-427
// CORRECTION 2 : mCurrentEvent garde la propriété unique_ptr ; le pointeur
// ...
mCurrentEvent = traits::NkMove(ev);
return mCurrentEvent.Get();
```

`mCurrentEvent` est un `unique_ptr` **membre** du système d'événements. À chaque `PollEvent`, il est réassigné : l'objet précédent est **détruit et sa mémoire rendue**. Mon pointeur gardé désigne donc, dès l'appel suivant, un objet détruit.

Si la lecture marche quand même, c'est que **personne n'a encore réutilisé le bloc** : l'allocateur l'a rendu libre, mais rien ne l'a réécrit, et les octets sont toujours là. C'est un accès à de la mémoire libérée, pas une lecture légitime.

Et j'en ai la preuve, par accident. Regardez l'adresse quand je passe à la version corrigée, 50 trames plus tard :

```text
########## ON PASSE A PollEventCopy ##########

trame 110 |  3 ev. | copie @00000142ff76df18 prise comme [NK_MOUSE_MOVE x=627 y=200] | ...
```

**`00000142ff76df18`** — exactement l'adresse que je gardais tout à l'heure. Le bloc avait donc bien été libéré, et l'allocateur l'a redonné à un autre objet. Si cette réutilisation était arrivée **pendant** que je lisais mon pointeur gardé, j'aurais lu les coordonnées d'un autre événement, ou son type, ou n'importe quoi.

Autrement dit : mon programme marche par chance, et la chance dépend de l'allocateur, du moment, et de ce que fait le reste du code. C'est ce genre de défaut qui apparaît en Release, sur une autre machine, six mois plus tard.

## La correction

```cpp
while (NkEventPtr ev = NkEvents().PollEventCopy()) {
    ...
    copie = traits::NkMove(ev);   // la copie m'appartient
}
```

`PollEventCopy` fait exactement une chose de plus que `PollEvent` :

```cpp
// NkEventSystem.cpp:466-471
NkEventPtr NkEventSystem::PollEventCopy() {
    NkEvent *raw = PollEvent();
    if (!raw)
        return NkEventPtr(nullptr);
    return NkEventPtr(raw->Clone());
}
```

Elle **clone** l'événement et me rend un `unique_ptr`. Le clone ne vit plus dans `mCurrentEvent` : les `PollEvent` suivants ne peuvent plus le détruire, et il reste valide tant que je le garde.

```text
trame 110 |  3 ev. | copie @...df18 prise comme [NK_MOUSE_MOVE x=627 y=200] | elle montre MAINTENANT [NK_MOUSE_MOVE x=627 y=200]
trame 111 |  1 ev. | copie @...df18 prise comme [NK_MOUSE_MOVE x=627 y=200] | elle montre MAINTENANT [NK_MOUSE_MOVE x=627 y=200]
trame 112 |  1 ev. | copie @...df18 prise comme [NK_MOUSE_MOVE x=627 y=200] | elle montre MAINTENANT [NK_MOUSE_MOVE x=627 y=200]
```

À l'écran, le résultat est **le même** que la version défectueuse. La différence n'est pas dans ce qu'on voit, elle est dans ce qui est garanti.

