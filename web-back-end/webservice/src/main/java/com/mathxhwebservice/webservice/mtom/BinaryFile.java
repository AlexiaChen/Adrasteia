package com.mathxhwebservice.webservice.mtom;

import javax.activation.DataHandler;
import javax.xml.bind.annotation.*;

@XmlRootElement(name = "BinaryFile")
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "BinaryFile")
public class BinaryFile {

    @XmlElement(nillable = true)
    private String title;

    @XmlMimeType("application/octet-stream")
    private DataHandler binaryData;

    public BinaryFile(){

    }
    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public DataHandler getBinaryData() {
        return binaryData;
    }

    public void setBinaryData(DataHandler binaryData) {
        this.binaryData = binaryData;
    }
}
