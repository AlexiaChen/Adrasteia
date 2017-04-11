package org.cma.rwstation.struct;

/**
 * @author MathxH
 *
 */
public class StationFileObj {
	private StationFileHeader header;
	private StationFileBody body;
	
	
	public StationFileObj(StationFileHeader header,
			StationFileBody body){
		this.header = header;
		this.body = body;
	}


	public StationFileHeader getHeader() {
		return header;
	}


	public void setHeader(StationFileHeader header) {
		this.header = header;
	}


	public StationFileBody getBody() {
		return body;
	}


	public void setBody(StationFileBody body) {
		this.body = body;
	}
	
	
}
