package org.cma.quartz.entity;

/**
 * 作业信息类，用户计划调度任务
 * 
 * @author Jeff
 *
 */
public class JobPlan {

	private String jobName;
	private String jobGroup;
	private String jobClazz;
	private String jobParams;
	private String cronExpress;
	private int status;

	/**
	 * 作业名称
	 * 
	 * @return
	 */
	public String getJobName() {
		return jobName;
	}

	/**
	 * 作业名称
	 * 
	 * @param jobName
	 */
	public void setJobName(String jobName) {
		this.jobName = jobName;
	}

	/**
	 * 作业组
	 * 
	 * @return
	 */
	public String getJobGroup() {
		return jobGroup;
	}

	/**
	 * 作业组
	 * 
	 * @param jobGroup
	 */
	public void setJobGroup(String jobGroup) {
		this.jobGroup = jobGroup;
	}

	/**
	 * 作业执行类，需继承接口
	 * 
	 * @return
	 */
	public String getJobClazz() {
		return jobClazz;
	}

	/**
	 * 作业执行类，需继承接口
	 * 
	 * @param jobClazz
	 */
	public void setJobClazz(String jobClazz) {
		this.jobClazz = jobClazz;
	}

	/**
	 * 作业的参数，暂时应该不需要
	 * 
	 * @return
	 */
	public String getJobParams() {
		return jobParams;
	}

	/**
	 * 作业的参数，暂时应该不需要
	 * 
	 * @param jobParams
	 */
	public void setJobParams(String jobParams) {
		this.jobParams = jobParams;
	}

	/**
	 * 作业调度计划
	 * 
	 * @return
	 */
	public String getCronExpress() {
		return cronExpress;
	}

	/**
	 * 作业调度计划
	 * 
	 * @param cronExpress
	 */
	public void setCronExpress(String cronExpress) {
		this.cronExpress = cronExpress;
	}

	/**
	 * 作业状态，0：禁用，1：启用
	 * 
	 * @return
	 */
	public int getStatus() {
		return status;
	}

	/**
	 * 作业状态，0：禁用，1：启用
	 * 
	 * @param status
	 */
	public void setStatus(int status) {
		this.status = status;
	}
}
