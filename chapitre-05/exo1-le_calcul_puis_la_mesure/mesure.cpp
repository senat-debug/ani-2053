// Exercice 1 : le calcul, puis la mesure
#include "NKImage/Core/NkImage.h"
#include "NKLogger/NkLog.h"
#include <cstdio>

using namespace nkentseu;

int main() {
	const char *fichiers[5] = {
		"images/icone.png",
		"images/capture.png",
		"images/dessin.png",
		"images/photo1.jpg",
		"images/photo2.jpg"
	};

	for (int i = 0; i < 5; i++) {
		logger.Info("Avant {0} : noter la memoire du processus puis Entree", fichiers[i]);
		getchar();

		NkImage img;
		if (!img.Load(fichiers[i])) {
			logger.Error("impossible de charger {0}", fichiers[i]);
			continue;
		}

		long long calcul = (long long)img.Width() * img.Height() * img.BytesPP();
		logger.Info("{0} : largeur {1}, hauteur {2}, BytesPP {3}, calcul {4} octets",
		            fichiers[i], img.Width(), img.Height(), img.BytesPP(), calcul);

		// l'image est encore en vie ici, on mesure avant la fin de la boucle
		logger.Info("Apres {0} : noter la memoire du processus puis Entree", fichiers[i]);
		getchar();
	} // img sort de sa portee ici et libere ses pixels

	return 0;
}
