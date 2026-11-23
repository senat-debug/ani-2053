#include <iostream>
#include <set>
#include <string>
#include <vector>

// Chaque liste arrive sous la forme « MOT n » suivie de n clés.
std::vector<std::string> LireListe(std::istream& entree)
{
    std::string mot;
    int n = 0;
    entree >> mot >> n;

    std::vector<std::string> cles(n > 0 ? n : 0);
    for (std::string& cle : cles)
        entree >> cle;
    return cles;
}

int main()
{
    std::vector<std::string> avant = LireListe(std::cin);
    std::vector<std::string> corrige = LireListe(std::cin);
    std::vector<std::string> apres = LireListe(std::cin);

    std::set<std::string> dansAvant(avant.begin(), avant.end());
    std::set<std::string> dansCorrige(corrige.begin(), corrige.end());
    std::set<std::string> dansApres(apres.begin(), apres.end());

    int vraies = 0, fausses = 0, oubliees = 0;
    for (std::size_t k = 0; k < avant.size(); ++k)
    {
        const std::string& cle = avant[k];
        std::string categorie;

        // La correction passe avant : une clé corrigée reste fausse même
        // si on l'a recopiée dans la liste d'aujourd'hui.
        if (dansCorrige.count(cle))
        {
            categorie = "DEVENUE_FAUSSE";
            ++fausses;
        }
        else if (dansApres.count(cle))
        {
            categorie = "ENCORE_VRAIE";
            ++vraies;
        }
        else
        {
            categorie = "OUBLIEE";
            ++oubliees;
        }

        std::cout << k + 1 << ' ' << cle << ' ' << categorie << '\n';
    }

    int nouvelles = 0;
    for (const std::string& cle : apres)
        if (!dansAvant.count(cle))
            ++nouvelles;

    int partOubliee = vraies + oubliees == 0 ? 0 : 100 * oubliees / (vraies + oubliees);

    std::cout << "VRAIES " << vraies << '\n';
    std::cout << "FAUSSES " << fausses << '\n';
    std::cout << "OUBLIEES " << oubliees << '\n';
    std::cout << "NOUVELLES " << nouvelles << '\n';
    std::cout << "PART_OUBLIEE " << partOubliee << '\n';
    return 0;
}
