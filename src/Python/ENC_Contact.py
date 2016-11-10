#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
This application has two main parts and initialization and a infinate run loop. 
    In the initialization, the program takes the information from the ENCs and 
    stores it to disk. Once that has happened, it then runs the infinate loop.
    In the infinate loop, a search polygon is built and information on the  
    objects from the ENC that are within the search area are then published to 
    the MOOSDB.
"""

# Python-MOOS Bridge
import pymoos

# Used for delays
import time

# Used to check if the file already exists
from os import path

# pyproj is a library that converts lat long coordinates into UTM
import pyproj 

# GDAL is a library that reads and writes shapefiles
from osgeo import ogr

# Numpy is a useful tool for mathematical operations 
import numpy as np

import json

from timeit import default_timer as timer
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
class MOOS_comms(object):
    """ This class id for MOOS communications. It has 2 parts:
          1. Initialize comms
              a. Register for variables
              b. Connect to the server
          2. Get new values.
    """
    
    def __init__(self):
        # Open a communication link to MOOS
        self.comms = pymoos.comms()
        # Placeholder for the list of varables that we want
        self.NAV_X = []
        self.NAV_Y = []
        self.NAV_HEAD = []
        # If there is no information on the current tide, then the first value 
        #   will be 0.
        self.Tide = []
        self.MHW_Offset = []
        # If there is no information on which ENC, use US5NH02
        self.ENCs = []
        self.origin_lat = []
        self.origin_lon = []

    def Register_Vars(self):
        """ This function registers for the updates of the X,Y and heading at 
            the rate of which it is being outputed from MOOS.
        """
        self.comms.register('NAV_X', 0)
        self.comms.register('NAV_Y', 0)
        self.comms.register('NAV_HEADING', 0)
        self.comms.register('Current_Tide',0)
        self.comms.register('MHW_Offset',0)
        self.comms.register('ENCs',0)
        self.comms.register('LatOrigin',0)
        self.comms.register('LonOrigin',0)
        return True
        
    def Set_time_warp(self, timewarp):
        if timewarp>1:
            # Time Warp and Scaling factor constant
            time_warp = timewarp
            scaling_factor = 0.04*time_warp    
            
            # Set the timewarp and scale factor
            pymoos.set_moos_timewarp(time_warp)
            self.comms.set_comms_control_timewarp_scale_factor(scaling_factor)
        
    def Initialize(self):
        """ This function registers for the current X,Y, and heading and then
            connects to the server.
        """
        # Set timewarp        
        self.Set_time_warp(1)
        
        # Register for desired variables
        self.comms.set_on_connect_callback(self.Register_Vars)
        
        # Connect to the server
        self.comms.run('localhost',9000,'Contact')
        
    def Get_mail(self):
        """ When called, this function fetches the new values for the ASV 
            position in meters (x,y) and heading and stores them into lists.
        """
        
        # Fetch the values of the ASV position in meters (x,y) and heading
        info = self.comms.fetch()
        
        # Store all values of the ASV's position
        for x in info:
            if x.is_double():
                if x.name()=='NAV_X':
                    self.NAV_X.append(x.double())
                elif x.name()=='NAV_Y':
                    self.NAV_Y.append(x.double())
                elif x.name()=='NAV_HEADING':
                    self.NAV_HEAD.append(x.double())
            elif x.is_string():
                if x.name()=='Current_Tide':
                    self.Tide.append(x.string())
                elif x.name()=='MHW_Offset':
                    self.MHW_Offset.append(x.string())
                elif x.name()=='ENCs':
                    self.ENCs.append(x.string())
                elif x.name()=='LatOrigin':
                    self.origin_lat.append(x.string())
                elif x.name()=='LongOrigin':
                    self.origin_lon.append(x.string())
                    

#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
class Search_ENC(object):
    """This application has two main parts: initialization and an infinate  
        run loop. In the initialization, the program takes the information from 
        the ENCs and stores it to disk. Once that has happened, it then runs 
        the infinate loop. In the infinate loop, a search polygon is built and 
        the objects from the ENC that are within the search area are then 
        published to the MOOSDB.
    """

    # Define which UTM zone we are in
    LonLat2UTM = pyproj.Proj(proj='utm', zone=19, ellps='WGS84')
    
    def __init__(self, search_dist=75, LatOrigin=43.071959194444446, 
                 LongOrigin=-70.711610833333339,
                 ENC_filename='../../src/ENCs/US5NH02M/US5NH02M.000', 
                 filename_pnt='../../src/ENCs/US5NH02M/Shape/point.shp', 
                 filename_poly='../../src/ENCs/US5NH02M/Shape/poly.shp', 
                 filename_line='../../src/ENCs/US5NH02M/Shape/line.shp'):
        """ Initialize varibles. 
            
            Inputs:
                search_dist - length of each side of the seach polygon
                LatOrigin - Latitude origin for the pyproj conversion to UTM
                LongOrigin - Longitude origin for the pyproj conversion to UTM
                ENC_filename - path to the ENC file
                filename_* - path to the file needed so that we can reorganize 
                            the ENC to shapefiles by geometry type 
        """
        self.LatOrigin = LatOrigin
        self.LongOrigin = LongOrigin
        self.search_dist = search_dist
        self.search_area_poly = ogr.Geometry(ogr.wkbPolygon)
        self.ENC_filename = ENC_filename
        self.filename_pnt = filename_pnt
        self.filename_poly = filename_poly
        self.filename_line = filename_line
        # Want the first itteration to calculate threat level at MLLW
        self.tide = 0
        self.MHW_Offset = 0
    
    def ang4MOOS(self, angle):
        """ This fucntion converts a typical angle to one that is 0 degrees at
        North
        """
        return -(angle-90)
        
    def LonLat2MOOSxy(self, lon, lat):
        """ This function converts Longitude and Latitude to MOOS X and Y
        
        Inputs:
            lat - Latitude to be converted to UTM
            lon - Longitude to be converted to UTM
            
        Output:
            x - Corresponding X location in UTM
            y - Corresponding Y location in UTM
        """
        x,y = self.LonLat2UTM(lon, lat)
        x += -self.x_origin
        y += -self.y_origin
        return x,y  
            
    def MOOSxy2LonLat(self, x, y):
        """ This function converts MOOS X,Y to Longitude and Latitude
        
        Inputs:
            x - X location in UTM coordinates
            y - Y location in UTM coordinates
            
        Output:
            lat - Corresponding latitude location
            lon - Corresponding longitude location
        """
        lat,lon = self.LonLat2UTM(x+self.x_origin, y+self.y_origin, inverse=True)
        return lat,lon
        
    def update_WL(self, WL):
        """ This function updates the Water Level attribute with the current 
            predicted tide. This is shoal biased. The values used in these  
            calculations are from NOAA's Nautical Chart User Manual:
                
            http://portal.survey.ntua.gr/main/labs/carto/academic/persons/bnakos_site_nafp/documentation/noaa_chart_users_manual.pdf
            
        Inputs:
            WL - Water level for the obstacle 
                    (qualitative depth measurement)
            
        Outputs:
            WL_depth - current depth in relation to the 
        """
        WL = float(WL)
        feet2meters = 0.3048
        if WL == 2:
            # At least 2 feet above MHW. Being shoal biased, we will take the 
            #   object's "charted" depth as 2 feet above MHW
            WL_depth = self.tide-(2*feet2meters+self.MHW_Offset)
            
        elif WL == 3:
            # At least 1 foot below MLLW. Being shoal biased, we will take the 
            #   object's "charted" depth as 1 foot below MLLW
            WL_depth = self.tide+1*feet2meters
            
        elif WL == 4:
            # The range for these attributes 1 foot below MLLW and 1 foot above MHW
            #   Therefore, we will be shoal biased and take 1 foot above MHW as the
            #   object's "charted" depth.
            WL_depth = self.tide-(1*feet2meters+self.MHW_Offset)
            
        elif WL == 5:
            # The range for these attributes 1 foot below MLLW and 1 foot above 
            #   MLLW. Therefore, we will be shoal biased and take 1 foot above MLLW
            #   as the object's "charted" depth.
            WL_depth = self.tide-1*feet2meters
            
        elif WL == 0:
            # Make the threat level completely dependant on the sounding depth
            WL_depth = 99
            
        else:
            # All other Water levels (1, 6, and 7) don't have a quantitative 
            #   descriptions. Therefore we will set it to 0.
            WL_depth = 0
            
        return WL_depth
         
    def calc_t_lvl(self, depth, WL, LayerName):
        """ This function uses the water level and depth attributes for a  
            feature and calculates the threat level for that obstacle.
            
            Inputs:
                depth - Recorded depth for the obstacle
                        (quantitative depth measurement)
                WL - Water level for the obstacle 
                        (qualitative depth measurement)
                self.tide - current tide
                
            Outputs:
                t_lvl - Calculated threat level
        """
        # If it is Land set threat level to 5
        if LayerName == 'LNDARE' or LayerName == 'DYKCON' or LayerName == 'PONTON' or LayerName == 'COALNE':
            t_lvl = 5
        elif LayerName == 'LIGHTS':
            t_lvl = -2
        # If it is a Buoy or Beacon set threat level to 3
        elif LayerName == 'BOYISD' or LayerName == 'BOYSPP' or LayerName == 'BOYSAW' or LayerName == 'BOYLAT' or LayerName == 'BCNSPP' or LayerName == 'BCNLAT':
            t_lvl = 3
        elif LayerName == 'LNDMRK':
            t_lvl = -1
        else:
            # Deal with the cases where the attributes are empty
            if WL == None:
                WL = 0 
            if depth == None or depth == 9999:
                current_depth = 9999
            else:
                current_depth = depth+self.tide    
            # WL is unknown
            if WL == 0:
                t_lvl = self.threat_level(current_depth)  
            # No Charted Depth
            elif current_depth == 9999:
                # If there is no depth, use the Water Level attribute to 
                #   calculate the threat level
                t_lvl = self.threat_level(self.update_WL(WL))
            # Neither the WL or depth are recorded - this is bad
            elif ((current_depth == 9999) and (WL == 0)):
                print 'FAILED, Threat Level will be set to 4'
                t_lvl = 4
            # If we have both the WL and the depth, use the depth measurement
            else:
#                WL_depth = self.calc_WL_depth(WL)
#                # Go with the recorded depth unless it is more than a meter off
#                if ((current_depth-WL_depth) < -1):
#                    z = current_depth
#                else:
#                    z = WL_depth
                t_lvl = self.threat_level(current_depth)

        return t_lvl
    
    def threat_level(self, depth):
        """ This function uses a depth for a feature (Determined by a sounding 
            or realative to the WL attribute) and calculates the threat level 
            for that obstacle.
            
        Inputs:
            depth - Recorded depth for the obstacle (actual or relative to WL)
            
        Outputs:
            t_lvl - Calculated threat level
        """
        # Above the water surface
        if (depth<=0):
            t_lvl = 4
        # Near the water surface
        elif (depth< 1):
            t_lvl = 3
        # Obstacle below surface
        elif (depth >= 1):
            # 1<=depth<2 
            if (depth < 2):
                t_lvl = 2
            # 2<=depth<4
            elif (depth >=2 and depth <= 4):
                t_lvl = 1
            # Obstacle is deep (depth > 4m)
            else:
                t_lvl = 0
                
        return t_lvl
            
                
    # Converts the number stored in Category of Lights to string    
    def category_lights(self, feat):
        """ This function takes in a light feature and converts the number 
        stored in category attribute to string.
        
        Inputs: 
            feat - Feature from the light layer of an ENC
            
        Outputs:
            A string that holds the information from the category of lights 
                attribute from the ENC
        """
        
        index = str(feat.GetField(12))
        if index == '1':
            return 'Directional Function'
        # Index 2 and 3 are not used
        elif index =='4':
            return 'Leading'
        elif index =='5':
            return 'Aero'
        elif index =='6':
            return 'Air Obstruction'
        elif index == '7':
            return 'Fog Detector'
        elif index == '8':
            return 'Flood'
        elif index =='9':
            return 'Strip'
        if index == '10':
            return 'Subsidiary'
        if index == '11':
            return 'Spot'
        if index == '12':
            return 'Front'
        if index == '13':
            return 'Rear'
        if index == '14':
            return 'Lower'
        if index == '15':
            return 'Upper'
        if index == '16':
            return 'Moire Effect'
        if index == '17':
            return 'Emergency'
        if index == '18':
            return 'Bearing'
        if index == '19':
            return 'Horizontally Disposed'
        elif index =='20':
            return 'Vertically Disposed'
        else:
            return 'Marine'

    # Converts the number stored in category of Landmark or Silo/Tank to a string
    def category_landmark(self, feat, name):
        """ This function takes in a landmark or Silo/Tank feature and converts
        the number stored in category attribute to string.
        
        Inputs: 
            feat - Feature from the light layer of an ENC
            name - Layer name (either Landmark or Silo/Tank)
            
        Outputs:
            A string that holds the information from the category of landmark 
                attribute from the ENC
        """
        if name == 'LNDMRK':
            index = str(feat.GetField(11))
            if index == '1':
                return 'Cairn'
            elif index =='2':
                return 'Cemetery'
            elif index =='3':
                return 'Chimney'
            elif index =='4':
                return 'Dish Aerial'
            elif index =='5':
                return 'Flagstaff'
            elif index =='6':
                return 'Flare Stack'
            elif index == '7':
                return 'Mast'
            elif index == '8':
                return 'Windsock'
            elif index =='9':
                return 'Monument'
            elif index == '10':
                return 'Column'
            elif index == '11':
                return 'Memorial Plaque'
            elif index == '12':
                return 'Obelisk'
            elif index == '13':
                return 'Statue'
            elif index == '14':
                return 'Cross'
            elif index == '15':
                return 'Dome'
            elif index == '16':
                return 'Radar Scanner'
            elif index == '17':
                return 'Tower'
            elif index == '18':
                return 'Windmill'
            elif index == '19':
                return 'Windmotor'
            elif index =='20':
                return 'Spire'
            elif index =='21':
                return 'Large On Land Rock'
            else:
                return 'Unknown Landmark'
                
        elif name == 'SILTNK':
            index = str(feat.GetField(12))
            if index == '1':
                return 'Silo'
            elif index =='2':
                return 'Tank'
            elif index =='3':
                return 'Grain Elevator'
            elif index =='4':
                return 'Water Tower'
            else:
                return 'Unknown' 
    
    def print_fields(self, LayerName):
        """ This function print the available fields for the inputted layer.
        
            Input:
                LayerName - Name of the layer that you want to display the 
                    available field for
            Ouput:
                Fields - String of the fields for that layer
        """
        Fields = []
        layer = self.ds.GetLayerByName(LayerName)
        layer_def = layer.GetLayerDefn()
        print str(LayerName)
        for i in range(layer_def.GetFieldCount()):
            print str(i) + ': ' + layer_def.GetFieldDefn(i).GetName()
            Fields.append(layer_def.GetFieldDefn(i).GetName())
        
        return Fields
            
    def Get_Layer_Names(self):
        """ This function returns the names of all of the layers in
            the ENC.
        
            Output:
                LayerNames - Names of the layer in the ENC
        """
        LayerNames = []
        for i in range(self.ds.GetLayerCount()):
            name = self.ds.GetLayerByIndex(i).GetName()
#            print str(i) + ': ' + name
            LayerNames.append(name)
        
        return LayerNames
            
    def BuildLayers(self):
        """ Create a OGR layer for the point obstacles, polygon obstacles and  
            the line obstacles.
        
        Outputs (these are not returned, but defined as self.):
            ENC_point_layer - OGR layer that holds all the information from the 
                                ENC that have point geometry
            ENC_poly_layer - OGR layer that holds all the information from the 
                                ENC that have polygon geometry
            ENC_line_layer - OGR layer that holds all the information from the 
                                ENC that have line geometry
        """
                
        # Use the Shapefile driver for gdal
        shape_driver = ogr.GetDriverByName('Esri Shapefile')
        
        ## Point Output File
        # If there already is a file with that name, delete it
        if path.exists(self.filename_pnt):
            shape_driver.DeleteDataSource(self.filename_pnt)
        
        # create an output file
        self.ds_pnt = shape_driver.CreateDataSource(self.filename_pnt)
        
        ## Polygon Output File
        # If there already is a file with that name, delete it
        if path.exists(self.filename_poly):
            shape_driver.DeleteDataSource(self.filename_poly)
        
        # create an output file
        self.ds_poly = shape_driver.CreateDataSource(self.filename_poly)
        
        ## Line Output File
        # If there already is a file with that name, delete it
        if path.exists(self.filename_line):
            shape_driver.DeleteDataSource(self.filename_line)
        
        # create an output file
        self.ds_line = shape_driver.CreateDataSource(self.filename_line)
        
        ## Buld the layers of obstacles seperated by geometry type
        # Create a layer for the point obstacles, polygon obstacles and the 
        #   line obstacles.
        self.ENC_point_layer = self.ds_pnt.CreateLayer('Points', None, ogr.wkbPoint)
        self.ENC_poly_layer = self.ds_poly.CreateLayer('Poly', None, ogr.wkbPolygon)
        self.ENC_line_layer = self.ds_line.CreateLayer('Line', None, ogr.wkbLineString)
        
        # Add the Threat Level and Type Attributes
        self.ENC_point_layer.CreateField(ogr.FieldDefn('T_lvl', ogr.OFTInteger))
        self.ENC_point_layer.CreateField(ogr.FieldDefn('WL', ogr.OFTInteger))
        self.ENC_point_layer.CreateField(ogr.FieldDefn('Depth', ogr.OFTInteger))
        self.ENC_point_layer.CreateField(ogr.FieldDefn('Type', ogr.OFTString))
        self.ENC_point_layer.CreateField(ogr.FieldDefn('Cat', ogr.OFTString))
        self.ENC_point_layer.CreateField(ogr.FieldDefn('Visual', ogr.OFTInteger))
        
        # Add the Threat Level and Type Attributes
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('T_lvl', ogr.OFTInteger))
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('WL', ogr.OFTInteger))
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('Depth', ogr.OFTInteger))
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('Type', ogr.OFTString))
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('Cat', ogr.OFTString))
        self.ENC_poly_layer.CreateField(ogr.FieldDefn('Visual', ogr.OFTInteger))
        
        # Add the Threat Level and Type Attributes
        self.ENC_line_layer.CreateField(ogr.FieldDefn('T_lvl', ogr.OFTInteger))
        self.ENC_line_layer.CreateField(ogr.FieldDefn('WL', ogr.OFTInteger))
        self.ENC_line_layer.CreateField(ogr.FieldDefn('Depth', ogr.OFTInteger))
        self.ENC_line_layer.CreateField(ogr.FieldDefn('Type', ogr.OFTString))
        self.ENC_line_layer.CreateField(ogr.FieldDefn('Cat', ogr.OFTString))
        self.ENC_line_layer.CreateField(ogr.FieldDefn('Visual', ogr.OFTInteger))
        
    def LayerMultiPoint (self, LayerName_mp):
        """ Adds the features from the inputed multipoints layer to a point 
            layer.
        
        Inputs:
            LayerName_mp - Name of the multipoint layer
        """
        
        layer_mp = self.ds.GetLayerByName(LayerName_mp)
        feat_mp = layer_mp.GetNextFeature()
        
        while feat_mp is not None:
            multi_geom = feat_mp.GetGeometryRef()
            for iPnt in range(multi_geom.GetGeometryCount()):
                # Create point
                point = ogr.Geometry(ogr.wkbPoint)
                pnt = multi_geom.GetGeometryRef(iPnt)
                point.SetPoint_2D(0,pnt.GetX(),pnt.GetY())
                
                defn_pnt = self.ENC_point_layer.GetLayerDefn() 
                
                # Create a new feature (attribute and geometry)
                depth = float(pnt.GetZ())
                new_feat = ogr.Feature(defn_pnt)
                new_feat.SetField('T_lvl', self.calc_t_lvl(depth,0, LayerName_mp))
                new_feat.SetField('Depth', depth)
                new_feat.SetField('Type', LayerName_mp)
        
                # Make a geometry for the new feature from Shapely object
                new_feat.SetGeometry(point)
    
                self.ENC_point_layer.CreateFeature(new_feat) 
            feat_mp = layer_mp.GetNextFeature()
              
    def ENC_Converter(self, LayerName):
        """ This function converts the inputed layer from the ENC to a 
            shapefile layer if the layer is in the ENC. If the inputted layer 
            is not in the ENC, the layer is skipped and the function prints a 
            warning to the user.
        """

        if LayerName in self.Get_Layer_Names():
            layer = self.ds.GetLayerByName(LayerName)
            layer_def = layer.GetLayerDefn() # needed to find the name of the fields
            feat = layer.GetNextFeature()
            
            while feat is not None:
                # Needed for determining the geom type and to export the object to
                #   WKT
                geom = feat.GetGeometryRef() 
                geo_name = geom.GetGeometryName()    
                
                # Export this feature to WKT for easy conversion to Shapefile
                wkt = geom.ExportToWkt()
            
                ## Initialize WL, depth, and threat level
                WL = 0
                depth = 9999
                t_lvl = 0
                
                ## Cycle through the fields and store the ones that are useful
                for i in range(feat.GetFieldCount()):
                    name = layer_def.GetFieldDefn(i).GetName()
                    if name == 'WATLEV':
                        WL = int(feat.GetField(i))
                        if (WL in [1,6,7]):
                            print '{} - weird Water Level {}'.format(LayerName,WL)
                    elif name == 'VALSOU':
                        depth = feat.GetField(i)
                        if depth is None:
                            depth = 9999
                    elif name == 'VALDCO': # value of the depth contour
                        depth = feat.GetField(i)
                        if depth is None:
                            depth = 9999

                ## Update Threat Level
                t_lvl = self.calc_t_lvl(depth, WL, LayerName)
                
                # Reset the output layer
                out_layer = None
                
                # Choose the correct layer for the geometry of the feature
                if geo_name == 'POINT':
                    out_layer = self.ENC_point_layer
                    obj = ogr.Geometry(ogr.wkbPoint)
                elif geo_name == 'POLYGON':
                    out_layer = self.ENC_poly_layer
                    obj = ogr.Geometry(ogr.wkbPolygon)
                elif geo_name == 'LINESTRING':
                    out_layer = self.ENC_line_layer
                    obj = ogr.Geometry(ogr.wkbLineString)
                    
                # Create a new feature (attribute and geometry)
                defn = out_layer.GetLayerDefn()
                new_feat = ogr.Feature(defn)
                new_feat.SetField('T_lvl', t_lvl)
                new_feat.SetField('Type', LayerName)
                
                if WL != 0:
                    new_feat.SetField('WL', WL)
                
                if depth!=9999:
                    new_feat.SetField('Depth', depth)
                
                # If it is a Landmark or Light set the catagory field 
                #   (and visual field for landmarks)
                if LayerName == 'LNDMRK':
                    new_feat.SetField('Cat', self.category_landmark(feat,LayerName)) # Category - store as string and not a number
                    new_feat.SetField('Visual', 2-feat.GetField(16)) # Visually Conspicuous (Y - store as 1, N - store as 0)
                elif LayerName == 'SILTNK':
                    new_feat.SetField('Cat', self.category_landmark(feat,LayerName)) # Category - store as string and not a #
                    new_feat.SetField('Visual', 2-feat.GetField(17)) # Visually Conspicuous (Y - store as 1, N - store as 0)
                elif LayerName == 'LIGHTS':
                    self.category_lights(feat)
                    
                # Make a geometry from wkt object
                obj = ogr.CreateGeometryFromWkt(wkt)
                new_feat.SetGeometry(obj)
        
                # Output the new feature to the correct layer
                if geo_name == 'POINT':
                    self.ENC_point_layer.CreateFeature(new_feat)
                    
                elif geo_name == 'POLYGON':
                    self.ENC_poly_layer.CreateFeature(new_feat)
                    
                elif geo_name == 'LINESTRING':
                    self.ENC_line_layer.CreateFeature(new_feat)
                
                # Get the next feature
                feat = layer.GetNextFeature()
        else:
            print "Layer " + LayerName + " is not in the ENC"
    
    def read_ENC(self):
        """ This converts the ENC into OGR layers by geometry types. This 
            information will then be used by the all other functions in this 
            class.
            
        Outputs:
            ENC_point_layer - OGR layer that holds all the information from the 
                                ENC that have point geometry
            ENC_poly_layer - OGR layer that holds all the information from the 
                                ENC that have polygon geometry
            ENC_line_layer - OGR layer that holds all the information from the 
                                ENC that have line geometry
        """
        
        # Create a OGR layer for the point obstacles, polygon obstacles and  
        #   the line obstacles. 
        self.BuildLayers()
        
        # Layers that are only points --> UWTROC, LIGHTS, BOYSPP, BOYISD, 
        #   BOYSAW, BOYLAT, BCNSPP, BCNLAT
        
        self.ENC_Converter('UWTROC')
        self.ENC_Converter('LIGHTS')
        self.ENC_Converter('BOYSPP')
        self.ENC_Converter('BOYISD')
        self.ENC_Converter('BOYSAW')
        self.ENC_Converter('BOYLAT')
        self.ENC_Converter('BCNSPP')
        self.ENC_Converter('BCNLAT')
        
        # Add the features from the SOUNDG multipoint layer to the point layer
        self.LayerMultiPoint('SOUNDG')
        
        # Layers that have multiple types --> WRECKS, OBSTRN, LNDARE, DEPARE, 
        #   PONTON, DEPCNT, DYKCON
        self.ENC_Converter('WRECKS')
        self.ENC_Converter('LNDARE')
        self.ENC_Converter('PONTON')
        self.ENC_Converter('DEPCNT')
        self.ENC_Converter('DYKCON')
        self.ENC_Converter('LNDMRK')
        self.ENC_Converter('SILTNK')
        
        return self.ENC_point_layer, self.ENC_poly_layer, self.ENC_line_layer
        
    def build_search_poly(self, X, Y, heading, comms):
        """ This function builds the polygon which represents the area that
            the ASV is going to search the ENC for potential threats. The shape 
            of the search area is a square that is centered on the ASV's 
            current X,Ylocation. It also prints the search area polygon to 
            pMarnine Viewer.
        
        Inputs: 
            X - Current X position of the ASV
            Y - Current Y position of the ASV
            heading - Current heading of the ASV
            comms - Communication link to MOOS
        
        Outputs:
            search_area_poly - OGR Polygon that describes the area in which we 
                            want to seach the input layer for objects
        """
        
        search_area_ring = None
        self.search_area_poly = None
            
        # Create the baseline for the search area polygon 
        search_area_ring = ogr.Geometry(ogr.wkbLinearRing)
        self.search_area_poly = ogr.Geometry(ogr.wkbPolygon)
        
        # Calculate the correct corrections to have the search area spin 
        #   rotate with respect to the ASV's heading
        theta = 45-np.mod(self.ang4MOOS(heading), 90) # Degrees
        add_sin = self.search_dist*np.sqrt(2)*np.sin(theta*np.pi/180)
        add_cos = self.search_dist*np.sqrt(2)*np.cos(theta*np.pi/180)
        
        # Convert the positions to latitude and longitude
        pt1_lon,pt1_lat = self.MOOSxy2LonLat(X+add_sin, Y+add_cos)
        pt2_lon,pt2_lat = self.MOOSxy2LonLat(X-add_cos, Y+add_sin)
        pt3_lon,pt3_lat = self.MOOSxy2LonLat(X-add_sin, Y-add_cos)
        pt4_lon,pt4_lat = self.MOOSxy2LonLat(X+add_cos, Y-add_sin)
        
        # Build a ring of the points for the search area polygon
        search_area_ring.AddPoint(pt1_lon, pt1_lat)
        search_area_ring.AddPoint(pt2_lon, pt2_lat)
        search_area_ring.AddPoint(pt3_lon, pt3_lat)
        search_area_ring.AddPoint(pt4_lon, pt4_lat)
        search_area_ring.CloseRings()
        self.search_area_poly.AddGeometry(search_area_ring) # Add the ring to the previously created polygon         
        
        # Show the Search Radius Polygon on pMarnineViewer
        s_poly_vert = 'pts={'+str(X+add_sin)+','+ str(Y+add_cos)+':'+ str(X-add_cos)+','+str(Y+add_sin)+':'+ str(X-add_sin)+','+str(Y-add_cos)+':' +str(X+add_cos)+','+str(Y-add_sin)+'},'
        comms.notify('VIEW_POLYGON', s_poly_vert+'label=Search,edge_size=10,vertex_size=1,edge_color=red',)
        
        return self.search_area_poly
        
    def filter_features(self):
        """ This function uses OGR's filter commands to limit the objects in 
            the OGR layer to the search area.
        """
        # Remove the old spatial filter
        self.ENC_point_layer.SetSpatialFilter(None)
        self.ENC_poly_layer.SetSpatialFilter(None)
        
        # Remove the old attribute filter
        self.ENC_point_layer.SetAttributeFilter(None)
        self.ENC_poly_layer.SetAttributeFilter(None)
        
        
        # Filter out data to determine the search area
        self.ENC_point_layer.SetSpatialFilter(self.search_area_poly)
#        self.ENC_poly_layer.SetSpatialFilter(self.search_area_poly)
        self.ENC_point_layer.SetAttributeFilter("t_lvl>0")
        self.ENC_poly_layer.SetAttributeFilter("t_lvl>0")
        self.ENC_line_layer.SetAttributeFilter("t_lvl>0")
        
    def publish_points(self, X, Y, heading, comms):
        """ This function filters the point layer and then cycles through that 
            layer of point obstacles from the ENC to highlight them in
            pMarnineViewer and publish the x,y position, threat level and 
            obstacle type to the MOOSDB. At the end, it checks to see if any of 
            the highlighted obstacles are no longer within the search and if 
            there are any wrongly highlighted obstacles, it removes them.
            
        Inputs:
            X - Current X position of the ASV
            Y - Current Y position of the ASV
            heading - Current heading position of the ASV
            comms - Communication link to MOOS          
        """
        
        # This counter is used to count the number of features in the 
        #   search radius. This is then used to determine if any of the old
        #   hazard polygons need to be removed.
        counter = 1
        
        # Counter to determine how many obstacles that are above threat 
        #   level 0 in the search radius
        num_obs = 0
        
        # Intialize the obstacle string
        obs_pos = ''
        
        feature = self.ENC_point_layer.GetNextFeature()
        
        # Highlight all point features in search radius in pMarineViewer 
        #   and store them into a MOOS variable called "Obstacles"
        while(feature):
            time.sleep(.01)
            geom_point = feature.GetGeometryRef()
            new_x, new_y = self.LonLat2MOOSxy(geom_point.GetX(), geom_point.GetY())
            
            pos = 'x='+str(new_x)+',y='+str(new_y)
            highlight_obs = 'format=radial,'+pos+',radius=25,pts=8,edge_size=5,vertex_size=2,edge_color=aquamarine,vertex_color=aquamarine,label='+str(counter)
            comms.notify('VIEW_POLYGON', highlight_obs)
            
            # Store information for the obstacle to be used later to post to the MOOSDB
            # x,y,t_lvl,type
            # if it isnt the first obstacle put a ! at the end
            if num_obs!=0:
                obs_pos += '!'
            num_obs += 1
            WL = feature.GetField(1)
            depth = feature.GetField(2)
            obs_type = feature.GetField(3)
            t_lvl = self.calc_t_lvl(depth, WL, obs_type)
            obs_pos += '{},{},{}'.format(pos, t_lvl, obs_type)

            # Go to the next feature and increment the counter
            feature = self.ENC_point_layer.GetNextFeature()
            counter += 1
            
        # Output to the MOOSDB a list of obstacles
        #   ASV_X,ASV_Y,Heading : # of Obstacles : x=x_obs,y=y_obs,t_lvl,type ! x=x_obs,y=y_obs,t_lvl,type ! ...
        obstacles = str(X)+','+str(Y)+','+str(heading)+':'+str(num_obs)+':'+obs_pos
        
        comms.notify('Obstacles', obstacles)
        # Determine if a new polygon was used
        if self.max_pnt_obs < counter:
            self.max_pnt_obs = counter  
            
        # Remove highlighted point obstacles (shown as polygons) from 
        #   pMarineViewer if they are outside of the search area
        for i in range(counter, self.max_pnt_obs):
            time.sleep(.002)
            remove_highlight = 'format=radial,x= 0,y=0,radius=25,pts=8,edge_size=5,vertex_size=2,active=false,label='+str(i)
            comms.notify('VIEW_POLYGON', remove_highlight)
    
    def verify_poly(self, ASV_X, ASV_Y, heading, comms, buff_polygon, cntr):
        """ This function posts position of the maximum angular envelope from 
            the ASV to the buffered obstacle and the vertex of the obstacle 
            that is closest to the ASV to the MOOSDB. These points are also 
            highlighted in pMarnineViewer by colored points where the white 
            points are the angular envelope and the blue point is min distance.
        
        Inputs:
            ASV_X - Current X position of the ASV
            ASV_Y - Current Y position of the ASV
            heading - Current heading position of the ASV
            comms - Communication link to MOOS
            buff_polygon - Buffered polygon
            cntr - Current value of the counter used the number of features in  
                    the search radius. 
            
        Output:
            poly_str - String that holds the x,y position of the maximum 
                        angular envelope and minimum distance
        """
        # Initialize variables
        x1 = 0
        y1 = 0
        x2 = 0
        y2 = 0
        x_small = 9999
        y_small = 9999
        
        min_angle = 360
        max_angle = -360
        min_dist  = 9999
        
        ring = buff_polygon.GetGeometryRef(0)
        num_points =  ring.GetPointCount()
        flag = True
        # Cycle through the to find the matching x and y coordinates
        for i in range (0, num_points):
            lon = ring.GetX(i)
            lat = ring.GetY(i)
            ptx, pty = self.LonLat2MOOSxy(lon, lat)
    
            d = np.sqrt(np.square(ASV_X-ptx)+np.square(ASV_Y-pty))
            angle = np.arctan2(ASV_Y-pty, ASV_X-ptx)*180/np.pi      
            
            # There is one major issue with finding the minimum and maximum angles.
            #   It is that when the angle switches from 360 to 0. This will lead to
            #   incorrect angles being saved as the minimum and maximum. Therefore
            #   when the angle is in the right half plane we will use -180 to 180 
            #   and when the angle is in the left half plane, we will use 0 to 360.
            
            # Check to see if the first point is in the right or left hand plane.
            #   From this, it will set if the angle will range from 0 to 360 or 
            #   -180 to 180.
            if i==0:
                # Case 1: Right half plane --> flag = false
                if (-90 <= angle < 90):
                    flag = False
                # Case 2: Left half plane --> flag = true
                else:
                    flag = True
                    
            # If it is Case 2, then have it go from 0 to 360. Otherwise the angle
            #   will range from -180 to 180
            if flag:
                angle = np.mod(angle, 360)
                
            # Determine if the angle is the minimum angle
            if angle < min_angle:
                min_angle = angle
                d_min_ang = d
                x1 = ptx
                y1 = pty
            
            # Determine if the angle is the maximum angle
            if angle > max_angle:
                max_angle = angle
                d_max_ang = d
                x2 = ptx
                y2 = pty
            
            # Check if this point is the points match the envelope and to find  
            #   the x and y coordinates is the shortest distance
            if min_dist > d:
                min_dist = d
                x_small = ptx
                y_small = pty
            
        # Post these points to pMarineViewer
        pt1 = 'x={},y={},vertex_size=6.5,vertex_color=white,active=true,label=pt1_{}'.format(x1, y1, cntr)
        comms.notify('VIEW_POINT', pt1)
        time.sleep(.001)
        pt2 = 'x={},y={},vertex_size=6.5,vertex_color=white,active=true,label=pt2_{}'.format(x2, y2, cntr)
        comms.notify('VIEW_POINT', pt2)
        pt3 = 'x={},y={},vertex_size=6.5,vertex_color=mediumblue,active=true,label=pt3_{}'.format(x_small, y_small, cntr)
        comms.notify('VIEW_POINT', pt3)
        
        if num_points>0:
        # min_ang_x,min_ang_y,min_dist_x,min_dist_y,max_ang_x,max_ang_y @ min_ang_dist,max_ang_dist,min_dist
            poly = '{},{},{},{},{},{}@{},{},{}'.format(x1,y1,x_small,y_small,x2,y2,d_min_ang, d_max_ang, min_dist)
#            poly = str(x1)+','+str(y1)+','+str(x_small)+','+str(y_small)+','+str(x2)+','+str(y2)+'@'+str(d_min_ang)+','+str(d_max_ang)+','+str(min_dist)
        else:
            poly = "No points"    
        
        return poly   
        
    def convert2np(self, feat):
        """ This function takes in a ogr feature and converts its vertices to a
            2D numpy array 
        
        Inputs:
            feat - Feature that we want its vertices
            
        Outputs:
            np_array - Numpy array of the feature's vertices
        """
        name = feat.GetGeometryRef().GetGeometryName()   
        
        # Get json representation of the feature
        feat_json = json.loads(feat.ExportToJson())
        
        # Convert json representation to list of vertices
        if name == 'POLYGON':
            vertices = feat_json["geometry"]['coordinates'][0]
        else:
            vertices = feat_json["geometry"]['coordinates']
        
        # Convert that list to a numpy array and return it
        np_array = np.asarray(vertices)
        
        return np_array
        
    def find_crit_pnts(self, feat, ASV_X, ASV_Y, comms, cntr):
        """ This function finds the maximum angular extent of the obstacle with
            respect to the ASV and the vertex of the obstacle that is closest 
            to the ASV.
            
        Inputs:
            feat - OGR feature of the obstacle
            ASV_X - X position of the ASV
            ASV_Y - Y position of the ASV
            comms - Communication link to MOOS
            cntr - Current value of the counter used the number of features in  
                    the search radius. 
            
        Ouputs:
            min_ang - Minimum angle with respect to the ASV
            max_ang - 
            min_dist - 
        """
        # Get the vertices of the polygon
        vertices_latlon = self.convert2np(feat)
        
        # Convert the vertices from lat/long to X/Y
        vertices_x, vertices_y = self.LonLat2MOOSxy(vertices_latlon[:,0],vertices_latlon[:,1])
        
        # Convert the position of the ASV to a matrix
        pos = np.ones_like(vertices_x)
        ASV_x = ASV_X*pos
        ASV_y = ASV_Y*pos
        
        # Calculte the distance and angle from the ASV to the polygon
        dist2poly = np.sqrt(np.square(ASV_x-vertices_x)+np.square(ASV_y-vertices_y))
        angle2poly = np.arctan2(ASV_y-vertices_y, ASV_x-vertices_x)*180/np.pi
        
        # Determine the minimum distance, minimum angle, and maximum angle
        index_min_dist = np.argmin(dist2poly)
        index_min_ang = np.argmin(angle2poly)
        index_max_ang = np.argmax(angle2poly)
        
        # There is one major issue with finding the minimum and maximum angles.
        #   It is that when the angle switches from 360 to 0. This will lead to
        #   incorrect angles being saved as the minimum and maximum. Therefore
        #   if any of the any of the angles are near the cross over point, 
        #   use the domain [0,360] instead of [-180, 180]
        if ((angle2poly[index_min_ang] >150) or (angle2poly[index_max_ang] < -150)):
            angle2poly = np.mod(angle2poly, 360)
            index_min_ang = np.argmin(angle2poly)
            index_max_ang = np.argmax(angle2poly)
        
        min_dist = [vertices_x[index_min_dist], vertices_y[index_min_dist], dist2poly[index_min_dist], angle2poly[index_min_dist]]
        min_ang = [vertices_x[index_min_ang], vertices_y[index_min_ang], dist2poly[index_min_ang], angle2poly[index_min_ang]]
        max_ang = [vertices_x[index_max_ang], vertices_y[index_max_ang], dist2poly[index_max_ang], angle2poly[index_max_ang]]
        
#        print '{} Min Dist: {},{} - Dist:{}, Ang:{}'.format(cntr,min_dist[0],min_dist[1],min_dist[2],min_dist[3])
#        print '{} Min Ang: {},{} - Dist:{}, Ang:{}'.format(cntr,min_ang[0],min_ang[1],min_ang[2],min_ang[3])
#        print '{} Max Ang: {},{} - Dist:{}, Ang:{}'.format(cntr,max_ang[0],max_ang[1],max_ang[2],max_ang[3])
        
        # Publish the critical points to the MOOSDB
        pt1 = 'x={},y={},vertex_size=6.5,vertex_color=white,active=true,label=pt1_{}'.format(vertices_x[index_min_ang], vertices_y[index_min_ang], cntr)
        comms.notify('VIEW_POINT', pt1)
        time.sleep(.0001)
        pt2 = 'x={},y={},vertex_size=6.5,vertex_color=white,active=true,label=pt2_{}'.format(vertices_x[index_max_ang], vertices_y[index_max_ang], cntr)
        comms.notify('VIEW_POINT', pt2)
        time.sleep(.0001)
        pt3 = 'x={},y={},vertex_size=6.5,vertex_color=mediumblue,active=true,label=pt3_{}'.format(vertices_x[index_min_dist], vertices_y[index_min_dist], cntr)
        comms.notify('VIEW_POINT', pt3)        
        
        crit_pnts  = '{},{},{},{},{},{}@{},{},{}'.format(min_ang[0],min_ang[1], min_dist[0],min_dist[1], max_ang[0],max_ang[1], min_ang[2], max_ang[2], min_dist[2])
        return crit_pnts#min_ang, max_ang, min_dist
        
    def publish_poly(self, X, Y, heading, comms):
        """ This function determines which obstacles with polygon geometry from
            the ENC are within the search area. 
        
        Inputs:
            X - Current X position of the ASV
            Y - Current Y position of the ASV
            heading - Current heading position of the ASV
            comms - Communication link to MOOS
        """
        # Initialize the polygon obstacle strings
        poly_obs_verticies = ''
        poly_info = ''
        
        # This counter is used to count the number of features in the search
        #   radius. 
        counter_poly = 0
        
        feat = self.ENC_poly_layer.GetNextFeature()
        while(feat): 
            geom_poly = feat.GetGeometryRef() # Polygon from shapefile
            if geom_poly.Intersects(self.search_area_poly): 
                
#                poly_obs_verticies = self.find_crit_pnts(feat, X, Y, comms, counter_poly)
                
                # Get the interesection of the polygon from the shapefile and
                #   the outline of tiff from pMarnineViewer
                intersection_poly = geom_poly.Intersection(self.search_area_poly) 
                
                # Get the ring of that intersection polygon
                p_ring = intersection_poly.GetGeometryRef(0) 
                
                # There are two potential cases - There are vertices of the 
                #   polygon within the search area and There are no vertices of
                #   the polygon within the search area.
                #
                # Case 1: There are vertices within the search area therefore 
                #   we will give the polygon string function the intersection
                #   of the polygon and the search area
                if p_ring:
                    poly_obs_verticies = self.verify_poly(X, Y, heading, comms, intersection_poly.Buffer(0.00003), counter_poly)
                    
                # Case 2: there are no points in the the search window, 
                #   therefore we are temperarily giving the entire polygon to 
                #   the polygon function.
                else:
                    poly_obs_verticies = self.verify_poly(X, Y, heading, comms, geom_poly.Buffer(0.00003), counter_poly)
       
                # If it is not the first polygon, then add a '!' to the end of 
                #   the  string.
                if poly_obs_verticies!="No points":
                    # Add in the threat level (index 0) and obstacle type (1)
                    #   to the poly_str.
                    WL = feat.GetField(1)
                    depth = feat.GetField(2)
                    obs_type = feat.GetField(3)
                    t_lvl = self.calc_t_lvl(depth, WL, obs_type)
                    poly_obs_verticies = '{},{}@{}'.format(t_lvl, obs_type, poly_obs_verticies)
                    if counter_poly==0:
                        poly_info = poly_obs_verticies
                    elif counter_poly>0:
                        poly_info += '!'
                        poly_info += poly_obs_verticies
                    # Increment counter
                    counter_poly += 1
            feat = self.ENC_poly_layer.GetNextFeature()
                    
        # Post an update if there are polygon obstacles
        if (counter_poly>0):
            poly_obs = '{},{},{}:{}:{}'.format(X, Y, heading, counter_poly, poly_info)
#            str(X)+','+str(Y)+','+str(heading)+':'+str(counter_poly)+':'+poly_info
            comms.notify('Poly_Obs', poly_obs)  
            
        # Determine if a new polygon was used
        if self.max_poly_obs < counter_poly:
            self.max_poly_obs = counter_poly    
            
        # Remove highlighted key points of the polygon obstacles from 
        #   pMarineViewer if they are no longer in the search area
        for ii in range(counter_poly, self.max_poly_obs+1):
            time.sleep(.002)
            pt_1 = 'x=1,y=1,vertex_color=white,active=false,label=pt1_'+str(ii)
            comms.notify('VIEW_POINT', pt_1)
            time.sleep(.002)
            pt_2 = 'x=1,y=1,vertex_color=white,active=false,label=pt2_'+str(ii)
            comms.notify('VIEW_POINT', pt_2)
            time.sleep(.002)
            pt_3 = 'x=1,y=1,vertex_color=mediumblue,active=false,label=pt3_'+str(ii)
            comms.notify('VIEW_POINT', pt_3)
                
    def Initialize(self):
        """ This initializes the comminications with MOOS which allows 
            everythingelse to use "comms"  which will be used for communicating
            with theMOOSDB and the lists of "new mail" and builds the OGR 
            layers of obstacles from the ENC based on obstacle geometry type.
            
        Output:
            MOOS
            
        """
        # Start connection to MOOS
        MOOS = MOOS_comms()
        MOOS.Initialize()
        time.sleep(.25)
        
        # Get the MHW_Offset 
        MOOS.Get_mail()
        if (len(MOOS.MHW_Offset) != 0):
            MLLW = MOOS.MHW_Offset[-1]
            self.MHW_Offset = float(MLLW)
            MOOS.MHW_Offset = []
        
        # Get the desired ENC 
        if (len(MOOS.ENCs) != 0):    
            ENC = MOOS.ENCs[-1]
            self.ENC_filename = '../../src/ENCs/{}/{}.000'.format(ENC, ENC)
            self.filename_pnt = '../../src/ENCs/{}/Shape/point.shp'.format(ENC)
            self.filename_poly = '../../src/ENCs/{}/Shape/poly.shp'.format(ENC)
            self.filename_line = '../../src/ENCs/{}/Shape/line.shp'.format(ENC)
            
        self.ds = ogr.Open(self.ENC_filename)
        
        # Get the Lat/Long Origin
        if (len(MOOS.origin_lat) != 0 and len(MOOS.origin_lon) != 0): 
            origin_lat = MOOS.origin_lat[-1]
            origin_lon =MOOS.origin_lon[-1]
            self.LatOrigin = float(origin_lat)
            self.LongOrigin = float(origin_lon)
        self.x_origin, self.y_origin = self.LonLat2UTM(self.LongOrigin, self.LatOrigin)
        
        # Build the shapefile layers
        self.read_ENC()
        
        # Counter to remove the highlighted obstacles from pMarnineViewer that
        #   are not within the search area.
        self.max_pnt_obs = 1
        
         # This counter is then used to remove the visual hints on 
        #   pMarnineViewer when the polygon is not within the search area
        self.max_poly_obs = 0
        
        print "Initialization Finished"
        
        return MOOS
        
    def run(self, Time=False):
        """ This function has two main parts and initialization and a infinate  
            loop. In the initialization, the program takes the information from
            the ENCs and stores it to disk as well as initializing MOOS  
            communications. Once that has happened, it then runs the infinate 
            loop. In the infinate loop, a search polygon is built and   
            information on the objects from the ENC that are within the search 
            area are then published to the MOOSDB.
            
        Input:
            Time - Boolean to print how long it takes to initialize the ENC_DB
        """
        if Time:
            start = timer()
            MOOS = self.Initialize()
            end = timer()
            print (end-start)
        else:
            MOOS = self.Initialize()
            
        while(True):
            # Get the new values for the ASV's current position (x, y) and 
            #   heading.
            time.sleep(.001)
            MOOS.Get_mail()
            if len(MOOS.MHW_Offset) != 0:
                MLLW = MOOS.MHW_Offset[-1]
                self.MHW_Offset = float(MLLW)
                MOOS.MHW_Offset = []
                
            if len(MOOS.Tide) != 0:
                TIDE = MOOS.Tide[-1]
                self.tide = float(TIDE)
                MOOS.Tide = []
                
            # If there is a new position and heading for the ASV, then filter  
            #   the data and highlight the ones in the search area
            if len(MOOS.NAV_X) != 0 and len(MOOS.NAV_Y)!= 0 and len(MOOS.NAV_HEAD)!=0:
                # Get most recent value of position and heading and then reset 
                #   the lists
                X = MOOS.NAV_X[-1]
                Y = MOOS.NAV_Y[-1]
                heading = MOOS.NAV_HEAD[-1]
                MOOS.NAV_X, MOOS.NAV_Y, MOOS.NAV_HEAD = [],[],[]
                
                # Build a polygon to limit the area of the ENC that we are 
                #   searching for obstacles.
                self.build_search_poly(X, Y, heading, MOOS.comms)
                self.filter_features()
                
                # Searches the ENC in the limitted area for obstacles with 
                #   point geometry with threat level > 0. If there are any, it 
                #   then highlights them on pMarnineViewer and publishes the 
                #   position and other pertinent information on the obstacle to
                #   the MOOSDB.
                self.publish_points(X, Y, heading, MOOS.comms)
                
                # Searches the ENC in the limitted area for obstacles with 
                #   point geometry with threat level > 0. If there are any, it 
                #   then highlights them on pMarnineViewer and publishes the 
                #   position and other pertinent information on the obstacle to
                #   the MOOSDB.
                self.publish_poly(X, Y, heading, MOOS.comms)
                
            # MOOS freaks out when nothing is posted to the MOOSDB so post this 
            #   variable to avoid this problem if nothing was posted during the
            #   last cycle
            else:
                MOOS.comms.notify('dummy_var','')
                

#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
e = Search_ENC()
#m = e.Initialize()
e.run(Time=True)