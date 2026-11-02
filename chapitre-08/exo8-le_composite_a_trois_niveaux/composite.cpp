Le premier pont :
NKCanvas/UI/NkGuiCanvasBackend.h, ligne 26 : class NkGuiCanvasBackend (kit
Canvas du 22 octobre 2026, header-only, espace nkentseu::renderer).

Ce qu'il reçoit :
- à l'initialisation, un NkIRenderer2D* (Init, l. 38) : il ne connaît ni la
  fenêtre ni l'API graphique, seulement l'interface du moteur 2D ;
- les pixels des textures, sous un identifiant texId : un atlas de police en
  niveaux de gris (UploadFontGray8, l. 45) et des images RGBA (UploadImageRGBA,
  l. 98) ;
- à chaque image, une const nkgui::NkGuiDrawList& et la taille du tampon
  fbW x fbH (Submit, l. 213). De la liste il ne lit que trois tableaux : vtx
  (pos, uv, col empaquetée a<<24|b<<16|g<<8|r), idx, et cmds (type,
  idxOffset, idxCount, texId, clipRect, blend). Les fenêtres de l'interface,
  les piles clipStack/blendStack et thickScale ne le concernent pas : NKGui
  les a déjà résolues en commandes.

Ses méthodes publiques : le destructeur (libère les NkTexture), Init,
UploadFontGray8, UploadImageRGBA, Submit, et Releve() (l. 208), statique, qui
ne sert qu'à l'instrumentation NK_PHASES et ne fait pas partie du pont.

Ce qu'il fait :
- UploadFontGray8 étend le gris en RGBA (blanc + alpha = couverture), puis
  NkTexture::Create / SetFilter(NK_LINEAR) / Update ; il recrée la texture si
  la taille change. UploadImageRGBA fait de même sans conversion.
- Submit convertit une fois tous les sommets NkGuiVertex en NkVertex2D
  (dépaquetage de la couleur en r, g, b, a), puis, pour chaque commande non
  vide, dans l'ordre :
    1. si clipRect n'est pas l'infini (< 1e8), le borne au tampon, saute la
       commande s'il est vide, sinon appelle SetClip(NkRect2i) ;
    2. traduit blend en SetBlendMode (Multiply, Screen, Darken, Lighten,
       PlusLighter, et NK_ALPHA par défaut) ;
    3. si type == TexturedTriangles, cherche texId parmi les polices puis
       parmi les images (nullptr si rien) ;
    4. calcule les bornes lo/hi des indices et les rebase, pour ne passer que
       les sommets utiles (limite de 65536 sommets du dorsal) ;
    5. appelle DrawVertices(sommets + lo, hi - lo + 1, indices, idxCount, tex) ;
    6. appelle PopClip si un rognage a été posé.
  À la fin, il remet SetBlendMode(NK_ALPHA).
Ce qu'il appelle sur le moteur se réduit donc à quatre méthodes de
NkIRenderer2D (SetClip, SetBlendMode, DrawVertices, PopClip ; GetStats pour la
mesure seulement) et à NkTexture (Create, Update, SetFilter).

Le second pont :
Un NkGuiImageBackend qui reçoit les mêmes choses que le premier, sauf le
moteur : à la place du NkIRenderer2D, une image RGBA8 de largeur x hauteur en
mémoire, et à la fin un chemin de fichier. Il garde les mêmes noms et les
mêmes signatures pour les textures et pour Submit, pour que le code qui
alimente le premier pont (NkCanvasGuiApp.h l. 142, 157, 246-247) puisse
alimenter celui-ci sans changement.

Pour chaque commande de la liste, il fait ce que le premier délègue au GPU :
- le rognage devient un rectangle entier (clipRect borné à l'image) dans
  lequel on écrit ; pas de pile SetClip/PopClip, il est local à la commande ;
- chaque triplet d'indices est un triangle rempli par fonctions d'arête, avec
  couleur et uv interpolés ; pas de rebasage, il n'y a pas de limite de
  sommets ;
- si la commande est texturée, on échantillonne la texture texId et on la
  multiplie par la couleur ;
- le pixel est composé selon blend avec les mêmes facteurs que le dorsal
  (Alpha, Multiply, Screen, Darken, Lighten, PlusLighter).
L'écriture du fichier n'est pas à réécrire : NKImage la sait déjà
(NkImage::Wrap sur les pixels, NkImage.h l. 827, puis SavePNG, l. 359).

Rien ne change dans NKGui : le pont ne lit que NkGuiDrawList, NkGuiVertex et
NkGuiDrawCmd tels qu'ils sont, et c'est l'application qui choisit l'un ou
l'autre pont.

Combien de méthodes :
Cinq méthodes publiques.
1. Init(largeur, hauteur, fond) : alloue l'image et la remplit du fond. Elle
   remplace Init(NkIRenderer2D*) ; à l'écran c'est la cible qui s'efface, ici
   personne ne le fait à notre place.
2. UploadFontGray8(texId, gris, w, h) : même signature que le premier.
3. UploadImageRGBA(texId, rgba, w, h) : même signature que le premier.
4. Submit(dl) : même rôle ; fbW/fbH ne sont plus nécessaires, la taille est
   celle de l'image.
5. SaveToFile(chemin) : la seule méthode sans équivalent, parce qu'à l'écran
   la présentation est faite par la fenêtre, hors du pont.
Justification : le premier pont a quatre méthodes utiles (Init, deux Upload,
Submit) ; on en garde quatre de même forme et on en ajoute une pour sortir le
résultat. Les quatre appels au moteur (SetClip, SetBlendMode, DrawVertices,
PopClip) ne deviennent pas des méthodes publiques : ce sont des étapes
internes de Submit (rognage, mélange, triangle). On pourrait fondre les deux
Upload en une seule avec un nombre de canaux, mais les garder identiques au
premier pont rend les deux interchangeables pour l'appelant.

Les données :
Ce qui suffit :
- la liste : vtx (pos, uv, col), idx, et pour chaque commande type, idxOffset,
  idxCount, texId, clipRect, blend. Rien d'autre de NKGui ;
- les textures, par texId : pixels, largeur, hauteur, et 1 ou 4 canaux (un
  atlas de police est un alpha sur blanc, comme l'étend UploadFontGray8) ;
- la taille de l'image et sa couleur de fond ;
- le chemin du fichier (et son format, que l'extension peut donner).
Ce qui n'est pas nécessaire : la fenêtre, le contexte graphique, le
NkIRenderer2D, les fenêtres de l'interface et leur ordre (déjà fusionnés dans
la liste par Append), les piles clipStack/blendStack et thickScale (déjà
appliqués aux sommets et aux commandes).
Justification : le premier pont ne lit lui-même rien d'autre de la liste ;
tout ce qu'il passe au moteur (sommets, indices, texture, rognage, mélange)
vient de ces champs. Un piège à garder : col est en a<<24|b<<16|g<<8|r
(NkGuiPackColor), il faut la dépaqueter dans cet ordre.

Dans le kit :
Le second pont existe : NKGui/Core/NkGuiDrawListRaster.h, ligne 62, class
NkGuiDrawListRaster (espace nkentseu::nkgui). Il n'inclut que NkVector.h et
NkGuiDrawList.h. Son en-tête dit qu'il a été écrit pour l'export PNG de
NkUIDesign. Aucun autre en-tête du kit ne l'inclut.

Ses méthodes publiques : Init(w, h) l. 66, Effacer(rgba) l. 77,
PoserTexture(id, px, w, h, canaux) l. 91, Rasteriser(dl) l. 116, Largeur(),
Hauteur(), Pixels(), Pixel(x, y) l. 151-155. Il fait ce que j'ai décrit pour
Submit : un triangle par triplet d'indices, fonctions d'arête au centre du
pixel avec règle haut-gauche, couleur et uv interpolés, échantillonnage
bilinéaire (le NK_LINEAR du premier pont), rognage par commande, et les six
modes de NkGuiBlend avec les facteurs du dorsal OpenGL. NKGui n'a pas été
modifié pour lui : il lit la même liste que NkGuiCanvasBackend.

Ce qui diffère de ma description :
- Il n'écrit pas de fichier. Il s'arrête aux pixels (Pixels(), RGBA8,
  largeur x hauteur) ; l'enregistrement reste à l'appelant, par NKImage
  (NkImage::Wrap(..., NK_RGBA32) puis SavePNG). Ma cinquième méthode n'y est
  pas, et c'est défendable : il n'a pas à dépendre de NKImage.
- Il est dans NKGui, pas dans NKCanvas, et ne dépend d'aucun moteur.
  L'en-tête explique pourquoi : le dorsal Software de NKCanvas ne
  s'initialise que sur une fenêtre (NkSoftwareContext::Initialize(const
  NkWindow &, ...), NkSoftwareContext.h l. 496), il n'y avait donc rien à
  réutiliser sans écran.
- Une seule méthode PoserTexture avec un nombre de canaux (1 ou 4) au lieu de
  mes deux Upload. Le gris n'est pas étendu en RGBA : il est lu comme alpha
  sur blanc à l'échantillonnage. Conséquence : ce n'est pas un remplaçant
  direct du premier pont, l'appelant doit adapter ses appels.
- Les textures sont référencées, pas copiées : l'appelant les garde vivantes
  jusqu'à Rasteriser. Le premier pont, lui, les recopie dans des NkTexture.
- Le fond est une méthode à part, Effacer(0xRRGGBBAA), et Init laisse
  l'image transparente ; je l'avais mis dans Init.
- Rasteriser ne prend pas de taille (celle de l'image sert de borne au
  rognage) et rend un nombre : les commandes texturées dont le texId était
  inconnu, peintes avec la seule couleur des sommets. Le premier pont, dans
  ce cas, passe tex = nullptr sans rien dire.
- Il ajoute de quoi vérifier sans écran : Pixel(x, y), Largeur(), Hauteur().
- Il lit les indices directement, sans rebasage, et vérifie qu'ils restent
  dans vtx et idx, ce que le premier pont ne fait pas.
- Le mélange Alpha est en alpha droit : identique au GPU sur un fond opaque,
  plus juste sur un fond transparent.
- Attention au format : Effacer et Pixel parlent en 0xRRGGBBAA, alors que la
  couleur des sommets est empaquetée a<<24|b<<16|g<<8|r.
Au total : huit méthodes publiques là où j'en comptais cinq, mais le cœur est
le même (une initialisation, des textures, une méthode qui consomme la
liste) ; les écarts viennent de la sortie (pixels au lieu d'un fichier) et
des accesseurs de test.
