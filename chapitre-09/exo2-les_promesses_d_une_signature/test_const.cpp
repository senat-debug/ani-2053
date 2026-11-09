#include "NkRef/NkRefView.h"

#include <cstdio>

int main() {
	const nkref::NkRefView vue{};
	const nkref::NkVec2f viewport{800.0f, 600.0f};

	// Le centre du viewport retombe sur le centre monde (0, 0).
	const nkref::NkVec2f monde = vue.PixelToWorld({400.0f, 300.0f}, viewport);
	std::printf("PixelToWorld(400, 300) = (%g, %g)\n", monde.x, monde.y);
	std::printf("GridSpacing(24) = %g\n", vue.GridSpacing(24.0f));

	// PanByPixels n'est pas const : le compilateur refuse l'appel sur une vue const.
	// vue.PanByPixels(1.0f, 0.0f);
	return 0;
}
