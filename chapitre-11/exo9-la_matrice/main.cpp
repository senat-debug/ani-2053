#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Entree
{
    std::string nom;
    int valeur = 0;
    int cout = 0;
    std::string etiquette;
    std::string genre;
    std::string cible;
    std::string raison; // vide si l'entrée est gardée
    int casePos = 0;
    int ordre = 0;
};

const char* const CASES[] = {"MAINTENANT", "DECOUPER", "EN_PASSANT", "ON_NE_LE_FAIT_PAS"};

int RangEtiquette(const std::string& etiquette)
{
    if (etiquette == "MESUREE")
        return 0;
    if (etiquette == "OBSERVEE")
        return 1;
    return 2;
}

int main()
{
    int n = 0;
    std::cin >> n;

    std::vector<Entree> entrees;
    for (int k = 0; k < n; ++k)
    {
        Entree e;
        if (!(std::cin >> e.nom >> e.valeur >> e.cout >> e.etiquette >> e.genre))
            break;
        if (e.genre == "SOCLE")
            std::cin >> e.cible;
        e.ordre = k;

        if (e.genre == "REECRITURE")
            e.raison = "reecriture";
        else if (e.genre == "CHOIX")
            e.raison = "choix";
        else if (e.etiquette == "-")
            e.raison = "sans_etiquette";
        entrees.push_back(e);
    }

    // Seule une cible MANQUE gardée prête sa valeur à son socle.
    std::map<std::string, int> manquesGardes;
    for (const Entree& e : entrees)
        if (e.raison.empty() && e.genre == "MANQUE")
            manquesGardes[e.nom] = e.valeur;

    std::vector<Entree> gardees;
    for (Entree e : entrees)
    {
        if (!e.raison.empty())
            continue;
        if (e.genre == "SOCLE")
        {
            auto cible = manquesGardes.find(e.cible);
            if (cible != manquesGardes.end())
                e.valeur = std::max(e.valeur, cible->second);
        }
        e.casePos = (e.valeur >= 3 ? 0 : 2) + (e.cout >= 3 ? 1 : 0);
        gardees.push_back(e);
    }

    std::sort(gardees.begin(), gardees.end(), [](const Entree& a, const Entree& b) {
        if (a.casePos != b.casePos)
            return a.casePos < b.casePos;
        int ra = RangEtiquette(a.etiquette), rb = RangEtiquette(b.etiquette);
        if (ra != rb)
            return ra < rb;
        if (a.valeur != b.valeur)
            return a.valeur > b.valeur;
        if (a.cout != b.cout)
            return a.cout < b.cout;
        return a.ordre < b.ordre;
    });

    bool refuse = false;
    for (const Entree& e : gardees)
    {
        std::cout << CASES[e.casePos] << ' ' << e.nom << ' ' << e.etiquette << '\n';
        if (e.casePos == 3)
            refuse = true;
    }

    for (const Entree& e : entrees)
        if (!e.raison.empty())
            std::cout << "HORS_LISTE " << e.nom << ' ' << e.raison << '\n';

    std::cout << "VERDICT " << (refuse ? "HONNETE" : "COMPLAISANT") << '\n';
    return 0;
}
