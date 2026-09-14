# Rapport — Workspace Jenga « Nkentseu »

> Source : sortie de la commande d'inspection du workspace Jenga (aucun code lu).

## 1. Fichier racine du workspace

| Élément | Valeur |
|---|---|
| Fichier racine | `Nkentseu.jenga` |
| Chemin complet | `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu\Nkentseu.jenga` |
| Dossier du workspace | `C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu` |

## 2. Projet de démarrage

 Le projet de démarrage déclaré est **Sandbox**


## 3. Nombre de projets par type

| Type (Kind) | Nombre |
|---|---:|
| ConsoleApp | 98 |
| StaticLib | 60 | 
| TestSuite | 60 | 
| WindowedApp | 55 |
| **Total** | **273** |



## 4. Chaînes de compilation (toolchains)

**6 chaînes disponibles**

| Nom | Famille | OS cible | Arch | Env |
|---|---|---|---|---|
| host-clang | clang | Windows | x86_64 | mingw |
| host-gcc | gcc | Windows | x86_64 | mingw |
| msvc | msvc | Windows | x86_64 | msvc |
| clang-mingw | clang | Windows | x86_64 | mingw |
| mingw | gcc | Windows | x86_64 | mingw |
| clang-cross-linux | clang | Linux | x86_64 | gnu |

5 ciblent Windows, 1 cible Linux (cross-compilation).

```cpp
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║                ██╗███████╗███╗   ██╗ ██████╗  █████╗             ║
║                ██║██╔════╝████╗  ██║██╔════╝ ██╔══██╗            ║
║                ██║█████╗  ██╔██╗ ██║██║  ███╗███████║            ║
║           ██   ██║██╔══╝  ██║╚██╗██║██║   ██║██╔══██║            ║
║           ╚█████╔╝███████╗██║ ╚████║╚██████╔╝██║  ██║            ║
║            ╚════╝ ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝            ║
║                                                                  ║
║             Multi-platform C/C++ Build System v2.8.0             ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝

[NKCode] ATTENTION : aucun wheel Jenga trouve (dist/*.whl) -> le paquet n'aura PAS de Jenga embarque, et les boutons Construire/Executer seront inoperants. Produisez-le avec ./cri.sh dans le depot Jenga.
========================== Jenga Workspace: Nkentseu ===========================

Location: C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu
Entry file: C:\Users\ngatc\Desktop\gap l2\chap 1\Nkentseu\Nkentseu.jenga
Configurations: Debug, Release
Platforms: Windows
Target OSes: Windows, Linux, macOS, Android, iOS, Web, HarmonyOS, XboxSeries, XboxOne
Target Architectures: x86_64, arm64, wasm32
Start project: Sandbox


Projects
------------------------------------------------------------
Name                         Kind          Language   Test   External
=====================================================================
__Unitest__                  StaticLib     C++        No     No
NKPlatform_Tests             TestSuite     C++        Yes    Yes
NKPlatform                   StaticLib     C++        No     Yes
NKCore                       StaticLib     C++        No     Yes
NKCore_Tests                 TestSuite     C++        Yes    Yes
NKLogger                     StaticLib     C++        No     Yes
NKLogger_Tests               TestSuite     C++        Yes    Yes
NKMath                       StaticLib     C++        No     Yes
NKMath_Tests                 TestSuite     C++        Yes    Yes
NKMemory_Tests               TestSuite     C++        Yes    Yes
NKMemory                     StaticLib     C++        No     Yes
NKContainers                 StaticLib     C++        No     Yes
NKContainers_Tests           TestSuite     C++        Yes    Yes
NKImage_Tests                TestSuite     C++        Yes    Yes
NKImage                      StaticLib     C++        No     Yes
NKFont                       StaticLib     C++        No     Yes
NKFont_Tests                 TestSuite     C++        Yes    Yes
NKAudio                      StaticLib     C++        No     Yes
NKAudio_Tests                TestSuite     C++        Yes    Yes
NKMedia                      StaticLib     C++        No     Yes
NKTime                       StaticLib     C++        No     Yes
NKTime_Tests                 TestSuite     C++        Yes    Yes
NKStream                     StaticLib     C++        No     Yes
NKStream_Tests               TestSuite     C++        Yes    Yes
NKThreading                  StaticLib     C++        No     Yes
NKThreading_Tests            TestSuite     C++        Yes    Yes
NKFileSystem_Tests           TestSuite     C++        Yes    Yes
NKFileSystem                 StaticLib     C++        No     Yes
NKReflection_Tests           TestSuite     C++        Yes    Yes
NKReflection                 StaticLib     C++        No     Yes
NKNetwork_Tests              TestSuite     C++        Yes    Yes
NKNetwork                    StaticLib     C++        No     Yes
NKSerialization              StaticLib     C++        No     Yes
NKSerialization_Tests        TestSuite     C++        Yes    Yes
NKGlad                       StaticLib     C          No     Yes
NKGLSlang                    StaticLib     C++        No     Yes
NKSPIRVCross                 StaticLib     C++        No     Yes
NKMbedTLS                    StaticLib     C          No     Yes
pybind11                     StaticLib     C++        No     Yes
NKEvent                      StaticLib     C++        No     Yes
NKEvent_Tests                TestSuite     C++        Yes    Yes
NKWindow                     StaticLib     C++        No     Yes
NKWindow_Tests               TestSuite     C++        Yes    Yes
NKSL                         StaticLib     C++        No     Yes
NKCanvas                     StaticLib     C++        No     Yes
NKCanvas_Tests               TestSuite     C++        Yes    Yes
NKRHI                        StaticLib     C++        No     Yes
NKRHI_Tests                  TestSuite     C++        Yes    Yes
NKUI                         StaticLib     C++        No     Yes
NKUI_Tests                   TestSuite     C++        Yes    Yes
NKGui                        StaticLib     C++        No     Yes
NKEditorKit                  StaticLib     C++        No     Yes
Unkeny                       StaticLib     C++        No     Yes
NkPdfProbe                   ConsoleApp    C++        No     Yes
NkPdfRasterTest              ConsoleApp    C++        No     Yes
NkFileWorkerTest             ConsoleApp    C++        No     Yes
NkPdfRenderProbe             ConsoleApp    C++        No     Yes
NKCode                       WindowedApp   C++        No     Yes
NK3DModeler                  WindowedApp   C++        No     Yes
NKImGuiIntegration           StaticLib     C++        No     Yes
NKUIIntegration              StaticLib     C++        No     Yes
NKGuiIntegration             StaticLib     C++        No     Yes
NKCamera                     StaticLib     C++        No     Yes
NKCamera_Tests               TestSuite     C++        Yes    Yes
NKCollision                  StaticLib     C++        No     Yes
NKCollision_Tests            TestSuite     C++        Yes    Yes
NKPhysics_Tests              TestSuite     C++        Yes    Yes
NKPhysics                    StaticLib     C++        No     Yes
NKAnimPhysics                StaticLib     C++        No     Yes
NKAnimation                  StaticLib     C++        No     Yes
NKNavigation                 StaticLib     C++        No     Yes
NKNavigation_Tests           TestSuite     C++        Yes    Yes
NKXR_Tests                   TestSuite     C++        Yes    Yes
NKXR                         StaticLib     C++        No     Yes
NewGeneration_Tests          TestSuite     C++        Yes    Yes
NkRHIDemoFullImage_Tests     TestSuite     C++        Yes    Yes
NKRHIDemo                    WindowedApp   C++        No     Yes
cr2d_Tests                   TestSuite     C++        Yes    Yes
NkSWRasterPrimitives         WindowedApp   C++        No     Yes
gltftest                     WindowedApp   C++        No     Yes
renderdemo_Tests             TestSuite     C++        Yes    Yes
NkCanvasDemo_Tests           TestSuite     C++        Yes    Yes
NkDrawableDemo               WindowedApp   C++        No     Yes
renderdemo                   WindowedApp   C++        No     Yes
r2d01_Tests                  TestSuite     C++        Yes    Yes
NewGeneration                WindowedApp   C++        No     Yes
NkFDV2                       WindowedApp   C++        No     Yes
NKRHIDemo_Tests              TestSuite     C++        Yes    Yes
NkRHIDemoFull                WindowedApp   C++        No     Yes
NkRHIInterpTest_Tests        TestSuite     C++        Yes    Yes
RendererRHI_Tests            TestSuite     C++        Yes    Yes
Gamepad                      WindowedApp   C++        No     Yes
NkFDV2_Tests                 TestSuite     C++        Yes    Yes
cr2d                         WindowedApp   C++        No     Yes
FontLoad                     WindowedApp   C++        No     Yes
RendererRHI                  WindowedApp   C++        No     Yes
gltftest_Tests               TestSuite     C++        Yes    Yes
NkRHIDemoFull_Tests          TestSuite     C++        Yes    Yes
firsttriangle_Tests          TestSuite     C++        Yes    Yes
NkSWRasterPrimitives_Tests   TestSuite     C++        Yes    Yes
firsttriangle                WindowedApp   C++        No     Yes
NkDrawableDemo_Tests         TestSuite     C++        Yes    Yes
NkCanvasDemo                 WindowedApp   C++        No     Yes
r2d01                        WindowedApp   C++        No     Yes
NkSpriteDemo_Tests           TestSuite     C++        Yes    Yes
NkFontDemo_Tests             TestSuite     C++        Yes    Yes
NkRHIInterpTest              WindowedApp   C++        No     Yes
NkSpriteDemo                 WindowedApp   C++        No     Yes
NkRHIDemoFullImage           WindowedApp   C++        No     Yes
Gamepad_Tests                TestSuite     C++        Yes    Yes
NkFontDemo                   WindowedApp   C++        No     Yes
ConquerorProto               WindowedApp   C++        No     Yes
NkAudioDemo                  ConsoleApp    C++        No     Yes
NkCameraDemos                WindowedApp   C++        No     Yes
SandboxNKFileSystem          ConsoleApp    C++        No     Yes
SandboxNKLogger              ConsoleApp    C++        No     Yes
SandboxNKReflection          ConsoleApp    C++        No     Yes
SandboxNKNetwork             ConsoleApp    C++        No     Yes
NKRenderer_Tests             TestSuite     C++        Yes    Yes
NKRenderer                   StaticLib     C++        No     Yes
NKTensor                     StaticLib     C++        No     Yes
NKTensor_Tests               TestSuite     C++        Yes    Yes
NKAutograd_Tests             TestSuite     C++        Yes    Yes
NKAutograd                   StaticLib     C++        No     Yes
NKNN_Tests                   TestSuite     C++        Yes    Yes
NKNN                         StaticLib     C++        No     Yes
NKOptim                      StaticLib     C++        No     Yes
NKOptim_Tests                TestSuite     C++        Yes    Yes
NKData                       StaticLib     C++        No     Yes
NKData_Tests                 TestSuite     C++        Yes    Yes
NKTrain_Tests                TestSuite     C++        Yes    Yes
NKTrain                      StaticLib     C++        No     Yes
NKInfer_Tests                TestSuite     C++        Yes    Yes
NKInfer                      StaticLib     C++        No     Yes
NKRL                         StaticLib     C++        No     Yes
NKRL_Tests                   TestSuite     C++        Yes    Yes
NKAgent                      StaticLib     C++        No     Yes
NKAgent_Tests                TestSuite     C++        Yes    Yes
NKEvolve_Tests               TestSuite     C++        Yes    Yes
NKEvolve                     StaticLib     C++        No     Yes
NKCivilization_Tests         TestSuite     C++        Yes    Yes
NKCivilization               StaticLib     C++        No     Yes
NKEmbodied_Tests             TestSuite     C++        Yes    Yes
NKEmbodied                   StaticLib     C++        No     Yes
NKGen                        StaticLib     C++        No     Yes
NKGen_Tests                  TestSuite     C++        Yes    Yes
NKGpt                        StaticLib     C++        No     Yes
NKSpeech                     StaticLib     C++        No     Yes
NKTensorDemo                 ConsoleApp    C++        No     Yes
NkSLComputeCheck             ConsoleApp    C++        No     Yes
NkGpuProbe                   ConsoleApp    C++        No     Yes
NkComputeNkSL                ConsoleApp    C++        No     Yes
NkTensorGpuTest              ConsoleApp    C++        No     Yes
NKGpuBenchTest               ConsoleApp    C++        No     Yes
NKConvBenchTest              ConsoleApp    C++        No     Yes
NKConvResidentBench          ConsoleApp    C++        No     Yes
NKMlpResidentBench           ConsoleApp    C++        No     Yes
NKMnistGpuTrain              ConsoleApp    C++        No     Yes
NKRebasinTest                ConsoleApp    C++        No     Yes
NKMnistCnnGpuTrain           ConsoleApp    C++        No     Yes
NKTransformerTest            ConsoleApp    C++        No     Yes
NKGptTrain                   ConsoleApp    C++        No     Yes
NKIlyana                     ConsoleApp    C++        No     Yes
NKAutogradTest               ConsoleApp    C++        No     Yes
NKNNTest                     ConsoleApp    C++        No     Yes
NKConvTest                   ConsoleApp    C++        No     Yes
NKDataTest                   ConsoleApp    C++        No     Yes
NKBpeTest                    ConsoleApp    C++        No     Yes
NKLlamaBlockTest             ConsoleApp    C++        No     Yes
NKRebasinTransformer         ConsoleApp    C++        No     Yes
NKTrainTest                  ConsoleApp    C++        No     Yes
NKFp16Test                   ConsoleApp    C++        No     Yes
NKRnnCtcTest                 ConsoleApp    C++        No     Yes
NKASRTest                    ConsoleApp    C++        No     Yes
NKImageCodecTest             ConsoleApp    C++        No     Yes
NkEditableMeshDemo           ConsoleApp    C++        No     Yes
NkLocomotionDemo             ConsoleApp    C++        No     Yes
NkAssetIODemo                ConsoleApp    C++        No     Yes
NkFBXParityDemo              ConsoleApp    C++        No     Yes
NkSVGImportDemo              ConsoleApp    C++        No     Yes
NKMeshAITest                 ConsoleApp    C++        No     Yes
NKEditMeshHarness            ConsoleApp    C++        No     Yes
NkAnimPhysTest               ConsoleApp    C++        No     Yes
NkMicRecord                  ConsoleApp    C++        No     Yes
NKSpeechTest                 ConsoleApp    C++        No     Yes
NKTTSTrain                   ConsoleApp    C++        No     Yes
NkVoiceLoopDemo              ConsoleApp    C++        No     Yes
NKSpeechFeatureDemo          ConsoleApp    C++        No     Yes
NKMediaTest                  ConsoleApp    C++        No     Yes
NkVideoReadTest              ConsoleApp    C++        No     Yes
NkAudioPlayer                WindowedApp   C++        No     Yes
NkVideoPlayer                WindowedApp   C++        No     Yes
NKOpusRef                    ConsoleApp    C++        No     Yes
NKVideoTest                  ConsoleApp    C++        No     Yes
NKInferTest                  ConsoleApp    C++        No     Yes
NKGGUFInspectTest            ConsoleApp    C++        No     Yes
NKLLMInferTest               ConsoleApp    C++        No     Yes
NKQwenTokenizerTest          ConsoleApp    C++        No     Yes
NKQwen2BackwardTest          ConsoleApp    C++        No     Yes
NKQwen2SftTest               ConsoleApp    C++        No     Yes
NKQ4MatmulTest               ConsoleApp    C++        No     Yes
NKQwen2GpuTest               ConsoleApp    C++        No     Yes
NKQwen2Chat                  ConsoleApp    C++        No     Yes
NKQwen2SftGpuTest            ConsoleApp    C++        No     Yes
NKQwen2Train                 ConsoleApp    C++        No     Yes
NKQwen2Ask                   ConsoleApp    C++        No     Yes
NKRLTest                     ConsoleApp    C++        No     Yes
NKAgentTest                  ConsoleApp    C++        No     Yes
NKAgentLLMTest               ConsoleApp    C++        No     Yes
NkAgentEcsDemo               ConsoleApp    C++        No     Yes
NKEmbodiedTest               ConsoleApp    C++        No     Yes
NKEvolveTest                 ConsoleApp    C++        No     Yes
NKEvolveNNTest               ConsoleApp    C++        No     Yes
NKCivilizationTest           ConsoleApp    C++        No     Yes
NKCivilizationSocialTest     ConsoleApp    C++        No     Yes
NKCivilizationScaleTest      ConsoleApp    C++        No     Yes
NKGenTest                    ConsoleApp    C++        No     Yes
NKVAETest                    ConsoleApp    C++        No     Yes
NKMnistVAETest               ConsoleApp    C++        No     Yes
NKMnistConvVAETest           ConsoleApp    C++        No     Yes
NKConvVAETest                ConsoleApp    C++        No     Yes
NKDiffusionTest              ConsoleApp    C++        No     Yes
NKVoxelGenTest               ConsoleApp    C++        No     Yes
NKObjectGenTest              ConsoleApp    C++        No     Yes
NKGen3DTest                  ConsoleApp    C++        No     Yes
NKGenMeshTest                ConsoleApp    C++        No     Yes
NKSmoothMeshTest             ConsoleApp    C++        No     Yes
NKMatTypeResetTest           ConsoleApp    C++        No     Yes
NKEditTargetTest             ConsoleApp    C++        No     Yes
NKMeshRenderTest             ConsoleApp    C++        No     Yes
Model                        WindowedApp   C++        No     Yes
NkSLCheck                    ConsoleApp    C++        No     Yes
NKPA                         WindowedApp   C++        No     Yes
NKECS_Tests                  TestSuite     C++        Yes    Yes
NKECS                        StaticLib     C++        No     Yes
Noge                         StaticLib     C++        No     Yes
Noge_Tests                   TestSuite     C++        Yes    Yes
Nogee                        WindowedApp   C++        No     Yes
NKEditorKitDemo              WindowedApp   C++        No     Yes
NKEditorKitTest              ConsoleApp    C++        No     Yes
NKUIDesign                   WindowedApp   C++        No     Yes
ConquerorLab                 WindowedApp   C++        No     Yes
NkRef                        WindowedApp   C++        No     Yes
NkAnimaEditor                WindowedApp   C++        No     Yes
ImGuiRef                     WindowedApp   C++        No     Yes
NKGuiDrawTest                ConsoleApp    C++        No     Yes
NKGuiDemo                    WindowedApp   C++        No     Yes
NKViewportDemo               WindowedApp   C++        No     Yes
PV3DE                        WindowedApp   C++        No     Yes
Pong                         WindowedApp   C++        No     Yes
Nkoung                       WindowedApp   C++        No     Yes
Mou                          WindowedApp   C++        No     Yes
NkImeTest                    WindowedApp   C++        No     Yes
NkImageDemo                  WindowedApp   C++        No     Yes
RihenDefi                    WindowedApp   C++        No     Yes
NkAudioECSDemo               ConsoleApp    C++        No     Yes
NkNetWorldDemo               ConsoleApp    C++        No     Yes
NkNavDemo                    ConsoleApp    C++        No     Yes
NkNavCoreDemo                ConsoleApp    C++        No     Yes
NkHotReloadDemo              ConsoleApp    C++        No     Yes
NkUIHudDemo                  ConsoleApp    C++        No     Yes
Tuto02Renderer               WindowedApp   C++        No     Yes
Tuto03Scene                  WindowedApp   C++        No     Yes
Tuto04Camera                 WindowedApp   C++        No     Yes
Tuto01Fenetre                WindowedApp   C++        No     Yes
Tuto05Meshes                 WindowedApp   C++        No     Yes
NKXRDemo                     WindowedApp   C++        No     Yes
NKARDemo                     WindowedApp   C++        No     Yes
GemCrush                     WindowedApp   C++        No     Yes
NkDames                      WindowedApp   C++        No     Yes
NkEchecs                     WindowedApp   C++        No     Yes
NkLudo                       WindowedApp   C++        No     Yes
UnkenyEditor                 WindowedApp   C++        No     Yes


Available Toolchains
------------------------------------------------------------
Name                Family   Target OS   Arch     Env  
=======================================================
host-clang          clang    Windows     x86_64   mingw
host-gcc            gcc      Windows     x86_64   mingw
msvc                msvc     Windows     x86_64   msvc
clang-mingw         clang    Windows     x86_64   mingw
mingw               gcc      Windows     x86_64   mingw
clang-cross-linux   clang    Linux       x86_64   gnu


Daemon
------------------------------------------------------------
Status: Not running