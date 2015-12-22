package com.datasummary.unit;

import android.util.Log;



public class LeoDataSummaryLog {
	public static final String TOTALTAG = "LeoDataSummary";

	public static void d(String TAG, String msg) {
		Log.d(TOTALTAG, TAG+":"+msg);
	}

	
}
