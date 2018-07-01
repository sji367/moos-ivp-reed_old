#ifndef POLYANGULARSWEEP_H
#define POLYANGULARSWEEP_H
#define PI 3.14159265

#include <vector>
#include <string>
#include <cmath> // for pow and sqrt
#include <algorithm> // for min_element and max_element
#include "geodesy.h" // Conversion between lat/lon and UTM
#include "ogrsf_frmts.h" // GDAL
#include "ogr_spatialref.h"
#include "polyInside.h"
#include <iostream>
#include <ctime>

using namespace std;

class polyAngularSweep
{
public:
    polyAngularSweep(double x, double y, double length, double searchDist, int SwathSize) {polyAngularSweep(x, y, length, searchDist, SwathSize, false);}
    polyAngularSweep(double x, double y, double length, double searchDist, int SwathSize, bool debug);
    ~polyAngularSweep() {if (Debug){GDALClose(DS); GDALClose(DS_pnt);}}

    void findIntersections(OGRGeometry *geomPoly, double threat_level);
    double calcUtil(double dist);
    void getOtherVertex(double angle, double &x, double &y) {getOtherVertex(angle, m_search_dist, x, y); }
    void getOtherVertex(double angle, double dist, double &x, double &y);
    void storeUtil(OGRGeometry *intersect_geom, int index);
    void setT_Lvl(double TLvl) { t_lvl=TLvl; }
    string getUtilVect_asString();
    double getMinDist();
    int getSwathSize() {return swathSize;}

    void build_search_poly();
    void setSearchPoly(OGRPolygon *searchPoly) {search_area_poly=searchPoly; }
    void resetUtilVector() {util.resize(360/swathSize); fill (util.begin(),util.end(),100);}

    void buildLineLayer();
    double ang4MOOS(double oldAngle);
    double index4MOOS(double oldindex) { return ang4MOOS(5*oldindex)/5;}

    void resetUtilityIfInside();

    void writeIntersectionPoint(double x, double y);

    void movingAverageFilter(int windowSize);

private:
    double m_ASV_x, m_ASV_y, m_ASV_length, m_search_dist;
    double t_lvl;
    vector<double> util, distance2poly;

    OGRPolygon *search_area_poly;
    OGRPoint *ASV_Location;
    GDALDataset *DS, *DS_pnt;
    OGRLayer *LineLayer, *PointLayer;
    double minDist;

    int swathSize;
    polyInside checkInside;

    // This creates a shapefile for debugging code
    bool Debug;
};

#endif // POLYANGULARSWEEP_H
