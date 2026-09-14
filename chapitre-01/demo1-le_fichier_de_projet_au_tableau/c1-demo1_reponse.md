# NKCanvas.jenga en dix minutes


## Plan

| Temps | Partie |
|---:|---|
| 0:00 | 1. Ce qu'il **déclare** |
| 2:30 | 2. Ce qu'il **filtre** |
| 5:30 | 3. Ce qu'il **délègue** |
| 8:00 | 4. **Où est décidé que c'est une bibliothèque statique ?** |
| 9:30 | 5. À retenir |

---

## 1. Ce qu'il déclare (2 min 30)

Quand on enlève les filtres, il reste peu de choses :

```python
33  with project("NKCanvas"):
34      language("C++")
35      cppdialect("C++17")
36      location(".")
41      _canvasDeps = ["NKWindow", "NKFont", "NKImage", "NKStream", "NKTime", "NKGlad", "NKThreading"]
64      nkentseudependson(_canvasDeps, selfexport="NKCanvas", ...)
71      files(["src/NKCanvas/**.cpp", "src/NKCanvas/**.h"])
76      objdir("%{wks.location}/Build/Obj/%{cfg.buildcfg}-%{cfg.system}/%{prj.name}")
77      targetdir("%{wks.location}/Build/Lib/%{cfg.buildcfg}-%{cfg.system}")
```

| Il déclare | Ligne |
|---|---|
| un projet nommé **NKCanvas**, en **C++17** | 33-35 |
| **7 dépendances** directes (plus NKUI si `NK_CANVAS_NKUI` est activé, ce qui n'est pas le cas par défaut) | 41, 61-63 |
| ses **sources** : tout `src/NKCanvas/`, en `.cpp` et `.h` | 71-74 |
| où ranger les **objets** et la **bibliothèque** (`Build/Lib/…`) | 76-77 |

Premier indice pour la fin de la présentation : le dossier de sortie est `Build/Lib`, mais **le fichier ne dit nulle part `staticlib()`**.

---

## 2. Ce qu'il filtre (3 min)

Un filtre, c'est « seulement si… ». NKCanvas en a **16**, et presque tous parlent de **plateformes** : il y a 5 backends graphiques (OpenGL, Vulkan, DirectX, Metal, Software), et chaque système a ses bibliothèques.

| Filtre | Ligne | Ce qu'il ajoute |
|---|---:|---|
| Windows + UWP (dossiers) | 79 | dossiers de sortie `-uwp` |
| **Windows desktop** | 84 | toolchain `TC_WINDOWS`, `WIN32_LEAN_AND_MEAN`, liens `gdi32 user32 d3d11 d3d12 dxgi dxguid` (+ `vulkan-1` si Vulkan) |
| UWP | 106 | toolchain Xbox, `d3d12 dxgi dxguid windowsapp` |
| Linux XLib | 112 | `X11 Xext GL` |
| Linux sans fenêtre | 119 | aucun backend Vulkan |
| Linux XCB | 123 | dossiers `-xcb`, `xcb xcb-image X11 Xext GL` |
| Linux Wayland | 132 | dossiers `-wayland`, `wayland-client xkbcommon EGL…` |
| macOS | 141 | frameworks `Metal QuartzCore`, fichiers `.mm` ajoutés à la main |
| Android | 154 | `android EGL GLESv3` |
| HarmonyOS | 163 | PCH désactivé, `hilog_ndk.z EGL GLESv3` |
| iOS | 181 | Metal seulement : **`excludefiles`** des dossiers OpenGL et Vulkan |
| Web | 198 | toolchain `emscripten` |
| Xbox | 203 | toolchain Xbox |
| Debug / Release | 207, 211 | `-O0 -g` / `-O2` |
| Tests (desktop) | 217 | crée `NKCanvas_Tests`, avec 4 filtres à l'intérieur (Wayland, XCB, XLib, Windows) |

Exemple lu tel quel, le bloc qui sert sur nos machines :

```python
84      with filter("system:Windows && !options:windows-runtime=uwp && !system:XboxSeries && !system:XboxOne"):
85          usetoolchain(TC_WINDOWS)
86          defines(["WIN32_LEAN_AND_MEAN", _VK_ON] + _GLAD_DEF_OTHER)
100         _WIN_LINKS = ["gdi32", "user32", "d3d11", "d3d12", "dxgi", "dxguid"]
103         links(_WIN_LINKS)
```

Deux remarques :
- **Un filtre ajoute ou remplace, il ne retire pas.** La seule exception est `excludefiles` pour iOS (ligne 192).
- Le filtre des tests (ligne 217) dit bien `&& !system:Web`. Dans NKMemory.jenga, la même ligne disait `|| system:Web`, ce qui activait les tests pour le Web par erreur.

---

## 3. Ce qu'il délègue (2 min 30)

C'est la partie qu'on ne voit pas en lisant le fichier : beaucoup de mots viennent **d'ailleurs**.

| Ce qui est utilisé ici | Défini où | Ce que ça fait |
|---|---|---|
| `nkentseudependson(...)` | `config/modules.jenga:334` | chemins d'en-têtes, `dependson`, defines `_STATIC_LIB`, **type du projet**, plus 4 filtres Linux cachés |
| `WANT_VULKAN`, `VULKAN_INCLUDE`, `VULKAN_LIB` | `config/graphics.jenga:49-58` | Vulkan activé seulement si `VULKAN_SDK` existe ou si `NK_ENABLE_VULKAN=on` |
| `PkgExists("libdecor-0")` | `config/graphics.jenga:30` | interroge `pkg-config` (ligne 17) |
| `USE_CANVAS_NKUI` | `config/graphics.jenga:90` | NKUI désactivé par défaut |
| `TC_WINDOWS` | `config/toolchain.jenga:14` | la toolchain clang de Nkentseu |
| `NK_USE_GLAD` | variable d'environnement, lue ligne 52 | glad par défaut sous Linux, chargement manuel ailleurs |
| `with test()` | Jenga, `Core/Api.py:893` | crée le projet `NKCanvas_Tests` |
| la compilation des tests | `Nkentseu.jenga:451` (`dutc`) | désactivée pour tout le workspace |

Un détail qui m'a surpris : le **registre** de `config/modules.jenga:105` donne pour NKCanvas une autre liste de dépendances que la ligne 41. NKCanvas lui-même suit sa ligne 41, mais les projets qui dépendent de NKCanvas suivent le registre.

---

## 4. « Où est décidé que ce module est une bibliothèque statique ? » (1 min 30)

**Réponse courte : pas dans NKCanvas.jenga.** C'est décidé en trois étapes.

**Étape 1 — `NKCanvas.jenga:66`** : le fichier dit seulement « je suis la bibliothèque NKCanvas ».

```python
64      nkentseudependson(
65          _canvasDeps,
66          selfexport="NKCanvas",
```

**Étape 2 — `config/modules.jenga:41` et `:356-359`** : la fonction applique le type choisi **pour tout Nkentseu**.

```python
41   _GLOBAL_KIND = ProjectKind.STATIC_LIB
...
356      is_lib  = selfexport is not None
359          kindexport(_GLOBAL_KIND, _REGISTRY[selfkey]["export"])
```

**Étape 3 — `Jenga/Core/Api.py:1617-1625`** : Jenga fixe le type et ajoute le define.

```python
1620         kind(k)
1624         if k == ProjectKind.STATIC_LIB:
1625             defines([f"{name}_STATIC_LIB"])      # -> NKENTSEU_CANVAS_STATIC_LIB
```

**La preuve, sur nos machines :**
- `jenga info` affiche `NKCanvas   StaticLib` ;
- la construction affiche `16. NKCanvas [STATIC_LIB]` ;
- le fichier produit est `Build/Lib/Debug-Windows/NKCanvas.lib` (10 008 358 octets).

**La décision est à la ligne 41 de `config/modules.jenga`.** Si on la change en `SHARED_LIB`, NKCanvas et tous les modules deviennent des `.dll`, sans toucher une seule ligne de NKCanvas.jenga.

---

## 5. À retenir (30 s)

- **Déclarer**, c'est court : un nom, un langage, des sources, des dossiers, et 7 dépendances.
- **Filtrer**, c'est l'essentiel du fichier : 16 filtres, parce qu'un module graphique change de bibliothèques sur chaque système.
- **Déléguer**, c'est ce qui trompe à la lecture : le type, les chemins d'en-têtes, Vulkan et la toolchain sont décidés dans `config/`. **Un `.jenga` ne se lit pas seul.**