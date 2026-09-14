# Faute de syntaxe dans NKMath — où la construction s'arrête



## Résultats

| | |
|---|---|
| **Temps avant l'arrêt** | **26,9 s** au chronomètre (19:28:12 → 19:28:39) ; 23,07 s selon Jenga, dont 5,08 s dans NKMath |
| **Projets construits** | **6 sur 16** : NKPlatform, NKGlad, NKCore, NKMemory, NKContainers, NKThreading |
| **Projet en échec** | NKMath : 11 fichiers `.cpp` sur 12 compilés, `NkAngle.cpp` en erreur, pas de `NKMath.lib` |
| **Projets non atteints** | **9** : NKLogger, NKFont, NKFileSystem, NKTime, NKStream, NKEvent, NKImage, NKWindow, NKCanvas |
| **Code de retour** | 1 (`Status: ✗ FAILURE`) |

## Le message

```text
║                                Compilation Error: NkAngle.cpp                                ║
║ …\Kernel\Foundation\NKMath\src\NKMath\NkAngle.cpp:22:27: error: expected expression           ║
║    22 | int nk_faute_de_syntaxe = ;                                                          ║
║       |                           ^                                                          ║
║ 1 error generated.                                                                           ║
...
Projects Built:  6/16
Failed:         1
Not reached:    9  (arret au premier echec — voir --keep-going)
Errors:         2
Time:           23.07s
Status:         ✗ FAILURE

Echecs (1) — a corriger :
  ✗ NKMath
```

## Ce que le message apprend sur l'ordre de construction

1. **Les projets sont construits un par un, dans l'ordre affiché.** L'échec survient au 7ᵉ projet sur 16, exactement la place de NKMath dans le `Build Order`. Les 6 projets placés avant lui sont terminés.
2. **La construction s'arrête au premier projet en échec.** `Not reached: 9` : aucun projet placé après NKMath n'est lancé.
3. **L'arrêt ne tient pas compte des dépendances.** Parmi les 9 projets non atteints, **4 ne dépendent pas de NKMath** : NKLogger, NKFileSystem, NKTime et NKStream (voir leurs listes `depends:`). Ils auraient pu être construits, mais ils sont placés après NKMath. À l'inverse, NKGlad et NKThreading, qui ne dépendent pas non plus de NKMath, ont été construits parce qu'ils sont placés avant.
4. **À l'intérieur d'un projet, les fichiers sont compilés en parallèle.** Les 11 autres fichiers de NKMath ont été compilés malgré l'erreur ; seule l'étape de création de la bibliothèque est abandonnée.
5. **Le message désigne le fichier, la ligne et la colonne** (`NkAngle.cpp:22:27`), et le projet à corriger (`✗ NKMath`).

## À savoir

- `Errors: 2` alors que clang ne signale qu'une erreur (`1 error generated`) : le compteur de Jenga compte autre chose que les erreurs du compilateur. Probablement l'échec du fichier en plus de l'erreur elle-même, mais je ne l'ai pas vérifié dans le code.
- `--keep-going` construirait tout ce qui ne dépend pas de NKMath : Jenga le suggère dans le bilan.

## Remise en état

| Vérification | Résultat |
|---|---|
| Restauration depuis la copie de sauvegarde | fait |
| sha256 avant / après | `1dc1f6bb…91be` = `1dc1f6bb…91be` : **identique** |
| `git status -- Kernel/Foundation/NKMath` | vide |
| `jenga build --target NKMath` | **5/5, SUCCESS** (20,6 s) |