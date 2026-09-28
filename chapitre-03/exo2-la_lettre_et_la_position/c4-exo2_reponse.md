# Chapitre 4 — La lettre et la position

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Pour chaque touche, le programme affiche quatre choses : `NkKey`, `NkScancode`, le code natif du système (VK Win32) et le caractère réellement produit, qui arrive dans un événement séparé (`NkTextInputEvent`).

J'ai envoyé **six positions physiques** du clavier, pas six lettres : les scancodes PS/2 `0x10`, `0x11`, `0x1E`, `0x2C`, `0x32` et `0x02`, c'est-à-dire les touches qui, sur un clavier américain, portent Q, W, A, Z, M et 1.

## Le relevé

```text
Disposition active du thread : HKL = 0x040C040C (langue 0x040C)

NkKey (position) | NkScancode       | VK natif   | caractere produit
---------------+------------------+------------+------------------
NK_Q           | SC_Q             | 0x41 ( 65) | 'a'  (U+0061)
NK_W           | SC_W             | 0x5A ( 90) | 'z'  (U+007A)
NK_A           | SC_A             | 0x51 ( 81) | 'q'  (U+0071)
NK_Z           | SC_Z             | 0x57 ( 87) | 'w'  (U+0077)
NK_M           | SC_M             | 0xBC (188) | ','  (U+002C)
NK_NUM1        | SC_1             | 0x31 ( 49) | '&'  (U+0026)
```

`0x040C` = français. Ma machine est donc en **AZERTY**, et la démonstration est déjà là :

- j'appuie sur la touche que le moteur appelle **`NK_Q`**, et il sort un **« a »** ;
- j'appuie sur **`NK_A`**, et il sort un **« q »** ;
- j'appuie sur **`NK_NUM1`**, et il sort un **« & »**, parce qu'en AZERTY les chiffres demandent Maj.

## Ce qui suit la disposition, et ce qui ne la suit pas

| Colonne | Suit la disposition ? | Ce qu'elle désigne |
|---|---|---|
| `NkKey` | **non** | la position de la touche, nommée comme sur un clavier US |
| `NkScancode` | **non** | la même position, en code USB HID |
| VK natif | **oui** | ce que Windows croit que la touche vaut : 0x41 (« A ») pour la touche Q |
| caractère | **oui** | ce que la touche écrit vraiment : « a » |

La raison est dans le backend : `NkKey` est déduit du **scancode**, pas du code virtuel. Le code virtuel n'est qu'un secours quand le scancode est inconnu.

```cpp
// Platform/Win32/NkWin32EventSystem.cpp:627-633
UINT sc = (lp >> 16) & 0xFF;
bool isExt = (lp >> 24) & 1;
NkScancode nkSc = NkScancodeFromWin32(sc, isExt);
NkKey k = NkScancodeToKey(nkSc);
if (k == NkKey::NK_UNKNOWN)
    k = NkWin32_VkeyToNkKey((UINT)wp, isExt);
```

Et l'en-tête l'annonce : « `NkScancode` : codes physiques USB HID (indépendants du layout clavier) » (`NkKeyboardEvent.h:9`).

## Le changement de disposition : ce que j'ai fait, et ce que je n'ai pas fait

**Ce que j'ai vérifié** : les dispositions installées sur la machine.

```text
dispositions installees : 2
  HKL = 0x040C2C0C  (fr-CM)
  HKL = 0x040C040C  (fr-FR)
```

Les deux portent le **même identifiant de disposition, 0000040C** : français AZERTY. Passer de l'une à l'autre ne changerait donc **rien** au relevé — j'aurais un tableau identique et une conclusion vide.

**Ce que je n'ai pas fait** : ajouter une disposition américaine dans les réglages de Windows. C'est un réglage du système, sur la machine de quelqu'un d'autre, et je ne le touche pas. Pour l'essayer soi-même : *Paramètres → Heure et langue → Langue et région → Français → Options → Ajouter un clavier → Anglais (États-Unis)*, puis basculer avec **Win + Espace** et relancer le programme.

**Ce que je peux prédire, et pourquoi** : les deux premières colonnes seraient **identiques ligne pour ligne** — c'est le scancode, il ne dépend pas de la disposition. Les deux dernières changeraient :

| Position | AZERTY (mesuré) | US-QWERTY (prédit) |
|---|---|---|
| `NK_Q` | VK 0x41, « a » | VK 0x51, « q » |
| `NK_W` | VK 0x5A, « z » | VK 0x57, « w » |
| `NK_A` | VK 0x51, « q » | VK 0x41, « a » |
| `NK_Z` | VK 0x57, « w » | VK 0x5A, « z » |
| `NK_M` | VK 0xBC, « , » | VK 0x4D, « m » |
| `NK_NUM1` | VK 0x31, « & » | VK 0x31, « 1 » |

Autrement dit, les VK d'AZERTY et de QWERTY sont **échangés deux à deux** : c'est exactement ce que voit un jeu qui se fie au code natif, et c'est pour ça qu'il faut `NkScancode` pour les touches de déplacement (QCM 2).

## La conséquence pratique

- **Déplacement d'un personnage** : `NkScancode`. La touche reste au même endroit sous les doigts, que le joueur soit en AZERTY, QWERTY ou Dvorak.
- **Raccourci affiché dans un menu** (« Ctrl+S ») : `NkKey`, ou plutôt le caractère, parce que c'est la lettre que l'utilisateur lit sur sa touche.
- **Saisie de texte** : ni l'un ni l'autre, mais `NkTextInputEvent`, le seul qui connaisse les accents, les majuscules et les touches mortes.