package org.cma.product.files;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class MicapsProductFileSet implements Serializable {
	private List<MicapsProductFile> m_files = new ArrayList<MicapsProductFile>();

	public synchronized void add(MicapsProductFile file) {
		m_files.add(file);
	}

	public synchronized boolean remove(MicapsProductFile file) {
		return m_files.remove(file);
	}

	public synchronized void clearAll() {
		m_files.clear();
	}
	
	public boolean isExist(String filename) {
		for (MicapsProductFile file : m_files) {
			if (file.getFileName().equals(filename)) {
				return true;
			}

		}
		
		return false;

	}
}
