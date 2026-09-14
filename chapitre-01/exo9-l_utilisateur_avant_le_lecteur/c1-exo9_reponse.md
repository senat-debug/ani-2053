# NkRef — dix minutes d'usage


## Ce qu'il fait — constaté le 2026-09-14

1. **Planche de références d'images** : fenêtre de 1280×800, grille et axe orange ; titre `NkRef - sans titre - 100% - 0 image`.
2. **Colle une image** du presse-papiers (menu « Coller ») : le titre passe à `sans titre*`, le compteur à 1 puis 2 images.
3. **Sélectionne** une image au clic (le panneau indique « 1 sélectionnée ») et la **déplace** au glisser.
4. **Miroir horizontal** avec la touche `X`.
5. **Pack** : range les images l'une sous l'autre.
6. **Zoom** à la molette : 100 % → 152 % en 3 crans, affiché dans le titre. « Origine (Début) » revient à 100 %.
7. **Grille** affichée ou masquée avec `G`.
8. **Thème** sombre ou clair (préréglages « Sombre - GitHub Dark Pro » et « Clair »).
9. **Crayon** : mode dessin (`D`), 7 couleurs, épaisseur réglable, « Annuler le dernier trait », « Effacer tous les traits ».
10. **Toujours devant** avec `T` (vérifié : la fenêtre passe au premier plan permanent, puis revient).
11. **Opacité de la fenêtre** avec `1`…`9` et `0` (vérifié : `5` → 50 %, `0` → opaque).
12. **Panneau « Propriétés »** repliable sur le bord droit. **Menu au clic droit** : Ouvrir, Enregistrer, Enregistrer sous, Nouvelle planche, Coller, Crayon, Pack, Origine, Fenêtre, Propriétés, Réglages, Fermer.
13. **Réglages** en 3 onglets : Préférences (dont « Réduire les grandes images à l'import (4096 px) »), Couleurs, Raccourcis (15 raccourcis listés).
14. **Journal** écrit dans `logs/app.log` (rendu OpenGL, police DroidSans). Aucun autre fichier écrit.

## Ce que j'aurais voulu qu'il fasse 

1. **Ajuster une image collée à la fenêtre** : le logo (5238×1929) déborde largement à 100 %, même après Pack.
2. **Montrer la sélection sur la toile** (cadre, poignées) : seul le panneau indique « 1 sélectionnée ».
3. **Proposer d'enregistrer avant de fermer** une planche modifiée (`*`) : la fenêtre s'est fermée sans avertissement, et la planche est perdue.
4. **Fermer vraiment avec « Fermer NkRef »** dans le menu : l'application est restée ouverte.
5. **Supprimer (Suppr) et annuler (Ctrl+Z) pour les images**, pas seulement pour les traits *(sans effet ici, non confirmé)*.
6. **Faire défiler le panneau « Propriétés »**, coupé en bas *(molette sans effet, non confirmé)*.
7. **Donner un retour quand Pack n'a rien à ranger** : avec une seule image, rien ne change à l'écran.
8. **Afficher les libellés en entier** : « Opacite fen… » et « Epaisseur (p… » sont coupés, « Nouvelle planche (Ctrl+K) » déborde du menu.
9. **Mettre le titre à jour en même temps que le panneau** : juste après le collage, le titre dit « 0 image » et le panneau « 1 image ».
10. **Accents dans l'interface** : « Proprietes », « Reglages », « selectionnee », « Debut ».
11. **Pas d'artefact** : une tache blanche sur le bouton « Noir » quand Réglages est ouvert (vu une fois).
12. **Embarquer les images dans le fichier de planche, avec fichiers récents et sauvegarde automatique** : annoncé « À venir avec le fichier .nkref (Étape 2) ».
13. **Réassigner les raccourcis et créer ses propres couleurs** : annoncé « à venir ».

## État après la séance

- NkRef est fermé. Le presse-papiers a retrouvé son texte d'origine.
- Aucun fichier suivi par git n'a été modifié ; seuls les journaux `logs/app*.log` ont été ajoutés.