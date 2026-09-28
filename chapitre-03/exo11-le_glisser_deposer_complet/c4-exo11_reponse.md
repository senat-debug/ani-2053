# Chapitre 4 — Les six événements de dépôt

Windows 11, backend Win32, NKCanvas en OpenGL, Jenga 2.8.0, Debug. Le 2026-09-28.

Les six, tels que le moteur les déclare dans `NkDropEvent.h` :

| # | Événement | Ce qu'il porte |
|---|---|---|
| 1 | `NkDropEnterEvent` | position, nombre de fichiers, `hasText`, `hasImage` |
| 2 | `NkDropOverEvent` | position seulement |
| 3 | `NkDropLeaveEvent` | rien |
| 4 | `NkDropFileEvent` | position + la liste des chemins |
| 5 | `NkDropTextEvent` | position + le texte + son type MIME |
| 6 | `NkDropImageEvent` | position + largeur, hauteur, URI source, type MIME |

Les trois premiers racontent **le geste**, les trois derniers **le contenu**. Et il faut `cfg.dropEnabled = true`, sinon rien n'arrive du tout (§3.2.3).

## Le retour visuel

Dès `NkDropEnterEvent`, le fond passe au vert sombre, un cadre épais de 10 px s'allume sur les quatre bords, et un marqueur clair suit la position du survol. Le titre annonce ce qui arrive **avant** le dépôt :

```text
2 fichier(s) | texte : oui | image : non | entree 1, survol 3, sortie 0 | fichier 0, texte 0, image 0
```

C'est ce que font les vrais outils : ils disent « je peux accepter ça » pendant que l'utilisateur tient encore son fichier. `NkDropEnterEvent` porte déjà `numFiles`, `hasText` et `hasImage`, donc le programme sait quoi annoncer sans rien ouvrir.

Sur `NkDropLeaveEvent` ou après un dépôt, tout s'éteint : fond sombre, plus de cadre, plus de marqueur.

## Le relevé

```text
dropEnabled = true

--- sequence simulee (evenements deposes par le programme) ---
ENTREE  en 120,90 : 2 fichier(s) | texte : oui | image : non
SURVOL  en 200,160
SURVOL  en 350,200
SURVOL  en 500,240
DEPOT TEXTE   : texte (text/plain) : "bonjour depuis le glisser-deposer"

--- sequence simulee ---
ENTREE  en 120,90 : 2 fichier(s) | texte : oui | image : non
SURVOL  en 200,160 / 350,200 / 500,240
SORTIE  : le contenu quitte la fenetre

ENTREE  recues : 2
SURVOL  recus  : 6
SORTIE  recues : 1
FICHIER recus  : 0
TEXTE   recus  : 1
IMAGE   recus  : 0
```

Les six chemins sont écrits, et cinq ont été empruntés. Mais il faut dire d'où viennent ces événements.

## Ce que je n'ai pas pu faire, et ce que j'ai fait à la place

**Je n'ai pas réussi à automatiser un vrai glisser-déposer** — même échec qu'à l'exercice 8 du chapitre 3 : un dépôt réel demande une main sur la souris depuis l'explorateur.

J'ai donc **déposé moi-même les événements** dans la file, avec `NkEvents().Enqueue_Public(...)`, exactement les mêmes objets que le backend construit. Ça prouve que **mes six gestionnaires et mon retour visuel sont bons**. Ça ne prouve pas que le backend les émette.

## Ce que le backend émet vraiment (lecture du code)

J'ai compté les émissions dans tous les backends du module :

| Événement | Win32 | Android | Web | Tous |
|---|:-:|:-:|:-:|---|
| `NkDropEnterEvent` | ✅ | ✅ | ✅ | 18 |
| `NkDropOverEvent` | ❌ | ✅ | ✅ | 6 |
| `NkDropLeaveEvent` | ✅ | ✅ | ✅ | 18 |
| `NkDropFileEvent` | ✅ | ✅ | ✅ | 19 |
| `NkDropTextEvent` | ✅ | ✅ | ✅ | 18 |
| `NkDropImageEvent` | ❌ | ❌ | ❌ | **0** |

**Le survol n'existe pas sur Windows.** La fonction est bien là, mais elle ne fait qu'accepter l'effet de copie :

```cpp
// Platform/Win32/NkWin32DropTarget.h:144-147
HRESULT STDMETHODCALLTYPE DragOver(DWORD, POINTL, DWORD *pdwEffect) override {
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}
```

Le `POINTL pt` — la position du curseur pendant le survol — est reçu et **jeté**. Sur Win32, mon marqueur ne bougerait donc jamais pendant un vrai glisser : il resterait à la position de l'entrée.

**Le dépôt d'image n'est émis nulle part**, sur aucune plateforme. `NkDropImageEvent` et `NkDropImageData` sont déclarés, documentés, et personne ne les construit : c'est le même cas que `maximizable` ou `canFullscreen` à l'exercice 2 du chapitre 3 — un chemin qu'aucun code ne prend est une réponse complète, pas un échec.

