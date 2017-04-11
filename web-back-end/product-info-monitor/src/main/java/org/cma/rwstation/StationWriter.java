package org.cma.rwstation;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.cma.rwstation.struct.ForecastTimeLine;
import org.cma.rwstation.struct.StationFileHeader;
import org.cma.rwstation.struct.StationFileObj;
import org.cma.rwstation.struct.StationObject;

public class StationWriter {
	
	private StationFileObj obj;
	String filename;
	
	
	private void writeHeader(BufferedWriter bw) throws IOException{
		
		bw.write(StationFileHeader.getFirstMark()); bw.newLine();
		
		bw.write(obj.getHeader().getFsc()); bw.write(" ");
		bw.write(obj.getHeader().getProvinceCode()); bw.write(" ");
		bw.write(obj.getHeader().getDayHourStamp()); bw.newLine();
		
		bw.write(obj.getHeader().getTitle()); bw.newLine();
		
		bw.write(obj.getHeader().getDataType()); bw.write("  ");
		bw.write(obj.getHeader().getDateTimeStamp()); bw.newLine();
		
		//6H格式的文件firstTime域非空
		if(obj.getHeader().getFirstTime().isEmpty()){
			
			String stationCnt = String.valueOf(
					obj.getHeader().getStationCount()
					); 
			bw.write(stationCnt); bw.newLine();
		}else{
			//6H的格式
			bw.write(obj.getHeader().getFirstTime()); bw.newLine();
			bw.write(obj.getHeader().getNumSequece()); bw.newLine();
			bw.write(obj.getHeader().getLastTime()); bw.newLine();
			
		}
		
		bw.flush();
	}
	
	private void writeBody(BufferedWriter bw)throws IOException{
		
		HashMap<String, StationObject> stations = 
				obj.getBody().getStationMap();
		
		Iterator<Map.Entry<String,  StationObject>> iterator = 
				stations.entrySet().iterator();
		
		while (iterator.hasNext()) {
			Map.Entry<String, StationObject> entry = iterator.next();
			String stationID = entry.getKey();
			StationObject item = entry.getValue();
			
			bw.write(stationID); bw.write("  ");
			bw.write(String.valueOf(item.getLon())); bw.write("  ");
			bw.write(String.valueOf(item.getLat())); bw.write("  ");
			bw.write(String.valueOf(item.getHeight())); bw.write("  ");
			bw.write(String.valueOf(item.getForecastLineCnt())); bw.write("  ");
			
			//只有none6H的文件才需要写入数据列数
			if(obj.getHeader().getFsc().equals("FSC150")){
				if(item.getDataColumn() != 0){
					bw.write(String.valueOf(item.getDataColumn())); 
				}
			}
			
			
			bw.newLine();
			
			//遍历每个站点的预报时效，一行一行的写入文件
			List<ForecastTimeLine> list = item.getForecastLineList();
			
			for(ForecastTimeLine line : list){
				String forecastTime = line.getForecastTime();
				bw.write(forecastTime); bw.write("    ");
				
				Iterator iter = line.getDataList().iterator();
				
				while(iter.hasNext()){
					float value = (Float)iter.next();
					
					bw.write(String.valueOf(value)); bw.write("   ");
				}
				
				bw.newLine();
				
			}
			
			
		}
		
		
		
		
		bw.flush();
	}
	
	private void writeEnd(BufferedWriter bw) throws IOException{
		bw.write(" "); bw.write("NNNN"); bw.newLine();
	}
	
	
	public StationWriter(StationFileObj obj,String filename){
		this.obj = obj;
		this.filename = filename;
	}
	
	public boolean save(){
		
		boolean bRet = true;
		//打开文件
		File file = new File(filename);
        FileWriter fw = null;
        BufferedWriter writer = null;
        try {
            fw = new FileWriter(file);
            writer = new BufferedWriter(fw);
           
            writeHeader(writer);
            writeBody(writer);
            writeEnd(writer);
            
            
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            bRet = false;
        }catch (IOException e) {
            e.printStackTrace();
            bRet = false;
        }finally{
        
        	try{
        		  writer.close();
                  fw.close();
        	}catch(IOException e){
        		e.printStackTrace();
        		bRet = false;
        	}
        	
       }
        
        return bRet;
		
	}
}
