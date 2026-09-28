# Chapitre 4 — La zone morte, mesurée

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

## Ce que le relevé donne

```text
manettes branchees      : 0
manette 0 connectee     : non
zone morte du moteur    : 0.080 (valeur par defaut)

duree du releve : 10.0 s, echantillons : 0

AUCUN ECHANTILLON : pas de manette branchee sur cette machine.
```

**Je n'ai pas de manette.** Le programme a bien tourné ses dix secondes, il a échantillonné à 250 Hz, et il n'a rien eu à échantillonner. Windows non plus n'en voit aucune : le seul périphérique HID de la machine est le pavé tactile du portable.

Je ne peux donc **pas** donner la plus grande valeur absolue au repos, ni choisir une zone morte à partir d'une mesure. Inventer un nombre ici serait le contraire de l'exercice.

Ce que je peux rendre : le programme qui fait la mesure, ce que j'ai appris du moteur en l'écrivant, et la méthode de choix avec sa justification.

## Où est la valeur *brute*, et pourquoi c'est important

Le moteur applique **déjà** une zone morte avant de publier ses événements d'axe :

```cpp
// Kernel/Runtime/NKEvent/src/NKEvent/NkGamepadSystem.cpp:348-355
// Axes avec deadzone et epsilon (domaine enum)
for (uint32 a = 0; a < EVENT_AXIS_COUNT; ++a) {
    float32 v  = ApplyDeadzone(SanitizeAxisValue(cur.axes[a]));
    float32 pv = ApplyDeadzone(SanitizeAxisValue(prev.axes[a]));
    if (math::NkFabs(v - pv) > mAxisEpsilon) {
        FireAxis(i, static_cast<NkGamepadAxis>(a), v, pv);
    }
}
```

Donc un programme qui écoute `NkGamepadAxisEvent` ne verra **jamais** le bruit de repos : il est déjà coupé. Pour mesurer, il faut la source avant filtre, et c'est `GetSnapshot(0).axes[a]` — le tableau brut du backend. C'est ce que lit mon programme.

Deux valeurs que j'ai relevées dans le code :

| Réglage | Valeur par défaut | Où |
|---|---|---|
| Zone morte | **0,08** (8 %) | `NkGamepadSystem.h:606`, modifiable par `SetDeadzone()` (ligne 443) |
| Epsilon d'axe | seuil de changement avant d'émettre un événement | `mAxisEpsilon`, ligne 472 |

## Un défaut que j'ai vu dans le filtre

```cpp
// NkGamepadSystem.h:581-589
float32 ApplyDeadzone(float32 value) const noexcept {
    if (!math::NkIsFinite(value)) return 0.f;
    if (value > mDeadzone)  return value;
    if (value < -mDeadzone) return value;
    return 0.f;
}
```

La zone morte **coupe sans remettre à l'échelle**. Conséquence : quand le joueur pousse doucement le stick et franchit 0,080, la valeur saute de **0 à 0,080** d'un coup. Le personnage démarre par un à-coup au lieu de partir doucement.

La correction habituelle est de réétirer ce qui reste :

```text
sortie = signe(v) * (|v| - zone) / (1 - zone)
```

Ainsi la sortie repart de 0 juste après le seuil et atteint quand même 1 à fond de course. Ce n'est pas ce que fait le moteur aujourd'hui.

