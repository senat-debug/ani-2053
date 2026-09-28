# Chapitre 4 — Le panneau qui consomme le clic

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Le panneau est **en pensée** : je ne dessine rien, je décide juste qu'il occupe le coin haut-gauche, `x < 240` et `y < 160`, dans une fenêtre de 800 × 460. Deux gestionnaires se suivent : d'abord le panneau, ensuite le fond.

## Le journal

J'ai visé six points précis : trois dans le coin, trois juste à côté.

```text
Panneau : x < 240 et y < 160

clic 1   (  40,  30) | PANNEAU   : c'est pour moi, je CONSOMME (true)
                       | FOND      : PAS APPELE (evenement deja marque traite)

clic 2   ( 200, 140) | PANNEAU   : c'est pour moi, je CONSOMME (true)
                       | FOND      : PAS APPELE (evenement deja marque traite)

clic 3   ( 230, 150) | PANNEAU   : c'est pour moi, je CONSOMME (true)
                       | FOND      : PAS APPELE (evenement deja marque traite)

clic 4   ( 260,  60) | PANNEAU   : pas chez moi, je laisse passer (false)
                       | FOND      : appele, je traite le clic (260,60)

clic 5   (  80, 200) | PANNEAU   : pas chez moi, je laisse passer (false)
                       | FOND      : appele, je traite le clic (80,200)

clic 6   ( 500, 300) | PANNEAU   : pas chez moi, je laisse passer (false)
                       | FOND      : appele, je traite le clic (500,300)

clics recus            : 6
consommes par le panneau : 3
traites par le fond      : 3
appels du 2e gestionnaire: 3 (doit valoir le nombre de clics hors panneau)
```

**6 clics, 3 consommés, 3 appels du second gestionnaire.** Les comptes ferment : le second n'a jamais été appelé pour un clic du coin, y compris au dernier pixel du panneau (230, 150).

## Comment la consommation marche

Je n'ai rien inventé : `NkEventDispatcher` fait tout le travail, en deux endroits de son code.

```cpp
// NkEventDispatcher.h:121-138
template <typename T> bool Dispatch(NkEventHandler<T> handler) {
    ...
    if (!mEvent || mEvent->IsHandled()) {
        return false;                    // <== le 2e Dispatch sort ici
    }
    if (mEvent->GetType() != T::GetStaticType()) {
        return false;
    }
    bool consumed = handler(static_cast<T &>(*mEvent));
    if (consumed) {
        mEvent->MarkHandled();           // <== le 1er l'a marque
    }
    return consumed;
}
```

- mon gestionnaire de panneau rend **`true`** quand le clic est chez lui → `MarkHandled()` ;
- le `Dispatch` suivant voit `IsHandled()` et **sort avant d'appeler quoi que ce soit**.

C'est la réponse du QCM 4 : rendre `true`, c'est consommer l'événement, pas « demander à être rappelé ».

## Un piège que j'ai trouvé en lisant le code

J'avais d'abord pensé m'abonner avec `AddEventCallback<NkMouseButtonPressEvent>(...)`. **Ça n'aurait pas marché** : le distributeur des abonnés ne regarde jamais si l'événement est déjà traité.

```cpp
// NkEventSystem.cpp:297-302
for (nk_size i = 0; i < vecT->Size(); ++i) {
    auto &cb = (*vecT)[i];
    if (cb)
        cb(ev);          // tous les abonnes sont appeles, marque ou pas
}
```

Et le wrapper qui enveloppe mon callback **jette sa valeur de retour** (`NkEventSystem.h:342-349`) : `callback(typed);`, sans rien en faire.

Donc, dans ce moteur aujourd'hui :

| Façon de traiter | La consommation arrête-t-elle la suite ? |
|---|---|
| `NkEventDispatcher` + `Dispatch<T>` dans ma boucle | **oui** |
| `AddEventCallback<T>` (abonnement) | **non**, tous les abonnés sont appelés |

Pour un panneau qui doit manger le clic, c'est donc la boucle et le dispatcher qu'il faut, avec un **ordre** explicite : le panneau d'abord, le fond ensuite. L'ordre des deux `Dispatch` est la priorité de l'interface, et il se lit dans le code.

