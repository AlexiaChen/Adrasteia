package org.cma.controller;

import java.io.IOException;
import java.sql.Connection;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.ibatis.session.SqlSession;
import org.apache.ibatis.session.SqlSessionFactory;
import org.cma.cached.ICommonsVars;
import org.cma.database.operation.DataBase;
import org.cma.quartz.QuartzMgmt;
import org.cma.quartz.entity.JobPlan;
import org.quartz.SchedulerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.servlet.ModelAndView;

import com.google.gson.Gson;

@Controller
@RequestMapping("/")
public class ViewController {

	private static transient final Logger logger = LoggerFactory.getLogger(ViewController.class);

	/**
	 * 显示总体的管理入口。现在先不加登录的处理，以后需要了再弄。
	 * 
	 * @param request
	 * @return
	 */
	private static SqlSessionFactory sessionFactory;
	private static SqlSession session;

	private static Connection conn;
	private static String fileName;

	@RequestMapping(value = "mgmt", method = RequestMethod.GET)
	public ModelAndView showTencentView(HttpServletRequest request) {

		ModelAndView modelAndView = new ModelAndView();
		modelAndView.setViewName("mgmt");

		modelAndView.addObject("data", "test data string");

		/*
		 * NwfdStationSend s1 = new NwfdStationSend(); NwfdStationSend s2 = new
		 * NwfdStationSend();
		 * 
		 * NwfdStationSend[] sArray = { s1, s2 };
		 * modelAndView.addObject("sArray", sArray);
		 */

		logger.debug("current in the mgmt controller.");

		return modelAndView;
	}

	/**
	 * 启动Quartz
	 * 
	 * @param request
	 * @return
	 */
	@RequestMapping(value = "quartz/start", method = RequestMethod.GET)
	public @ResponseBody String startQuartz(HttpServletRequest request) {

		try {
			if (!QuartzMgmt.getInstance().state()) {
				QuartzMgmt.getInstance().start();
				QuartzMgmt.getInstance().scheduleJobPlans();
			}

		} catch (SchedulerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return "OK";
	}

	/**
	 * 停止Quartz
	 * 
	 * @param request
	 * @return
	 */
	@RequestMapping(value = "quartz/stop", method = RequestMethod.GET)
	public @ResponseBody String stopQuartz(HttpServletRequest request, HttpServletResponse response) throws Exception {

		return "OK";
	}

	/**
	 * 获取当前Quartz的状态
	 * 
	 * @param request
	 * @return
	 */
	@RequestMapping(value = "quartz/status", method = RequestMethod.GET)
	public @ResponseBody List<JobPlan> statusQuartz(HttpServletRequest request) {

		return QuartzMgmt.getInstance().getJobs();
	}

	/**
	 * 获取Quartz的作业列表
	 * 
	 * @param request
	 * @return
	 */
	@RequestMapping(value = "quartz/list", method = RequestMethod.GET, produces = "application/json")
	public @ResponseBody String listQuartz(HttpServletRequest request, HttpServletResponse response) {

		return null;

	}

	/**
	 * 获取MQ接收文件监控
	 * 
	 * @param request
	 * @return
	 */

	@RequestMapping(value = "recvmonitor/list", method = RequestMethod.GET)
	public @ResponseBody void RecvProvinceList(HttpServletRequest request, HttpServletResponse response) {

		response.setCharacterEncoding("UTF-8");

		String json;

		Gson g = new Gson();

		json = g.toJson(ICommonsVars.globalRecvProducts);

		try {
			response.getWriter().println(json);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * 获取MQ发送文件监控
	 * 
	 * @param request
	 * @return
	 */

	@RequestMapping(value = "sendmonitor/list", method = RequestMethod.GET)
	public @ResponseBody void SendMonitorList(HttpServletRequest request, HttpServletResponse response) {

		response.setCharacterEncoding("UTF-8");

		String json;

		Gson g = new Gson();

		json = g.toJson(ICommonsVars.globalSendProducts);

		try {
			response.getWriter().println(json);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * 获取本地Micaps文件监控
	 * 
	 * @param request
	 * @return
	 */

	@RequestMapping(value = "localmicaps/list", method = RequestMethod.GET)
	public @ResponseBody void LocalMicapsList(HttpServletRequest request, HttpServletResponse response) {

		response.setCharacterEncoding("UTF-8");

		String json;

		Gson g = new Gson();

		json = g.toJson(ICommonsVars.globalLocalMicpProducts);

		try {
			response.getWriter().println(json);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * 任务操作 目前支持 reprocess
	 * 
	 * @param request
	 * @return
	 */

	@RequestMapping(value = "task/reprocess", method = RequestMethod.GET)
	public @ResponseBody void TaskReprocess(HttpServletRequest request, HttpServletResponse response) {

		String productKey = request.getParameter("productKey");
		String reportTime = request.getParameter("reportTime");

		
		DataBase.getInstance().UpdateToTaskStatus(productKey, reportTime);

	}

	/**
	 * 格点任务监控
	 * 
	 * @param request
	 * @return
	 */
	@RequestMapping(value = "task/list", method = RequestMethod.GET)
	public @ResponseBody void TaskList(HttpServletRequest request, HttpServletResponse response) {

		response.setCharacterEncoding("UTF-8");

		DataBase.getInstance().getCurrentTaskStatus(ICommonsVars.globalMergeProducts);

		String json;

		Gson g = new Gson();

		json = g.toJson(ICommonsVars.globalMergeProducts);

		try {
			response.getWriter().println(json);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
