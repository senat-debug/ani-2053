#include <iostream>
#include <string>
#include <vector>

// Une image de la planche, telle qu'elle est lue sur l'entrée.
// Les champs que le fichier .nkref écrit sont gardés en texte : ils
// reviennent identiques, on n'a jamais besoin de calculer avec.
struct Image
{
    unsigned long long id = 0;
    std::string x, y, echelle, rotation, opacite;
    std::string miroirX, miroirY, largeur, hauteur;
    int selectionne = 0;
    std::string chemin;
    unsigned long long octets = 0;
};

std::istream &operator>>(std::istream &entree, Image &image)
{
    return entree >> image.id >> image.x >> image.y >> image.echelle >> image.rotation
                  >> image.opacite >> image.miroirX >> image.miroirY >> image.largeur
                  >> image.hauteur >> image.selectionne >> image.chemin >> image.octets;
}

// Les champs que le fichier n'écrit pas, et qui reviennent donc à leur
// valeur par défaut : id à 0, sélection à 0, chemin vide.
std::vector<std::string> ChampsChanges(const Image &image)
{
    std::vector<std::string> champs;
    if (image.id != 0)
        champs.push_back("id");
    if (image.selectionne == 1)
        champs.push_back("selected");
    if (image.chemin != "-")
        champs.push_back("sourcePath");
    return champs;
}

int main()
{
    int n = 0;
    std::cin >> n;

    int restent = 0;
    int changes = 0;

    for (int k = 1; k <= n; ++k)
    {
        Image image;
        std::cin >> image;

        // Sans octets, l'image ne se décode pas à la réouverture : elle
        // disparaît, et les suivantes prennent sa place.
        if (image.octets == 0)
        {
            std::cout << "ITEM " << k << " SAUTE\n";
            continue;
        }

        ++restent;
        std::vector<std::string> champs = ChampsChanges(image);
        changes += static_cast<int>(champs.size());

        std::cout << "ITEM " << k << ' ' << restent;
        if (champs.empty())
            std::cout << " AUCUN";
        for (const std::string &champ : champs)
            std::cout << ' ' << champ;
        std::cout << '\n';
    }

    std::cout << "RESTENT " << restent << '\n';
    std::cout << "CHANGES " << changes << '\n';
    return 0;
}
