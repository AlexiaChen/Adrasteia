package org.cma.cached;

import java.util.ArrayList;
import java.util.List;

import org.cma.config.JobConfig;
import org.cma.product.LocalMicpProduct;
import org.cma.product.MergeProduct;
import org.cma.product.RecvMonProduct;
import org.cma.product.SendMonProduct;

public class ICommonsVars {
   public static List<SendMonProduct> globalSendProducts = new ArrayList<SendMonProduct>();
   public static List<RecvMonProduct> globalRecvProducts = new ArrayList<RecvMonProduct>();
   public static List<LocalMicpProduct> globalLocalMicpProducts =new ArrayList<LocalMicpProduct>();
 
   
   public static List<MergeProduct> globalMergeProducts = JobConfig.getInstance().getMergeProductLits();
}
