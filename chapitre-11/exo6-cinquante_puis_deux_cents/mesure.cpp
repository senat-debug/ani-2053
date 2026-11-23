#include "NkRef/NkRefFile.h"
#include "NKTime/NkChrono.h"

#include <cstdio>
#include <cstdlib>

using nkentseu::NkChrono;
using nkref::NkRefBoard;
using nkref::NkRefFileImage;
using nkref::NkRefLoadedItem;
using nkref::NkRefStroke;
using nkref::NkRefView;
using nkref::NkVector;
using nkref::uint8;
using nkref::usize;

namespace {

	constexpr int kAppels = 20;
	constexpr usize kOctetsSource = 1000;

	// Des rangées de dix images de 100 x 80, espacées de 20 : la position
	// ne change rien au nombre d'octets, elle évite juste un empilement.
	void ConstruirePlanche(NkRefBoard &planche, int nombreImages) {
		for (int i = 0; i < nombreImages; ++i) {
			const float x = (i % 10) * 120.0f;
			const float y = (i / 10) * 100.0f;
			planche.AddItem({x, y}, 100, 80, "");
		}
	}

} // namespace

int main(int argc, char **argv) {
	const long nombreImages = argc > 1 ? std::strtol(argv[1], nullptr, 10) : 0;
	if (nombreImages < 1 || nombreImages > 100000) {
		std::fprintf(stderr, "usage : mesure N (1 a 100000 images)\n");
		return 1;
	}

	// Les mêmes 1000 octets inventés pour toutes les images : le format les
	// recopie sans les décoder.
	static uint8 source[kOctetsSource];
	for (usize i = 0; i < kOctetsSource; ++i)
		source[i] = (uint8)(i * 31u + 7u);

	NkRefBoard planche;
	ConstruirePlanche(planche, (int)nombreImages);
	NkVector<NkRefFileImage> images;
	for (long i = 0; i < nombreImages; ++i)
		images.PushBack({source, kOctetsSource});
	const NkRefView vue;

	NkVector<uint8> octets;
	NkChrono chronoEcrire;
	for (int i = 0; i < kAppels; ++i)
		nkref::NkRefSerialize(octets, planche, images.Data(), vue, false);
	const double ecrire = chronoEcrire.Elapsed().milliseconds / kAppels;

	NkVector<NkRefLoadedItem> lus;
	NkVector<NkRefStroke> traits;
	NkRefView vueLue;
	bool sombre = false;
	bool grille = true;
	bool relu = true;
	NkChrono chronoRelire;
	for (int i = 0; i < kAppels; ++i)
		relu = nkref::NkRefDeserialize(octets.Data(), octets.Size(), lus, traits, vueLue, sombre, grille) && relu;
	const double relire = chronoRelire.Elapsed().milliseconds / kAppels;

	if (!relu || lus.Size() != (usize)nombreImages) {
		std::fprintf(stderr, "relecture refusee\n");
		return 1;
	}

	std::printf("images : %ld, octets : %zu, ecrire : %.3f ms, relire : %.3f ms\n", nombreImages,
				(size_t)octets.Size(), ecrire, relire);
	return 0;
}
