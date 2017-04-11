package org.cma.quartz.job;

import java.io.File;
import java.text.Format;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import org.cma.cached.ICommonsVars;
import org.cma.config.JobConfig;
import org.cma.product.LocalMicpProduct;
import org.cma.product.RecvMonProduct;
import org.cma.product.files.MicapsProductFile;
import org.cma.product.files.RecvProductFile;
import org.cma.utils.ScanFiles;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RecvMonitor implements IJob {

	// 日志
	private static transient final Logger logger = LoggerFactory.getLogger(RecvMonitor.class);

	private List<RecvMonProduct> m_recvMonProducts = JobConfig.getInstance().getRecvMonProductList();
	private List<LocalMicpProduct> m_micapsProducts = JobConfig.getInstance().getLocalMicpProductList();

	private static boolean running = false;
	private static String yesterdayDate;

	static {
		SimpleDateFormat df = new SimpleDateFormat("yyyyMMDD");
		yesterdayDate = df.format(new Date());
	}

	public RecvMonitor() {
		ICommonsVars.globalRecvProducts = m_recvMonProducts;
		ICommonsVars.globalLocalMicpProducts = m_micapsProducts;
	}

	private boolean status() {
		return running;
	}

	private void running() {
		running = true;
	}

	private void runover() {
		running = false;
	}

	private void cleanYesterdayData() {

		// 得到当前日期
		SimpleDateFormat df = new SimpleDateFormat("yyyyMMDD");// 设置日期格式
		String currentDate = df.format(new Date());

		if (!currentDate.equals(yesterdayDate)) {
			for (RecvMonProduct prod : ICommonsVars.globalRecvProducts) {

			   prod.getFileSet().clearAll();

			}

			for (LocalMicpProduct prod : ICommonsVars.globalLocalMicpProducts) {

				prod.getFileSet().clearAll();

			}

			// 更新状态
			yesterdayDate = currentDate;
		}

	}

	private void processRecvMonProducts() {
		/* 遍历每个产品的源文件路径，查看是否文件存在 */
		for (RecvMonProduct prod : m_recvMonProducts) {

			String prodSrcPath = prod.getSrcfolder();
			String fileFormat = prod.getFileformat();
			String range = (prod.getRange().length() < 2) ? "0" + prod.getRange()
					: prod.getRange();

			String filename08 = ScanFiles.replaceWithDate(fileFormat, range, prod.getReporttimes().get(0));
			String filename20 = ScanFiles.replaceWithDate(fileFormat, range, prod.getReporttimes().get(1));

			/* 08 的pattern */

			File[] files = ScanFiles.isFileExists(prodSrcPath, filename08);

			if (files != null) {
				if (files.length != 0) {

					for (File file : files) {
						
						RecvProductFile File = new RecvProductFile();
						File.setFileName(file.getName());
						File.setShouldRecvTime(prod.getRecvtime());
						File.setReportTime("08");
						File.setResult(true);
						
						/*得到实际收到时间*/
						long time = file.lastModified();
						Format simpleFormat = new SimpleDateFormat("hh:mm:ss");
						String timeString = simpleFormat.format(new Date(time));
						File.setRecvTime(timeString);
						
						if(!prod.getFileSet().isExist(file.getName())){
							prod.getFileSet().add(File);
						}
						
						
						
					}

				} else {

				}
			}

			/* 20的pattern */
			files = ScanFiles.isFileExists(prodSrcPath, filename20);

			if (files != null) {
				if (files.length != 0) {

					for (File file : files) {
						RecvProductFile File = new RecvProductFile();
						File.setFileName(file.getName());
						File.setShouldRecvTime(prod.getRecvtime());
						File.setReportTime("20");
						File.setResult(true);
						
						/*得到实际收到时间*/
						long time = file.lastModified();
						Format simpleFormat = new SimpleDateFormat("hh:mm:ss");
						String timeString = simpleFormat.format(new Date(time));
						File.setRecvTime(timeString);
						
						if(!prod.getFileSet().isExist(file.getName())){
							prod.getFileSet().add(File);
						}
					}

				} else {

				}
			}

		} // foreach product end
	}

	private void processLocalMicpProducts() {
		
		for (LocalMicpProduct prod : m_micapsProducts){
			/* for 003 to maxforcast instead of [FFF] */
			int rangeStep = Integer.parseInt(prod.getRange());
			int maxForecast = Integer.parseInt(prod.getMaxforecast());
            
			String fileFormat = prod.getFileformat();
			String srcfolder = prod.getSrcfolder();
            
			String filename08 = ScanFiles.replaceWithDate(fileFormat, "", prod.getReporttimes().get(0));
			String filename20 = ScanFiles.replaceWithDate(fileFormat, "", prod.getReporttimes().get(1));
			
			/* 08 的pattern */
			for (int i = 3; i <= maxForecast; i += rangeStep) {
				String extend = String.format("%03d", i);
				String fileName = filename08.replaceAll("\\[FFF\\]", extend);

				File[] files = ScanFiles.isFileExists(srcfolder, fileName);

				if (files != null) {
					// 如果文件存在就加入HashTable中置true，不存在则是false
					if (files.length != 0) {

						for (File file : files) {
							MicapsProductFile File = new MicapsProductFile();
							File.setFileName(file.getName());
							File.setShouldRecvTime(prod.getRecvtime());
							File.setReportTime("08");
							File.setResult(true);
							
							/*得到实际收到时间*/
							long time = file.lastModified();
							Format simpleFormat = new SimpleDateFormat("hh:mm:ss");
							String timeString = simpleFormat.format(new Date(time));
							File.setRecvTime(timeString);
							
							if(!prod.getFileSet().isExist(file.getName())){
								prod.getFileSet().add(File);
							}
						}

					} else {
						MicapsProductFile File = new MicapsProductFile();
						File.setFileName(fileName);
						File.setShouldRecvTime(prod.getRecvtime());
						File.setReportTime("08");
						File.setResult(false);
						
						
						File.setRecvTime("");
						
						if(!prod.getFileSet().isExist(fileName)){
							prod.getFileSet().add(File);
						}
						
						
					}
				}

			}

			/* 20的pattern */
			for (int i = 3; i <= maxForecast; i += rangeStep) {

				String extend = String.format("%03d", i);
				String fileName = filename20.replaceAll("\\[FFF\\]", extend);

				File[] files = ScanFiles.isFileExists(srcfolder, fileName);

				if (files != null) {
					if (files.length != 0) {

						for (File file : files) {
							MicapsProductFile File = new MicapsProductFile();
							File.setFileName(file.getName());
							File.setShouldRecvTime(prod.getRecvtime());
							File.setReportTime("20");
							File.setResult(true);
							
							/*得到实际收到时间*/
							long time = file.lastModified();
							Format simpleFormat = new SimpleDateFormat("hh:mm:ss");
							String timeString = simpleFormat.format(new Date(time));
							File.setRecvTime(timeString);
							
							if(!prod.getFileSet().isExist(file.getName())){
								prod.getFileSet().add(File);
							}
						}

					} else {
						MicapsProductFile File = new MicapsProductFile();
						File.setFileName(fileName);
						File.setShouldRecvTime(prod.getRecvtime());
						File.setReportTime("20");
						File.setResult(false);
						
						File.setRecvTime("");
						
						if(!prod.getFileSet().isExist(fileName)){
							prod.getFileSet().add(File);
						}
					}
				}

			}
		}
		
		
		
	}

	private boolean ExecTask() {

		processRecvMonProducts();
		processLocalMicpProducts();
		
		cleanYesterdayData();

		return true;
	}

	@Override
	public void run() {
		if (status()) {
			// 正在运行，则本次不执行
			return;
		}
		// 开始执行
		running();

		try {
			boolean bRet = ExecTask();
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
		}

		// 执行结束
		runover();
	}// run function end
}
