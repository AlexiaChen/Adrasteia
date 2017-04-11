package org.cma.utils;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ScanFiles {
	private static ArrayList<File> filePattern(File file, Pattern p) {
		if (file == null) {
			return null;
		} else if (file.isFile()) {
			Matcher fMatcher = p.matcher(file.getName());
			if (fMatcher.matches()) {
				ArrayList<File> list = new ArrayList<File>();
				list.add(file);
				return list;
			}
		} else if (file.isDirectory()) {
			File[] files = file.listFiles();
			if (files != null && files.length > 0) {
				ArrayList<File> list = new ArrayList<File>();
				for (int i = 0; i < files.length; i++) {
					ArrayList<File> rlist = filePattern(files[i], p);
					if (rlist != null) {
						list.addAll(rlist);
					}
				}
				return list;
			}
		}
		return null;
	}

	private static File[] getFiles(String dir, String s) {
		// 开始的文件夹
		File file = new File(dir);

		s = s.replace('.', '#');
		s = s.replaceAll("#", "\\\\.");
		s = s.replace('*', '#');
		s = s.replaceAll("#", ".*");
		s = s.replace('?', '#');
		s = s.replaceAll("#", ".?");
		s = "^" + s + "$";

		System.out.println(s);
		Pattern p = Pattern.compile(s);
		ArrayList<File> list = filePattern(file, p);
		
		if(list == null){
			return null;
		}

		File[] rtn = new File[list.size()];
		list.toArray(rtn);
		return rtn;
	}

	public static File[] isFileExists(String dir, String fileNamePattern) {

		File[] files = getFiles(dir, fileNamePattern);

		return files;
	}

	public static String replaceWithDate(String fileformat,String range,String reporttime) {

		/* 获得系统当前日期 */
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd");
		String date = df.format(new Date());

		String year = date.split("-")[0];
		String yearYY = year.substring(2);
		String month = date.split("-")[1];
		String day = date.split("-")[2];

		fileformat = fileformat.replaceAll("\\[MM\\]", month);
		fileformat = fileformat.replaceAll("\\[DD\\]", day);

		fileformat = fileformat.replaceAll("\\[YYYY\\]", year);

		fileformat = fileformat.replaceAll("\\[YY\\]", yearYY);

		fileformat = fileformat.replaceAll("\\[HH\\]", reporttime);
		fileformat = fileformat.replaceAll("\\[TT\\]", range);

		return fileformat;
	}
}
