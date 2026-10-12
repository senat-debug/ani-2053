#include <iostream>
#include <string>

int main() {
    long long seuil = 0;
    int n = 0;
    std::cin >> seuil >> n;

    long long memoire = 0;
    int flux = 0, refuses = 0;

    for (int i = 0; i < n; i++) {
        std::string nom;
        long long frequence, canaux, bits, duree, fichier;
        std::cin >> nom >> frequence >> canaux >> bits >> duree >> fichier;

        if (bits != 8 && bits != 16 && bits != 24 && bits != 32) {
            std::cout << nom << " REFUSE\n";
            refuses++;
            continue;
        }

        // on multiplie tout avant de diviser par 1000, sinon on perd les millisecondes
        long long brut = frequence * canaux * (bits / 8) * duree / 1000;
        long long pourcent = fichier * 100 / brut;

        if (brut > seuil) {
            std::cout << nom << " " << brut << " " << pourcent << " FLUX\n";
            flux++;
        } else {
            std::cout << nom << " " << brut << " " << pourcent << " MEMOIRE\n";
            memoire += brut;
        }
    }

    std::cout << "MEMOIRE " << memoire << "\n";
    std::cout << "FLUX " << flux << "\n";
    std::cout << "REFUSES " << refuses << "\n";
    return 0;
}
