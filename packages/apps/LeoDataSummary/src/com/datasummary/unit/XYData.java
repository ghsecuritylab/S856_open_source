package com.datasummary.unit;


import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Locale;
import java.text.DecimalFormat;


import com.datasummary.DAO.DetailDAO;
import com.datasummary.DAO.TotalDAO;
import com.datasummary.model.DetailInfo;
import com.datasummary.model.TotalInfo;
import com.datasummary.unit.LeoDataSummaryLog;


import android.R.anim;
import android.R.integer;
import android.content.Context;
import static android.net.NetworkTemplate.buildTemplateMobileAll;
import android.telephony.TelephonyManager;
import android.telephony.MSimTelephonyManager;
import android.text.format.Time;
import android.net.INetworkStatsService;
import android.net.NetworkStatsHistory;
import android.net.NetworkTemplate;
import android.os.ServiceManager;
import android.content.Context;
import android.net.INetworkStatsSession;
import android.net.NetworkStats;
import com.codeaurora.telephony.msim.MSimPhoneFactory;

import com.datasummary.LeoDataSummary.MainActivity;



public class XYData {
	public Context context;
	Calendar calendar =Calendar.getInstance();
	public int year=calendar.get(Calendar.YEAR);
	public int month=calendar.get(Calendar.MONTH)+1;//4
	public int day=calendar.get(Calendar.DAY_OF_MONTH);//6
	public int monthday= calendar.getActualMaximum(Calendar.DATE);//30
	public int pageX=monthday+1;
	public ArrayList<DetailInfo> detailinfos = new ArrayList<DetailInfo>();
	

	public double[] minValues = new double[monthday];

	
	public double[] maxValues = new double[monthday] ;

	public String[] maxstring =	new String[monthday];

	public int maxY=0; 
	public int maxX=monthday+1;
	public int minX=0;
	public int minY=0;
	static final String TAG = "XYData";
	public int sub=0;
	public double max=0;
	public double total=0.0;
	public String dds_imsi="";
	
	
	public double[] panLimits=new double[]{minX,maxX,minY,maxY};
	
	public XYData(Context context){
		this.context=context;
		Arrays.fill(minValues, 0.0);
		Arrays.fill(maxValues, 0.0);
		Arrays.fill(maxstring, "0.0B");
		this.sub=MainActivity.mySubscription;
		LeoDataSummaryLog.d(TAG,"sub="+sub);
		
		this.getmaxvalues();
		maxY=this.getmax();  
	}

	
	public  void getmaxvalues() {
		
		TotalDAO totalDAO=new TotalDAO(context);
		//TotalInfo totalInfo=new TotalInfo(6, "4600", 1, 100.1, 200.2, 20140406L);
		//totalDAO.add(totalInfo);
		//totalDAO.update(20140406L);

		//int dds = MSimPhoneFactory.getDataSubscription();
		dds_imsi = MSimTelephonyManager.getDefault().getSubscriberId(sub);
		if(dds_imsi==null){
			dds_imsi="0000";
			return;
			}
		LeoDataSummaryLog.d(TAG,"dds_imsi="+dds_imsi);

        final TelephonyManager telephony = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        final INetworkStatsService statsService = INetworkStatsService.Stub
                .asInterface(ServiceManager
                        .getService("netstats"/*Context.NETWORK_STATS_SERVICE*/));

        NetworkTemplate mTemplate = buildTemplateMobileAll(MSimTelephonyManager.getDefault().getSubscriberId(sub));
        LeoDataSummaryLog.d(TAG,"mTemple mTemple mTemple="+mTemplate);
        
        Time mTimeNow = new Time();
        mTimeNow.setToNow();

        Time mTimeStart = new Time();
        // monthDay:1-31 month:0-11
        mTimeStart.set(0, 0, 0, day, mTimeNow.month, mTimeNow.year);

        final long start = mTimeStart.toMillis(true);
        final long now = mTimeNow.toMillis(true);
        final long end = now;
        LeoDataSummaryLog.d(TAG,start + "/" + end + "/" + now);
		LeoDataSummaryLog.d(TAG,"111111111111111");

		try {
			INetworkStatsSession mStatsSession;
			mStatsSession = statsService.openSession();
			LeoDataSummaryLog.d(TAG,"22222222222222");
			NetworkStats networkStats = mStatsSession.getSummaryForAllUid(mTemplate, start, end, false);
			NetworkStats.Entry entry_app = null;
			LeoDataSummaryLog.d(TAG,"networkStats = "+networkStats);
			for (int i = 0; i < networkStats.size(); i++) {
				entry_app = networkStats.getValues(i, entry_app);
				LeoDataSummaryLog.d(TAG,"uid = "+entry_app.uid+" total  = "+((entry_app.rxBytes + entry_app.txBytes)/1024)+"KB");
				DetailInfo tmpdetailinfo=new DetailInfo(i,entry_app.uid,(entry_app.rxBytes)*1.0,(entry_app.txBytes)*1.0," ");
				detailinfos.add(tmpdetailinfo);
			}
			LeoDataSummaryLog.d(TAG,detailinfos.toString());

		}catch(Exception e) {
			LeoDataSummaryLog.d(TAG,"e = "+e);
		}
		

        try {
            NetworkStatsHistory nh = statsService.openSession().getHistoryForNetwork(
                    mTemplate, 10);
			//today
            android.net.NetworkStatsHistory.Entry entry = nh.getValues(start/*now*/,
                    end, now, null);
            final long totalBytes = entry != null ? entry.rxBytes
                    + entry.txBytes : -1;
			LeoDataSummaryLog.d(TAG,"2222222222");
			LeoDataSummaryLog.d(TAG,"totalBytes = "+totalBytes);
			LeoDataSummaryLog.d(TAG,"totalBytes = "+totalBytes);
			if(entry != null && totalDAO.ifExistCurrentTable(Long.parseLong(gettoday()),sub,dds_imsi)) {
				totalDAO.updateCurrentTable(Long.parseLong(gettoday()),entry.rxBytes,entry.txBytes);
			}else if(entry != null) {
				totalDAO.addCurrentTable(new TotalInfo(0, dds_imsi,sub, entry.rxBytes, entry.txBytes, Long.parseLong(gettoday())));
			}
			
        } catch (Exception e) {
            e.printStackTrace();
        }		
		long cur_date = 0;
		long date = 0;
		String cur_imsi = " ";
		String imsi = " ";
		int cur_sub = 0;
		double cur_rx = 0;
		double rx = 0;
		double cur_tx = 0;
		double tx = 0;		
		ArrayList<TotalInfo> current_Infos=totalDAO.findbetweendateCurrentTable(dds_imsi,sub,getminday(),getmaxday());
		//ArrayList<TotalInfo> total_Infos=totalDAO.findbetweendate(getminday(),getmaxday());
		LeoDataSummaryLog.d(TAG,"getmaxvalues current_Infos.size() = "+current_Infos.size());
		//LeoDataSummaryLog.d(TAG,"getmaxvalues total_Infos.size() = "+total_Infos.size());
		for (int i = 0; i < current_Infos.size(); i++) {
			
			cur_date = current_Infos.get(i).getDate();
			cur_imsi = current_Infos.get(i).getImsi();
			cur_sub = current_Infos.get(i).getSub();
			cur_rx = current_Infos.get(i).getRx();
			cur_tx = current_Infos.get(i).getTx();
			total=total+cur_rx+cur_tx;
			LeoDataSummaryLog.d(TAG,"getmaxvalues cur_date:"+cur_date+" cur_imsi:"+cur_imsi+
					" cur_sub:"+cur_sub+" cur_rx:"+cur_rx+" cur_tx:"+cur_tx);			
			String str=current_Infos.get(i).getDate().toString();//20140406
			str=str.substring(str.length()-2, str.length());//06
			maxValues[Integer.parseInt(str) - 1]=cur_rx+cur_tx;

			
			/*if(total_Infos.size() > 0) {
				for(int j = 0; j < total_Infos.size(); j++) {
					date = total_Infos.get(j).getDate();
					imsi = total_Infos.get(j).getImsi();
					sub = total_Infos.get(j).getSub();
					rx = total_Infos.get(j).getRx();
					tx = total_Infos.get(j).getTx();
					if((cur_date == date) && (cur_sub == sub) && (cur_imsi.equals(cur_imsi))) {
						maxValues[Integer.parseInt(str) - 1]=cur_rx+cur_tx +rx + tx;
					}else {
						maxValues[Integer.parseInt(str) - 1]=cur_rx+cur_tx;
					}
				}
			}else {
				maxValues[Integer.parseInt(str) - 1]=cur_rx+cur_tx;
			}*/
			
		}
	 
	 
	}
	
	public String gettoday(){
		if(month<10){
			if(day<10){
				return year+"0"+month+"0"+day;
			}
			return year+"0"+month+day;
		}
		return year+month+day+"";
	}
	public String getmaxday(){
		if(month<10){
			
			return year+"0"+month+monthday;
		}
		return year+month+monthday+"";
	}
	public String getminday(){
		if(month<10){
			
				return year+"0"+month+"01";
			
		}
		return year+month+"01";
	}
	
	public String getDataString(double d) {
		double d_tmp = 0;
		DecimalFormat myformat = new DecimalFormat();
		myformat.applyPattern("####.00");
		if(d > 1024) {
			d_tmp = d / 1024;
		}else {
			return myformat.format(d)+"B";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return myformat.format(d_tmp)+"KB";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return myformat.format(d_tmp)+"MB";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return myformat.format(d_tmp)+"GB";
		}			

		return "B";
	}	
	/*
	public double getDataValue(double d) {
		double d_tmp = 0;;
		if(d > 1024) {
			d_tmp = d / 1024;
		}else {
			return d;
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return d_tmp;
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return d_tmp;
		}

		return d_tmp;
	}	
	*/
	protected int getmax(){
		max=0;
		LeoDataSummaryLog.d(TAG,"getmax maxValues.length = "+maxValues.length);
		for (int i = 0; i < maxValues.length; i++) {
			maxstring[i]=getDataString(maxValues[i]);
			LeoDataSummaryLog.d(TAG,"maxstring-----------"+maxstring[i]);
			if(maxValues[i]>max){
				max=maxValues[i];
			}
		}
		
		LeoDataSummaryLog.d(TAG,"=========================");
		LeoDataSummaryLog.d(TAG,"getmax max = "+max);
		//max = getDataValue(max);
		int intmax= (int)(max *2 + 1.5);
		/*
		if(max<500000){
			intmax=(((int)(max/50000))/4+1)*200000;
		}else if(max<1000000){
			intmax=(((int)(max/100000))/4+1)*400000;
		}else if(max<5000000){
			intmax=(((int)(max/500000))/4+1)*2000000;
		}else if(max<10000000){
			intmax=(((int)(max/1000000))/4+1)*4000000;
		}else if(max<50000000){
			intmax=(((int)(max/5000000))/4+1)*20000000;
		}else if(max<100000000){
			intmax=(((int)(max/10000000))/4+1)*40000000;
		}else{
			intmax=(((int)(max/50000000))/4+1)*200000000;
		}*/
		LeoDataSummaryLog.d(TAG,"getmax intmax = "+intmax);
		return intmax ;
	}
}
