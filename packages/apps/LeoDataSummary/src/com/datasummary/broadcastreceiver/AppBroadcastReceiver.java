package com.datasummary.broadcastreceiver;

import com.datasummary.LeoDataSummary.MainActivity;
import com.datasummary.unit.LeoDataSummaryLog;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Message;
import android.widget.Toast;

public class AppBroadcastReceiver extends BroadcastReceiver {
	private final String ADD_APP = "android.intent.action.PACKAGE_ADDED";
	private final String REMOVE_APP = "android.intent.action.PACKAGE_REMOVED";
	private final String BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";
	private final int EVENT_BOOT_COMPLETED = 0x112;
	public static String TAG="AppBroadcastReceiver";

	@Override
	public void onReceive(Context context, Intent intent) {
		// TODO Auto-generated method stub
		String action = intent.getAction();
		Message message=new Message();
		LeoDataSummaryLog.d(TAG,"AppBroadcastReceiver action = "+action);
		if(BOOT_COMPLETED.equals(action)) {
			message.what=EVENT_BOOT_COMPLETED;
			MainActivity.handler.sendMessage(message);
		}
		
		if (ADD_APP.equals(action)) {

			String packageName = intent.getDataString();
			//Toast.makeText(context, "install"+packageName, 500).show();
			
			message.what=0x111;
			if(MainActivity.handler != null) {
				MainActivity.handler.sendMessage(message);
			}else {
				LeoDataSummaryLog.d(TAG,"AppBroadcastReceiver ADD_APP MainActivity.handler = null");
			}
			
		}

		if (REMOVE_APP.equals(action)) {

			String packageName = intent.getDataString();
			//Toast.makeText(context, "uninstall"+packageName, 500).show();
			message.what=0x110;
			if(MainActivity.handler != null) {
				MainActivity.handler.sendMessage(message);
			}else {
				LeoDataSummaryLog.d(TAG,"AppBroadcastReceiver REMOVE_APP MainActivity.handler = null");
			}


		}

	}

}
