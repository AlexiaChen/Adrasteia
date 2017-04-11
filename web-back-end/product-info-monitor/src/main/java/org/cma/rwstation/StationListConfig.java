package org.cma.rwstation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

import org.cma.rwstation.struct.MetaStation;
import org.cma.utils.Config;

public class StationListConfig {
	private static HashMap<String,MetaStation> stations;
	
	static{
		
		stations = new HashMap<String,MetaStation>();
		//得到站点列表文件的路径
		String filename = Config.GetConfig().getProperty("stations_file");
		
		File file = new File(filename);
        FileReader fr = null;
        BufferedReader reader = null;
        try {
            fr = new FileReader(file);
            reader = new BufferedReader(fr);
           
            initStations(reader);
           
            
        } catch (FileNotFoundException e) {
            e.printStackTrace();
           
        }catch (IOException e) {
            e.printStackTrace();
          
        }
		
	}
	
	private static void initStations(BufferedReader br) throws IOException{
		String line;
		while((line = br.readLine()) != null){
			line = line.trim();
			
			String stationID = line.split("\\s+")[0];
			String lat =line.split("\\s+")[1];
			String lon = line.split("\\s+")[2];
			String height = line.split("\\s+")[3];
			
			lat = String.format("%.1f", Float.valueOf(lat));
			lon = String.format("%.1f", Float.valueOf(lon));
			height = String.format("%.1f", Float.valueOf(height));
			
			if(!stations.containsKey(stationID)){
				MetaStation station = new MetaStation(
						Float.valueOf(lon), 
						Float.valueOf(lat), 
						Float.valueOf(height)
						);
				
				stations.put(stationID, station);
			}
			
			
		}
	}
	
	public static HashMap<String,MetaStation> getStations(){
		return stations;
	}
}
