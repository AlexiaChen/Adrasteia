package org.cma.product.files;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class RecvProductFileSet implements Serializable {

private List<RecvProductFile> m_files = new ArrayList<RecvProductFile>();
	
	public synchronized void add(RecvProductFile file){
		m_files.add(file);
	}
	
	public synchronized boolean remove(RecvProductFile file){
		return m_files.remove(file);
	}
	
	public synchronized void clearAll(){
		m_files.clear();
	} 
	
	public boolean isExist(String filename) {
		for (RecvProductFile file : m_files) {
			if (file.getFileName().equals(filename)) {
				return true;
			}

		}
		
		return false;

	}
}
