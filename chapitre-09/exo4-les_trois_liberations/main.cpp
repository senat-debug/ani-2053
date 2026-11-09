#include <iostream>
#include <string>
#include <vector>

// Libère `fois` fois la texture. Renvoie false si l'une de ces libérations
// tombe sur une texture déjà libérée : le programme meurt à cet instant.
bool Liberer(int texture, int fois, std::vector<bool> &liberee)
{
    for (int f = 0; f < fois; ++f)
    {
        if (liberee[texture])
            return false;
        liberee[texture] = true;
    }
    return true;
}

// Libère chaque texture du tableau, de la première à la dernière.
// Renvoie la texture qui fait planter, ou 0 si tout s'est bien passé.
int LibererTout(const std::vector<int> &tableau, int fois, std::vector<bool> &liberee)
{
    for (int texture : tableau)
        if (!Liberer(texture, fois, liberee))
            return texture;
    return 0;
}

int main()
{
    int s = 0;
    int v = 0;
    int f = 0;
    int n = 0;
    std::cin >> s >> v >> f >> n;

    // Le tableau contient des numéros de texture, pas des index :
    // liberee[t] dit si la texture t a déjà été rendue.
    std::vector<int> tableau;
    std::vector<bool> liberee(1, false);
    int creees = 0;

    for (int k = 1; k <= n; ++k)
    {
        std::string operation;
        std::cin >> operation;

        int plantage = 0;
        if (operation == "AJOUTE")
        {
            ++creees;
            tableau.push_back(creees);
            liberee.push_back(false);
        }
        else if (operation == "SUPPRIME")
        {
            int i = 0;
            std::cin >> i;
            int texture = tableau[i];
            if (!Liberer(texture, s, liberee))
                plantage = texture;
            tableau.erase(tableau.begin() + i);
        }
        else if (operation == "VIDE")
        {
            plantage = LibererTout(tableau, v, liberee);
            tableau.clear();
        }

        if (plantage != 0)
        {
            std::cout << "PLANTAGE " << k << " TEXTURE " << plantage << '\n';
            return 0;
        }
    }

    int plantage = LibererTout(tableau, f, liberee);
    if (plantage != 0)
    {
        std::cout << "PLANTAGE FIN TEXTURE " << plantage << '\n';
        return 0;
    }

    std::vector<int> perdues;
    for (int t = 1; t <= creees; ++t)
        if (!liberee[t])
            perdues.push_back(t);

    std::cout << "CREEES " << creees << '\n';
    std::cout << "FUITES " << perdues.size() << '\n';
    std::cout << "PERDUES";
    if (perdues.empty())
        std::cout << " AUCUN";
    for (int t : perdues)
        std::cout << ' ' << t;
    std::cout << '\n';
    return 0;
}
