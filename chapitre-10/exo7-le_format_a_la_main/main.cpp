#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Lecteur séquentiel : chaque lecture vérifie d'abord qu'il reste assez
// d'octets. Au premier manque, le lecteur passe en échec et n'en sort plus.
class Lecteur
{
public:
    explicit Lecteur(const std::vector<std::uint8_t>& octets) : m_octets(octets) {}

    bool Echec() const { return m_echec; }

    std::uint64_t Restants() const { return m_octets.size() - m_position; }

    bool Reserver(std::uint64_t n)
    {
        if (m_echec || Restants() < n)
            m_echec = true;
        return !m_echec;
    }

    std::uint8_t U8()
    {
        if (!Reserver(1))
            return 0;
        return m_octets[m_position++];
    }

    // Petit-boutiste : le premier octet lu est le poids faible.
    std::uint32_t U32()
    {
        if (!Reserver(4))
            return 0;
        std::uint32_t valeur = 0;
        for (int i = 0; i < 4; ++i)
            valeur |= static_cast<std::uint32_t>(m_octets[m_position++]) << (8 * i);
        return valeur;
    }

    // On reconstruit les bits en entier, puis memcpy : pas de dépendance à
    // l'ordre des octets de la machine, ni d'aliasing douteux.
    float F32()
    {
        std::uint32_t bits = U32();
        float valeur = 0.0f;
        std::memcpy(&valeur, &bits, sizeof valeur);
        return valeur;
    }

    void Sauter(std::uint64_t n)
    {
        if (Reserver(n))
            m_position += static_cast<std::size_t>(n);
    }

private:
    const std::vector<std::uint8_t>& m_octets;
    std::size_t m_position = 0;
    bool m_echec = false;
};

std::string Nombre(float valeur)
{
    char texte[64];
    std::snprintf(texte, sizeof texte, "%.2f", static_cast<double>(valeur));
    std::string resultat = texte;
    if (resultat == "-0.00")
        resultat = "0.00";
    return resultat;
}

// Taille minimale d'une image (sans ses octets source) et d'un trait
// (sans ses points) : sert à refuser un compteur absurde avant de boucler.
const std::uint64_t kOctetsParImage = 5 * 4 + 4 + 3 * 4;
const std::uint64_t kOctetsParTrait = 4 + 4 + 4;
const std::uint64_t kOctetsParPoint = 2 * 4;

// Remplit la sortie et renvoie le refus éventuel (chaîne vide si accepté).
std::string Decoder(const std::vector<std::uint8_t>& octets, std::string& sortie)
{
    Lecteur lecteur(octets);
    const std::string tronque = "TRONQUE";

    char magic[4];
    for (char& c : magic)
        c = static_cast<char>(lecteur.U8());
    if (lecteur.Echec())
        return tronque;
    if (std::memcmp(magic, "NKRF", 4) != 0)
        return "PAS_UNE_PLANCHE";

    std::uint32_t version = lecteur.U32();
    if (lecteur.Echec())
        return tronque;
    if (version != 1)
        return "VERSION_INCONNUE " + std::to_string(version);

    float cx = lecteur.F32();
    float cy = lecteur.F32();
    float zoom = lecteur.F32();
    std::uint8_t theme = lecteur.U8();
    std::uint8_t grille = lecteur.U8();
    lecteur.Sauter(2);

    std::uint32_t nbImages = lecteur.U32();
    if (!lecteur.Reserver(nbImages * kOctetsParImage))
        return tronque;

    sortie += "VUE " + Nombre(cx) + ' ' + Nombre(cy) + ' ' + Nombre(zoom) + '\n';
    sortie += theme != 0 ? "THEME SOMBRE\n" : "THEME CLAIR\n";
    sortie += grille == 0 ? "GRILLE AFFICHEE\n" : "GRILLE CACHEE\n";
    sortie += "IMAGES " + std::to_string(nbImages) + '\n';

    for (std::uint32_t k = 1; k <= nbImages; ++k)
    {
        float x = lecteur.F32();
        float y = lecteur.F32();
        float echelle = lecteur.F32();
        float rotation = lecteur.F32();
        float opacite = lecteur.F32();
        int miroirX = lecteur.U8() != 0;
        int miroirY = lecteur.U8() != 0;
        lecteur.Sauter(2);
        std::uint32_t largeur = lecteur.U32();
        std::uint32_t hauteur = lecteur.U32();
        std::uint32_t taille = lecteur.U32();
        lecteur.Sauter(taille);
        if (lecteur.Echec())
            return tronque;

        sortie += "IMAGE " + std::to_string(k) + ' ' + Nombre(x) + ' ' + Nombre(y) + ' '
                + Nombre(echelle) + ' ' + Nombre(rotation) + ' ' + Nombre(opacite) + ' '
                + std::to_string(miroirX) + ' ' + std::to_string(miroirY) + ' '
                + std::to_string(largeur) + ' ' + std::to_string(hauteur) + ' '
                + std::to_string(taille) + '\n';
    }

    std::uint32_t nbTraits = lecteur.U32();
    if (!lecteur.Reserver(nbTraits * kOctetsParTrait))
        return tronque;

    sortie += "TRAITS " + std::to_string(nbTraits) + '\n';

    for (std::uint32_t k = 1; k <= nbTraits; ++k)
    {
        unsigned r = lecteur.U8();
        unsigned g = lecteur.U8();
        unsigned b = lecteur.U8();
        unsigned a = lecteur.U8();
        float epaisseur = lecteur.F32();
        std::uint32_t nbPoints = lecteur.U32();
        if (!lecteur.Reserver(nbPoints * kOctetsParPoint))
            return tronque;

        sortie += "TRAIT " + std::to_string(k) + ' ' + std::to_string(r) + ' '
                + std::to_string(g) + ' ' + std::to_string(b) + ' ' + std::to_string(a) + ' '
                + Nombre(epaisseur) + ' ' + std::to_string(nbPoints) + '\n';

        for (std::uint32_t p = 0; p < nbPoints; ++p)
        {
            float px = lecteur.F32();
            float py = lecteur.F32();
            sortie += "POINT " + Nombre(px) + ' ' + Nombre(py) + '\n';
        }
    }

    // Les octets qui suivent le dernier trait sont ignorés.
    return "";
}

int main()
{
    std::uint64_t total = 0;
    std::cin >> total;

    // Si l'entrée annonce plus d'octets qu'elle n'en donne, le fichier est
    // simplement ce qu'on a pu lire : le décodage dira TRONQUE au besoin.
    std::vector<std::uint8_t> octets;
    unsigned valeur = 0;
    while (octets.size() < total && std::cin >> std::hex >> valeur)
        octets.push_back(static_cast<std::uint8_t>(valeur));

    std::string sortie;
    std::string refus = Decoder(octets, sortie);
    if (refus.empty())
        std::cout << sortie;
    else
        std::cout << refus << '\n';
    return 0;
}
