package org.cma.quartz;

import java.util.ArrayList;
import java.util.List;

import org.cma.quartz.entity.JobPlan;
import org.cma.utils.Config;
import org.quartz.CronScheduleBuilder;
import org.quartz.CronTrigger;
import org.quartz.JobBuilder;
import org.quartz.JobDetail;
import org.quartz.JobExecutionContext;
import org.quartz.JobKey;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.SchedulerFactory;
import org.quartz.TriggerBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 该类处理所有的对Quartz的控制接口，启动、停止等。单例模式。
 * 
 * @author Jeff
 *
 */
public class QuartzMgmt {

	private static transient final Logger logger = LoggerFactory.getLogger(QuartzMgmt.class);

	private static QuartzMgmt _instance = new QuartzMgmt();
	private boolean probeEnble = Boolean.parseBoolean(Config.GetConfig().getProperty("ProbeEnable"));
	private boolean recvEnable = Boolean.parseBoolean(Config.GetConfig().getProperty("RecvEnable"));
	private boolean sendMsgEnable = Boolean.parseBoolean(Config.GetConfig().getProperty("SendMsgEnable"));
	private boolean RecvMonEnable = Boolean.parseBoolean(Config.GetConfig().getProperty("RecvMonEnable"));
	private boolean SendMonEnable = Boolean.parseBoolean(Config.GetConfig().getProperty("SendMonEnable"));


	private QuartzMgmt() {
		try {
			initScheduler();
		} catch (SchedulerException e) {
			logger.error(e.getMessage(), e);
		}
	}

	public static QuartzMgmt getInstance() {
		return _instance;
	}

	private void initScheduler() throws SchedulerException {

		if (scheduler == null) {
			SchedulerFactory schedFact = new org.quartz.impl.StdSchedulerFactory();
			scheduler = schedFact.getScheduler();
		}

		scheduler.clear();
	}

	/**
	 * 作业开始执行
	 * 
	 * @throws org.quartz.SchedulerException
	 */
	public void start() throws SchedulerException {
		if (scheduler == null) {
			return;
		}
		if (!scheduler.isStarted()) {
			scheduler.start();
		}
	}

	/**
	 * 停止作业调度器
	 * 
	 * @throws org.quartz.SchedulerException
	 */
	public void destroy() throws SchedulerException {
		scheduler.shutdown();
	}

	/**
	 * 获取Quartz的运行状态
	 * 
	 * @return
	 */
	public boolean state() {
		try {
			return scheduler.isStarted();
		} catch (SchedulerException e) {
			// TODO Auto-generated catch block
			return false;
		}
	}

	/**
	 * 获取当前作业列表
	 * 
	 * @return
	 */
	public List<JobPlan> getJobs() {
		try {

			List<JobExecutionContext> jobs = scheduler.getCurrentlyExecutingJobs();

			List<JobPlan> plans = new ArrayList<JobPlan>();

			for (JobExecutionContext job : jobs) {
				JobPlan p = new JobPlan();
				p.setCronExpress(((CronTrigger) job.getTrigger()).getCronExpression());
				p.setJobClazz(job.getJobDetail().getJobClass().getName());
				p.setJobGroup(job.getJobDetail().getKey().getGroup());
				p.setJobName(job.getJobDetail().getKey().getName());

				plans.add(p);
			}

			return plans;
		} catch (SchedulerException e) {
			return null;
		}
	}

	/////////////////////////////////
	// 下面是对Quartz本身的控制
	private Scheduler scheduler;

	/**
	 * 从配置文件里读取作业列表，然后对其进行初始化。当前只需要一个作业，暂时写死，回头在改为从配置文件中读的。
	 * 
	 * @throws Exception
	 */
	public void scheduleJobPlans() {

		try {

			// 当前先写死，回头改为从配置文件中读
			// 发送格点文件
			
			if(probeEnble){
				JobPlan job = new JobPlan();
				job.setJobName("ProbeDisk");
				job.setJobGroup("MQJob");
				job.setJobClazz("org.cma.quartz.job.ProbeJob");
				job.setJobParams("");
				job.setStatus(1);
				job.setCronExpress("0/20 * * * * ?");
				scheduleJobPlan(job);
			}
			
			if(recvEnable){
				// 接收格点文件
				JobPlan jobRecv = new JobPlan();
				jobRecv.setJobName("RecvJob");
				jobRecv.setJobGroup("MQJob");
				jobRecv.setJobClazz("org.cma.quartz.job.ReceiverJob");
				jobRecv.setJobParams("");
				jobRecv.setStatus(1);
				jobRecv.setCronExpress("0/20 * * * * ?");
				scheduleJobPlan(jobRecv);
			}
			
			if(sendMsgEnable){
				// 发送站点消息
				JobPlan jobSendMsg = new JobPlan();
				jobSendMsg.setJobName("SendMsgJob");
				jobSendMsg.setJobGroup("MQJob");
				jobSendMsg.setJobClazz("org.cma.quartz.job.SendMsgJob");
				jobSendMsg.setJobParams("");
				jobSendMsg.setStatus(1);
				jobSendMsg.setCronExpress("0/20 * * * * ?");
				scheduleJobPlan(jobSendMsg);	
			}
			
			if(RecvMonEnable){
				//监测扫描任务
				JobPlan jobScan = new JobPlan();
				jobScan.setJobName("ProductRecvScan");
				jobScan.setJobGroup("MonitorJob");
				jobScan.setJobClazz("org.cma.quartz.job.RecvMonitor");
				jobScan.setJobParams("");
				jobScan.setStatus(1);
				jobScan.setCronExpress("0 * * * * ?");
				scheduleJobPlan(jobScan);
			}
			
			if(SendMonEnable){
				//监测扫描任务
				JobPlan jobSenMon = new JobPlan();
				jobSenMon.setJobName("ProductSendScan");
				jobSenMon.setJobGroup("MonitorJob");
				jobSenMon.setJobClazz("org.cma.quartz.job.SendMonitor");
				jobSenMon.setJobParams("");
				jobSenMon.setStatus(1);
				jobSenMon.setCronExpress("0 * * * * ?");
				scheduleJobPlan(jobSenMon);
			}
			
			
			
			// 接收站点消息
			
		} catch (Exception ex) {
			logger.error(ex.getMessage(), ex);
		}
	}

	private void scheduleJobPlan(JobPlan jobPlan) throws Exception {

		JobKey key = new JobKey("job_" + jobPlan.getJobName(), jobPlan.getJobGroup());

		if (!scheduler.checkExists(key)) {
			JobDetail job = JobBuilder.newJob(JobDispatcher.class).withIdentity("job_" + jobPlan.getJobName(), jobPlan.getJobGroup()).usingJobData("param", jobPlan.getJobParams()).usingJobData("clazzName", jobPlan.getJobClazz()).build();

			CronTrigger trigger = (CronTrigger) TriggerBuilder.newTrigger().withIdentity("trigger_" + jobPlan.getJobName(), jobPlan.getJobGroup()).withSchedule(CronScheduleBuilder.cronSchedule(jobPlan.getCronExpress())).build();

			scheduler.scheduleJob(job, trigger);
		}
	}
}
