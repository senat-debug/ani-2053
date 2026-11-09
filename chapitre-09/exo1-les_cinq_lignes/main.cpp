#include <iostream>
#include <string>

enum Membre { DESTRUCTEUR, COPIE, AFFECTATION_COPIE, DEPLACEMENT, AFFECTATION_DEPLACEMENT, NB_MEMBRES, AUCUN = -1 };
enum Etat { ABSENT, ECRIT, DEFAUT, SUPPRIME };

// Un seul tableau pour les noms : l'ordre d'affichage suit celui de l'enum.
const char *const NOMS_MEMBRES[NB_MEMBRES] = {
    "DESTRUCTEUR", "COPIE", "AFFECTATION_COPIE", "DEPLACEMENT", "AFFECTATION_DEPLACEMENT"};
const char *const NOMS_ETATS[] = {"ABSENT", "ECRIT", "DEFAUT", "SUPPRIME"};

std::string Nettoyer(const std::string &ligne)
{
    std::string code = ligne.substr(0, ligne.find("//"));
    std::string propre;
    for (char c : code)
        if (c != ' ' && c != '\t' && c != '\r')
            propre += c;
    return propre;
}

bool CommencePar(const std::string &texte, const std::string &debut)
{
    return texte.compare(0, debut.size(), debut) == 0;
}

bool FinitPar(const std::string &texte, const std::string &fin)
{
    return texte.size() >= fin.size() && texte.compare(texte.size() - fin.size(), fin.size(), fin) == 0;
}

// Vrai si la ligne commence par le motif et que le caractère suivant n'est pas
// un second '&' : C(const C&&) n'est pas une copie.
bool CommenceParReferenceSimple(const std::string &ligne, const std::string &motif)
{
    return CommencePar(ligne, motif) && (ligne.size() == motif.size() || ligne[motif.size()] != '&');
}

Membre Reconnaitre(const std::string &ligne, const std::string &classe)
{
    if (ligne.find("~" + classe + "(") != std::string::npos)
        return DESTRUCTEUR;
    if (CommenceParReferenceSimple(ligne, classe + "(const" + classe + "&"))
        return COPIE;
    if (CommenceParReferenceSimple(ligne, classe + "&operator=(const" + classe + "&"))
        return AFFECTATION_COPIE;
    if (CommencePar(ligne, classe + "(" + classe + "&&"))
        return DEPLACEMENT;
    if (CommencePar(ligne, classe + "&operator=(" + classe + "&&"))
        return AFFECTATION_DEPLACEMENT;
    return AUCUN;
}

Etat LireEtat(const std::string &ligne)
{
    if (FinitPar(ligne, "=delete;"))
        return SUPPRIME;
    if (FinitPar(ligne, "=default;"))
        return DEFAUT;
    return ECRIT;
}

int main()
{
    std::string ligne;
    std::getline(std::cin, ligne);
    const std::string classe = Nettoyer(ligne);

    Etat etats[NB_MEMBRES] = {ABSENT, ABSENT, ABSENT, ABSENT, ABSENT};
    while (std::getline(std::cin, ligne))
    {
        const std::string code = Nettoyer(ligne);
        if (code.empty())
            continue;
        const Membre membre = Reconnaitre(code, classe);
        if (membre != AUCUN)
            etats[membre] = LireEtat(code);
    }

    int nbDeclares = 0;
    for (int m = 0; m < NB_MEMBRES; ++m)
    {
        std::cout << NOMS_MEMBRES[m] << ' ' << NOMS_ETATS[etats[m]] << '\n';
        if (etats[m] != ABSENT)
            ++nbDeclares;
    }

    const bool unDeplacement = etats[DEPLACEMENT] != ABSENT || etats[AFFECTATION_DEPLACEMENT] != ABSENT;
    const bool lesTroisDeCopie = etats[DESTRUCTEUR] != ABSENT && etats[COPIE] != ABSENT && etats[AFFECTATION_COPIE] != ABSENT;

    if (nbDeclares == 0)
        std::cout << "REGLE ZERO\n";
    else if (nbDeclares == NB_MEMBRES)
        std::cout << "REGLE CINQ\n";
    else if (lesTroisDeCopie && !unDeplacement)
        std::cout << "REGLE TROIS\n";
    else
    {
        std::cout << "REGLE INCOMPLETE\n";
        const int dernier = unDeplacement ? NB_MEMBRES : DEPLACEMENT;
        for (int m = 0; m < dernier; ++m)
            if (etats[m] == ABSENT)
                std::cout << "MANQUE " << NOMS_MEMBRES[m] << '\n';
    }

    const bool possede = etats[DESTRUCTEUR] == ECRIT;
    const bool unique = etats[COPIE] == SUPPRIME && (etats[DEPLACEMENT] == ECRIT || etats[DEPLACEMENT] == DEFAUT);
    std::cout << "POSSEDE " << (possede ? "OUI" : "NON") << '\n';
    std::cout << "UNIQUE " << (unique ? "OUI" : "NON") << '\n';
    return 0;
}
