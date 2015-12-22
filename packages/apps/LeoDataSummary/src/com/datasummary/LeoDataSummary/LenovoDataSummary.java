//package com.lenovo.apntab;
package com.datasummary.LeoDataSummary;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.TabActivity;
import android.content.DialogInterface;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.widget.TabHost;
import android.widget.Toast;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TabHost.TabSpec;;
import android.content.Intent;
import com.datasummary.LeoDataSummary.R;
import android.view.Menu;
import android.view.MenuItem;
import android.util.Log;
import android.telephony.MSimTelephonyManager;
import android.telephony.TelephonyManager;
import android.content.Context;
import android.provider.Settings;
import android.os.SystemProperties;
import com.android.internal.telephony.TelephonyProperties;



public class LenovoDataSummary extends TabActivity {
  	private static boolean mIsCTA = SystemProperties.getBoolean("ro.lenovo.cta",false);	
	private TabHost mTabHost;
	public static final String SUBSCRIPTION_KEY = "subscription";
	private static String PACKAGE_NAME = "com.datasummary.LeoDataSummary";
       private static String CLASS_NAME = "com.datasummary.LeoDataSummary.MainActivity";
       
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		checkIsDualCard();
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_test_android_tab);
		setContentView(R.layout.tab_widget);
		setTitle(getResources().getString(R.string.data_summary));
		mTabHost=getTabHost();

		Intent intent_sub1 = new Intent().setClassName(PACKAGE_NAME, CLASS_NAME);
		Intent intent_sub2 = new Intent().setClassName(PACKAGE_NAME, CLASS_NAME);
		//Intent intent_sub2 = new Intent().setClassName("com.android.settings", "com.android.settings.Settings$DataUsageSummaryActivity");
		intent_sub1.putExtra(SUBSCRIPTION_KEY, 0);
		intent_sub2.putExtra(SUBSCRIPTION_KEY, 1);
		
		mTabHost.addTab(mTabHost.newTabSpec("tab_test1").setIndicator(getResources().getString(R.string.data_usage_tab_slot1)).setContent(intent_sub1/*R.id.textview1*/));
		mTabHost.addTab(mTabHost.newTabSpec("tab_test2").setIndicator(getResources().getString(R.string.data_usage_tab_slot2)).setContent(intent_sub2));
		//mTabHost.addTab(mTabHost.newTabSpec("tab_test3").setIndicator("TAB3",getResources().getDrawable(R.drawable.img3)).setContent(R.id.textview3));
		

	    //mTabHost.setBackgroundColor(Color.argb(150, 22, 70, 150));
	    

	    mTabHost.setCurrentTab(0);
        this.getActionBar().setDisplayHomeAsUpEnabled(true);		
	    

	    mTabHost.setOnTabChangedListener(new OnTabChangeListener() {
			@Override
			public void onTabChanged(String tabId) {

			}
		});
	}

    public boolean isAirplaneModeOn(Context context) {
    	boolean airplane = Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    	Log.d("LenovoDataSummary", "isAirplaneModeOn, airplane->"+airplane);
        return airplane;
    }

  private void enterAPPWithSub(String packName, String className,int sub){
      Log.d("LenovoDataSummary", "packName="+packName+",className="+className+" sub= "+sub);
      Intent intent = new Intent();
      intent.setClassName(packName,className);
	  intent.putExtra(SUBSCRIPTION_KEY, sub);
	  intent.putExtra(APP_FLAG_KEY, "LenovoApnSingleCard"); 
      startActivity(intent);
  }

  private String getSimVariants() {
	  String SimConfig =SystemProperties.get(TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG,"tsts");
	  return SimConfig;

  }
    public static String getTelephonyProperty(String property, int index, String defaultVal) {
        String propVal = null;
        String prop = SystemProperties.get(property);

        if ((prop != null) && (prop.length() > 0)) {
            String values[] = prop.split(",");
            if ((index >= 0) && (index < values.length) && (values[index] != null)) {
                propVal = values[index];
            }
        }
        return propVal == null ? defaultVal : propVal;
    }  
  public int getSimState(int slotId) {
	  String prop =
		  getTelephonyProperty(TelephonyProperties.PROPERTY_SIM_STATE, slotId, "");
	  if ("ABSENT".equals(prop)) {
		  return TelephonyManager.SIM_STATE_ABSENT;
	  }
	  else if ("PIN_REQUIRED".equals(prop)) {
		  return TelephonyManager.SIM_STATE_PIN_REQUIRED;
	  }
	  else if ("PUK_REQUIRED".equals(prop)) {
		  return TelephonyManager.SIM_STATE_PUK_REQUIRED;
	  }
	  else if ("NETWORK_LOCKED".equals(prop)) {
		  return TelephonyManager.SIM_STATE_NETWORK_LOCKED;
	  }
	  else if ("READY".equals(prop)) {
		  return TelephonyManager.SIM_STATE_READY;
	  }
	  else {
		  return TelephonyManager.SIM_STATE_UNKNOWN;
	  }
  }

   private static final String DATA_USAGE_SETTINGS_CLASS_SINGLE_CARD = mIsCTA ? "com.android.settings.Settings$DataUsageSummaryActivity" : "com.datasummary.LeoDataSummary.MainActivity";
   private static final String DATA_USAGE_SETTINGS_PACKAGE = mIsCTA ? "com.android.settings" : "com.datasummary.LeoDataSummary";
   public static final String APP_FLAG_KEY = "app_flag_key";

	private void checkIsDualCard() {
		int SimState0 = -1;
		int SimState1 = -1;
		if(("dsds".equals(getSimVariants())) || ("dsda".equals(getSimVariants()))) {
			SimState0 = getSimState(0);
			SimState1 = getSimState(1);
			Log.d("LenovoDataSummary", "SimState(0) = "+SimState0+" SimState(1) = "+SimState1);
		  }
		if(((SimState0 == TelephonyManager.SIM_STATE_READY) && (SimState1 == TelephonyManager.SIM_STATE_READY)) && (! isAirplaneModeOn(getApplicationContext()))) {
			return;
		}else if(((SimState0 == TelephonyManager.SIM_STATE_READY) && (SimState1 != TelephonyManager.SIM_STATE_READY)) && (! isAirplaneModeOn(getApplicationContext()))){
			enterAPPWithSub(DATA_USAGE_SETTINGS_PACKAGE,DATA_USAGE_SETTINGS_CLASS_SINGLE_CARD,0);
			finish();

		}else if(((SimState0 != TelephonyManager.SIM_STATE_READY) && (SimState1 == TelephonyManager.SIM_STATE_READY)) && (! isAirplaneModeOn(getApplicationContext()))){
		    enterAPPWithSub(DATA_USAGE_SETTINGS_PACKAGE,DATA_USAGE_SETTINGS_CLASS_SINGLE_CARD,1);
			finish();
		}

	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int itemID = item.getItemId();
		if (itemID == android.R.id.home) {
			finish();
			return true;
		} else {
			return false;
		}
	}		

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		//getMenuInflater().inflate(R.menu.test_android_tab, menu);
		return true;
	}

}
