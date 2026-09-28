# Chapitre 4 — Trois façons d'avancer un carré

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

Trois carrés, un seul geste : 5 appuis brefs sur la flèche droite, puis un maintien (20 enfoncements sans relâchement), puis 3 appuis sur **D**.

## Les chiffres

```text
trames dessinees : 563

methode                | pas faits  | x final
A. evenements          | 25         | 170
B. etat (par trame)    | 95         | 590
C. action nommee       | 9          | 74
```

Le même pas de 6 pixels, la même séquence de touches, et **trois distances différentes**.

## A. Par événements

```cpp
if (auto *k = ev->As<NkKeyPressEvent>()) {
    if (k->GetKey() == NkKey::NK_RIGHT) { xEvenement += kPas; ++pasEvenement; }
} else if (auto *r = ev->As<NkKeyRepeatEvent>()) {
    if (r->GetKey() == NkKey::NK_RIGHT) { xEvenement += kPas; ++pasEvenement; }
}
```

**25 pas** = 5 appuis + 20 pendant le maintien. Le carré avance au **rythme de la répétition du clavier**, réglé par le système d'exploitation : le délai avant la première répétition et la cadence ne viennent pas de mon programme.

Ce code connaît : la touche `NK_RIGHT`, et le fait qu'il existe deux événements différents, l'appui et la répétition. Oublier `NkKeyRepeatEvent` fait avancer le carré **d'un seul pas** même si on reste appuyé une minute.

## B. Par interrogation d'état

```cpp
if (NkEvents().GetInputState().GetKeyboard().IsKeyPressed(NkKey::NK_RIGHT)) {
    xEtat += kPas;
    ++pasEtat;
}
```

**95 pas** pour le même geste : un par trame où la touche était enfoncée. Le carré avance de façon **continue et régulière**, sans à-coup, et il ne dépend plus du réglage du clavier — mais il dépend de **ma cadence d'images**. Sur une machine deux fois plus rapide, il irait deux fois plus vite : il faudrait multiplier par le temps écoulé.

Ce code connaît : la touche `NK_RIGHT`, et rien d'autre. Il n'a même pas besoin de savoir qu'il existe des événements.

## C. Par action nommée

```cpp
NkActionManager &actions = NkEvents().GetActionManager();
actions.CreateAction("AvancerADroite",
    [&](const NkString &nom, const NkInputCode &code, bool enfoncee, bool repetition) {
        if (enfoncee) { xAction += kPas; ++pasAction; }
    });
actions.AddCommand(NkActionCommand("AvancerADroite", NkInputCode::Key(NkKey::NK_RIGHT)));
actions.AddCommand(NkActionCommand("AvancerADroite", NkInputCode::Key(NkKey::NK_D)));
```

**9 pas** = 5 appuis sur la flèche + 1 pour le maintien + **3 appuis sur D**. Le carré vert a bougé sur une touche que le gestionnaire ne nomme jamais.

Ce code connaît : **rien du clavier**. Le corps de l'action ne parle que de « AvancerADroite ». Les deux lignes `AddCommand` sont la seule partie qui nomme des touches, et elles pourraient venir d'un fichier de configuration (QCM 11), ou lier en plus un bouton de manette : le gestionnaire, lui, ne changerait pas d'une ligne.

C'est ce que le chapitre appelle le premier bénéfice : le code des règles ne connaît plus le clavier (QCM 10).

## Ce que chacun connaît, en un tableau

| | A. événements | B. état | C. action nommée |
|---|---|---|---|
| Connaît la touche | oui | oui | **non** (seulement le nom) |
| Connaît le rythme | celui du système | celui de ma boucle | celui du système |
| Un appui = | 1 pas, puis N répétitions | 1 pas par trame | **1 pas**, sans répétition |
| Plusieurs sources | il faut un `if` par touche | il faut un `if` par touche | une ligne `AddCommand` par source |
| Manette, tactile | il faut d'autres événements | il faut d'autres états | **rien à changer** |

## Un détail que j'ai trouvé dans le moteur

L'action nommée n'a compté qu'**un** pas pour tout le maintien, alors que les événements en comptaient 20. La cause est dans le branchement automatique des actions :

```cpp
// Kernel/Runtime/NKWindow/src/NKWindow/Core/NkWESystem.cpp:142-145
mEventSystem.AddEventCallback<NkKeyPressEvent>([&actions](NkKeyPressEvent *e) {
    actions.TriggerAction(NkInputCode::Key(e->GetKey()), true);
});
mEventSystem.AddEventCallback<NkKeyReleaseEvent>([&actions](NkKeyReleaseEvent *e) {
    actions.TriggerAction(NkInputCode::Key(e->GetKey()), false);
});
```

Les actions sont alimentées par **l'appui et le relâchement**, jamais par `NkKeyRepeatEvent`. Le paramètre `repetition` du gestionnaire existe pourtant dans la signature — il vaut donc toujours `false` sur cette plateforme.

