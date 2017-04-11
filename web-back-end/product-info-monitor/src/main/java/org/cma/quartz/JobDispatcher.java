package org.cma.quartz;

import java.beans.Beans;

import org.cma.quartz.job.IJob;
import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 所有的内部作业类，必须实现该接口，用于统一进行调度管理
 * 
 * @author Jeff
 *
 */
public class JobDispatcher implements Job  {

	private static transient final Logger logger = LoggerFactory.getLogger(JobDispatcher.class);
	
	public void execute(JobExecutionContext context) throws JobExecutionException {
		
		String jobName = context.getJobDetail().getKey().getName();
        String clazz = context.getJobDetail().getJobDataMap().get("clazzName").toString();
        String param = context.getJobDetail().getJobDataMap().get("param").toString();
		
        logger.info("jobName = " + jobName);
        logger.info("QuartzJob : className = " + clazz);
        logger.info("Param = " + param);
        
        logger.info("开始调度作业 " + clazz);

        try {
            // 根据作业类的名称，实例化我们实际的类
            Object obj = Beans.instantiate(Thread.currentThread().getContextClassLoader(), clazz);

            // 判断此实例是否实现了 IJob 接口，如果有，则开始调用，
            // 否则退出
            if (obj instanceof IJob) {

                IJob job = (IJob) obj;
                job.run();

                logger.info("作业执行成功完成：" + jobName);
            }
        } catch (Exception e) {
            logger.error("未成功执行作业类 " + jobName + " :" + e.getMessage(), e);
        }        
	}	
}
