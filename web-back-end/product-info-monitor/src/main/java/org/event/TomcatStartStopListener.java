package org.event;

import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;

import org.cma.quartz.QuartzMgmt;
import org.quartz.SchedulerException;


public class TomcatStartStopListener implements ServletContextListener {

	@Override
	public void contextDestroyed(ServletContextEvent arg0) {

	}

	@Override
	public void contextInitialized(ServletContextEvent arg0) {

		try {
			if (!QuartzMgmt.getInstance().state()) {
				QuartzMgmt.getInstance().start();
				QuartzMgmt.getInstance().scheduleJobPlans();
			}

		} catch (SchedulerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
