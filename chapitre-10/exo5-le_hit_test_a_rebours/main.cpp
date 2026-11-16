#include <cmath>
#include <iostream>
#include <string>
#include <vector>

struct Item
{
    std::string nom;
    double x = 0;
    double y = 0;
    double largeur = 0; // taille native, en pixels
    double hauteur = 0;
    double echelle = 1;
};

// Le bord compte : on teste avec <=, comme Contains dans NkRefBoard.
bool Contient(const Item& image, double px, double py)
{
    double demiLargeur = image.largeur * image.echelle / 2;
    double demiHauteur = image.hauteur * image.echelle / 2;
    return std::fabs(px - image.x) <= demiLargeur && std::fabs(py - image.y) <= demiHauteur;
}

// La dernière image du tableau est dessinée par-dessus les autres : le clic
// doit donc la tester en premier.
int HitTest(const std::vector<Item>& planche, double px, double py)
{
    for (int i = static_cast<int>(planche.size()) - 1; i >= 0; --i)
        if (Contient(planche[i], px, py))
            return i;
    return -1;
}

// La recherche fautive, celle qui part du fond.
int HitTestALEndroit(const std::vector<Item>& planche, double px, double py)
{
    for (int i = 0; i < static_cast<int>(planche.size()); ++i)
        if (Contient(planche[i], px, py))
            return i;
    return -1;
}

std::string NomOuAucun(const std::vector<Item>& planche, int index)
{
    return index < 0 ? "AUCUN" : planche[index].nom;
}

int main()
{
    int n = 0;
    std::cin >> n;

    std::vector<Item> planche(n > 0 ? n : 0);
    for (Item& image : planche)
        std::cin >> image.nom >> image.x >> image.y >> image.largeur >> image.hauteur >> image.echelle;

    int q = 0;
    std::cin >> q;

    int erreurs = 0;
    for (int k = 1; k <= q; ++k)
    {
        double px = 0, py = 0;
        std::cin >> px >> py;

        int dessus = HitTest(planche, px, py);
        int dessous = HitTestALEndroit(planche, px, py);
        if (dessus != dessous)
            ++erreurs;

        std::cout << "CLIC " << k << ' ' << NomOuAucun(planche, dessus) << ' '
                  << NomOuAucun(planche, dessous) << '\n';
    }

    std::cout << "ERREURS " << erreurs << '\n';
    return 0;
}
