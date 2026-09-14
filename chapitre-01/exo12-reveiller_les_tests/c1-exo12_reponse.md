# Tests désactivés — lancer malgré tout NKMath_Tests


## La ligne qui désactive les tests

`Nkentseu.jenga`, lignes 451 et 453 :

```python
    dutc(enable=True)    # disable unit test compilation
    dute(enable=True)    # disable unit test execution
```

`dutc` et `dute` sont des raccourcis de `disableunittestcompilation` et `disableunittestexecution` (`Jenga/Core/Api.py:1453-1461`).

## Les commandes

```bash
jenga test --project NKMath_Tests
jenga test --project NKMath_Tests --force
./Build/Tests/Debug-Windows/NKMath_Tests.exe
```

| Commande | Résultat |
|---|---|
| sans `--force` | **refusé** en 2,5 s, code 1 : `Unit-test compilation is disabled by workspace policy (disableunittestcompilation) and 'NKMath_Tests' is not in its allow list.` |
| avec `--force` | construit 7 projets (les 5 de NKMath, `__Unitest__`, NKMath_Tests) en 34,7 s, puis `All tests passed for NKMath_Tests.`, code 0 |
| exécutable lancé directement | rapport détaillé, en 36 ms |

`jenga test` n'affiche aucun compte : les nombres ci-dessous viennent du rapport de l'exécutable.

## Les nombres

| | Nombre | Source |
|---|---:|---|
| **Suites de tests dans le workspace** | **60** | projets `TestSuite` listés par `jenga info` |
| **Suites exécutées** | **0** par défaut ; **1** avec `--force --project NKMath_Tests` | `dutc` / `dute` |
| **Groupes de tests dans NKMath_Tests** | 2 : `NKmathmoke` (7 tests), `NKMathBenchmark` (1 test) | `tests/test_smoke.cpp`, `tests/benchmark_smoke.cpp` |
| **Tests exécutés** | **8** | `Number of tests: 8` |
| **Tests réussis** | **8 / 8** (100 %) | `Tests : 8 réussis, 8 au total` |
| **Assertions** | 499 / 499 | `Assertions : 499 réussies, 499 au total` |

Détail du rapport :

```text
✓ NKMathBenchmark_TrigonometryLoopVsStd         [OK]  3/3 assertions  (35ms)
✓ NKmathmoke_BitAndIntegerUtilities             [OK]  7/7 assertions
✓ NKmathmoke_DivisionAndInterpolationEdges      [OK]  10/10 assertions
✓ NKmathmoke_QuaternionComposition              [OK]  28/28 assertions
✓ NKmathmoke_QuaternionRotateVector             [OK]  184/184 assertions
✓ NKmathmoke_QuaternionToMatrix                 [OK]  259/259 assertions
✓ NKmathmoke_ScalarFunctions                    [OK]  4/4 assertions
✓ NKmathmoke_VectorAndRectTypes                 [OK]  4/4 assertions
Tests :      8 réussis, 8 au total
Assertions : 499 réussies, 499 au total
```

## À savoir

- Pour autoriser cette suite de façon durable, sans `--force`, le message de Jenga propose `dutc(True, allow=['NKMath_Tests'])`. Je ne l'ai pas appliqué.
- Le groupe s'appelle `NKmathmoke` dans le code (`test_smoke.cpp:9`), probablement une coquille pour « NKMathSmoke ».
- Le benchmark affiche NkMath à 18 ms et `std::sin`/`std::cos` à 14 ms, en Debug.
- Aucun fichier du dépôt n'a été modifié ; seul `Build/Tests/Debug-Windows/NKMath_Tests.exe` a été produit.