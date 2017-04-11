package org.cma.product.files;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class SendProductFileSet implements Serializable {

	private List<SendProductFile> m_files = new ArrayList<SendProductFile>();

	public synchronized void add(SendProductFile file) {
		m_files.add(file);
	}

	public synchronized boolean remove(SendProductFile file) {
		return m_files.remove(file);
	}

	public synchronized void clearAll() {
		m_files.clear();
	}

	public boolean isExist(String filename) {
		for (SendProductFile file : m_files) {
			if (file.getFileName().equals(filename)) {
				return true;
			}

		}
		
		return false;

	}
}
