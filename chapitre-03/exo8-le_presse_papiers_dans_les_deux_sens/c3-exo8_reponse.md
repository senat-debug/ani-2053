# Chapitre 3 — Exercice 8 : le presse-papiers, dans les deux sens

Windows 11, backend Win32, Jenga 2.8.0, Debug. Le 2026-09-28.

Tout passe par `NkWindow` : `SetClipboardText` / `GetClipboardText` (`NkWindow.h:156-157`) et `NkClipboardImage` / `Set` / `Get` / `HasClipboardImage` (`NkWindow.h:71-79, 237-243`). Aucun appel à `OpenClipboard` ou à `SetClipboardData` dans mon code.

## Moitié 1 — le texte : ça marche

Avant de lancer, j'ai mis dans le presse-papiers, depuis un autre programme :

```text
Bonjour Nkentseu, chapitre 3.
```

Sortie du programme :

```text
== TEXTE ==
avant  (29 octets) : "Bonjour Nkentseu, chapitre 3."
ecrit  (29 octets) : "BONJOUR NKENTSEU, CHAPITRE 3."
relu   (29 octets) : "BONJOUR NKENTSEU, CHAPITRE 3."
aller-retour identique : oui
```

Et ce que l'autre programme trouve après :

```text
BONJOUR NKENTSEU, CHAPITRE 3.
```

Même longueur avant et après (29 octets), chiffres et ponctuation intacts.

Je ne mets en majuscules que les lettres **ASCII**. Le presse-papiers rend de l'UTF-8 : un « é » y occupe deux octets, et les passer un par un à une table de majuscules casserait le caractère. Avec un texte accentué, mon programme laisserait donc les accents en minuscules — c'est un choix, pas un oubli.

## Moitié 2 — l'image : ça marche dans le moteur, pas en croisant

J'ai mis dans le presse-papiers une image de test **8 × 6** fabriquée par un autre programme, où chaque pixel encode sa position : `R = x × 30`, `V = y × 40`, `B = 7` partout.

```text
== IMAGE ==
HasClipboardImage() : oui
lue    : 8 x 6, 192 octets, 32 bits par pixel
premier pixel lu   : R=150 V=40 B=7 A=255
premier pixel ecrit: R=105 V=215 B=248 A=255
SetClipboardImage() : oui
relue  : 8 x 6, 192 octets
premier pixel relu : R=105 V=215 B=248 A=255
octets differents apres aller-retour : 0 sur 192
```

Ce qui marche, et que je peux vérifier :

- **Les dimensions sont justes** : 8 × 6, comme l'image posée.
- **192 octets pour 48 pixels = 32 bits par pixel**, conforme à `NkClipboardImage` (`pixels.Size() == width * height * 4`).
- **L'inversion ne touche pas l'alpha** : A = 255 avant, A = 255 après. 105 = 255 − 150, 215 = 255 − 40, 248 = 255 − 7 : les trois composantes de couleur sont bien inversées.
- **L'aller-retour par le moteur est exact** : 0 octet différent sur 192. Relancé une seconde fois sur sa propre image, le programme relit exactement ce qu'il avait écrit (`premier pixel lu` du 2ᵉ passage = `premier pixel ecrit` du 1ᵉʳ).

**Ce qui ne marche pas** : les pixels ne sont pas au bon endroit quand l'image vient d'un autre programme. J'ai relu le résultat avec le programme qui avait posé l'image, en déduisant de chaque couleur la position d'origine :

```text
  y=0 : (5,1) (6,1) (7,1) (0,0) (1,0) (2,0) (3,0) (4,0)
  y=1 : (5,2) (6,2) (7,2) (0,1) (1,1) (2,1) (3,1) (4,1)
  y=2 : (5,3) (6,3) (7,3) (0,2) (1,2) (2,2) (3,2) (4,2)
  y=3 : (5,4) (6,4) (7,4) (0,3) (1,3) (2,3) (3,3) (4,3)
  y=4 : (5,5) (6,5) (7,5) (0,4) (1,4) (2,4) (3,4) (4,4)
```

L'image revient **décalée de 3 pixels** (12 octets) : chaque ligne glisse, et ce qui déborde passe sur la ligne précédente. Les couleurs sont justes, la taille est juste, c'est la position qui ne l'est pas. Sur une photo, ça se verrait comme une bande décalée sur le côté.

## Où je me suis arrêté

Le décalage n'est **pas** dans mon inversion : elle travaille pixel par pixel sur place, sans décaler quoi que ce soit, et l'aller-retour moteur → moteur est exact au bit près.

Je suis allé lire le décodeur du backend, `NkDecodeDibToRgba` (`Platform/Win32/NkWin32Window.cpp:1190-1250`). Il gère bien le cas qui décale de 12 octets, c'est-à-dire les trois masques qui suivent un `BITMAPINFOHEADER` en `BI_BITFIELDS` :

```cpp
// NkWin32Window.cpp:1212-1227
SIZE_T pixelOffset = hdrSize;
if (comp == BI_BITFIELDS) {
    if (hdrSize >= 108) { ... }   // V4/V5 : masques dans l'en-tête
    else {
        if (dibSize < hdrSize + 12) return false;
        ...
        pixelOffset += 12;        // 3 DWORD de masques après l'en-tête
    }
}
```

12 octets, c'est exactement mon décalage de 3 pixels en 32 bpp. Le compte est donc à un `+12` près, dans un sens ou dans l'autre, selon la variante de DIB (`CF_DIBV5` ou `CF_DIB`) que le programme source a posée. Je n'ai pas réussi à désigner la ligne fautive avec certitude : il faudrait afficher l'en-tête reçu (`biSize`, `biCompression`, `biBitCount`) pour savoir dans quelle branche on passe, et je n'ai pas fait cette mesure.

**Résumé honnête** : la moitié texte est complète et vérifiée des deux côtés ; la moitié image lit, inverse et réécrit correctement — taille, bits par pixel, alpha préservé, aller-retour interne exact — mais l'échange avec un autre programme décale l'image de 3 pixels, et je m'arrête à la mesure de ce décalage.