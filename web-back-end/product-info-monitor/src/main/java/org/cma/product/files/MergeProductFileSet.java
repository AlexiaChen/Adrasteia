package org.cma.product.files;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class MergeProductFileSet implements Serializable{
   private List<MergeProductFile> m_files = new ArrayList<MergeProductFile>();
   
   public synchronized void add(MergeProductFile file) {
		m_files.add(file);
	}

	public synchronized boolean remove(MergeProductFile file) {
		return m_files.remove(file);
	}

	public synchronized void clearAll() {
		m_files.clear();
	}
	
	public boolean isExist(String filename) {
		for (MergeProductFile file : m_files) {
			if (file.getFileName().equals(filename)) {
				return true;
			}

		}
		
		return false;

	}
}
