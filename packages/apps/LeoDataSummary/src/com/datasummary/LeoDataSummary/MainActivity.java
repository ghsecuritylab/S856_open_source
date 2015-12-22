package com.datasummary.LeoDataSummary;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.achartengine.GraphicalView;
import org.achartengine.ChartFactory;
import org.achartengine.chart.BarChart.Type;
import org.achartengine.model.RangeCategorySeries;
import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.renderer.SimpleSeriesRenderer;
import org.achartengine.renderer.XYMultipleSeriesRenderer;
import com.datasummary.unit.LeoDataSummaryLog;
import com.datasummary.unit.AppInfoComparator;
import com.datasummary.DAO.DetailDAO;
import com.datasummary.DAO.TotalDAO;
import com.datasummary.adapter.AppAdapter;
import com.datasummary.model.AppInfo;
import com.datasummary.model.DetailInfo;
import com.datasummary.model.TotalInfo;
import com.datasummary.unit.MyFile;
import com.datasummary.unit.XYData;
import com.googlecode.droidwall.Api;

import android.graphics.Bitmap;
import android.R.layout;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Paint.Align;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;

import android.view.Menu;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.HorizontalScrollView;
import android.widget.RelativeLayout;
import android.telephony.MSimTelephonyManager;
import android.telephony.TelephonyManager;
import android.widget.TextView;
import android.view.MenuItem;
import android.content.res.Configuration;
import android.text.format.Formatter;




public class MainActivity extends Activity {
	private AppBroadcastReceiver mAppBroadcastReceiver;
	private AppAdapter myappAdapter;
	private ListView listView;
	private ArrayList<AppInfo> appList;
        private ArrayList<AppInfo> dataList;
	public static Handler handler;
	private MyFile myFile = new MyFile();
	private final int EVENT_BOOT_COMPLETED = 0x112;
	private final int EVENT_UPDATE_VIEW = 0x113;
	private final int EVENT_UPDATE_XYDATA_VIEW = 0x114;
	private final int EVENT_UPDATE_APPINFO_VIEW = 0x115;
	
	public static int mySubscription;
	public static final String SUBSCRIPTION_KEY="subscription";
	public static String TAG="MainActivity";
	public double mounthtotalrate;
	

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();

	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub

		if (mAppBroadcastReceiver != null) {
			this.unregisterReceiver(mAppBroadcastReceiver);
		}
		super.onDestroy();

	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		updateXYDataView();
		updateAppInfoView();

	}

	public String read(String filename) {
		try {

			FileInputStream inputStream = this.openFileInput(filename);
			byte[] bytes = new byte[1024];
			ByteArrayOutputStream arrayOutputStream = new ByteArrayOutputStream();
			while (inputStream.read(bytes) != -1) {
				arrayOutputStream.write(bytes, 0, bytes.length);
			}
			inputStream.close();
			arrayOutputStream.close();
			String s = new String(arrayOutputStream.toByteArray());

			return s;
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;

	}

	public void save(String fileName, String s) {

		try {

			FileOutputStream outputStream = openFileOutput(fileName,
					Activity.MODE_PRIVATE);
			outputStream.write(s.getBytes());
			outputStream.flush();
			outputStream.close();
			//Toast.makeText(MainActivity.this, "save successful.", Toast.LENGTH_LONG).show();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	public void reinit(String packagename, int position, int uid) {
		myFile.s = "";
		myFile.s = read(myFile.filename);
		String arrays[] = myFile.split_s();

		for (int i = 0; i < arrays.length; i++) {
			if (arrays[i].equals(packagename)) {
				if (arrays[i + 1].equals("1")) {
					arrays[i + 1] = "0";
					/*
					SystemProperties.set("sys.jcommand_para","[DISABLEAPPNET]["+uid+"][null]");
					try {
						Thread.sleep(300);						
					}catch(Exception e) {

					}
					SystemProperties.set("ctl.start","jcommand");
					try {
						Thread.sleep(300);						
					}catch(Exception e) {

					}*/
					

				} else {
					arrays[i + 1] = "1";
					/*
					SystemProperties.set("sys.jcommand_para","[ENABLEAPPNET]["+uid+"][null]");
					try {
						Thread.sleep(300);						
					}catch(Exception e) {

					}
					SystemProperties.set("ctl.start","jcommand");
					try {
						Thread.sleep(300);					
					}catch(Exception e) {

					}*/
				}
			}
		}
		myFile.s = "";
		for (int i = 0; i < arrays.length; i++) {
			myFile.s = myFile.s + arrays[i] + " ";
		}
		save(myFile.filename, myFile.s);
		if (appList.get(position).bool) {
			appList.get(position).bool = false;
		} else {
			appList.get(position).bool = true;
		}

		//LeoDataSummaryLog.d(TAG,"position", appList.toString());
		appList.toString();
		myappAdapter.notifyDataSetChanged();
                //listView.setAdapter(myappAdapter);

	}

	
public String getDataType(double d){
	
		double d_tmp = 0.0;
		if(d > 1024) {
			d_tmp = d / 1024;
		}else {
			return "B";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return "KB";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return "MB";
		}
		if(d_tmp >1024) {
			d_tmp = d_tmp / 1024;
		}else {
			return "GB";
		}			

		return "B";
	
}
	public double getDataValue(String type, double d) {
		DecimalFormat df = new DecimalFormat("#.00");
		LeoDataSummaryLog.d(TAG,"getDataValue type= "+type);
		LeoDataSummaryLog.d(TAG,"getDataValue d= "+d);
		LeoDataSummaryLog.d(TAG,"getDataValue df.format((double)d/1024/1024)= "+df.format((double)d/1024/1024));
		if(type.equalsIgnoreCase("B"))
			return Double.parseDouble(df.format((double)d));
		else if(type.equalsIgnoreCase("KB"))
			return Double.parseDouble(df.format((double)d/1024));
		else if(type.equalsIgnoreCase("MB"))
			return Double.parseDouble(df.format((double)d/1024/1024));
		else if(type.equalsIgnoreCase("GB"))
			return Double.parseDouble(df.format((double)d/1024/1024/1024));
		else
			return 0;
		
	}

        public double getDataValuerow(String type, double d) {
                DecimalFormat df = new DecimalFormat("#.00");
		LeoDataSummaryLog.d(TAG,"getDataValue type= "+type);
		LeoDataSummaryLog.d(TAG,"getDataValue d= "+d);
                if(type.equalsIgnoreCase("B"))
			return (double)d;
		else if(type.equalsIgnoreCase("KB"))
			return (double)d/1024;
		else if(type.equalsIgnoreCase("MB"))
			return (double)d/1024/1024;
		else if(type.equalsIgnoreCase("GB"))
			return (double)d/1024/1024/1024;
		else
			return 0;
        }
	
	
	private GraphicalView mchartView;
	private XYMultipleSeriesDataset dataset;
	private XYMultipleSeriesRenderer renderer;
	private String strMonthData = "";
	public void init() {

	
		mySubscription=getIntent().getIntExtra(SUBSCRIPTION_KEY,0);//get sub
		LeoDataSummaryLog.d(TAG,"XXXXXXXXXXXXXXXXX"+mySubscription);
		
		XYData xyData = new XYData(getApplication());
		String type=getDataType(xyData.max);
		LeoDataSummaryLog.d(TAG,"99999999999999999999999"+type);
                /*lenovo-sw renjun1 begin modify to not format the number for row product 2014.5.23*/
		for(int j=0;j<xyData.maxValues.length;j++){
			xyData.maxValues[j]=this.getDataValuerow(type,xyData.maxValues[j]);
			LeoDataSummaryLog.d(TAG,"99999999999999999999999"+xyData.maxValues[j]);
			}
		xyData.maxY=(int)(getDataValuerow(type,xyData.maxY)+1.5);
		LeoDataSummaryLog.d(TAG,"99999999999999999999999"+xyData.maxY);
		LeoDataSummaryLog.d(TAG,"xydata,xydata,xydata,xydata,xydata");
		dataset = new XYMultipleSeriesDataset();
                //double monthtotalrate=getDataValuerow(type,xyData.total);
                /*lenovo-sw renjun1 begin modify to format data 2014.6.20*/
		strMonthData = getResources().getString(R.string.current_month)+Formatter.formatFileSize(this,(long)xyData.total);
                /*lenovo-sw renjun1 end modify to format data 2014.6.20*/
                /*lenovo-sw renjun1 end modify to not format the number for row product 2014.5.23*/

		RangeCategorySeries series = new RangeCategorySeries("");
		LeoDataSummaryLog.d(TAG,"init xyData.minValues.length = "+xyData.minValues.length);

		//String mData_str = getDataString(xyData.getMaxTxRX());

		double month_data = 0;
		
		for (int k = 0; k < xyData.minValues.length; k++) {
			LeoDataSummaryLog.d(TAG,"init k: "+k+" minValues:"+xyData.minValues[k]+" maxValues:"+xyData.maxValues[k]);
			month_data = month_data + xyData.maxValues[k];
			//series.add(xyData.minValues[k], getDataValue(mData_str,xyData.maxValues[k]));
			series.add(xyData.minValues[k], xyData.maxValues[k]);
		}
		LeoDataSummaryLog.d(TAG,"init origin xmonth_data = "+month_data);
		//LeoDataSummaryLog.d(TAG,"init result xmonth_data = "+getDataValue(getDataString(month_data),month_data));
		dataset.addSeries(series.toXYSeries());

		int[] colors = new int[] { Color.RED };//
		renderer = buildBarRenderer(colors);
		LeoDataSummaryLog.d(TAG,"xyData.maxY = "+xyData.maxY);
		setChartSettings(renderer, null, null, null, xyData.minX, xyData.pageX,
				xyData.minY, xyData.maxY, Color.rgb(77,77,77),Color.rgb(77,77,77));

		renderer.setBarSpacing(2);
		// renderer.setXLabelsColor(Color.TRANSPARENT);
		renderer.setXLabels(0);
		LeoDataSummaryLog.d(TAG,"xyData.monthday = "+xyData.monthday);
		for (int i = 1; i <= xyData.monthday; i++) {
			LeoDataSummaryLog.d(TAG,"init i: "+i+" xyData.day:"+xyData.day+" xyData.month:"+xyData.month);
			if (i == xyData.day) {

				renderer.addXTextLabel(i, " "+getResources().getString(R.string.today)+" ");

			} else {

				renderer.addXTextLabel(i, xyData.month + "." + i);

			}

		}
		renderer.setXLabelsColor(Color.rgb(77,77,77));
		renderer.setYLabels(5);
		renderer.setMargins(new int[] { 20, 0, 5, 0 });
		renderer.setGridColor(Color.rgb(77,77,77));
		renderer.setShowGrid(true);
		renderer.setApplyBackgroundColor(true);
		renderer.setBackgroundColor(Color.WHITE);
		renderer.setMarginsColor(Color.WHITE);
		renderer.setYLabelsAlign(Align.RIGHT);
		renderer.setXLabelsAlign(Align.CENTER);
		renderer.setPanLimits(xyData.panLimits);
		renderer.setPanEnabled(true, false);
		renderer.setZoomEnabled(false, false);
		
					
		
		
		SimpleSeriesRenderer r = renderer.getSeriesRendererAt(0);
		r.setDisplayChartValues(true);
		r.setChartValuesTextSize(14);
		r.setChartValuesSpacing(3);
		// r.setGradientEnabled(true);
		// r.setGradientStart(20, Color.GRAY);
		// r.setGradientStop(100, Color.GREEN);
		r.setColor(Color.rgb(77,77,77));
		DecimalFormat myformat = new DecimalFormat();
		myformat.applyPattern("##.##"+type);
		r.setChartValuesFormat(myformat);
		
		monthday = xyData.monthday;
		day=xyData.day;
		while(handler == null) {
			try {
				Thread.sleep(100);
			}catch(Exception e) {

			}		
		}
		handler.sendEmptyMessage(EVENT_UPDATE_XYDATA_VIEW);


		// 这里的绝对路径
		File file = new File("data/data/com.datasummary.LeoDataSummary/files/"
				+ myFile.filename);
		if (!file.exists()) {
			this.save(myFile.filename, myFile.s);
			//Toast.makeText(MainActivity.this, "not exists", 300).show();
		}

		
		myFile.s = read(myFile.filename);
		String arrays[] = myFile.split_s();
		myFile.s = "";

		
		//DetailDAO detailDAO = new DetailDAO(getApplication());
		LeoDataSummaryLog.d(TAG,xyData.gettoday());
		ArrayList<DetailInfo> detailinfos = xyData.detailinfos;
		// Toast.makeText(MainActivity.this, detailinfos.toString(),300).show();

		appList = new ArrayList<AppInfo>(); 
		AppInfoComparator appinfocomparator=new AppInfoComparator();
		/*lenovo-sw renjun1 begin modify to resolve FC when packagemanager died 2014.7.10 SHELLROW-1938*/
                List<PackageInfo> packages=null;
                try {
		      packages = getPackageManager().getInstalledPackages(
				PackageManager.GET_PERMISSIONS
						| PackageManager.GET_UNINSTALLED_PACKAGES);
                } catch (Exception e){
                }

		LeoDataSummaryLog.d(TAG,"8888888888888888888888");
                if (packages != null){
		  for (int i = 0; i < packages.size(); i++) {
			PackageInfo packageInfo = packages.get(i);
			AppInfo tmpInfo = new AppInfo();

			tmpInfo.appName = packageInfo.applicationInfo.loadLabel(
					getPackageManager()).toString();
			tmpInfo.uid = packageInfo.applicationInfo.uid;
			tmpInfo.permissions = packageInfo.requestedPermissions;
			tmpInfo.packageName = packageInfo.packageName;
			tmpInfo.versionName = packageInfo.versionName;
			tmpInfo.appIcon = packageInfo.applicationInfo
					.loadIcon(getPackageManager());
			
			for (int j = 0; j < detailinfos.size(); j++) {
				if (detailinfos.get(j).getUid() == tmpInfo.uid) {
					tmpInfo.rate =tmpInfo.rate+detailinfos.get(j).getRx()
							+ detailinfos.get(j).getTx();

				}
			}
			if (tmpInfo.appIcon == null) {
				Resources resources = getResources();
				tmpInfo.appIcon = resources
						.getDrawable(R.drawable.gallery_photo_frame);
			}
			// Only display the non-system app info
			// if ((packageInfo.applicationInfo.flags &
			// ApplicationInfo.FLAG_SYSTEM) == 0) {
			
			String[] permissions = packageInfo.requestedPermissions;
			int uid = packageInfo.applicationInfo.uid;
			if (permissions != null && permissions.length > 0
					&& tmpInfo.uid > 9999) {
				for (String permission : permissions) {
					

					if ("android.permission.INTERNET".equals(permission)) {

						for (int k = 0; k < arrays.length; k++) {
							if (arrays[k].equals(tmpInfo.packageName)) {
								String aString = "1";
								if (arrays[k + 1].trim().equals(aString)) {
									tmpInfo.bool = true;
									break;
								} else {
									tmpInfo.bool = false;
									break;
								}

							} else {
								tmpInfo.bool = true;
							}
						}

						//LeoDataSummaryLog.d(TAG,"arr", "package "+tmpInfo.packageName);
						// tmpInfo.bool=true;


						if (tmpInfo.bool) {
							myFile.s = myFile.s + tmpInfo.packageName + " "
									+ "1" + " ";
						} else {
							myFile.s = myFile.s + tmpInfo.packageName + " "
									+ "0" + " ";
						}
						
						LeoDataSummaryLog.d(TAG,tmpInfo.appName+"  "+tmpInfo.packageName+" "+tmpInfo.uid+" "+tmpInfo.rate);
						appList.add(tmpInfo);
						appList=appinfocomparator.Appinfossort(appList);					
						
						while(handler == null) {
							try {
								Thread.sleep(100);
							}catch(Exception e) {

							}
						}
						Message msg = new Message();
						msg.what = EVENT_UPDATE_APPINFO_VIEW;
						handleAppInfoView = 1;
						msg.arg1 = handleAppInfoView;
						if(i<10) handler.sendMessage(msg);
						while(handleAppInfoView == 1 && i<10) {
							try {
								Thread.sleep(300);
							}catch(Exception e) {

							}							
						}						
					}
				}
			}

			// }

		 }
               }
               /*lenovo-sw renjun1 end modify to resolve FC when packagemanager died 2014.7.10 SHELLROW-1938*/
/*
fix
*/
		Message msg = new Message();
		msg.what = EVENT_UPDATE_APPINFO_VIEW;
		handler.sendMessage(msg);

		
		LeoDataSummaryLog.d(TAG,"8888888888888888888888");
		
	}
	private int monthday;
	private int day;
	private HorizontalScrollView horizontalScrollView;
	private int smoothx=0;
	private void updateView() {
		LeoDataSummaryLog.d(TAG,"updateView enter !");
		LinearLayout mylayout = (LinearLayout) findViewById(R.id.app_linearlayout);
		horizontalScrollView=(HorizontalScrollView)findViewById(R.id.app_horizontalscrollview);
		if(dataset == null || renderer == null) return;
		mchartView = ChartFactory.getRangeBarChartView(MainActivity.this,
				dataset, renderer, Type.DEFAULT);
		mchartView.setDrawingCacheEnabled(true);
		int screenWidth  = getWindowManager().getDefaultDisplay().getWidth();

		mchartView.layout(0, 0, screenWidth/9*monthday, screenWidth/3);
		Bitmap bitmap = mchartView.toBitmap();
		bitmap = Bitmap.createBitmap(bitmap,0,0,screenWidth/9*monthday,screenWidth/3 - 22);
		ImageView imageView = new ImageView(getBaseContext());


		imageView.setImageBitmap(bitmap);
		imageView.setAdjustViewBounds(true);
		if(mylayout.getChildCount() > 0) mylayout.removeAllViews();
		mylayout.addView(imageView, new LayoutParams(LayoutParams.FILL_PARENT,LayoutParams.FILL_PARENT));

		if(day<=1){
			smoothx=0;
		}else{
			smoothx=(day-1)*screenWidth/9;
		}
		new Handler().postDelayed((new Runnable() {
			@Override
			public void run() {
				horizontalScrollView.smoothScrollTo(smoothx,0 );
			}
			}),5);
						
		listView = (ListView) findViewById(R.id.app_ListView);
		if(myappAdapter == null) return;
		listView.setAdapter(myappAdapter);
		LeoDataSummaryLog.d(TAG,"updateView exit !");

	}
	private void updateXYDataView() {
		LeoDataSummaryLog.d(TAG,"updateXYDataView enter !");
		LinearLayout mylayout = (LinearLayout) findViewById(R.id.app_linearlayout);
		horizontalScrollView=(HorizontalScrollView)findViewById(R.id.app_horizontalscrollview);
		if(dataset == null || renderer == null) return;
		mchartView = ChartFactory.getRangeBarChartView(MainActivity.this,
				dataset, renderer, Type.DEFAULT);
		/*
		add log to KSEVEN-3757 20140428 by maozd1
		*/
		if(mchartView==null){
			LeoDataSummaryLog.d(TAG,"mchartview ==null!"+"dataset=="+dataset.toString()+"renderer=="+renderer.toString());
			mchartView.repaint();
			}
		mchartView.setDrawingCacheEnabled(true);
		int screenWidth  = getWindowManager().getDefaultDisplay().getWidth();
		int screenHeight  = getWindowManager().getDefaultDisplay().getHeight();
		LeoDataSummaryLog.d(TAG,"updateXYDataView screenWidth = "+screenWidth+" screenHeight = "+screenHeight+ "monthday"+monthday);
		if("kingdomt".equals(SystemProperties.get("ro.product.device"))|| "kingdom_row".equals(SystemProperties.get("ro.product.device"))) {
			mchartView.layout(0, 0, screenWidth/9*(monthday-10), screenHeight/5 - 70);
		}else if("shellw".equals(SystemProperties.get("ro.product.device"))) {
			mchartView.layout(0, 0, screenWidth/9*monthday, screenHeight/5);
		}else {
			mchartView.layout(0, 0, screenWidth/9*monthday, screenHeight/5);
		}
		Bitmap bitmap = mchartView.toBitmap();
                /*lenovo-sw renjun1 begin modify to resolve FC sometime for SHELLROW-1938 2014.7.10*/
                if ( monthday <= 0 ) {
                    return;
                }
                /*lenovo-sw renjun1 end modify to resolve FC sometime for SHELLROW-1938 2014.7.10*/
		if("kingdomt".equals(SystemProperties.get("ro.product.device")) || "kingdom_row".equals(SystemProperties.get("ro.product.device"))) {
			bitmap = Bitmap.createBitmap(bitmap,0,0,screenWidth/9*(monthday-10),screenHeight/5 -32 - 100);
		}else if("shellw".equals(SystemProperties.get("ro.product.device"))) {
			bitmap = Bitmap.createBitmap(bitmap,0,0,screenWidth/9*monthday,screenHeight/5 - 25);
		}else {
			bitmap = Bitmap.createBitmap(bitmap,0,0,screenWidth/9*monthday,screenHeight/5 - 25);
		}
		ImageView imageView = new ImageView(getBaseContext());


		imageView.setImageBitmap(bitmap);
		imageView.setAdjustViewBounds(true);
		if(mylayout.getChildCount() > 0) mylayout.removeAllViews();
		mylayout.addView(imageView, new LayoutParams(LayoutParams.FILL_PARENT,LayoutParams.FILL_PARENT));

		if(day<=1){
			smoothx=0;
		}else{
			if("kingdomt".equals(SystemProperties.get("ro.product.device")) || "kingdom_row".equals(SystemProperties.get("ro.product.device"))) {
				smoothx=(day-1 - 6)*screenWidth/9;
			}else if("shellw".equals(SystemProperties.get("ro.product.device"))) {
				smoothx=(day-1 - 2)*screenWidth/9;
			}else {
				smoothx=(day-1 - 2)*screenWidth/9;
			}
		}
		new Handler().postDelayed((new Runnable() {
			@Override
			public void run() {
				horizontalScrollView.smoothScrollTo(smoothx,0 );
			}
			}),5);		
		TextView tv = (TextView)findViewById(R.id.info_textview);
		tv.setText(strMonthData+", "+getResources().getString(R.string.info));
		
		LeoDataSummaryLog.d(TAG,"updateXYDataView exit !");

	}	
	private int OldAppAdapterCount = 0;
	private int handleAppInfoView = 0;
	private void updateAppInfoView() {
		LeoDataSummaryLog.d(TAG,"updateAppInfoView enter:handleAppInfoView = "+handleAppInfoView);
		listView = (ListView) findViewById(R.id.app_ListView);
		if(appList == null) return;
                /*lenovo-sw renjun1 begin modify to resolve the FC sometimes for SHELLROW-1405 2014.6.26*/
                dataList.clear();
                dataList.addAll(appList);
		myappAdapter = new AppAdapter(this, dataList, R.layout.items);
                /*lenovo-sw renjun1 end modify to resolve the FC sometimes for SHELLROW-1405 2014.6.26*/
		if(myappAdapter == null) return;
		LeoDataSummaryLog.d(TAG,"updateAppInfoView getCount():"+myappAdapter.getCount()+" OldAppAdapterCount:"+OldAppAdapterCount);		
		if(OldAppAdapterCount != myappAdapter.getCount()) {
			myappAdapter.notifyDataSetChanged();			
			listView.setAdapter(myappAdapter);
		}
		OldAppAdapterCount = myappAdapter.getCount();
		handleAppInfoView = 0;
		LeoDataSummaryLog.d(TAG,"updateAppInfoView exit !");

	}
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            return true;			
			
        }
        return super.onOptionsItemSelected(item);
    }	
	public DetailDAO detailDAO;
	private String mAppFlagKey;
	public static final String APP_FLAG_KEY = "app_flag_key";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.app_main);
		MSimTelephonyManager msim_instance = (MSimTelephonyManager)getApplicationContext().getSystemService(Context.MSIM_TELEPHONY_SERVICE);
		int sub =getIntent().getIntExtra(SUBSCRIPTION_KEY,0);//get sub
		mAppFlagKey =  getIntent().getStringExtra(APP_FLAG_KEY);
		int sim_state = msim_instance.getSimState(sub);
                dataList = new ArrayList<AppInfo>(); 
		LeoDataSummaryLog.d(TAG,"onCreate sub["+sub+"] Simstate = "+sim_state);
		if(sim_state == TelephonyManager.SIM_STATE_ABSENT) {
			setContentView(R.layout.app_empty);
			return;
		}
		if(mAppFlagKey == null) {
			//do nothing
		}else if(mAppFlagKey.equalsIgnoreCase("LenovoApnSingleCard")) {
			this.setTitle(getResources().getString(R.string.data_summary));
			this.getActionBar().setDisplayHomeAsUpEnabled(true);
		}

		
		detailDAO=new DetailDAO(getApplicationContext());
		//DetailInfo detailInfo=new DetailInfo(3, 10017, 15.5, 101.6,
		//"20140410");
		//detailDAO.add(detailInfo);

		new Thread(new Runnable() {
		
			public void run(){
				init();
				save(myFile.filename, myFile.s);
			}
		}).start();

		handler = new Handler() {

			@Override
			public void handleMessage(Message msg) {
				// TODO Auto-generated method stub
				super.handleMessage(msg);
				if (msg.what == 0x111) {
					//Toast.makeText(MainActivity.this, "install", 500).show();
					init();
				} else if (msg.what == 0x110) {
					//Toast.makeText(MainActivity.this, "uninstall", 500).show();
					init();
				} else if (msg.what == 0x101) {
					String packagename = msg.getData().getString("packagename");
					int position = msg.getData().getInt("position");
					int uid = msg.getData().getInt("uid");
					LeoDataSummaryLog.d(TAG,"position"+"   "+packagename + String.valueOf(position));
					// Toast.makeText(MainActivity.this,
					// packagename+String.valueOf(position)+String.valueOf(uid),
					// 200).show();
					reinit(packagename, position, uid);
				}else if(msg.what == EVENT_BOOT_COMPLETED) {
					//get current table, if current != 0, do {current + total} and clear the current.
					//LeoDataSummaryLog.d(TAG,"yql EVENT_BOOT_COMPLETED");
					//MoveDataFromCurrentTable();
				}else if(msg.what == EVENT_UPDATE_VIEW) {
						updateView();
				}else if(msg.what == EVENT_UPDATE_XYDATA_VIEW) {
						updateXYDataView();
				}else if(msg.what == EVENT_UPDATE_APPINFO_VIEW) {
						updateAppInfoView();
				}
			}

		};

		mAppBroadcastReceiver = new AppBroadcastReceiver();
		IntentFilter intentFilter = new IntentFilter(
				Intent.ACTION_PACKAGE_CHANGED);
		this.registerReceiver(mAppBroadcastReceiver, intentFilter);

		// TotalDAO totalDAO=new TotalDAO(getApplication());

		// TotalInfo totalInfo=new TotalInfo(6, "4600", 1, 100.1, 200.2,
		// 20140406L);
		// totalDAO.add(totalInfo);
		// ArrayList<TotalInfo>
		// tInfos=totalDAO.findbetweendate("20140406","20140408");
		// Toast.makeText(MainActivity.this, tInfos.toString(), 300).show();

	}
	/*

	public void MoveDataFromCurrentTable() {
		
		TotalDAO totalDAO=new TotalDAO(getApplication());
		XYData xyData = new XYData(getApplication());
		ArrayList<TotalInfo> Info_current=totalDAO.findbetweendateCurrentTable(xyData.dds_imsi,mySubscription,xyData.getminday(),xyData.getmaxday());
		ArrayList<TotalInfo> Info_total=null;

		boolean is_current_table_empty = true;
		long cur_date = 0;
		long date = 0;
		String cur_imsi = " ";
		String imsi = " ";
		int cur_sub = 0;
		int sub = 0;
		double cur_rx = 0;
		double rx = 0;
		double cur_tx = 0;
		double tx = 0;
		
		for (int i = 0; i < Info_current.size(); i++) {
			if((Info_current.get(i).getRx()+Info_current.get(i).getTx()) > 0) {
				cur_date = Info_current.get(i).getDate();
				cur_imsi = Info_current.get(i).getImsi();
				cur_sub = Info_current.get(i).getSub();
				cur_rx = Info_current.get(i).getRx();
				cur_tx = Info_current.get(i).getTx();
				is_current_table_empty = false;
				LeoDataSummaryLog.d(TAG,"MoveDataFromCurrentTable cur_date:"+cur_date+" cur_imsi:"+cur_imsi+
					" cur_sub:"+cur_sub+" cur_rx:"+cur_rx+" cur_tx:"+cur_tx);
				//clear current table
				totalDAO.deleteCurrentTable(cur_date);
				break;
			}
		}
		Info_total=totalDAO.findbetweendate(xyData.dds_imsi,mySubscription,Long.toString(cur_date),Long.toString(cur_date));
		if(Info_total.size() > 0) {
			for (int i = 0; i < Info_total.size(); i++) {
					date = Info_total.get(i).getDate();
					imsi = Info_total.get(i).getImsi();
					sub = Info_total.get(i).getSub();
					rx = Info_total.get(i).getRx();
					tx = Info_total.get(i).getTx(); 
					LeoDataSummaryLog.d(TAG,"MoveDataFromCurrentTable date:"+date+" imsi:"+imsi+
					" sub:"+sub+" rx:"+rx+" tx:"+tx);					
				if((! is_current_table_empty) && (imsi.equals(cur_imsi)) && (sub == cur_sub)){
					totalDAO.update(date,rx+cur_rx,tx+cur_tx);
					break;
				}			
			}

		}else {
			totalDAO.add(new TotalInfo(0, cur_imsi, cur_sub, cur_rx, cur_tx, cur_date));
		}
	 
	 
	}	
	*/

	public class AppBroadcastReceiver extends BroadcastReceiver {
		private final String ADD_APP = "android.intent.action.PACKAGE_ADDED";
		private final String REMOVE_APP = "android.intent.action.PACKAGE_REMOVED";

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String action = intent.getAction();

			/*if (ADD_APP.equals(action)) {

				String packageName = intent.getDataString();

				Toast.makeText(context, packageName, 500).show();

			}

			if (REMOVE_APP.equals(action)) {

				String packageName = intent.getDataString();
				Toast.makeText(context, packageName, 500).show();

			}*/

		}

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		//getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}


	protected XYMultipleSeriesRenderer buildBarRenderer(int[] colors) {
		XYMultipleSeriesRenderer renderer = new XYMultipleSeriesRenderer();
		// renderer.setAxisTitleTextSize(16);
		// renderer.setChartTitleTextSize(20);
		renderer.setLabelsTextSize(15);
		renderer.setLegendTextSize(15);
		int length = colors.length;
		for (int i = 0; i < length; i++) {
			SimpleSeriesRenderer r = new SimpleSeriesRenderer();
			r.setColor(colors[i]);
			renderer.addSeriesRenderer(r);
		}
		return renderer;
	}

	protected void setChartSettings(XYMultipleSeriesRenderer renderer,
			String title, String xTitle, String yTitle, double xMin,
			double xMax, double yMin, double yMax, int axesColor,
			int labelsColor) {
		renderer.setChartTitle(title);
		renderer.setXTitle(xTitle);
		renderer.setYTitle(yTitle);
		renderer.setXAxisMin(xMin);
		renderer.setXAxisMax(xMax);
		renderer.setYAxisMin(yMin);
		renderer.setYAxisMax(yMax);
		renderer.setAxesColor(axesColor);
		renderer.setLabelsColor(labelsColor);

	}

}
