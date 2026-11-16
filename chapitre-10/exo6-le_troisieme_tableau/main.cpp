#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using Tableau = std::vector<std::string>;

// Ce que le programme d'affichage répète, pour une opération donnée :
// le modèle bouge toujours, textures et sources seulement si elles sont citées.
struct Repetition
{
    bool textures = false;
    bool sources = false;
};

struct Planche
{
    Tableau modele;
    Tableau textures;
    Tableau sources;
};

// Une ligne de config peut ne citer aucun nom : on la lit donc en entier.
void LireConfig(Repetition& ajout, Repetition& suppression, Repetition& echange)
{
    std::string ligne;
    int lues = 0;
    while (lues < 3 && std::getline(std::cin, ligne))
    {
        if (!ligne.empty() && ligne.back() == '\r')
            ligne.pop_back();

        std::istringstream mots(ligne);
        std::string operation;
        if (!(mots >> operation))
            continue;

        Repetition* cible = nullptr;
        if (operation == "AJOUTE")
            cible = &ajout;
        else if (operation == "SUPPRIME")
            cible = &suppression;
        else if (operation == "ECHANGE")
            cible = &echange;
        else
            continue;

        std::string nom;
        while (mots >> nom)
        {
            if (nom == "textures")
                cible->textures = true;
            else if (nom == "sources")
                cible->sources = true;
        }
        ++lues;
    }
}

// Index croissants en entrée : on retire du dernier au premier, sinon les
// suivants reculent et on retire la mauvaise image.
void Retirer(Tableau& tableau, const std::vector<int>& index)
{
    for (auto it = index.rbegin(); it != index.rend(); ++it)
        if (*it >= 0 && *it < static_cast<int>(tableau.size()))
            tableau.erase(tableau.begin() + *it);
}

void Echanger(Tableau& tableau, int a, int b)
{
    int taille = static_cast<int>(tableau.size());
    if (a >= 0 && b >= 0 && a < taille && b < taille)
        std::swap(tableau[a], tableau[b]);
}

void Ajouter(Planche& planche, const Repetition& repetee, const std::string& nom)
{
    planche.modele.push_back(nom);
    if (repetee.textures)
        planche.textures.push_back(nom);
    if (repetee.sources)
        planche.sources.push_back(nom);
}

void Supprimer(Planche& planche, const Repetition& repetee, const std::vector<int>& index)
{
    Retirer(planche.modele, index);
    if (repetee.textures)
        Retirer(planche.textures, index);
    if (repetee.sources)
        Retirer(planche.sources, index);
}

// Sans voisine dans le modèle (MONTE sur la dernière, DESCEND sur la
// première), rien ne bouge nulle part.
void Deplacer(Planche& planche, const Repetition& repetee, int i, int voisine)
{
    if (voisine < 0 || voisine >= static_cast<int>(planche.modele.size()))
        return;

    Echanger(planche.modele, i, voisine);
    if (repetee.textures)
        Echanger(planche.textures, i, voisine);
    if (repetee.sources)
        Echanger(planche.sources, i, voisine);
}

const std::string& Case(const Tableau& tableau, std::size_t i)
{
    static const std::string absente = "-";
    return i < tableau.size() ? tableau[i] : absente;
}

int main()
{
    Repetition ajout, suppression, echange;
    LireConfig(ajout, suppression, echange);

    int n = 0;
    std::cin >> n;

    Planche planche;
    for (int k = 0; k < n; ++k)
    {
        std::string operation;
        std::cin >> operation;

        if (operation == "AJOUTE")
        {
            std::string nom;
            std::cin >> nom;
            Ajouter(planche, ajout, nom);
        }
        else if (operation == "SUPPRIME")
        {
            int c = 0;
            std::cin >> c;
            std::vector<int> index(c > 0 ? c : 0);
            for (int& i : index)
                std::cin >> i;
            Supprimer(planche, suppression, index);
        }
        else if (operation == "MONTE" || operation == "DESCEND")
        {
            int i = 0;
            std::cin >> i;
            int voisine = operation == "MONTE" ? i + 1 : i - 1;
            Deplacer(planche, echange, i, voisine);
        }
    }

    std::size_t longueur = std::max({planche.modele.size(), planche.textures.size(), planche.sources.size()});

    int decales = 0;
    for (std::size_t i = 0; i < longueur; ++i)
    {
        const std::string& modele = Case(planche.modele, i);
        const std::string& texture = Case(planche.textures, i);
        const std::string& source = Case(planche.sources, i);

        // "-" n'est jamais un nom : une case absente rend la ligne décalée.
        bool alignee = modele != "-" && modele == texture && modele == source;
        if (!alignee)
            ++decales;

        std::cout << i << ' ' << modele << ' ' << texture << ' ' << source << '\n';
    }

    std::cout << "DECALES " << decales << '\n';
    return 0;
}
