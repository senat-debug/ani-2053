#include "NkRef/NkRefFile.h"

#include <cstdio>

using nkref::NkRefBoard;
using nkref::NkRefFileImage;
using nkref::NkRefLoadedItem;
using nkref::NkRefStroke;
using nkref::NkRefView;
using nkref::NkVector;
using nkref::uint8;
using nkref::usize;

namespace {

	// Des octets « source » inventés : le format les recopie sans les décoder,
	// il n'a pas besoin d'un vrai PNG pour être testé.
	const uint8 kSourceA[] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A};
	const uint8 kSourceB[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x42};

	// Chaque champ écrit vaut autre chose que sa valeur par défaut : un champ
	// laissé à 0 ou à 1 se relirait pareil même s'il était mal écrit.
	void ConstruirePlanche(NkRefBoard &planche, NkVector<NkRefFileImage> &images, NkRefView &vue) {
		planche.AddItem({-120.0f, 45.5f}, 640, 480, "a.png");
		planche.items[0].scale = 0.75f;
		planche.items[0].rotationDeg = 30.0f;
		planche.items[0].opacity = 0.8f;
		planche.items[0].mirrorX = true;
		images.PushBack({kSourceA, sizeof(kSourceA)});

		planche.AddItem({310.25f, -72.0f}, 200, 300, "b.jpg");
		planche.items[1].scale = 1.5f;
		planche.items[1].rotationDeg = -12.5f;
		planche.items[1].mirrorY = true;
		images.PushBack({kSourceB, sizeof(kSourceB)});

		planche.BeginStroke(30, 144, 255, 4.5f, {-50.0f, -50.0f});
		planche.AppendStrokePoint({20.0f, 10.0f}, 1.0f);
		planche.AppendStrokePoint({95.0f, -35.0f}, 1.0f);
		planche.strokes[0].a = 200;

		vue.ZoomAtPixel(2.5f, {620.0f, 140.0f}, {800.0f, 600.0f});
		vue.PanByPixels(-75.0f, 40.0f);
	}

	// Remonte une planche à partir de ce que NkRefDeserialize a rendu, comme le
	// ferait la glue après une ouverture.
	void Reconstruire(NkVector<NkRefLoadedItem> &lus, NkVector<NkRefStroke> &traits, NkRefBoard &planche,
					  NkVector<NkRefFileImage> &images) {
		for (usize i = 0; i < lus.Size(); ++i) {
			planche.items.PushBack(lus[i].item);
			images.PushBack({lus[i].src.Data(), lus[i].src.Size()});
		}
		planche.strokes = traits;
	}

	// Rend l'indice du premier octet qui diffère, ou -1 si les deux suites sont identiques.
	long PremierEcart(const NkVector<uint8> &a, const NkVector<uint8> &b) {
		const usize n = a.Size() < b.Size() ? a.Size() : b.Size();
		for (usize i = 0; i < n; ++i)
			if (a[i] != b[i])
				return (long)i;
		return a.Size() == b.Size() ? -1 : (long)n;
	}

} // namespace

int main() {
	NkRefBoard planche;
	NkVector<NkRefFileImage> images;
	NkRefView vue;
	ConstruirePlanche(planche, images, vue);

	NkVector<uint8> premiere;
	nkref::NkRefSerialize(premiere, planche, images.Data(), vue, true, false);

	NkVector<NkRefLoadedItem> lus;
	NkVector<NkRefStroke> traits;
	NkRefView vueLue;
	bool sombreLu = false, grilleLue = true;
	const bool relu = nkref::NkRefDeserialize(premiere.Data(), premiere.Size(), lus, traits, vueLue, sombreLu, grilleLue);

	long ecart = 0;
	if (relu) {
		NkRefBoard plancheRelue;
		NkVector<NkRefFileImage> imagesRelues;
		Reconstruire(lus, traits, plancheRelue, imagesRelues);

		NkVector<uint8> seconde;
		nkref::NkRefSerialize(seconde, plancheRelue, imagesRelues.Data(), vueLue, sombreLu, grilleLue);
		ecart = PremierEcart(premiere, seconde);
	}

	std::printf("octets : %zu, relu : %s, identiques : %s\n", (size_t)premiere.Size(), relu ? "OUI" : "NON",
				relu && ecart < 0 ? "OUI" : "NON");
	if (relu && ecart >= 0)
		std::printf("premier octet different : %ld\n", ecart);
	return relu && ecart < 0 ? 0 : 1;
}
