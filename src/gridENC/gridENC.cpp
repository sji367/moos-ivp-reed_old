#include "gridENC.h"

GridENC::GridENC(string MOOS_path, string ENC_Name, double buffer_dist, double gridSize, double search_radius, double MHW_offset, bool SimpleGrid, bool CATZOC_poly)
{
    cout << "here" << endl;
    geod = Geodesy();
    grid_size = gridSize;
    ENC_name = ENC_Name;
    ENC_filename = "src/ENCs/"+ENC_name+"/"+ENC_name+".000";
    MOOS_Path = ENC_name+"/";
    searchRadius = search_radius;
    buffer_size = buffer_dist;
    MHW_Offset = MHW_offset;
    minX = 0;
    minY = 0;
    maxX = 0;
    maxY = 0;
    landZ = -(MHW_offset+2);
    simpleGrid = SimpleGrid;
    CATZOC_z = -7.5;
    CATZOC_polys = CATZOC_poly;
    cout << landZ<< " " << MHW_Offset << " " <<MHW_offset << endl;
}

void GridENC::buildLayers()
{
    GDALAllRegister();
    // Build the datasets
    const char *pszDriverName = "ESRI Shapefile";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
    OGRFieldDefn oField_depth( "Depth", OFTReal);
    OGRFieldDefn oField_inside( "Inside_ENC", OFTReal);

    // Build the datasource and layer that will hold the wrecks and land area polygons
    string polyPath = MOOS_Path+"polygon.shp";
    ds_poly = poDriver->Create(polyPath.c_str(), 0, 0, 0, GDT_Unknown, NULL );
    if( ds_poly == NULL )
    {
        printf( "Creation of output file failed.\n" );
        exit( 1 );
    }
    // Create the layers
    layer_poly = ds_poly->CreateLayer( "poly", NULL, wkbPolygon25D, NULL );
    if( layer_poly == NULL )
    {
        printf( "poly Layer creation failed.\n" );
        exit( 1 );
    }
    // Create the field
    if(layer_poly->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    feat_def_poly = layer_poly->GetLayerDefn();

    // Build the datasource and layer that will hold independant points
    string pntPath = MOOS_Path+"point.shp";
    ds_pnt = poDriver->Create(pntPath.c_str(), 0, 0, 0, GDT_Unknown, NULL );
    if( ds_pnt == NULL )
    {
        printf( "Creation of output file failed.\n" );
        exit( 1 );
    }
    // Create the layers
    layer_pnt = ds_pnt->CreateLayer( "point", NULL, wkbPoint25D, NULL );
    if( layer_pnt == NULL )
    {
        printf( "line Layer creation failed.\n" );
        exit( 1 );
    }
    // Create the field
    if(layer_pnt->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    feat_def_pnt = layer_pnt->GetLayerDefn();


    // Build the datasource layer that will hold the depth area polygons
    string depthPath = MOOS_Path+"depth_area.shp";
    ds_depth = poDriver->Create(depthPath.c_str(), 0, 0, 0, GDT_Unknown, NULL );
    if( ds_depth == NULL )
    {
        printf( "Creation of DA file failed.\n" );
        exit( 1 );
    }
    // Create the layer
    layer_depth = ds_depth->CreateLayer( "depth", NULL, wkbPolygon25D, NULL );
    if( layer_depth == NULL )
    {
        printf( "DA Layer creation failed.\n" );
        exit( 1 );
    }
    // Create the field
    if(layer_depth->CreateField( &oField_depth ) != OGRERR_NONE )
    {
        printf( "Creating Depth field failed.\n" );
        exit( 1 );
    }
    feat_def_depth = layer_depth->GetLayerDefn();


    // Build the datasource and layer that will hold the outline of the ENC
    string outlinePath = MOOS_Path+"outline.shp";
    ds_outline = poDriver->Create(outlinePath.c_str(), 0, 0, 0, GDT_Unknown, NULL );
    if( ds_outline == NULL )
    {
        printf( "Creation of output file failed.\n" );
        exit( 1 );
    }
    // Create the layers
    layer_outline = ds_outline->CreateLayer( "outline", NULL, wkbPolygon25D, NULL );
    if( layer_outline == NULL )
    {
        printf( "outline Layer creation failed.\n" );
        exit( 1 );
    }
    // Create the field
    if(layer_outline->CreateField( &oField_inside ) != OGRERR_NONE )
    {
        printf( "Creating Inside_ENC field failed.\n" );
        exit( 1 );
    }
    feat_def_outline = layer_outline->GetLayerDefn();
}

// Set the grid lize to (compliation scale)/4
//  Input:
//      ds - data source for the ENC for which we want the compilation scale
void GridENC::setGridSize2Default(GDALDataset *ds)
{
    grid_size = ds->GetLayerByName("DSID")->GetFeature(0)->GetFieldAsInteger("DSPM_CSCL")/4000.0;
    cout << "Grid size = " << grid_size << endl;
}

// This function creates a grid from the soundings, land areas, rocks, wrecks, depth areas
//  and depth contours and store it as a 2D vector of ints where the depth is stored in cm.
//
//  Inputs:
//      csv - boolean describing if you want to build a csv file from the 1D vectors(DA, raw_interp, postProcessed,polygon, etc.)
//      mat - boolean describing if you want to build a .mat file from the 1D vectors (DA, raw_interp, postProcessed,polygon, etc.)
void GridENC::Run(bool csv, bool mat)
{
    clock_t start = clock();
    GDALAllRegister();
    GDALDataset* ds;
    buildLayers();

    ds = (GDALDataset*) GDALOpenEx( ENC_filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );

    getENC_MinMax(ds);

    // If the grid size is not explictly set, set it to (compliation scale)/4
    if (grid_size == -1)
        setGridSize2Default(ds);

    vector<double> poly_rasterdata, depth_area_rasterdata, point_rasterdata;
    vector<int> ENC_outline_rasterdata;
    int poly_extentX, poly_extentY, depth_extentX, depth_extentY, outline_extentX, outline_extentY, point_extentX, point_extentY;

    // Add the vertices from the soundings, land areas, rocks, wrecks, and depth contours
    //  to vectors for gridding
    layer2XYZ(ds->GetLayerByName("SOUNDG"), "SOUNDG");
    layer2XYZ(ds->GetLayerByName("DEPCNT"), "DEPCNT");
    layer2XYZ(ds->GetLayerByName("M_COVR"), "M_COVR");
    layer2XYZ(ds->GetLayerByName("DEPARE"), "DEPARE");
    layer2XYZ(ds->GetLayerByName("LNDARE"), "LNDARE");
    layer2XYZ(ds->GetLayerByName("PONTON"), "PONTON");

    if (!simpleGrid)
    {
        if (CATZOC_polys)
            layer2XYZ(ds->GetLayerByName("M_QUAL"), "M_QUAL"); // Mark unreliable data as obstacles
        layer2XYZ(ds->GetLayerByName("UWTROC"), "UWTROC");
        layer2XYZ(ds->GetLayerByName("WRECKS"), "WRECKS");
        layer2XYZ(ds->GetLayerByName("FLODOC"), "FLODOC");
        layer2XYZ(ds->GetLayerByName("OBSTRN"), "OBSTRN");
        layer2XYZ(ds->GetLayerByName("DYKCON"), "DYKCON");
    }

    // Save the rasters
    GDALClose(ds_depth);
    GDALClose(ds_poly);
    GDALClose(ds_outline);
    GDALClose(ds_pnt);

    // Rasterize the polygon, point, outline and depth area layers
    ENC_Rasterize polygon_Tiff = ENC_Rasterize(MOOS_Path, "polygon.shp", minX+geod.getXOrigin(), minY+geod.getYOrigin(),
                                               maxX+geod.getXOrigin(), maxY+geod.getYOrigin());
    ENC_Rasterize depthArea_Tiff = ENC_Rasterize(MOOS_Path, "depth_area.shp", minX+geod.getXOrigin(), minY+geod.getYOrigin(),
                                                 maxX+geod.getXOrigin(), maxY+geod.getYOrigin());
    ENC_Rasterize outline_Tiff = ENC_Rasterize(MOOS_Path, "outline.shp", minX+geod.getXOrigin(), minY+geod.getYOrigin(),
                                               maxX+geod.getXOrigin(), maxY+geod.getYOrigin());
    ENC_Rasterize point_Tiff = ENC_Rasterize(MOOS_Path, "point.shp", minX+geod.getXOrigin(), minY+geod.getYOrigin(),
                                             maxX+geod.getXOrigin(), maxY+geod.getYOrigin());

    polygon_Tiff.rasterize("polygon.tiff", (grid_size), "Depth", "Float64");
    depthArea_Tiff.rasterize("depth_area.tiff", (grid_size), "Depth", "Float64");
    outline_Tiff.rasterize("outline.tiff", (grid_size), "Inside_ENC", "Int32");
    point_Tiff.rasterize("point.tiff", (grid_size), "Depth", "Float64");

    cout << "rasterized" << endl;

    // Store the rasterized data as 1D vectors
    getRasterData("polygon.tiff", poly_extentX, poly_extentY, poly_rasterdata);
    getRasterData("depth_area.tiff", depth_extentX, depth_extentY, depth_area_rasterdata);
    getRasterData("outline.tiff", outline_extentX, outline_extentY, ENC_outline_rasterdata);
    getRasterData("point.tiff", point_extentX, point_extentY, point_rasterdata);

    // Grid the data
    GDALGridLinearOptions options;
    options.dfRadius = searchRadius;
    options.dfNoDataValue = -10;
    cout << "Search Radius: " << searchRadius << endl;

    int x_res = static_cast<int>(round((maxX - minX)/grid_size));
    int y_res = static_cast<int>(round((maxY - minY)/grid_size));

    cout << "X: " << x_res << " Y: " << y_res << endl;

    griddedData.resize(y_res*x_res);
    vector<double> new_rast_data(y_res*x_res, -10);

    Map.clear();
    Map.resize(x_res, vector<double> (y_res,0));

    printf("Starting to grid data\n\tData points: %d, Grid: %dx%d\n", static_cast<int>(X.size()), y_res, x_res);

    // Grid the data
    if (GDALGridCreate(GGA_Linear, &options, static_cast<int>(X.size()), X.data(), Y.data(), depth.data(), minX, maxX, minY, maxY, x_res, y_res, GDT_Float64, griddedData.data(), NULL, NULL) == CE_Failure)
    {
        cout << "Gridding Failed" << endl;
        exit(1);
    }

    cout << "Gridded" << endl;
    writeRasterData("rawInterp_"+ ENC_name +".tiff", x_res, y_res, griddedData);

    // Combine the 3 1D vectors to one 2D vector
    updateMap(poly_rasterdata, depth_area_rasterdata, ENC_outline_rasterdata, point_rasterdata, new_rast_data, x_res, y_res);

    cout << "Min: " << *min_element(new_rast_data.begin(), new_rast_data.end());
    cout << ", Max: " << *max_element(new_rast_data.begin(), new_rast_data.end()) << endl;

    if (csv)
        write2csv(poly_rasterdata, depth_area_rasterdata, point_rasterdata, x_res, y_res, mat);

    writeRasterData("PostProcess_" + ENC_name +".tiff", x_res, y_res, new_rast_data);

    clock_t end = clock();
    double total_time = double(end - start) / CLOCKS_PER_SEC;
    cout << "Elapsed Time: " << total_time << " seconds." << endl;
}

vector<vector<double> > GridENC::transposeMap()
{
    vector <vector<double> > t_map (Map[0].size(), vector<double> (Map.size(),0));

    for (int i =0; i<Map.size(); i++)
    {
        for (int j=0; j<Map[0].size(); j++)
        {
            t_map[j][i] = Map[i][j];
        }
    }
    return t_map;
}

// Build the 2D map from the 4 1D vectors
void GridENC::updateMap(vector<double> &poly_data, vector<double> &depth_data, vector<int> &outline_data, vector<double> &point_data, vector<double> &new_rast_data, int x_res, int y_res)
{
    int x, y;
    int rasterIndex = 0;

    // Make sure that the size of all of the 1D vectors are the same
    if ((poly_data.size() == depth_data.size()) && (poly_data.size() == griddedData.size()) &&
            (poly_data.size() == outline_data.size())&&((simpleGrid)||(poly_data.size() == point_data.size())))
    {
        for (int i =0; i<griddedData.size(); i++)
        {
            rasterIndex2gridIndex(i,rasterIndex, x_res, y_res);
            row_major2grid(i, x, y, x_res);

            // Make sure that the datapoint is inside of the ENC
            if (outline_data[rasterIndex] == 1)
            {
                // Check to see if the index is on land.
                if (poly_data[rasterIndex] == landZ)
                    Map[y][x] =poly_data[rasterIndex];
                else
                {
                    // If it is not land make sure that the recorded depth is not below the minimum depth given by the depth area
                    if((depth_data[rasterIndex] != -10) && (depth_data[rasterIndex] > griddedData[i]))
                        Map[y][x] = depth_data[rasterIndex];
                    else
                        Map[y][x] = griddedData[i];

                    // If it not on land then we must still check to see if the depth value is less than the one given by
                    //  the previous statement (for example if it is a wreck or dock)
                    if((poly_data[rasterIndex] != -10) && (poly_data[rasterIndex] < Map[y][x]))
                    {
                        Map[y][x] = poly_data[rasterIndex];
                    }
                    if (!simpleGrid)
                    {
                        // The last check is to see if the point data depth value is less than the one given by
                        //  the previous statements (ie pontoon or rock)
                        if((point_data[rasterIndex] != -10) && (point_data[rasterIndex] < Map[y][x]))
                        {
                            Map[y][x] = point_data[rasterIndex];
                        }
                    }
                }
            }
            else
                Map[y][x] = -10;
            /*
            // Store the Map data so that it can be converted to a raster
            if (Map[y][x] <= landZ)
                new_rast_data[rasterIndex] = NAN;
            else
            */
            new_rast_data[rasterIndex] = (Map[y][x]);
        }

    }
    else
    {
        printf("Poly: %d, Depth: %d, Grid:%d, ENC Outline:%d\n", poly_data.size(), depth_data.size(), griddedData.size(), outline_data.size());
        cout << "The vectors are the wrong size. Exiting...\n";
        exit(1);
    }
}

// Save the 1D vectors (polygon, depth area, pre-processed interpolated grid) and the 2D
//  post-processed interpolated grid as csv files
void GridENC::write2csv(vector<double> &poly_data, vector<double> &depth_data, vector<double> &point_data, int x_res, int y_res, bool mat)
{
    ofstream combined, grid, poly, DA, point;
    int x,y;
    int prev_x = 0;
    int rasterIndex = 0;

    string postcsv= MOOS_Path+"post.csv";
    string precsv= MOOS_Path+"pre.csv";
    string polycsv= MOOS_Path+"poly.csv";
    string DAcsv= MOOS_Path+"DA.csv";
    string pntcsv = MOOS_Path+"point.csv";


    // File to write data to csv format
    combined.open(postcsv.c_str());
    grid.open(precsv.c_str());
    poly.open(polycsv.c_str());
    DA.open(DAcsv.c_str());
    point.open(pntcsv.c_str());

    cout << "Writing data to CSV files." << endl;

    // Make sure that the size of all of the 1D vectors are the same
    if ((poly_data.size() == depth_data.size()) && (poly_data.size() == griddedData.size()))
    {
        for (int i =0; i<griddedData.size(); i++)
        {
            rasterIndex2gridIndex(i,rasterIndex, x_res, y_res);
            row_major2grid(i, x, y, x_res);

            if (x!=prev_x)
            {
                grid << "\n";
                poly << "\n";
                DA << "\n";
                combined << "\n";
                point << "\n";
            }
            // Store the data in a csv
            combined << Map[y][x]<< ",";
            grid << griddedData[i] << ",";
            poly << poly_data[rasterIndex] << ",";
            DA << depth_data[rasterIndex] << ",";
            point << point_data[rasterIndex] << ",";

            /*
            // Convert unknowns (-10) to NaNs
            if (griddedData[i] == -10)
                griddedData[i] = NAN;
            else
                grid << griddedData[i] << ",";

            if (poly_data[rasterIndex] == -10)
                poly_data[rasterIndex] = NAN;
            else
                poly << poly_data[rasterIndex] << ",";

            if (depth_data[rasterIndex] == -10)
                depth_data[rasterIndex] = NAN;
            else
                DA << depth_data[rasterIndex] << ",";

            if (point_data[rasterIndex] == -10)
                point_data[rasterIndex] = NAN;
            else
                point << point_data[rasterIndex] << ",";
            */
            prev_x =x;
        }
        combined.close();
        grid.close();
        poly.close();
        DA.close();
        point.close();

        // Write these files to .mat
        if (mat)
        {
            cout << "Writing data to .mat files." << endl;
            writeMat("post");
            writeMat("pre");
            writeMat("poly");
            writeMat("DA");
            //writeMat("point");
        }
    }
    else
    {
        printf("Poly: %d, Depth: %d, Grid:%d\n", poly_data.size(), depth_data.size(), griddedData.size());
        cout << "The vectors are the wrong size. Exiting...\n";
        exit(1);
    }
}

void GridENC::writeMat(string filename)
{
    string toMat = "python scripts/csv2mat.py " + filename+".csv " + filename+".mat";
    cout << toMat << endl;
    system(toMat.c_str());
}

// Get the minimum and maximum x/y values (in local UTM) of the ENC
void GridENC::getENC_MinMax(GDALDataset* ds)
{
    OGRLayer* layer;
    OGRFeature* feat;
    OGRLinearRing* ring;
    OGRGeometry* geom;
    OGRPolygon* coverage;

    vector<double> x,y;
    double UTM_x, UTM_y;
    double remainder;
    double lat, lon;
    int UTM_Zone;

    layer = ds->GetLayerByName("M_COVR");
    layer->ResetReading();
    feat = layer->GetNextFeature();

    // Cycle through the points and store all x and y values of the layer
    for (int i=0; i<layer->GetFeatureCount(); i++)
    {
        if (feat->GetFieldAsDouble("CATCOV") == 1)
        {
            geom = feat->GetGeometryRef();
            coverage = ( OGRPolygon * )geom;
            ring = coverage->getExteriorRing();
            for (int j =0;  j<ring->getNumPoints(); j++)
            {
                // Set the geod member variable using the first point on the outline of the ENC
                if (j==0)
                {
                    lat = ring->getY(j);
                    lon = ring->getX(j);
                    UTM_Zone=1+(lon+180.0)/6.0;
                    UTM.SetWellKnownGeogCS("WGS84");
                    UTM.SetUTM(UTM_Zone, signbit(-lat));
                    EPSG_code = UTM.GetEPSGGeogCS();
                    geod = Geodesy(lat, lon);
                    //char    *pszWKT = NULL;
                    //UTM.exportToWkt( &pszWKT );
                    //printf( "%s\n", pszWKT );
                }
                geod.LatLong2LocalUTM(ring->getY(j), ring->getX(j), UTM_x, UTM_y);
                x.push_back(UTM_x);
                y.push_back(UTM_y);
            }
        }
        feat = layer->GetNextFeature();
    }

    // Find the min and max of the X and Y values
    minX = *min_element(x.begin(), x.end());
    maxX = *max_element(x.begin(), x.end());
    minY = *min_element(y.begin(), y.end());
    maxY = *max_element(y.begin(), y.end());

    // Make sure each one of min/max values are divisable by grid_size
    remainder = fmod(minX, grid_size);
    if (remainder != 0)
        minX -= remainder+grid_size;

    remainder = fmod(minY, grid_size);
    if (remainder != 0)
        minY -= remainder+grid_size;

    remainder = fmod(maxX, grid_size);
    if (remainder != 0)
        maxX += grid_size - remainder;

    remainder = fmod(maxY, grid_size);
    if (remainder != 0)
        maxY += grid_size - remainder;

}

// This function takes in an OGRLayer and sorts out which function it should
//  use based upon the geometry type of the feature for interpolation
void GridENC::layer2XYZ(OGRLayer* layer, string layerName)
{
    OGRFeature* feat;
    OGRGeometry* geom;
    string geom_name;

    if (layer != NULL)
    {
        layer->ResetReading();
        while( (feat = layer->GetNextFeature()) != NULL )
        {
            geom = feat->GetGeometryRef();
            geom_name = geom->getGeometryName();
            if (geom_name == "MULTIPOINT")
                multipointFeat(feat, geom);
            else if (geom_name == "POLYGON")
                polygonFeat(feat, geom, layerName);
            else if (geom_name == "LINESTRING")
                lineFeat(feat, geom, layerName);
            else if (geom_name == "POINT")
                pointFeat(feat, geom, layerName);
        }
    }
    else
        cout << layerName << " is not in the ENC!" << endl;
}

// This function converts the mulitpoints into Local UTM, and
//  stores the vertices for interpolation
void GridENC::multipointFeat(OGRFeature* feat, OGRGeometry* geom)
{
    OGRGeometry* poPointGeometry;
    OGRMultiPoint *poMultipoint;
    OGRPoint * poPoint;

    double lat,lon, x, y, z;

    // Cycle through all points in the multipoint and store the xyz location of each point
    geom = feat->GetGeometryRef();
    poMultipoint = ( OGRMultiPoint * )geom;
    for(int iPnt = 0; iPnt < poMultipoint->getNumGeometries(); iPnt++ )
    {
        // Make the point
        poPointGeometry = poMultipoint ->getGeometryRef(iPnt);
        poPoint = ( OGRPoint * )poPointGeometry;
        // Get the location in the grid coordinate system
        lon = poPoint->getX();
        lat = poPoint->getY();
        geod.LatLong2UTM(lat,lon, x,y);

        // Get depth in cm
        z= poPoint->getZ();
        if (!z)
            return;

        storePoint(x,y,z);
    }
}

// This function converts the lines into Local UTM, segements the lines, and
//  stores the vertices for interpolation
void GridENC::lineFeat(OGRFeature* feat, OGRGeometry* geom, string layerName)
{
    OGRLineString *UTM_line;
    OGRGeometry *geom_UTM, *buff_geom;
    OGRFeature *new_feat;
    OGRPolygon *Buff_line;

    double x,y,z;
    bool WL_flag;
    int WL=-1;

    string Z;

    geom_UTM = geod.LatLong2UTM(geom);
    UTM_line = ( OGRLineString * )geom_UTM;
    if (buffer_size>0)
    {
        buff_geom = UTM_line->Buffer(buffer_size);
        Buff_line = (OGRPolygon *) buff_geom;
        Buff_line->segmentize(grid_size/2);
    }
    else
        UTM_line->segmentize(grid_size/2);

    if (layerName=="DEPCNT")
    {
        z = feat->GetFieldAsDouble("VALDCO");
        UTM_line->segmentize(grid_size);
        // Get the location in the grid coordinate system
        for (int j=0; j<UTM_line->getNumPoints(); j++)
        {
            x = UTM_line->getX(j)-geod.getXOrigin();// convert to local UTM
            y = UTM_line->getY(j)-geod.getYOrigin();// convert to local UTM

            // Store the XYZ of the vertices
            X.push_back(x);
            Y.push_back(y);
            depth.push_back(z); // in cm
        }
        return; // do not store the line as a polygon
    }
    else if ((layerName =="PONTON")||(layerName =="FLODOC")||(layerName =="DYKCON")||(layerName=="LNDARE"))
    {
        z = landZ;
    }
    else if ((layerName == "OBSTRN"))
    {
        Z = feat->GetFieldAsString("VALSOU");
        WL_flag = Z.empty();
        if (WL_flag)
        {
            WL = feat->GetFieldAsDouble("WATLEV");
            z = calcDepth(WL);
            if (z==-100)
                return;
        }
        else
            z = feat->GetFieldAsDouble("VALSOU");
    }
    else
    {
        cout << "Unknown Line Layer: " << layerName << endl;
        return;
    }
    if (buffer_size >0)
    {
        // If it is a valid layer that we know how to deal with then store the buffered line
        //  (which is now a polygon)
        new_feat =  OGRFeature::CreateFeature(feat_def_poly);
        new_feat->SetField("Depth", z);
        new_feat->SetGeometry(Buff_line);
        // Build the new feature
        if( layer_poly->CreateFeature( new_feat ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in polygon shapefile.\n" );
            exit( 1 );
        }
        OGRFeature::DestroyFeature(new_feat);
        store_vertices(Buff_line, z);
    }
    else
        store_vertices(UTM_line, z);
}

void GridENC::pointFeat(OGRFeature* feat, OGRGeometry* geom, string layerName)
{
    OGRPoint *poPoint;
    int WL;
    double lat,lon, x,y, z;

    poPoint = poPoint = ( OGRPoint * )geom;
    lon = poPoint->getX();
    lat = poPoint->getY();

    // Get the location in the grid coordinate system
    geod.LatLong2UTM(lat, lon, x,y);

    bool WL_flag = false;
    string Z;

    if (layerName == "LNDARE")
        z = landZ;

    else if ((layerName == "UWTROC")||(layerName == "WRECKS")||(layerName == "OBSTRN"))
    {
        Z = feat->GetFieldAsString("VALSOU");
        WL_flag = Z.empty();
        if (WL_flag)
        {
            WL = feat->GetFieldAsDouble("WATLEV");
            z = calcDepth(WL);
            if (z==-100)
                return;
        }
        else
            z = feat->GetFieldAsDouble("VALSOU");
    }
    else
    {
        cout << "Unknown point layer: " << layerName << endl;
        return;
    }

    storePoint(x,y,z, WL_flag);
}


// This function stores the X, Y, and depth of the point in the shapefile for post-processing
//  and in the vector to be interpolated if the depth value was given and not assumed.
//      Inputs:
//          x - X position of the point (in UTM)
//          y - Y position of the point (in UTM)
//          z - Depth of the point (in meters)
//          WL_flag - boolean describing if the if the depth value was calculated using WL
void GridENC::storePoint(double x, double y, double z, bool WL_flag)
{
    OGRPoint pt;
    double  local_x, local_y;
    OGRFeature *new_feat;

    vector<double> dx = {-grid_size,0,grid_size};
    vector<double> dy = {-grid_size,0,grid_size};

    local_x = x-geod.getXOrigin();
    local_y = y-geod.getYOrigin();

    // Store the point and its Moore Neighbors
    for (auto DX:dx)
    {
        for (auto DY: dy)
        {
            // Put into point layer
            pt = OGRPoint(x+DX,y+DY);
            new_feat =  OGRFeature::CreateFeature(feat_def_pnt);
            new_feat->SetField("Depth", z);
            new_feat->SetGeometry(&pt);

            // Build the new feature
            if( layer_pnt->CreateFeature( new_feat ) != OGRERR_NONE )
            {
                printf( "Failed to create feature in polygon shapefile.\n" );
                exit( 1 );
            }
            OGRFeature::DestroyFeature(new_feat);

            // Store the XYZ of the vertices
            // Only use the feature to interpolate if the depth is known
            if (!WL_flag)
            {
                X.push_back(local_x+DX);
                Y.push_back(local_y+DY);
                depth.push_back(z);
            }
        }
    }
}

// Store the polygon as shapefiles for rasterization and store the vertices for interpolation
void GridENC::polygonFeat(OGRFeature* feat, OGRGeometry* geom, string layerName)
{
    OGRPolygon *UTM_poly, *poly, *UTM_poly_buff;
    OGRGeometry *geom_UTM, *buff_geom;
    OGRFeature *new_feat;

    int WL;
    double z;
    string Z;

    poly = ( OGRPolygon * )geom;
    geom_UTM = geod.LatLong2UTM(geom);
    // Build the layer for the area covered by the ENC
    if (layerName == "M_COVR")
    {
        if (feat->GetFieldAsDouble("CATCOV") == 1)
        {
            UTM_poly = ( OGRPolygon * )geom_UTM;
            UTM_poly->segmentize(grid_size);
            new_feat =  OGRFeature::CreateFeature(feat_def_outline);
            new_feat->SetField("Inside_ENC", 1);
            new_feat->SetGeometry(UTM_poly);

            // Build the new feature
            if( layer_outline->CreateFeature( new_feat ) != OGRERR_NONE )
            {
                printf( "Failed to create feature in polygon shapefile.\n" );
                exit( 1 );
            }
            OGRFeature::DestroyFeature(new_feat);
        }
    }

    // Buffer and segmentize the polygon
    buff_geom = geom_UTM->Buffer(buffer_size);
    UTM_poly_buff = ( OGRPolygon * )buff_geom;
    UTM_poly_buff->segmentize(grid_size);

    if (layerName == "LNDARE")
    {
        z = landZ;
        new_feat =  OGRFeature::CreateFeature(feat_def_poly);
        new_feat->SetField("Depth", z);
        new_feat->SetGeometry(UTM_poly_buff);

        // Build the new feature
        if( layer_poly->CreateFeature( new_feat ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in polygon shapefile.\n" );
            exit( 1 );
        }
        // To limit the amount of points stored, only store the vertices of the
        //  buffered polygon.
        store_vertices(UTM_poly_buff, landZ);
        OGRFeature::DestroyFeature(new_feat);
    }
    else if ((layerName == "WRECKS")||(layerName == "OBSTRN"))
    {
        Z = feat->GetFieldAsString("VALSOU");
        if (Z.empty())
        {
            WL = feat->GetFieldAsDouble("WATLEV");
            z = calcDepth(WL);
            if (z==-100)
                return;
        }
        else
            z = feat->GetFieldAsDouble("VALSOU");

        new_feat =  OGRFeature::CreateFeature(feat_def_poly);
        new_feat->SetField("Depth", z);
        new_feat->SetGeometry(UTM_poly_buff);

        // Build the new feature
        if( layer_poly->CreateFeature( new_feat ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in polygon shapefile.\n" );
            exit( 1 );
        }
        OGRFeature::DestroyFeature(new_feat);
    }
    else if (layerName == "DEPARE")
    {
        Z = feat->GetFieldAsString("DRVAL1"); // Minimum depth in range
        if (Z.empty())
        {
            cout << "DEPARE failed" << endl;
            return;
        }

        new_feat =  OGRFeature::CreateFeature(feat_def_depth);
        new_feat->SetField("Depth", atof(Z.c_str()));
        new_feat->SetGeometry(UTM_poly_buff);

        // Build the new feature
        if( layer_depth->CreateFeature( new_feat ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in depth shapefile.\n" );
            exit( 1 );
        }
        OGRFeature::DestroyFeature(new_feat);
    }
    else if ((layerName == "PONTON")||(layerName =="FLODOC")||(layerName == "DYKCON"))
    {
        z = landZ;
        new_feat =  OGRFeature::CreateFeature(feat_def_poly);
        new_feat->SetField("Depth", z);
        new_feat->SetGeometry(UTM_poly_buff);

        // Build the new feature
        if( layer_poly->CreateFeature( new_feat ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in polygon shapefile.\n" );
            exit( 1 );
        }
        OGRFeature::DestroyFeature(new_feat);
    }
    else if (layerName=="M_QUAL")
    {
        // If the data is unreliable (ZoC D or U) do not navigate through these areas
        if (feat->GetFieldAsDouble("CATZOC") >= 4)
        {
            new_feat =  OGRFeature::CreateFeature(feat_def_poly);
            new_feat->SetField("Depth", CATZOC_z);
            new_feat->SetGeometry(UTM_poly_buff);

            // Build the new feature
            if( layer_poly->CreateFeature( new_feat ) != OGRERR_NONE )
            {
                printf( "Failed to create feature in polygon shapefile.\n" );
                exit( 1 );
            }
            OGRFeature::DestroyFeature(new_feat);
        }
    }
    else
    {
        if (layerName != "M_COVR")
        {
            cout << "Unknown Polygon layer: " << layerName << endl;
            return;
        }
    }
}

// This function stores the vertices of a polygon (that is in UTM) for interpolation
void GridENC::store_vertices(OGRPolygon *UTM_Poly, double z)
{
    OGRLinearRing *ring;

    ring = UTM_Poly->getExteriorRing();

    for (int j = 0 ; j< ring->getNumPoints(); j++)
    {
            X.push_back(ring->getX(j)-geod.getXOrigin());
            Y.push_back(ring->getY(j)-geod.getYOrigin());
            depth.push_back(z);
    }
}

// This function stores the vertices of a line (that is in UTM) for interpolation
void GridENC::store_vertices(OGRLineString *UTM_line, double z)
{
    for (int j = 0 ; j< UTM_line->getNumPoints(); j++)
    {
            X.push_back(UTM_line->getX(j)-geod.getXOrigin());
            Y.push_back(UTM_line->getY(j)-geod.getYOrigin());
            depth.push_back(z);
    }
}

void GridENC::getRasterData(string filename, int &nXSize, int &nYSize, vector<double> &RasterData)
{
    GDALDataset  *poDataset;
    GDALRasterBand *poRasterBand;
    string full_Filename = MOOS_Path +filename;

    GDALAllRegister();
    poDataset = (GDALDataset *) GDALOpen( full_Filename.c_str(), GA_ReadOnly );

    poRasterBand = poDataset -> GetRasterBand(1);

    nXSize = poRasterBand->GetXSize(); // width
    nYSize = poRasterBand->GetYSize(); // height

    // Resize the vector to fit the raster dataset
    RasterData.resize(nXSize*nYSize);

    // Read the raster into a 1D row-major vector where it is organized in left to right,top to bottom pixel order
    if( poRasterBand->RasterIO( GF_Read, 0, 0, nXSize, nYSize, RasterData.data(), nXSize, nYSize, GDT_Float64, 0, 0 ) != CE_None )
    {
        printf( "Failed to access raster data.\n" );
        GDALClose(poDataset);
        exit( 1 );
    }

    GDALClose(poDataset);
}

void GridENC::getRasterData(string filename, int &nXSize, int &nYSize, vector<int> &RasterData)
{
    GDALDataset  *poDataset;
    GDALRasterBand *poRasterBand;
    string full_Filename = MOOS_Path +filename;

    GDALAllRegister();
    poDataset = (GDALDataset *) GDALOpen( full_Filename.c_str(), GA_ReadOnly );

    poRasterBand = poDataset -> GetRasterBand(1);

    nXSize = poRasterBand->GetXSize(); // width
    nYSize = poRasterBand->GetYSize(); // height

    // Resize the vector to fit the raster dataset
    RasterData.resize(nXSize*nYSize);

    // Read the raster into a 1D row-major vector where it is organized in left to right,top to bottom pixel order
    if( poRasterBand->RasterIO( GF_Read, 0, 0, nXSize, nYSize, RasterData.data(), nXSize, nYSize, GDT_Int32, 0, 0 ) != CE_None )
    {
        printf( "Failed to access raster data.\n" );
        GDALClose(poDataset);
        exit( 1 );
    }

    GDALClose(poDataset);
}

void GridENC::writeRasterData_int(string filename, int nXSize, int nYSize, vector<int>&RasterData)
{
    GDALDataset *poDataset;
    GDALRasterBand *poBand;
    OGRSpatialReference oSRS;
    GDALDriver *poDriver;
    char *pszSRS_WKT = NULL;
    char **papszOptions = NULL;
    const char *pszFormat = "GTiff";

    string full_Filename = MOOS_Path+"/" +filename;

    //cout << "File name: " << full_Filename << endl;

    // Open file for writing
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    poDataset = poDriver->Create( full_Filename.c_str(), nXSize, nYSize, 1, GDT_Int32, papszOptions );

    // Set geo transform
    double adfGeoTransform[6] = {minX+geod.getXOrigin(), grid_size, 0, maxY+geod.getYOrigin(), 0, -grid_size};
    poDataset->SetGeoTransform( adfGeoTransform );

    // Set Coordinate system
    oSRS = geod.getUTM();
    oSRS.exportToWkt( &pszSRS_WKT );
    poDataset->SetProjection( pszSRS_WKT );
    CPLFree( pszSRS_WKT );

    // Write the file
    poBand = poDataset->GetRasterBand(1);
    if (poBand->RasterIO( GF_Write, 0, 0, nXSize, nYSize,
                      RasterData.data(), nXSize, nYSize, GDT_Int32, 0, 0 ) != CE_None )
    {
        printf( "Failed to write raster data.\n" );
        GDALClose(poDataset);
        exit( 1 );
    }

    // Once we're done, close properly the dataset
    GDALClose( (GDALDatasetH) poDataset );
}

void GridENC::writeRasterData(string filename, int nXSize, int nYSize, vector<double> &RasterData)
{
    GDALDataset *poDataset;
    GDALRasterBand *poBand;
    OGRSpatialReference oSRS;
    GDALDriver *poDriver;
    char *pszSRS_WKT = NULL;
    char **papszOptions = NULL;
    const char *pszFormat = "GTiff";

    string full_Filename = MOOS_Path +filename;

    // Open file for writing
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    poDataset = poDriver->Create( full_Filename.c_str(), nXSize, nYSize, 1, GDT_Float64, papszOptions );

    // Set geo transform
    double adfGeoTransform[6] = {minX+geod.getXOrigin(), grid_size, 0, maxY+geod.getYOrigin(), 0, -grid_size};
    poDataset->SetGeoTransform( adfGeoTransform );

    // Set Coordinate system
    oSRS = geod.getUTM();
    oSRS.exportToWkt( &pszSRS_WKT );
    poDataset->SetProjection( pszSRS_WKT );
    CPLFree( pszSRS_WKT );

    // Write the file
    poBand = poDataset->GetRasterBand(1);
    if (poBand->RasterIO( GF_Write, 0, 0, nXSize, nYSize,
                      RasterData.data(), nXSize, nYSize, GDT_Float64, 0, 0 ) != CE_None )
    {
        printf( "Failed to write raster data.\n" );
        GDALClose(poDataset);
        exit( 1 );
    }

    // Once we're done, close properly the dataset
    GDALClose( (GDALDatasetH) poDataset );
}

// Cycle through the raster data and store the xyz coordinates to be used in the interpolation
void GridENC::raster2XYZ(vector<double> &RasterData, int nXSize)
{
    double x,y;

    // Store the Polygon data before gridding
    for (int i=0; i<RasterData.size(); i++)
    {
        // Convert the coordinates from 1D to 2D then grid to local_UTM
        row_major_to_2D(i, x, y, nXSize);
        X.push_back(x);
        Y.push_back(y);
        depth.push_back(RasterData[i]);
    }
}

// Rasterize the shapefile using the commandline function gdal_rasterize
void GridENC::rasterizeSHP(string outfilename, string infilename, string attribute)
{
    // Strings to hold the data for the input/output filenames for gdal_rasterize
    string full_inFilename = MOOS_Path +infilename;
    string full_outFilename = MOOS_Path +outfilename;
    string filenames = full_inFilename + " " + full_outFilename;

    // String to hold the data for the georeferenced extents for gdal_rasterize
    string georef_extent = "-te "+to_string(minX+geod.getXOrigin())+ " "+to_string(minY+geod.getYOrigin())+ " "
            +to_string(maxX+geod.getXOrigin())+ " "+to_string(maxY+geod.getYOrigin())+ " ";

    // String for all the options for gdal_rasterize
    string options = "-a_nodata -10 -at -a " + attribute + " -tr " + to_string(grid_size)+ " " +
            to_string(grid_size)+ " -a_srs EPSG:"+to_string(EPSG_code)+" -ot Float64 " + georef_extent + filenames;

    string rasterize = "gdal_rasterize " + options;

    cout << "Rasterizing " << outfilename << endl;

    system(rasterize.c_str());
}

// Converts depths to
double GridENC::calcDepth(int WL)
{
    double WL_depth = 0;
    double feet2meters = 0.3048;

    if (WL == 2) // Always Dry
    {
        // At least 2 feet above MHW. Being shoal biased, we will take the
        //   object's "charted" depth as 2 feet above MHW
        WL_depth = -(1.0*feet2meters+MHW_Offset);
    }

    else if (WL == 3) // Always underwater
    {
        // At least 1 foot below MLLW. Being shoal biased, we will take the
        //   object's "charted" depth as 1 foot below MLLW
        WL_depth = 1.0*feet2meters;
    }

    else if (WL == 4) // Covers and Uncovers
    {
        // The range for these attributes 1 foot below MLLW to 1 foot above MHW
        //   Therefore, we will be shoal biased and take 1 foot above MHW as the
        //   object's "charted" depth.
        WL_depth = -(1.0*feet2meters+MHW_Offset);
    }

    else if (WL == 5) // Awash
    {
        // The range for these attributes 1 foot below MLLW to 1 foot above
        //   MLLW. Therefore, we will be shoal biased and take 1 foot above MLLW
        //   as the object's "charted" depth.
        WL_depth = -1.0*feet2meters;
    }

    else
    {
        // All other Water levels (1, 6, and 7) don't have a quantitative
        //   descriptions. Therefore we will set it to -100 and ignore it
        //   in the next step.
        WL_depth = -100;
    }

    return WL_depth;
}

void GridENC::xy2grid(double x, double y, int &gridX, int &gridY)
{
  // Converts the local UTM to the grid coordinates.
  gridX = int(round((x-minX-grid_size/2.0)/grid_size));
  gridY = int(round((y-minY-grid_size/2.0)/grid_size));
}

void GridENC::grid2xy(int gridX, int gridY, double &x, double &y)
{
  // Converts the grid coordinates to the local UTM.
  x = int(round(gridX*grid_size+ minX+ grid_size/2.0));
  y = int(round(gridY*grid_size+ minY+ grid_size/2.0));
}

void GridENC::LatLong2Grid(double lat, double lon, int &gridX, int &gridY)
{
    double x,y;
    geod.LatLong2UTM(lat,lon, x,y);
    xy2grid(x,y, gridX, gridY);
}


void GridENC::Grid2LatLong(int gridX, int gridY, double &lat, double &lon)
{
    double x,y;
    grid2xy(gridX, gridY, x,y);
    geod.UTM2LatLong(x,y, lat,lon);
}

void GridENC::row_major_to_2D(int index, double &x, double &y, int numCols)
{
    int gridX, gridY;
    gridY = index%numCols;
    gridX = (index-gridY)/numCols;
    grid2xy(gridX, gridY, x,y);
}

void GridENC::row_major2grid(int index, int &gridX, int &gridY, int numCols)
{
    gridY = index%numCols;
    gridX = (index-gridY)/numCols;
}

void GridENC::rasterIndex2gridIndex(int rasterIndex, int &gridIndex, int x_res, int y_res)
{
    int x,y;

    row_major2grid(rasterIndex, x,y, x_res);

    // Raster indexing is top to bottom and the grid indexing is bottom to top
    gridIndex = (y_res-1 - x)*x_res +y;
}

