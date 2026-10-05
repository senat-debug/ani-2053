// main.cpp — exercice 5 : le cercle qui n'en est pas un.
// Un cercle dessine est un polygone. Au milieu de chaque segment, le polygone
// passe plus pres du centre que le vrai cercle : c'est cet ecart qui se voit
// des qu'on zoome. Trente-deux segments sont invisibles a taille normale et
// grossiers a deux cents pour cent.
#include <cmath>
#include <iostream>

int main() {
	const double pi = 3.141592653589793;

	long long nb = 0;
	long long visibles = 0, refuses = 0;

	if (std::cin >> nb) {
		for (long long i = 0; i < nb; ++i) {
			long long r = 0, n = 0;
			if (!(std::cin >> r >> n)) {
				break;
			}

			// Moins de trois segments ne font pas un cercle.
			if (n < 3) {
				std::cout << r << ' ' << n << " REFUSE\n";
				++refuses;
				continue;
			}

			// cos attend des RADIANS, et pi / n en est deja.
			const double g = static_cast<double>(r) * (1.0 - std::cos(pi / static_cast<double>(n)));
			const long long ecart = static_cast<long long>(std::floor(g * 1000.0));

			// Rayon nul : aucun agrandissement ne montre les segments, et
			// surtout on ne divise pas par zero.
			if (g <= 0.0) {
				std::cout << r << ' ' << n << ' ' << ecart << " JAMAIS\n";
				continue;
			}

			// L'ecart s'arrondit vers le BAS, le zoom vers le HAUT : les deux
			// arrondis ne vont pas dans le meme sens.
			const long long zoom = static_cast<long long>(std::ceil(100.0 / g));
			const bool visible = (zoom <= 100);
			if (visible) {
				++visibles;
			}

			std::cout << r << ' ' << n << ' ' << ecart << ' ' << zoom << ' '
					  << (visible ? "VISIBLE" : "INVISIBLE") << '\n';
		}
	}

	std::cout << "VISIBLES " << visibles << '\n';
	std::cout << "REFUSES " << refuses << '\n';
	return 0;
}