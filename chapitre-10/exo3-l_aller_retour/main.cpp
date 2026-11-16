#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>

struct Vec2
{
    double x = 0;
    double y = 0;
};

// La caméra du canevas : un centre et un zoom, rien d'autre. La taille de
// la fenêtre arrive en paramètre à chaque calcul, comme dans NkRefView.
struct Vue
{
    Vec2 centre;
    double zoom = 1; // pixels par unité du monde

    static constexpr double kZoomMin = 0.02;
    static constexpr double kZoomMax = 64;

    Vec2 VersMonde(Vec2 pixel, Vec2 fenetre) const
    {
        return {centre.x + (pixel.x - fenetre.x / 2) / zoom,
                centre.y + (pixel.y - fenetre.y / 2) / zoom};
    }

    Vec2 VersPixel(Vec2 monde, Vec2 fenetre) const
    {
        return {(monde.x - centre.x) * zoom + fenetre.x / 2,
                (monde.y - centre.y) * zoom + fenetre.y / 2};
    }

    // Glisser la planche vers la droite fait reculer le centre.
    void Deplace(double dx, double dy)
    {
        centre.x -= dx / zoom;
        centre.y -= dy / zoom;
    }

    // Le point sous le curseur est noté avant de toucher au zoom, puis le
    // centre est recalé pour qu'il y reste, même si le zoom a été borné.
    void ZoomAuPixel(double facteur, Vec2 pixel, Vec2 fenetre)
    {
        Vec2 ancre = VersMonde(pixel, fenetre);
        zoom = std::clamp(zoom * facteur, kZoomMin, kZoomMax);
        centre.x = ancre.x - (pixel.x - fenetre.x / 2) / zoom;
        centre.y = ancre.y - (pixel.y - fenetre.y / 2) / zoom;
    }

    // Les deux boucles se terminent parce que le zoom reste dans ses bornes.
    double PasGrille(double minPixels) const
    {
        double pas = 1;
        while (pas * zoom < minPixels)
            pas *= 2;
        while (pas * 0.5 * zoom >= minPixels)
            pas *= 0.5;
        return pas;
    }

    void Origine() { *this = Vue{}; }
};

// Arrondi au plus proche, sans jamais écrire « -0.00 ».
std::string Nombre(double valeur, int decimales = 2)
{
    char texte[64];
    std::snprintf(texte, sizeof texte, "%.*f", decimales, valeur);
    std::string resultat = texte;
    if (resultat.front() == '-' && resultat.find_first_not_of("-0.") == std::string::npos)
        resultat.erase(0, 1);
    return resultat;
}

int main()
{
    Vec2 fenetre;
    std::cin >> fenetre.x >> fenetre.y;

    Vue vue;
    std::string commande;

    while (std::cin >> commande)
    {
        if (commande == "VERS_MONDE")
        {
            Vec2 pixel;
            std::cin >> pixel.x >> pixel.y;
            Vec2 monde = vue.VersMonde(pixel, fenetre);
            std::cout << "MONDE " << Nombre(monde.x) << ' ' << Nombre(monde.y) << '\n';
        }
        else if (commande == "VERS_PIXEL")
        {
            Vec2 monde;
            std::cin >> monde.x >> monde.y;
            Vec2 pixel = vue.VersPixel(monde, fenetre);
            std::cout << "PIXEL " << Nombre(pixel.x) << ' ' << Nombre(pixel.y) << '\n';
        }
        else if (commande == "DEPLACE")
        {
            double dx = 0, dy = 0;
            std::cin >> dx >> dy;
            vue.Deplace(dx, dy);
        }
        else if (commande == "ZOOM")
        {
            double facteur = 1;
            Vec2 pixel;
            std::cin >> facteur >> pixel.x >> pixel.y;
            vue.ZoomAuPixel(facteur, pixel, fenetre);
        }
        else if (commande == "GRILLE")
        {
            double minPixels = 1;
            std::cin >> minPixels;
            std::cout << "GRILLE " << Nombre(vue.PasGrille(minPixels), 6) << '\n';
        }
        else if (commande == "ORIGINE")
        {
            vue.Origine();
        }
    }

    std::cout << "CENTRE " << Nombre(vue.centre.x) << ' ' << Nombre(vue.centre.y) << '\n';
    std::cout << "ZOOM " << Nombre(vue.zoom) << '\n';
    return 0;
}
