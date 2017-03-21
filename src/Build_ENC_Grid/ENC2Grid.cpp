/************************************************************/
/*    NAME: Sam Reed                                        */
/*    ORGN: UNH CCOM                                        */
/*    FILE: ENC2Grid.cpp                                    */
/*    DATE: March 2017                                      */
/************************************************************/

#include "ENC2Grid.h"

int main()
{
  double dfLatOrigin  = 43.071959194444446;
  double dfLongOrigin = -70.711610833333339;
  

  // Initalize UTM to Lat/Long and visa versa
  if (!m_Geodesy.Initialise(dfLatOrigin, dfLongOrigin))
    cout <<"Geodesy Init failed - FAIL\n";

  BuildLayers();

  return 0;
}

//---------------------------------------------------------
// Procedure:
/*
  Create a OGR layer for the point obstacles, polygon obstacles and  
  the line obstacles. Then open them so that can be used in the rest
  of the application.
        
  Outputs (these are not returned, but defined as self.):
   Point_Layer - OGR layer that holds all the information from the 
    ENC that have point geometry
   Poly_Layer - OGR layer that holds all the information from the 
    ENC that have polygon geometry
   Line_Layer - OGR layer that holds all the information from the 
    ENC that have line geometry
*/
void BuildLayers()
{
    GDALAllRegister();
    // Build the datasets
    const char *pszDriverName = "ESRI Shapefile";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
    GDALDataset *ds_pnt, *ds_poly, *ds_line, *ds_ENC;
    OGRLayer *PointLayer, *PolyLayer, *LineLayer;
    string ENC_filename="../../src/ENCs/US5NH02M/US5NH02M.000";
    
    OGRFieldDefn oField_depth( "Depth", OFTReal);
    
    // Create the shapefile
    ds_pnt = poDriver->Create( "../../src/ENCs/Shape/grid/Point.shp", 0, 0, 0, GDT_Unknown, NULL );
    if( ds_pnt == NULL )
      {
	printf( "Creation of output file failed.\n" );
	exit( 1 );
      }
    ds_poly = poDriver->Create( "../../src/ENCs/Shape/grid/Poly.shp", 0, 0, 0, GDT_Unknown, NULL );
    if( ds_poly == NULL )
      {
	printf( "Creation of output file failed.\n" );
	exit( 1 );
      }
    ds_line = poDriver->Create( "../../src/ENCs/Shape/grid/Line.shp", 0, 0, 0, GDT_Unknown, NULL );
    if( ds_line == NULL )
      {
	printf( "Creation of output file failed.\n" );
	exit( 1 );
      }
    
    // Create the layers (point, polygon, and lines)
    PointLayer = ds_pnt->CreateLayer( "Point", NULL, wkbPoint25D, NULL );
    if( PointLayer == NULL )
    {
        printf( "Layer creation failed.\n" );
        exit( 1 );
    }
    PolyLayer = ds_poly->CreateLayer( "Poly", NULL, wkbPolygon, NULL );
    if( PointLayer == NULL )
    {
        printf( "Layer creation failed.\n" );
        exit( 1 );
    }
    LineLayer = ds_line->CreateLayer( "Line", NULL, wkbLineString, NULL );
    if( PointLayer == NULL )
    {
        printf( "Layer creation failed.\n" );
        exit( 1 );
    }
    	
    ds_ENC = (GDALDataset*) GDALOpenEx( ENC_filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( ds_ENC == NULL )
      {
	printf( "Open failed.\n" );
	exit( 1 );
      }
    
    if(PointLayer->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    
    if(PolyLayer->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    
    if(LineLayer->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    
    LayerMultiPoint(ds_ENC->GetLayerByName("SOUNDG"), PointLayer, "SOUNDG");
    // Other Types
    
    ENC_Converter(ds_ENC->GetLayerByName("LNDARE"), PointLayer, PolyLayer, LineLayer, "LNDARE");
    
    ENC_Converter(ds_ENC->GetLayerByName("DEPCNT"), PointLayer, PolyLayer, LineLayer, "DEPCNT");
	//}
    // close the data sources - need this to save the new files
    GDALClose( ds_ENC );
    GDALClose( ds_pnt );
    GDALClose( ds_poly );
    GDALClose( ds_line );

}

//---------------------------------------------------------
// Procedure:LayerMultiPoint
/*
  Adds the features from the inputed multipoints layer to a point 
  layer.
        
  Inputs:
  LayerName_mp - Name of the multipoint layer
*/
void LayerMultiPoint(OGRLayer *layer_mp, OGRLayer *PointLayer, string LayerName_mp)
{
  OGRFeature *feat_mp, *new_feat;
  OGRFeatureDefn *feat_def;
  OGRGeometry *geom,*geom2, *poPointGeometry;
  OGRPoint *poPoint, *pt, *poPoint2;
  OGRMultiPoint *poMultipoint;
  double depth = 9999;
  int num_geom = 0;
  int WL = 0;
  double x,y, lat, lon;
  if (layer_mp != NULL)
      {
	PointLayer->ResetReading();
	layer_mp->ResetReading();
	feat_def = PointLayer->GetLayerDefn();
	while( (feat_mp = layer_mp->GetNextFeature()) != NULL )
	  {
	    geom = feat_mp->GetGeometryRef();
	    poMultipoint = ( OGRMultiPoint * )geom;
	    num_geom = poMultipoint->getNumGeometries();
	    for(int iPnt = 0; iPnt < num_geom; iPnt++ )
	      {
		
		depth = 9999;
		// Make the point
		poPointGeometry = poMultipoint ->getGeometryRef(iPnt);
		poPoint = ( OGRPoint * )poPointGeometry;
		pt = (OGRPoint *)OGRGeometryFactory::createGeometry(wkbPoint25D);
		// Get the x,y, and depth in UTM
		lon = poPoint->getX();
		lat = poPoint->getY();
		m_Geodesy.LatLong2LocalUTM(lat,lon,y,x);
		depth = poPoint->getZ();
		
		pt->setX(x);
		pt->setY(y);
		pt->setZ(depth);


		//pt=OGRPoint(x,y,depth);
		//cout << pt.getZ()<< endl;
		new_feat =  OGRFeature::CreateFeature(feat_def);
		new_feat->SetField("Depth", depth);
		new_feat->SetGeometry(pt);
		
		/*geom2 = new_feat->GetGeometryRef();
		poPoint2 = ( OGRPoint * )geom2;
		cout << depth << poPoint2->getZ() << endl;
		*/

		if( PointLayer->CreateFeature( new_feat ) != OGRERR_NONE )
		  {
		    printf( "Failed to create feature in shapefile.\n" );
		    exit( 1 );
		  }

		OGRFeature::DestroyFeature( new_feat );
	      }
	  }
      }
  else
    cout << "Layer "<< LayerName_mp << " did not open correctly" << endl;
}

//---------------------------------------------------------
// Procedure: ENC_Converter
/*
  This function converts the inputed layer from the ENC to a 
  shapefile layer if the layer is in the ENC. If the inputted layer 
  is not in the ENC, the layer is skipped and the function prints a 
  warning to the user.
*/
void ENC_Converter(OGRLayer *Layer_ENC, OGRLayer *PointLayer, OGRLayer *PolyLayer, OGRLayer *LineLayer, string LayerName)
{
  OGRFeature *poFeature, *new_feat;
  OGRFeatureDefn *poFDefn, *poFDefn_ENC;
  OGRFieldDefn *poFieldDefn;
  OGRGeometry *geom;
  OGRPoint *poPoint,*vertex, *pt;
  OGRPolygon *poPoly, *UTM_poly;
  OGRLineString *poLine, *UTM_line;
  OGRLinearRing *ring, *UTM_ring;

  double x,y, lat,lon;
  double WL;
  int vis;
  int i=0;
  double t_lvl = 0;
  double depth = 9999;
  int iField = 0;
  string name = "";
  string cat;
  
  if (Layer_ENC != NULL)
    {
      poFDefn_ENC = Layer_ENC->GetLayerDefn();
      Layer_ENC->ResetReading();
      
      while( (poFeature = Layer_ENC->GetNextFeature()) != NULL )
	{
	  geom = poFeature->GetGeometryRef();
	  depth = 9999;
	  
	  if (LayerName=="SOUNDG")
	    depth = poFeature->GetFieldAsDouble("VALSOU");
	  
	  else if (LayerName=="DEPCNT")
	    depth = poFeature->GetFieldAsDouble("VALDCO");

	  else if (LayerName=="LNDARE")
	    depth = -10;
	  
	  if (geom ->getGeometryType()  == wkbPoint)
	    {
	      // Set attributes of the new feature
	      poFDefn = PointLayer->GetLayerDefn();
	      new_feat =  OGRFeature::CreateFeature(poFDefn);
	      
	      // Get the old point from the ENC
	      poPoint = ( OGRPoint * )geom;
	      pt = (OGRPoint *)OGRGeometryFactory::createGeometry(wkbPoint25D);
	      
	      // Convert lat/long to UTM
	      lon = poPoint->getX();
	      lat = poPoint->getY();
	      m_Geodesy.LatLong2LocalUTM(lat,lon,y,x);
	      
		
	      // Make the new point in UTM
	      pt->setX(x);
	      pt->setY(y);
	      pt->setZ(depth);
	      
	      new_feat->SetField("Depth", depth);
	      new_feat->SetGeometry(pt);
	      
	      // Build the new feature
	      if( PointLayer->CreateFeature( new_feat ) != OGRERR_NONE )
		{
		  printf( "Failed to create feature in shapefile.\n" );
		  exit( 1 );
		}
	    }
	  else if(geom ->getGeometryType() == wkbPolygon)
	    {
	      // Set attributes of the new feature
	      poFDefn = PolyLayer->GetLayerDefn();
	      new_feat =  OGRFeature::CreateFeature(poFDefn);
	      
	      poPoly = ( OGRPolygon * )geom;
	      ring = poPoly->getExteriorRing();
	      // Build the new UTM ring and polygon
	      UTM_ring = (OGRLinearRing *) OGRGeometryFactory::createGeometry(wkbLinearRing);
	      UTM_poly = (OGRPolygon*) OGRGeometryFactory::createGeometry(wkbPolygon);
	      for (int j=0; j<ring->getNumPoints(); j++)
		{
		  lon = ring->getX(j);
		  lat = ring->getY(j);
	      
		  m_Geodesy.LatLong2LocalUTM(lat,lon,y,x);
		  UTM_ring->addPoint(x,y,depth);
		}
	      // Build the UTM polygon from the ring
	      UTM_ring->closeRings();
	      UTM_poly->addRing(UTM_ring);
	      UTM_poly->closeRings();
	      
	      new_feat->SetField("Depth", depth);
	      new_feat->SetGeometry(UTM_poly);
	      
	      // Build the new feature
	      if( PolyLayer->CreateFeature( new_feat ) != OGRERR_NONE )
		{
		  printf( "Failed to create feature in shapefile.\n" );
		  exit( 1 );
		}
	    }
	  else if(geom ->getGeometryType() == wkbLineString)
	    {
	      // Set attributes of the new feature
	      poFDefn = LineLayer->GetLayerDefn();
	      new_feat =  OGRFeature::CreateFeature(poFDefn);

	      poLine = ( OGRLineString * )geom;
	      UTM_line = (OGRLineString *)OGRGeometryFactory::createGeometry(wkbLineString);
	      // Make all lines in UTM not lat/long
	      for (int j=0; j<poLine->getNumPoints(); j++)
		{
		  // Convert to UTM
		  lon = poLine->getX(j);
		  lat = poLine->getY(j);
		  m_Geodesy.LatLong2LocalUTM(lat,lon,y,x);
		  
		  UTM_line->addPoint(x,y,depth);
		}
	      new_feat->SetField("Depth", depth);
	      new_feat->SetGeometry(UTM_line);
	      
	      // Build the new feature
	      if( LineLayer->CreateFeature( new_feat ) != OGRERR_NONE )
		{
		  printf( "Failed to create feature in shapefile.\n" );
		  exit( 1 );
		}
	    }
	  
	  OGRFeature::DestroyFeature( new_feat );
	}
    }
  else
    {
      cout << "Layer " << LayerName << " is not in the ENC." << endl;
      //exit(1);
    }
}