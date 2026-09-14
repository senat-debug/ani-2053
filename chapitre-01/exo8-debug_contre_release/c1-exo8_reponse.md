# Debug ou Release — MonEssai

> Jenga 2.8.0, toolchain `clang-mingw`, Windows x86_64, 2026-09-14.
> Commandes : `jenga rebuild --config Debug --target MonEssai` et `jenga rebuild --config Release --target MonEssai`.

## Résultats

| | Debug | Release |
|---|---:|---:|
| **Taille `MonEssai.exe`** | **277 053 o** | **260 585 o** (−5,9 %) |
| **Temps de construction** (moyenne Jenga) | **2,26 s** | **2,25 s** |
| `NKPlatform.lib` | 44 182 o | 12 162 o |
| `src_main.obj` | 3 530 o | 3 530 o (identique) |
| Sections `.debug_*` de l'exécutable | 174 990 o | 163 281 o |

Moyennes sur 2 essais Debug et 3 essais Release. Le premier essai Debug (4,67 s, démarrage à froid) est écarté. Les deux exécutables se lancent sans rien afficher.

## Les lignes qui expliquent les quatre nombres

| Nombre | Lignes | Effet |
|---|---|---|
| **277 053 o** (Debug) | `NKPlatform.jenga:63-64` : `optimize("Off")`, `symbols(True)` → `-O0 -g` (`Windows.py:646-655`) | NKPlatform garde un code non optimisé et 20 Ko de débogage |
| **260 585 o** (Release) | `NKPlatform.jenga:67-68` : `optimize("Speed")`, `symbols(False)` → `-O2`, sans `-g` | retire ≈ 14 Ko de l'exécutable (11,7 Ko de débogage, 2,1 Ko de code) |
| *…pourquoi si peu* | `MonEssai.jenga` : aucune ligne `config:` → valeurs par défaut `optimize=OFF`, `symbols=True` (`Api.py:350-351`) ; `toolchain.jenga:55` : aucun retrait du débogage à l'édition de liens | `main.obj` est identique, et ≈ 161 Ko de débogage apportés par la toolchain restent (63 % de l'exécutable) |
| **2,26 s** (Debug) | `NKPlatform.jenga:63-64` | `-O0` compile vite, mais `-g` coûte |
| **2,25 s** (Release) | `NKPlatform.jenga:67-68` | `-O2` coûte, mais sans `-g` : les deux coûts s'annulent |

Les deux binaires coexistent grâce à `Nkentseu.jenga:457` (`configurations(["Debug", "Release"])`) et `MonEssai.jenga:29-30` (`%{cfg.buildcfg}` dans `objdir` et `targetdir`).

## À savoir

- `rebuild --config Debug --target MonEssai` a aussi effacé les fichiers Debug de NKCore, NKMemory, NKContainers et NKMath (88 fichiers).
- Aucun fichier source ni `.jenga` n'a été modifié.