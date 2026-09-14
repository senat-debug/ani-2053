# Journal d'installation — construire Nkentseu dans un environnement propre

>
## Le journal, dans l'ordre où ça a cassé

| # | Ce qui manquait | Ce que j'ai vu | Ce que j'ai fait | Sur une vraie machine neuve |
|---:|---|---|---|---|
| 1 | **Python** | `python` : « n'est pas reconnu comme nom d'applet de commande… » | ajouté `C:\Python314` au `PATH` | installer Python ≥ 3.8 (exigence de Jenga, `requires-python = ">=3.8"`) ; j'avais 3.14.3 |
| 2 | **Jenga** | `jenga` : « n'est pas reconnu… » | — | installer Jenga depuis son dépôt ; ses dépendances sont `watchdog>=2.1.0` et `requests>=2.28.0` |
| 3 | **Jenga installé pour l'utilisateur seulement** | `python -s -c "import Jenga"` → `ModuleNotFoundError: No module named 'Jenga'` | gardé le site utilisateur | savoir que `pip install` sans droits administrateur installe dans le profil |
| 4 | **`jenga.exe` hors du `PATH`** | toujours « n'est pas reconnu », même avec `C:\Python314\Scripts` | ajouté `%APPDATA%\Python\Python314\Scripts` | ajouter ce dossier au `PATH` : l'installeur ne le fait pas |
| 5 | **Version de Jenga** | rien : 2.8.0 installé, 2.8.0 exigé | — | vérifier `Applications/NKCode/JENGA_VERSION` (2.8.0). D'après ce fichier, un Jenga 2.4.0 s'arrêtait sur `name 'appurl' is not defined` |
| 6 | **Sous-modules git vides** | `jenga info` : `Error loading workspace: External file not found: Externals\Libs\NKGlad\NKGlad.jenga` — **aucune commande ne marche** | recopié NKGlad, puis NKGLSlang, puis NKSPIRVCross depuis ma copie de travail (30, 459 et 4 436 fichiers) ; au 4ᵉ essai, le workspace se charge | cloner avec `--recursive`. Il y a 7 sous-modules ; ces 3-là suffisent pour charger le workspace. Les 3 dépôts sont accessibles avec mes identifiants |
| 7 | **MSYS2 / clang** | rien : NKMath se construit **sans** MSYS2 dans le `PATH` | — | `config/toolchain.jenga:24-31` cherche clang dans le `PATH`, puis au chemin fixe `C:\msys64\ucrt64\bin`. Il faut donc MSYS2 **exactement** à `C:\msys64`, avec clang (ici 21.1.5), ou clang dans le `PATH`. *Je n'ai pas pu tester son absence sans le désinstaller.* |
| 8 | **DLL de MinGW à l'exécution** | `NKMath_Tests.exe` lancé seul : code `0xC0000135` (DLL introuvable). Lancé par `jenga test`, il marche | — | ajouter `C:\msys64\ucrt64\bin` au `PATH` pour lancer un binaire hors de Jenga. DLL demandées : `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll` |
| 9 | **Tests désactivés par le workspace** | `jenga test` refuse : `disableunittestcompilation` | ajouté `--force` | c'est voulu (`Nkentseu.jenga:451-453`) |
| 10 | **Chemin avec espaces** | dans le dépôt d'origine (`…\gap l2\chap 1\…`), chaque construction recompile tout. Dans le clone, 2ᵉ construction identique : **0 fichier, 1,5 s au lieu de 11,6 s** | cloné dans un chemin sans espace | cloner le dépôt dans un chemin **sans espace** |
| 11 | **Noms de fichiers trop longs** | quand j'ai extrait le dépôt dans un dossier profond (tentative de PR) : `Filename too long` au checkout. Pas de problème dans `C:\Users\ngatc\nkclean` | cloné près de la racine du profil | `git config --global core.longpaths true`, ou cloner dans un chemin court |

**Ce qui n'a pas manqué** : le SDK Vulkan (`VULKAN_SDK` vide : le backend Vulkan est simplement désactivé et NKCanvas se construit), CMake et ninja (inutiles pour ces cibles), et les variables `NK_*` (toutes facultatives).

## Résultat dans l'environnement propre

| Commande | Résultat |
|---|---|
| `jenga info` | OK, après les 3 sous-modules |
| `jenga build --target NKMath` | 5/5 projets, 15 s |
| `jenga test --project NKMath_Tests --force` | 7/7 projets construits, 8/8 tests réussis |
| `jenga build --target NKCanvas` | 16/16 projets, 75 s |

## La doc d'installation que j'aurais voulu trouver (Windows)

1. Installer **Git**, puis : `git config --global core.longpaths true`
2. Installer **Python 3.8 ou plus récent**.
3. Installer **MSYS2 dans `C:\msys64`**, puis clang pour UCRT64 (commande MSYS2 habituelle, que je n'ai pas exécutée ici) : `pacman -S mingw-w64-ucrt-x86_64-clang`
4. Ajouter au `PATH` :
   - `C:\msys64\ucrt64\bin`, pour lancer les programmes construits ;
   - `%APPDATA%\Python\Python3xx\Scripts`, pour `jenga`.
5. Installer **Jenga à la version écrite dans `Applications/NKCode/JENGA_VERSION`** :
   ```bash
   git clone https://github.com/Rihen-Universe/Jenga.git
   cd Jenga && git checkout v2.8.0
   python -m pip install -e . --no-build-isolation
   ```
6. Cloner Nkentseu **avec ses sous-modules**, dans un chemin **sans espace** :
   ```bash
   git clone --recursive https://github.com/Rihen-Universe/Nkentseu.git C:\dev\Nkentseu
   ```
7. Vérifier : `jenga info`, puis `jenga build --target NKMath`.

## Ce que je retiens

Sur ma machine, tout « marchait » parce que tout y avait été installé petit à petit, sans que personne ne le note. Dans un environnement propre, le premier obstacle n'est pas le compilateur : ce sont **les sous-modules**. Leur absence bloque même `jenga info`, avec un message qui ne parle jamais de git. Le reste tient en quelques lignes de `PATH`, et en deux pièges qu'aucun message n'explique : le chemin avec espaces, et `C:\msys64` écrit en dur.

## Limites et état

- **Même machine** : MSYS2 est installé à `C:\msys64`, et ce chemin est codé en dur. Je n'ai donc pas pu simuler un Windows sans compilateur.
- **Cache** : même dans le clone, changer de cible, passer de `jenga test` à `jenga build` ou changer de terminal a relancé une compilation complète. Seule une construction répétée à l'identique est incrémentale. Je ne l'ai pas analysé plus loin.
- **Le clone reste sur le disque** : `C:\Users\ngatc\nkclean\Nkentseu`. Pour le supprimer : `Remove-Item -Recurse -Force C:\Users\ngatc\nkclean`. Le dépôt d'origine n'a pas été modifié.