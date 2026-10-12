#include "NKFont/NkFont.h"
#include "NKImage/Core/NkImage.h"
#include <cstdio>

using namespace nkentseu;

// construit l'atlas pour une taille, affiche les chiffres
// et sauvegarde l'image seulement si nomPng n'est pas nul
static void faireAtlas(const char *police, float taille, const char *nomPng) {
    NkFontAtlas atlas;
    NkFont *font = atlas.AddFontFromFile(police, taille);
    if (font == nullptr || !atlas.Build()) {
        printf("erreur : impossible de charger %s\n", police);
        return;
    }

    nkft_uint8 *pixels = nullptr;
    nkft_int32 largeur = 0, hauteur = 0;
    atlas.GetTexDataAsRGBA32(&pixels, &largeur, &hauteur);

    // pixels perdus = pixels ou alpha vaut 0 (rien de dessine)
    long long total = (long long)largeur * hauteur;
    long long vides = 0;
    for (long long i = 0; i < total; i++) {
        if (pixels[i * 4 + 3] == 0) vides++;
    }
    long long perdu = total > 0 ? vides * 100 / total : 0;

    printf("taille : %d, glyphes : %u, texture : %d %d, perdu : %lld\n",
           (int)taille, font->glyphCount, largeur, hauteur, perdu);

    if (nomPng != nullptr) {
        // Wrap ne copie rien, l'atlas doit rester vivant jusqu'a SavePNG
        NkImage img = NkImage::Wrap(pixels, largeur, hauteur, NkImagePixelFormat::NK_RGBA32);
        if (!img.SavePNG(nomPng)) printf("erreur : sauvegarde de %s\n", nomPng);
    }
}

int main(int argc, char **argv) {
    const char *police = argc > 1 ? argv[1] : "police.ttf";
    faireAtlas(police, 18.f, "atlas.png");
    faireAtlas(police, 48.f, nullptr);
    return 0;
}
