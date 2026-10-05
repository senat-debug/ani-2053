// main.cpp — exercice 6 : evenement ou interrogation.
// Les deux hauteurs ne servent pas aux memes choses. L'evenement convient a ce
// qui ARRIVE : il est date, et le systeme le repete quand on tient la touche,
// d'ou le saut qui se declenche trois fois. L'interrogation convient a ce qui
// DURE : un booleen par image, sans histoire - elle ne voit donc jamais un
// appui relache avant la fin de l'image.
#include <iostream>
#include <string>

int main() {
	long long v = 0, images = 0;
	long long xe = 0, xi = 0; // le carre des evenements, celui de l'interrogation
	long long sautsEvt = 0, sautsInt = 0, manques = 0;
	bool space = false, left = false, right = false;

	if (std::cin >> v >> images) {
		for (long long i = 1; i <= images; ++i) {
			long long k = 0;
			if (!(std::cin >> k)) {
				break;
			}

			long long appuisSpace = 0; // les +SPACE recus pendant CETTE image

			for (long long j = 0; j < k; ++j) {
				std::string e;
				if (!(std::cin >> e)) {
					break;
				}
				if (e.size() < 2 || (e[0] != '+' && e[0] != '-')) {
					continue;
				}

				const bool enfonce = (e[0] == '+');
				const std::string nom = e.substr(1);

				// L'etat se met a jour AVANT l'interrogation, qui vient apres
				// la boucle. Un '-' ne fait bouger personne.
				if (nom == "SPACE") {
					if (enfonce) {
						++sautsEvt; // chaque +SPACE compte, repetitions comprises
						++appuisSpace;
					}
					space = enfonce;
				} else if (nom == "RIGHT") {
					if (enfonce) {
						xe += v;
					}
					right = enfonce;
				} else if (nom == "LEFT") {
					if (enfonce) {
						xe -= v;
					}
					left = enfonce;
				}
				// tout autre nom est ignore, etat compris
			}

			// L'interrogation : UNE SEULE FOIS par image, apres les evenements.
			if (space) {
				++sautsInt;
			}
			if (right) {
				xi += v;
			}
			if (left) {
				xi -= v;
			}

			// Les appuis que l'interrogation n'a jamais vus : la touche a ete
			// relachee avant la fin de l'image.
			if (!space) {
				manques += appuisSpace;
			}

			std::cout << i << ' ' << xe << ' ' << xi << '\n';
		}
	}

	std::cout << "SAUTS EVENEMENTS " << sautsEvt << '\n';
	std::cout << "SAUTS INTERROGATION " << sautsInt << '\n';
	std::cout << "MANQUES " << manques << '\n';
	return 0;
}