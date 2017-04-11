package org.cma.utils;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import org.cma.quartz.job.ProbeJob;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Config {
	
	private static transient final Logger logger = LoggerFactory.getLogger(Config.class);
	
	private static Properties _config = null;

    public static Properties GetConfig() {

        if (_config == null) {
        	
            InputStream in = Config.class.getClassLoader().getResourceAsStream("config.properties");

            String config = ProbeJob.class.getClassLoader().getResource("config.properties").getPath();
            
            logger.info(config);
            
            _config = new Properties();

            try {
                _config.load(in);
            } catch (IOException e) {

            }
        }

        return _config;
    }

    
}
