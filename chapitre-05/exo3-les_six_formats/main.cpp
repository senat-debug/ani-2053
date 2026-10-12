#include <iostream>
#include <string>

struct Format {
    const char *nom;
    long long octets;
    bool couleur;
    bool transparence;
    bool flottants;
};

const Format FORMATS[6] = {
    {"GRAY8", 1, false, false, false},
    {"GRAY_A16", 2, false, true, false},
    {"RGB24", 3, true, false, false},
    {"RGBA32", 4, true, true, false},
    {"RGB96F", 12, true, false, true},
    {"RGBA128F", 16, true, true, true},
};

const Format *chercher(const std::string &nom) {
    for (int i = 0; i < 6; i++) {
        if (nom == FORMATS[i].nom) return &FORMATS[i];
    }
    return nullptr;
}

int main() {
    long long w = 0, h = 0;
    int n = 0;
    std::cin >> w >> h >> n;

    long long total = 0;
    int sansPerte = 0, refuses = 0;

    for (int i = 0; i < n; i++) {
        std::string src, dst;
        std::cin >> src >> dst;
        const Format *a = chercher(src);
        const Format *b = chercher(dst);

        if (a == nullptr || b == nullptr) {
            std::cout << src << " " << dst << " REFUSE\n";
            refuses++;
            continue;
        }

        long long octetsSrc = w * h * a->octets;
        long long octetsDst = w * h * b->octets;

        std::string pertes;
        if (a->transparence && !b->transparence) pertes += "TRANSPARENCE";
        if (a->couleur && !b->couleur) pertes += (pertes.empty() ? "" : "+") + std::string("COULEUR");
        if (a->flottants && !b->flottants) pertes += (pertes.empty() ? "" : "+") + std::string("ETENDUE");
        if (pertes.empty()) {
            pertes = "AUCUNE";
            sansPerte++;
        }

        total += octetsDst;
        std::cout << src << " " << dst << " " << octetsSrc << " " << octetsDst << " " << pertes << "\n";
    }

    std::cout << "TOTAL " << total << "\n";
    std::cout << "SANS_PERTE " << sansPerte << "\n";
    std::cout << "REFUSES " << refuses << "\n";
    return 0;
}

