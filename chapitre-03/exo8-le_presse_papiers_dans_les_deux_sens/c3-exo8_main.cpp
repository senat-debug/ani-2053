// PressePapiers — exercice 8 du chapitre 3.
// 1) lit le texte du presse-papiers, le met en majuscules, le remet ;
// 2) lit l'image du presse-papiers, inverse ses COULEURS (pas l'alpha), la remet.
// Tout passe par NkWindow (NkWindow.h:156-157 et 221-243), jamais par l'API
// du systeme. Releve dans presse-papiers.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

#include <cstdio>

using namespace nkentseu;

// Majuscules ASCII seulement : le reste (accents en UTF-8, emoji) est laisse
// tel quel, faute de quoi on casserait les octets d'un caractere multi-octets.
static NkString EnMajuscules(const NkString &s) {
	NkString out = s;
	for (usize i = 0; i < out.Size(); ++i) {
		char c = out[i];
		if (c >= 'a' && c <= 'z')
			out[i] = (char)(c - 'a' + 'A');
	}
	return out;
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Presse-papiers";
	cfg.width = 480;
	cfg.height = 200;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("presse-papiers.txt", "w");

	// ---------------------------------------------------------------- TEXTE
	const NkString avant = window.GetClipboardText();
	std::fprintf(f, "== TEXTE ==\n");
	std::fprintf(f, "avant  (%u octets) : \"%s\"\n", (unsigned)avant.Size(), avant.CStr());

	if (avant.Size() == 0) {
		std::fprintf(f, "presse-papiers texte vide : rien a faire\n\n");
	} else {
		const NkString majuscules = EnMajuscules(avant);
		window.SetClipboardText(majuscules);
		const NkString relu = window.GetClipboardText();
		std::fprintf(f, "ecrit  (%u octets) : \"%s\"\n", (unsigned)majuscules.Size(), majuscules.CStr());
		std::fprintf(f, "relu   (%u octets) : \"%s\"\n", (unsigned)relu.Size(), relu.CStr());
		std::fprintf(f, "aller-retour identique : %s\n\n", (relu == majuscules) ? "oui" : "NON");
	}

	// ---------------------------------------------------------------- IMAGE
	std::fprintf(f, "== IMAGE ==\n");
	std::fprintf(f, "HasClipboardImage() : %s\n", window.HasClipboardImage() ? "oui" : "non");

	NkClipboardImage img;
	if (!window.GetClipboardImage(img)) {
		std::fprintf(f, "GetClipboardImage() a echoue : pas d'image exploitable\n");
	} else {
		const usize n = img.pixels.Size();
		std::fprintf(f, "lue    : %u x %u, %u octets, %u bits par pixel\n",
			img.width, img.height, (unsigned)n,
			img.height && img.width ? (unsigned)(n * 8 / ((usize)img.width * img.height)) : 0u);
		std::fprintf(f, "premier pixel lu   : R=%u V=%u B=%u A=%u\n",
			img.pixels[0], img.pixels[1], img.pixels[2], img.pixels[3]);

		// Inversion des composantes de couleur ; l'alpha reste tel quel.
		for (usize i = 0; i + 3 < n; i += 4) {
			img.pixels[i + 0] = (uint8)(255 - img.pixels[i + 0]);
			img.pixels[i + 1] = (uint8)(255 - img.pixels[i + 1]);
			img.pixels[i + 2] = (uint8)(255 - img.pixels[i + 2]);
			// img.pixels[i + 3] : alpha inchange
		}
		std::fprintf(f, "premier pixel ecrit: R=%u V=%u B=%u A=%u\n",
			img.pixels[0], img.pixels[1], img.pixels[2], img.pixels[3]);

		const bool pose = window.SetClipboardImage(img);
		std::fprintf(f, "SetClipboardImage() : %s\n", pose ? "oui" : "NON");

		NkClipboardImage relue;
		if (window.GetClipboardImage(relue)) {
			std::fprintf(f, "relue  : %u x %u, %u octets\n",
				relue.width, relue.height, (unsigned)relue.pixels.Size());
			std::fprintf(f, "premier pixel relu : R=%u V=%u B=%u A=%u\n",
				relue.pixels[0], relue.pixels[1], relue.pixels[2], relue.pixels[3]);

			usize differents = 0;
			const usize m = relue.pixels.Size() < n ? relue.pixels.Size() : n;
			for (usize i = 0; i < m; ++i)
				if (relue.pixels[i] != img.pixels[i]) ++differents;
			std::fprintf(f, "octets differents apres aller-retour : %u sur %u\n",
				(unsigned)differents, (unsigned)m);
		} else {
			std::fprintf(f, "relecture impossible\n");
		}
	}

	std::fclose(f);
	return 0;
}